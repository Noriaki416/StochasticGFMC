#include<iostream>
#include<cmath>
#include<fstream>
#include<random>
#include<cstdlib>
#include<vector>
#include<list>
#include<string>
using namespace std;

inline double sqr(double x) {
  return x*x;
}

struct parameters { 
  int nsite; // # of sites
  int np; // # of particles (Bosons)
  int nthermal; // # of MCS for thermalization
  int nbin; // # of bin 
  int nmeasure; // # of measurement (# of mcs = nbin*nmeasure);
  int nseed; // seed for random generator
  double beta;
  double v; // nearest neighbor coulomb interaction
  double t;
};

struct model_struct {
  int nhopping, nint;
  vector<vector<int> > hopping;
  // hopping[i][0] ... annniholation site
  // hopping[i][1] ... creation site 
  vector<vector<int> > interaction;
  vector<double> v;
  model_struct(int nhopping_in,int nint_in) :
    nhopping(nhopping_in), nint(nint_in),
    hopping(nhopping_in, vector<int> (2,0)),
    interaction(nint_in, vector<int> (2,0)),
    v(nint_in)
  {} 
};

struct vertex_element {
public:
  int vertex_type; // vertex_type = 0 only in the hardcore boson model
  int site_annihilation; // the site position of the vertex; annihilation site
  int site_creation; // the site position of the vertex; creation site
  double tau;
  vertex_element(int vertex_type_in, int site_annihilation_in,
		 int site_creation_in, double tau_in) :
    vertex_type(vertex_type_in), site_annihilation(site_annihilation_in),
    site_creation(site_creation_in), tau(tau_in) {}
};

struct site_operator { // used for only Green element
  bool is_creation; // true ... creation, false ... annihilation
  int site; // the site position 
  int spin; // index of spin (for spinless syste, no use)
  // for spinless case, no use,
  // for spinful case 0 ... up spin, 1 ... down spin
  site_operator(bool is_creation_in, int site_in, int spin_in) :
    is_creation(is_creation_in), site(site_in), spin(spin_in) {}  
};

struct green_operator {
public:
  list<site_operator> op_list;
  int p,q;
  double tau; //tau of green_oprator at present position.
  green_operator() {
    p = 0;
    q = 0;
    tau = 0.;
  }
  // for test (for specified configuration of psiR,L)
  green_operator(vector<int> psiR,vector<int> psiL) {
    p = 0;
    q = 0;
    tau = 0.0;
    
    /*
    cout << "  i       ";
    for(int i=0; i<psiR.size(); i++){
      cout << " " << i ;
    }
    cout << endl;
    
    cout << "|psiR> = |";
    for(int i=0; i<psiR.size(); i++){
      cout << " " << psiR[i] ;
    }
    cout << " >" <<endl;

    
    cout << "<psiL| = <";
    for(int i=0; i<psiR.size(); i++){
      cout << " " << psiL[i] ;
    }
    cout << " |" <<endl;
    */

    //active term of g
    for (int i=0; i<psiR.size(); i++) {
      if ((psiR[i] == 0) && (psiL[i] == 1)) {
	op_list.push_back(site_operator(true,i,0));
	p++;
      } else if ((psiR[i] == 1) && (psiL[i] == 0)) {
	op_list.push_back(site_operator(false,i,0));
	q++;
      }
    }
    /*
    cout << "g_pq:" << p << " " << q << endl;
    cout << "weight:" << weight() << endl;
    cout << "g =" ;
    for(list<site_operator>::iterator itr = op_list.begin(); itr != op_list.end();itr++){
      if((*itr).is_creation){
      cout << " a*" <<  (*itr).site ;
      }
    }
    for(list<site_operator>::iterator itr = op_list.begin(); itr != op_list.end();itr++){
      if(!(*itr).is_creation){
      cout << " a" <<  (*itr).site ;
      }
    }    
    cout << endl;
    cout << endl;
    */
    //show_g();
  }

  void add(int i, int j, int spin) {
    // i ... site of added creation operator
    // j ... site fo added aannihilation operator
    // spin ... spin index
    // do simplification also.
   
    // creation check
    bool found = false;
    for (list<site_operator>::iterator it = op_list.begin();
	 it!=op_list.end(); ++it) {
	if ((it->site == i) && (!it->is_creation)) {
	   //it's site is i and annihilation > simplification
	  op_list.erase(it);
	found = true;
	q--;
	break;
      }
    }
    if (!found) {
      op_list.push_back(site_operator(true,i,spin)); 
      p++; // just add without simplification
    }

    // check annihilation     
    found = false;
    for (list<site_operator>::iterator it=op_list.begin();
	 it!=op_list.end(); ++it) {
      if ((it->site == j) && (it->is_creation)) {
	op_list.erase(it);
	p--;
	found = true;
	break;
      }
    }
    if (!found) {
      op_list.push_back(site_operator(false,j,spin)); 
      q++;
    }
    //cout << "p q: " << p << ' ' << q << " ( "  << "a*" << j << " a" << i << " ) " <<  endl;
	//show_g();
  }
  double weight() {
    int r = p + q;
    double result;
    if (r > 2) {
      result = exp(-4.*(double)((2-r)*(2-r)));
    } else {
      result = 1.;
    }
    return result;    
  }
  void show_g(){
    //cout << "g_pq:" << p << " " << q << endl;
    //cout << "weight:" << weight() << endl;
    /*
    cout << "g =" ;
    for(list<site_operator>::iterator itr = op_list.begin(); itr != op_list.end();itr++){
      if((*itr).is_creation){
      cout << " a*" <<  (*itr).site ;
      }
    }
    for(list<site_operator>::iterator itr = op_list.begin(); itr != op_list.end();itr++){
      if(!(*itr).is_creation){
      cout << " a" <<  (*itr).site ;
      }
    }
    if(op_list.size() == 0)  cout << " E";
    cout << endl;
    */
    
    
  }
};

class stochastic_green_MC_method {  
private:
  parameters p;
  model_struct model;
  green_operator green_op;
  list<vertex_element> vertex_list;
  vector<int> psiL,psiR;
  mt19937 mt;
  uniform_real_distribution<double> rnd;
  double R_rate,V_expec;
public:
  stochastic_green_MC_method(const parameters p_in,
			     const model_struct model_in,
			     const vector<int> psi_init,
                             const list<vertex_element> T_in):
    p(p_in), model(model_in), psiL(p_in.nsite), psiR(p_in.nsite),
    mt(p_in.nseed), rnd(0.0,1.0), vertex_list(T_in) {
    for (int i=0; i<p.nsite; i++) {
      psiL[i] = psi_init[i];
      psiR[i] = psi_init[i];
    }
    green_operator green_op(psiR,psiL);
  }
  stochastic_green_MC_method(const parameters p_in,
			     const model_struct model_in,
			     const vector<int> psi_init):
    p(p_in), model(model_in), psiL(p_in.nsite), psiR(p_in.nsite),
    mt(p_in.nseed), rnd(0.0,1.0) {
    for (int i=0; i<p.nsite; i++) {
      psiL[i] = psi_init[i];
      psiR[i] = psi_init[i];
    }
    green_operator green_op(psiR,psiL);
    //cout << "vertex_list.size : " << vertex_list.size() << endl;
    //cout << "p.beta : " << p.beta <<endl;
  }
  stochastic_green_MC_method(const parameters p_in,
			     const model_struct model_in,
			     const vector<int> psiR_init,
			     const vector<int> psiL_init) :
    p(p_in), model(model_in), psiL(p_in.nsite), psiR(p_in.nsite),
    mt(p_in.nseed),rnd(0.0,1.0) {
    for (int i=0; i<p.nsite; i++) {
      psiL[i] = psiL_init[i];
      psiR[i] = psiR_init[i];
    }
    green_operator green_op(psiR,psiL);
  }
  void change_parameter(parameters p_in) {
    p = p_in;
  }
  void update() {
    vector<vector<int> > active_list_R; //for ket|psiR> [0]:a [1]:a*
    vector<int> tmp(2);
    //cout << "active term of |psiR>" << endl;
    for (int i=0; i<model.hopping.size(); i++) {
      if ((psiR[model.hopping[i][0]] == 1) &&
	  (psiR[model.hopping[i][1]] == 0)) {
	tmp[0] = model.hopping[i][0]; //term able to annihilate (a)
	tmp[1] = model.hopping[i][1]; //creation (a*)
	active_list_R.push_back(tmp);
	//cout << "a*" << tmp[1] << " a" << tmp[0] <<endl;
      }
    }
    vector<vector<int> > active_list_L;//for bra <psiL|
    //cout << "active term of <psiL|" << endl;    
    for (int i=0; i<model.hopping.size(); i++) {
      if ((psiL[model.hopping[i][0]] == 1) &&
	  (psiL[model.hopping[i][1]] == 0)) {
	tmp[0] = model.hopping[i][1]; //be careful of conjudgate 
	tmp[1] = model.hopping[i][0]; 
	active_list_L.push_back(tmp);
	//cout << "a*" << tmp[1] << " a" << tmp[0] <<endl;
      }
    }
    
    double n_g = green_op.weight();
    //cout << "g_pq: " << green_op.p << ' ' << green_op.q<< endl;
    
    //cout << "n_gt" << endl;
    vector<double> n_gt(active_list_R.size());
    double n_gt_sum = 0.;
    vector<green_operator> green_op_R_new(n_gt.size(),green_op);
    for (int i=0; i<n_gt.size(); i++) {
      green_op_R_new[i].add(active_list_R[i][0],active_list_R[i][1],0);
      n_gt[i] = green_op_R_new[i].weight()*p.t;
      n_gt_sum += n_gt[i];
    }

    //cout << "n_tg" << endl;
    vector<double> n_tg(active_list_L.size());
    double n_tg_sum = 0.;
    vector<green_operator> green_op_L_new(n_tg.size(),green_op);
    for (int i=0; i<n_tg.size(); i++) {
      green_op_L_new[i].add(active_list_L[i][0],active_list_L[i][1],0);
      n_tg[i] = green_op_L_new[i].weight()*p.t;
      n_tg_sum += n_tg[i];   
    }
    //cout << endl;
    
    double V_L = 0., V_R = 0.;
    for (int i=0; i<model.interaction.size(); i++){
      if((psiR[model.interaction[i][0]] == 1)
	 && (psiR[model.interaction[i][1]] == 1)){
	V_R += p.v;
      }
      if((psiL[model.interaction[i][0]] == 1)
	 && (psiL[model.interaction[i][1]] == 1)){
	V_L += p.v;
      }
    }    
    //cout << "VL VR:" << V_L << "  " <<  V_R << endl;
    //cout <<"n_g:" << n_g << endl; 
    //cout << "n_gt n_tg:" <<  n_gt_sum << "  "  << n_tg_sum << endl ;
    
    double r_L = V_L + n_gt_sum/n_g;
    double r_R = V_R + n_tg_sum/n_g;
    double R_i = r_L + r_R;
    double P_L = r_L/R_i;
    double P_R = r_R/R_i;
    // cout << "r_L r_R:" <<  r_L << "   "  << r_R << endl ;
    // cout << "R_i:" << R_i << endl;
    //cout << "P_L:" <<  P_L << endl;

    //start to move
    /*
    bool move_left = false;
    double delta = 0.0001;
    if(P_R < P_L) move_left = true;
    else if( ( fabs(P_L - P_R) <= delta ) && (rnd(mt) < P_L) ) move_left = true;
    */
    if(rnd(mt) < P_L){
      // move left
      //cout << "move left" << endl;
      // insert vertex at the present time ??
      double total_weight = n_gt_sum + V_L*n_g;
      double r = rnd(mt)*total_weight;
      double sum = 0.;
      for (int i=0; i<active_list_R.size(); i++) { // to be improved? binary search?
	sum += n_gt[i];
	if (r < sum) {
	  // insert (causion: vertex_list[i] ... ===> tau_i > tau_{i+1}, ordering is opposite)
	  //cout << "insert" << endl;
	  vertex_list.push_front(vertex_element
				     (0,active_list_R[i][0],
				      active_list_R[i][1],green_op.tau));	  
	  green_op = green_op_R_new[i];
	  psiR[active_list_R[i][0]] = 0;
	  psiR[active_list_R[i][1]] = 1;
	  //cout << "Insert vertex : a*" << active_list_R[i][1]
	  //<< " " << "a" << active_list_R[i][0]  << endl;
	  green_op.show_g();
	  break;
	}
        else if (i == active_list_R.size()-1) {
	  //cout << "Not Insert vertex." << endl;
	  green_op.show_g();
	}
      }
      // calculation of interaction
      // to be improved: interaction ene. is changed only at the changed vertex site
      double V_R_2 = 0.;
      for (int i=0; i<model.interaction.size(); i++){
	if ((psiR[model.interaction[i][0]] == 1)
	    && (psiR[model.interaction[i][1]] == 1)){
	  V_R_2 += p.v;
	}
      }
      double delta_tau = fabs(log(1.0-rnd(mt))/V_R_2);
      //cout << "V_R_2 delta_tau:" << " " << V_R_2 << " " << delta_tau << endl;
      // vertex destroy???
      vertex_element nearest_vertex = vertex_list.back(); //left side fo g
      if(vertex_list.size() == 0) nearest_vertex.tau = green_op.tau;
      double tau_nearest = nearest_vertex.tau - green_op.tau;
      //cout << "nearest_vertex.tau green_op.tau delta_tau:  " << nearest_vertex.tau
      //<< " " << green_op.tau
      //<< " " << delta_tau << endl;
      
      if (tau_nearest < 0) {
	tau_nearest += p.beta;
      }      
      //cout << "tau_nearest:" << tau_nearest << endl;
      bool do_destroy = false;
      if( ( (nearest_vertex.tau != green_op.tau) || (delta_tau >= p.beta) ) && (vertex_list.size() != 0)){
	if (tau_nearest < delta_tau) {
	// destroy
	//cout << "destroy" << endl;
	psiL[nearest_vertex.site_annihilation] = 0; // * <psiL'| = ( nearest_vertex |psiL>)*
	psiL[nearest_vertex.site_creation] = 1;
	green_op.add(nearest_vertex.site_creation,
			 nearest_vertex.site_annihilation,0);
	green_op.tau = nearest_vertex.tau;
	vertex_list.pop_back();
	//if(vertex_list_new.size() == 0) vertex_list_new.front().tau = green_op_new.tau;
	//cout << "T is destroyed:" << " "
	//	     << "a*" << nearest_vertex.site_creation << " "
	//<< "a" << nearest_vertex.site_annihilation <<  endl;
	green_op.show_g();
	do_destroy = true;
      }
      }
      if(!do_destroy) {
	// no destroy
	green_op.tau += delta_tau;
	while (green_op.tau > p.beta) {
	  green_op.tau -= p.beta;
	}
	//cout << "Not destroy" << endl;
	green_op.show_g();
      }
    }
    else {
      // move right
      //cout << "move right" << endl;
      // insert vertex at the present time ??
      double total_weight = n_tg_sum + V_R*n_g;
      double r = rnd(mt)*total_weight;
      double sum = 0.;
      for (int i=0; i<active_list_L.size(); i++) { // to be improved
	sum += n_tg[i];
	if (r < sum) {
	  // insert
	  //cout << "insert" << endl;
	  vertex_list.push_back(vertex_element
				    (0,active_list_L[i][0],
				     active_list_L[i][1],green_op.tau));	  
	  green_op = green_op_L_new[i];
	  psiL[active_list_L[i][0]] = 1;//<psiL'| condjugate
	  psiL[active_list_L[i][1]] = 0;
	  //cout << "Insert vertex : a*" << active_list_L[i][1]
	  // << " " << "a" << active_list_L[i][0]  << endl;
	  green_op.show_g();
	  break;
	}
	else if(i == active_list_L.size() -1) {
	  //cout << "Not Insert vertex." << endl;
	  green_op.show_g();
	}
      }
      // to be improved
      double V_L_2 = 0.;
      for (int i=0; i<model.interaction.size(); i++){
	if((psiL[model.interaction[i][0]] == 1)
	   && (psiL[model.interaction[i][1]] == 1)){
	  V_L_2 += p.v;
	}
      }
      double delta_tau = fabs(log(1.0-rnd(mt))/V_L_2);
      //cout << "delta_tau:" << delta_tau << endl;
      vertex_element nearest_vertex = vertex_list.front();
      if(vertex_list.size() == 0) nearest_vertex.tau = green_op.tau;
      //cout << "nearest_vertex.tau green_op.tau delta_tau: " << nearest_vertex.tau
      // << " " << green_op.tau
      // << " " << delta_tau << endl;
      double tau_nearest =  green_op.tau - nearest_vertex.tau;
      if (tau_nearest < 0) {
	tau_nearest += p.beta;
      }
      //cout << "tau_nearest:" << " " << tau_nearest << endl;
      bool do_destroy = false;
      if(((nearest_vertex.tau != green_op.tau) || (delta_tau >= p.beta)) && (vertex_list.size() != 0)){
      if (tau_nearest < delta_tau) {
	// destroy
	//cout << "destroy" << endl;	
	psiR[nearest_vertex.site_annihilation] = 1;
	psiR[nearest_vertex.site_creation] = 0; // * |psiR'> = (<psiR| active_list )*
	green_op.add(nearest_vertex.site_creation,
			 nearest_vertex.site_annihilation,0);
	green_op.tau = nearest_vertex.tau;
	vertex_list.pop_front();
	//if(vertex_list_new.size() == 0) vertex_list_new.back().tau = green_op_new.tau;
	//	cout << "T is destroyed: a*" << nearest_vertex.site_creation
	// << " a" << nearest_vertex.site_annihilation << endl;
	green_op.show_g();	
	do_destroy = true;
      } 
      }
      if(!do_destroy) {
	// no destroy
	green_op.tau -= delta_tau;
	while (green_op.tau < 0) {
	  green_op.tau += p.beta;
	}	
	//cout << "Not destroy" << endl;
	green_op.show_g();
       }
    }

    //cout << endl;
    //show_psi_new(psiL_new,psiR_new);
    //cout << endl;
    
    //new configuration
    vector<vector<int> > active_list_R2; //for ket|psiR> [0]:a [1]:a*
    vector<int> tmp2(2);
    for (int i=0; i<model.hopping.size(); i++) {
      if ((psiR[model.hopping[i][0]] == 1) &&
	  (psiR[model.hopping[i][1]] == 0)) {
	tmp2[0] = model.hopping[i][0]; //term able to annihilate (a)
	tmp2[1] = model.hopping[i][1]; //creation (a*)
	active_list_R2.push_back(tmp2);
      }     
    }
    vector<vector<int> > active_list_L2;//for bra <psiL|
    for (int i=0; i<model.hopping.size(); i++) {
      if ((psiL[model.hopping[i][0]] == 1) &&
	  (psiL[model.hopping[i][1]] == 0)) {
	tmp2[0] = model.hopping[i][1]; //be careful of conjudgate 
	tmp2[1] = model.hopping[i][0]; 
	active_list_L2.push_back(tmp2);
	//cout << "a*" << tmp[1] << " a" << tmp[0] <<endl;
      }
    }
    
    double n_g2 = green_op.weight();    
    //cout << "n_gt2" << endl;
    vector<double> n_gt2(active_list_R2.size());
    double n_gt_sum2 = 0.;
    vector<green_operator> green_op_R2_new(n_gt2.size(),green_op);
    for (int i=0; i<n_gt2.size(); i++) {
      green_op_R2_new[i].add(active_list_R2[i][0],active_list_R2[i][1],0);
      n_gt2[i] = green_op_R2_new[i].weight()*p.t;
      n_gt_sum2 += n_gt2[i];
    }
    //cout << "n_tg2" << endl;
    vector<double> n_tg2(active_list_L2.size());
    double n_tg_sum2 = 0.;
    vector<green_operator> green_op_L2_new(n_tg2.size(),green_op);
    for (int i=0; i<n_tg2.size(); i++) {
      green_op_L2_new[i].add(active_list_L2[i][0],active_list_L2[i][1],0);
      n_tg2[i] = green_op_L2_new[i].weight()*p.t;
      n_tg_sum2 += n_tg2[i];   
    }
    // cout << endl;
    
    double V_L2 = 0., V_R2 = 0.;
    for (int i=0; i<model.interaction.size(); i++){
      if((psiR[model.interaction[i][0]] == 1)
	 && (psiR[model.interaction[i][1]] == 1)){
	V_R2 += p.v;
      }
      if((psiL[model.interaction[i][0]] == 1)
	 && (psiL[model.interaction[i][1]] == 1)){
	V_L2 += p.v;
      }
    }    
    //cout << "VL2 VR2:" << V_L2 << "  " <<  V_R2 << endl;
    //    cout << "n_g2:" << n_g2 << endl; 
    // cout << "n_g2t n_tg2:" <<  n_gt_sum2 << "  "  << n_tg_sum2 << endl ;
    
    double r_L2 = V_L2 + n_gt_sum2/n_g2;
    double r_R2 = V_R2 + n_tg_sum2/n_g2;
    double R_f = r_L2 + r_R2;
    //    double P_L2 = r_L2/R_f;
    // double P_R2 = r_R2/R_f;
    //cout << "r_L2 r_R2:" <<  r_L2 << "   "  << r_R2 << endl ;
    //cout << "R_i R_f:" << " " << R_i << " " << R_f << endl;
    //    cout << "rnd R_i/R_f" << " " << rnd(mt) << " " << R_i/R_f << endl;

    R_rate = R_f;
    V_expec = V_L2;
    //show_psi();
    //cout << "green_op.tau:" << " " << green_op.tau << endl;
  }
  double get_energy() {
    double e = 0;
    return (double) e;
  }
  double get_kin() {
    return  vertex_list.size()/p.beta;
  }
  double get_R(){
    return R_rate;
  }
  double get_V(){
    return V_expec;
  }
  double get_intV(){
    double V_L = 0.;
    for (int i=0; i<model.interaction.size(); i++){
      if((psiL[model.interaction[i][0]] == 1)
	 && (psiL[model.interaction[i][1]] == 1)){
	V_L += p.v;
      }
    }
    return  V_L;
      }
  void show_psi(){
    cout << "  i       ";
    for(int i=0; i<psiR.size(); i++){
      cout << " " << i ;
    }
    cout << endl;
    
    cout << "|psiR> = |";
    for(int i=0; i<psiR.size(); i++){
      cout << " " << psiR[i] ;
    }
    cout << " >" <<endl;
    
    cout << "<psiL| = <";
    for(int i=0; i<psiR.size(); i++){
      cout << " " << psiL[i] ;
    }
    cout << " |" <<endl;
  }
  void show_psi_new(vector<int>psiL_new_in,vector<int> psiR_new_in){
    vector<int> psiL_new(psiL_new_in),psiR_new(psiR_new_in);
    cout << "  i           ";
    for(int i=0; i<psiR_new.size(); i++){
      cout << " " << i ;
    }
    cout << endl;
    
    cout << "|psiR_new> = |";
    for(int i=0; i<psiR_new.size(); i++){
      cout << " " << psiR_new[i] ;
    }
    cout << " >" <<endl;
    
    cout << "<psiL_new| = <";
    for(int i=0; i<psiR_new.size(); i++){
      cout << " " << psiL_new[i] ;
    }
    cout << " |" <<endl;
  }
  bool is_diagonal() {
    bool flag = false;
    if ((green_op.p == 0) && (green_op.q == 0)) {
      flag = true;
    }
    return flag;
  }
  double get_gtau() {
    return green_op.tau;
  }
};

class observable {
  double sum,sum2,sum_R,sum_V;
  int n;
public:
  observable() {
    sum = 0.;
    sum2 = 0.;
    sum_R = 0.;
    sum_V = 0.;
    n = 0;
  }
  void add(double x) {
    sum += x;
    sum2 += x*x;
    n++;
  }
  void add(double x,double R) {
    sum += x/R;
    sum2 += x*x;
    sum_R += 1.0/R;
    n++;
  }
  void add(double x,double V,double R) {
    sum += x/R;
    sum2 += x*x;
    sum_V += V/R;
    sum_R += 1.0/R;
    n++;
  }
  void add2(double x,double y){
    sum += x;
    sum2 += y;
    n++;
  }
  
  double get_av(){
    return sum/sum_R;
  }
    double get_V_av(){
    return sum_V/sum_R;
  }
  /*
  double get_av() {
    return sum;//(double) n;
  }
  */
  double get_av_sq() {
    return sum2;//(double) n;
  }
  double get_err() {
    double av = get_av();
    double av2 = get_av_sq();
    return sqrt((av2 - av*av)/(double) n);
  }
  int get_n(){
    return n;
  }
};

int main() {
  parameters p;
  p.nsite = 6;
  p.np = 3;
  p.v = 1.0;
  p.t = 1.0;
  p.nseed = 1432;//1567
  p.beta = 1.0;
  
  uniform_int_distribution<> rand1(0, p.nsite-1);
  mt19937 mt(p.nseed);
  vector<int> psi_initial(p.nsite,0);
    for(;;){
      int sumP=0;
    for(int i=0; i<p.nsite; i++){
      sumP += psi_initial[i];
    }
    if(sumP != p.np) psi_initial[rand1(mt)] = 1;
    if(sumP == p.np) break;
  }
  /*
  psi_initial[0] = 1;
  psi_initial[1] = 1;
  psi_initial[2] = 1;
  psi_initial[3] = 1;
  psi_initial[4] = 0;
  psi_initial[5] = 1;
  */

  /* //for check
  mt19937 mt(p.nseed);
  uniform_real_distribution<double> rnd2(0.0,p.beta);
  vector<double> tau(4,0.0);
  for(int i=0;i<tau.size();i++){
    tau[i] = rnd2(mt);
    //cout << tau[i] << endl;
  }
  sort(tau.begin(),tau.end());
  for(int i=0;i<tau.size();i++){
    cout << "tau["<< i << "]:" << " " << tau[i] << endl;
  }
  list<vertex_element> T;
  T.push_back(vertex_element(0,4,3,tau[3]));
  T.push_back(vertex_element(0,1,0,tau[2]));
  T.push_back(vertex_element(0,3,4,tau[1]));
  T.push_back(vertex_element(0,0,1,tau[0]));
  */
  
  vector<int> psiR_initial(p.nsite);
  psiR_initial[0] = 1;
  psiR_initial[1] = 0;
  psiR_initial[2] = 1;
  psiR_initial[3] = 0;
  psiR_initial[4] = 1;
  psiR_initial[5] = 0;
  vector<int> psiL_initial(p.nsite);
  psiL_initial[0] = 1;
  psiL_initial[1] = 1;
  psiL_initial[2] = 0;
  psiL_initial[3] = 0;
  psiL_initial[4] = 1;
  psiL_initial[5] = 0;
  
  model_struct model(p.nsite*2,p.nsite); // hardcore boson model
  for (int i=0; i<p.nsite; i++) {
    model.hopping[i][0] = i;
    model.hopping[i][1] = (i + 1) % p.nsite; // periodic boundary
    model.hopping[i+p.nsite][0] = i % p.nsite;    
    model.hopping[i+p.nsite][1] = (i + p.nsite - 1)  % p.nsite;
    model.interaction[i][0] = i;
    model.interaction[i][1] = (i + 1) % p.nsite;
    model.v[i] = p.v;
  }
  /*
  for(int i=0;i<p.nsite;i++){
    cout << "hop[" << i << "][0] " << "hop[" << i << "][1] : " << model.hopping[i][0] << " " << model.hopping[i][1] << endl;
  }
  for(int i=0;i<p.nsite;i++){
    cout << "hop[" << i+p.nsite << "][0] " << "hop[" << i+p.nsite << "][1] : " << model.hopping[i+p.nsite][0] << " " << model.hopping[i+p.nsite][1] << endl;    
  }
  */
  //stochastic_green_MC_method mc(p,model,psiR_initial,psiL_initial);
  //  stochastic_green_MC_method mc(p,model,psi_initial,T);

  //stochastic_green_MC_method mc(p,model,psi_initial);
  //mc.update();
  p.nthermal = 10000;
  p.nbin = 10000;
  p.nmeasure = 10;
  // nmcs = nbin * nmeasure
  //stochastic_green_MC_method mc(p,model,psi_initial);
  for (double temp = 0.2; temp <= 11.0; temp += 0.5 ) {
    p.beta = 1.0/temp;
    stochastic_green_MC_method mc(p,model,psi_initial);
    //mc.change_parameter(p);
    for (int k=0; k<p.nthermal; k++) {
      mc.update();
    }
    observable kin;
    for (int j=0; j<p.nmeasure*p.nbin; j++) { // to be implemented about BIN sampling
      mc.update();
      if ( mc.is_diagonal() ) {
	kin.add(mc.get_kin(),mc.get_V(),mc.get_R());
	      }
    }
    cout << 1.0/p.beta  << ' ' << kin.get_av() << ' ' <<  kin.get_V_av() << endl;
    //cout << "kin.n : " << kin.get_n() << endl;
  }
  
  /*
  for (double temp = 0.001; temp <=1.5; temp += 0.1) {
    p.beta = 1./temp;
     mc.change_parameter(p);
    // thermalization
    for (int k=0; k<p.nthermal; k++) {
      mc.update();
    }
    observable ene2;
    observable zika2;
    observable hinetsu2;
    for (int k=0; k<p.nmeasure; k++) {
      observable zika;
      observable ene;
      for(int j=0; j<p.nbin; j++){
      mc.update();
      ene.add(mc.get_energy());
      zika.add(mc.get_zika());
      }
      ene2.add(ene.get_av());
      zika2.add(zika.get_av());
      hinetsu2.add2(ene.get_av(),ene.get_av_sq());
    }
    ofs1  << temp  << ' ' << ene2.get_av()  << ' ' <<ene2.get_err() << endl;
    ofs2  << temp  << ' ' << hinetsu2.get_hinetsu(temp)   << endl;
    ofs3  << temp  << ' ' << zika2.get_av()  << ' ' << zika2.get_err() << endl;
  }
  */
  // hinetsu N*(1/x)**2*(2/(exp(1/x)+exp(-1/x)))**2
}
