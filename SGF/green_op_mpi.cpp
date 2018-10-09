#include<iostream>
#include<cmath>
#include<fstream>
#include<iomanip>
#include<sstream>
#include<random>
#include<cstdlib>
#include<vector>
#include<list>
#include<string>
#include<complex>
#include<mpi.h>
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
  double mu; //chemical potential
  string header;
  int v_i; //initial site of V
  int v_f; //final site of V
  double pot;
  int pot_i;
  int pot_f;
  int omega_ncut;
};

class read_parameter_file {
private:
  vector<int*> int_pointers;
  vector<double*> double_pointers;
  vector<string*> string_pointers;
  vector<string> type;
  vector<int> number;
  vector<string> keys,comments;
  vector<string> command_line;
  vector<bool> parsed;
public:
  read_parameter_file() {}
  void add(const string key, const string comment, int &param) {
    keys.push_back(key);
    comments.push_back(comment);
    type.push_back("int");
    int_pointers.push_back(&param);
    number.push_back(int_pointers.size()-1);
    parsed.push_back(false);
  }
  void add(const string key, const string comment, double &param) {
    keys.push_back(key);
    comments.push_back(comment);
    type.push_back("double");
    double_pointers.push_back(&param);
    number.push_back(double_pointers.size()-1);
    parsed.push_back(false);
  }
  void add(const string key, const string comment, string &param) {
    keys.push_back(key);
    comments.push_back(comment);
    type.push_back("string");
    string_pointers.push_back(&param);
    number.push_back(string_pointers.size()-1);
    parsed.push_back(false);
  }
  void help() {
    cout << "# Usage: ./a.out input_file_name" << endl;
    cout << "# Format of input file" << endl;
    cout << "# Keyword Value" << endl;
    for (int i=0; i<keys.size(); i++) {
      cout << keys[i] << " # (" << type[i] <<"): " << comments[i] << endl;
    }
  }
  void parse(const int argc, char** argv) {
    if ((argc <= 1) || (argc >=3)) {
      cout << "# ERROR: input file name is not given." << endl;
      help();
      exit(1);
    }
    ifstream ifs;
    ifs.open(argv[1]);
    if (ifs.fail()) {
      cout << "# ERROR: no input file : " << argv[1] << endl;
      help();
      exit(1);
    }
    string str;
    while(getline(ifs,str)) {
      string str_keyword;
      istringstream str_stream(str);
      if (getline(str_stream,str_keyword,' ')) {
	if (str_keyword != "#") {
	  for (int i=0; i<keys.size(); i++) {
	    //	    cout << keys[i] << ' ' << str_keyword << endl;
	    if (keys[i] == str_keyword) {
	      if (type[i] == "int") {
		string str_val_int;
		if (!getline(str_stream,str_val_int,' ')) {
		  cout << "# ERROR: a integer value is not provided for " 
		       << keys[i] << endl;
		  help();
		  exit(1);
		}
		*int_pointers[number[i]] = atoi(str_val_int.c_str());
	      } else if (type[i] == "double") {
		string str_val_double;
		if (!getline(str_stream,str_val_double,' ')) {
		  cout << "# ERROR: a double-precision value is not provided for " 
		       << keys[i] << endl;
		  help();
		  exit(1);
		}
		*double_pointers[number[i]] = atof(str_val_double.c_str());
	      } else if (type[i] == "string") {
		string str_val_string;
		if (!getline(str_stream,str_val_string,' ')) {
		  cout << "# ERROR: a string value is not provided for " 
		       << keys[i] << endl;
		  help();
		  exit(1);
		}
		*string_pointers[number[i]] = str_val_string;
	      }
	      parsed[i] = true;
	      break;
	    }
	  }
	}
      }
    }
    for (int i=0; i<keys.size(); i++) {
      if (!parsed[i]) {
	cout << "# ERROR: keyword " << keys[i] << " is not given." << endl;
	help();
	exit(1);
      }
    }
  }
  /*
  void bcast() {
    // Broad cast int parameter
    int *data_int = new int [int_pointers.size()];
    if (id == 0) {
      for (int i=0; i<int_pointers.size(); i++) {
	data_int[i] = *int_pointers[i];
      }    
    }
    MPI_Bcast(&data_int, int_pointers.size(), MPI_INT, 0, MPI_COMM_WORLD);
    delete[] data_int;
    int *data_double = new int [double_pointers.size()];
    if (id == 0) {
      for (int i=0; i<double_pointers.size(); i++) {
	data_double[i] = *double_pointers[i];
      }    
    }
    MPI_Bcast(&data_double, int_pointers.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    delete[] data_double;
  }
  */
};

struct model_struct {
  int nhopping, nint;
  vector<vector<int> > hopping;
  // hopping[i][0] ... annniholation site
  // hopping[i][1] ... creation site 
  vector<vector<int> > interaction;
  vector<double> v;
  vector<double> mu;
  vector<double> pot;
  model_struct(int nhopping_in,int nint_in) :
    nhopping(nhopping_in), nint(nint_in),
    hopping(nhopping_in, vector<int> (2,0)),
    interaction(nint_in, vector<int> (2,0)),
    v(nint_in),
    mu(nint_in),pot(nint_in)
  {} 
};

struct vertex_element {
public:
  bool operator<(const vertex_element &rhs) const
    {
        return tau < rhs.tau;
    }
  int vertex_type; // vertex_type = 0 only in the hardcore boson model
  int site_annihilation; // the site position of the vertex; annihilation site
  int site_creation; // the site position of the vertex; creation site
  double tau;
  vector<int> psi;
  vertex_element(int vertex_type_in, int site_annihilation_in,
		 int site_creation_in, double tau_in,vector<int> psi_in) :
    vertex_type(vertex_type_in), site_annihilation(site_annihilation_in),
    site_creation(site_creation_in), tau(tau_in),psi(psi_in) {}
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
  complex<double> ci;
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
    ci = complex<double> (0.0,1.0);
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
    for (int i=0; i<model.hopping.size(); i++) {
      if ((psiR[model.hopping[i][0]] == 1) &&
	  (psiR[model.hopping[i][1]] == 0)) {
	tmp[0] = model.hopping[i][0]; //term able to annihilate (a)
	tmp[1] = model.hopping[i][1]; //creation (a*)
	active_list_R.push_back(tmp);
      }
    }
    vector<vector<int> > active_list_L;//for bra <psiL|
    for (int i=0; i<model.hopping.size(); i++) {
      if ((psiL[model.hopping[i][0]] == 1) &&
	  (psiL[model.hopping[i][1]] == 0)) {
	tmp[0] = model.hopping[i][1]; //be careful of conjudgate 
	tmp[1] = model.hopping[i][0]; 
	active_list_L.push_back(tmp);
      }
    }
    
    double n_g = green_op.weight();
    vector<double> n_gt(active_list_R.size());
    double n_gt_sum = 0.;
    vector<green_operator> green_op_R_new(n_gt.size(),green_op);
    for (int i=0; i<n_gt.size(); i++) {
      green_op_R_new[i].add(active_list_R[i][0],active_list_R[i][1],0);
      n_gt[i] = green_op_R_new[i].weight()*p.t;
      n_gt_sum += n_gt[i];
    }

    vector<double> n_tg(active_list_L.size());
    double n_tg_sum = 0.;
    vector<green_operator> green_op_L_new(n_tg.size(),green_op);
    for (int i=0; i<n_tg.size(); i++) {
      green_op_L_new[i].add(active_list_L[i][0],active_list_L[i][1],0);
      n_tg[i] = green_op_L_new[i].weight()*p.t;
      n_tg_sum += n_tg[i];   
    }
    
    double V_L = 0., V_R = 0.;
    for (int i=0; i<model.interaction.size(); i++){
      if((psiR[model.interaction[i][0]] == 1)
	 && (psiR[model.interaction[i][1]] == 1)){
	V_R += model.v[i];
      }
      if((psiL[model.interaction[i][0]] == 1)
	 && (psiL[model.interaction[i][1]] == 1)){
	V_L += model.v[i];
      }
      if(psiR[model.interaction[i][0]] == 1) V_R -= model.mu[i];
      if(psiL[model.interaction[i][0]] == 1) V_L -= model.mu[i];
    }
    double r_L = V_L + n_gt_sum/n_g;
    double r_R = V_R + n_tg_sum/n_g;
    double R_i = r_L + r_R;
    double P_L = r_L/R_i;
    double P_R = r_R/R_i;
    if (rnd(mt) < P_L){
      // move left
      // insert vertex at the present time ??
      double total_weight = n_gt_sum + V_L*n_g;
      double r = rnd(mt)*total_weight;
      double sum = 0.;
      for (int i=0; i<active_list_R.size(); i++) { // to be improved? binary search?
	sum += n_gt[i];
	if (r < sum) {
	  // insert (causion: vertex_list[i] ... ===> tau_i > tau_{i+1}, ordering is opposite)
	  psiR[active_list_R[i][0]] = 0;
	  psiR[active_list_R[i][1]] = 1;
	  vertex_list.push_front(vertex_element
				 (0,active_list_R[i][0],
				  active_list_R[i][1],green_op.tau,psiR));	  
	  green_op = green_op_R_new[i];
	  //psiR[active_list_R[i][0]] = 0;
	  //psiR[active_list_R[i][1]] = 1;
	  //	  green_op.show_g();
	  break;
	}
        else if (i == active_list_R.size()-1) {
	  //	  green_op.show_g();
	}
      }
      // calculation of interaction
      // to be improved: interaction ene. is changed only at the changed vertex site
      double V_R_2 = 0.;
      for (int i=0; i<model.interaction.size(); i++){
	if ((psiR[model.interaction[i][0]] == 1)
	    && (psiR[model.interaction[i][1]] == 1)){
	  V_R_2 += model.v[i];
	}
	if(psiR[model.interaction[i][0]] == 1) V_R_2 -= model.mu[i];
      }
      double delta_tau = fabs(log(1.0-rnd(mt))/V_R_2);
      // vertex destroy???
      vertex_element nearest_vertex = vertex_list.back(); //left side fo g
      if(vertex_list.size() == 0) nearest_vertex.tau = green_op.tau;
      double tau_nearest = nearest_vertex.tau - green_op.tau;
      
      if (tau_nearest < 0) {
	tau_nearest += p.beta;
      }      
      bool do_destroy = false;
      if( ( (nearest_vertex.tau != green_op.tau) || (delta_tau >= p.beta) )
	  && (vertex_list.size() != 0)) {
	if (tau_nearest < delta_tau) {
	  // destroy
	  psiL[nearest_vertex.site_annihilation] = 0;
	  // * <psiL'| = ( nearest_vertex |psiL>)*
	  psiL[nearest_vertex.site_creation] = 1;
	  green_op.add(nearest_vertex.site_creation,
		       nearest_vertex.site_annihilation,0);
	  green_op.tau = nearest_vertex.tau;
	  vertex_list.pop_back();
	  //	green_op.show_g();
	  do_destroy = true;
	}
      }
      if(!do_destroy) {
	// no destroy
	green_op.tau += delta_tau;
	while (green_op.tau > p.beta) {
	  green_op.tau -= p.beta;
	}
	//green_op.show_g();
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
	  psiL[active_list_L[i][0]] = 1;//<psiL'| condjugate
	  psiL[active_list_L[i][1]] = 0;
	  vertex_list.push_back(vertex_element
				    (0,active_list_L[i][0],
				     active_list_L[i][1],green_op.tau,psiL));	  
	  green_op = green_op_L_new[i];
	  //psiL[active_list_L[i][0]] = 1;//<psiL'| condjugate
	  //psiL[active_list_L[i][1]] = 0;
	  //	  green_op.show_g();
	  break;
	}
	else if(i == active_list_L.size() -1) {
	  //green_op.show_g();
	}
      }
      // to be improved
      double V_L_2 = 0.;
      for (int i=0; i<model.interaction.size(); i++){
	if((psiL[model.interaction[i][0]] == 1)
	   && (psiL[model.interaction[i][1]] == 1)){
	  V_L_2 += model.v[i];
	}
	if(psiL[model.interaction[i][0]] == 1) V_L_2 -= model.mu[i];
      }
      double delta_tau = fabs(log(1.0-rnd(mt))/V_L_2);
      vertex_element nearest_vertex = vertex_list.front();
      if(vertex_list.size() == 0) nearest_vertex.tau = green_op.tau;
      double tau_nearest =  green_op.tau - nearest_vertex.tau;
      if (tau_nearest < 0) {
	tau_nearest += p.beta;
      }
      bool do_destroy = false;
      if(((nearest_vertex.tau != green_op.tau) || (delta_tau >= p.beta)) && (vertex_list.size() != 0)){
	if (tau_nearest < delta_tau) {
	  // destroy
	  psiR[nearest_vertex.site_annihilation] = 1;
	  psiR[nearest_vertex.site_creation] = 0; // * |psiR'> = (<psiR| active_list )*
	  green_op.add(nearest_vertex.site_creation,
		       nearest_vertex.site_annihilation,0);
	  green_op.tau = nearest_vertex.tau;
	  vertex_list.pop_front();
	  //	  green_op.show_g();	
	  do_destroy = true;
	} 
      }
      if(!do_destroy) {
	// no destroy
	green_op.tau -= delta_tau;
	while (green_op.tau < 0) {
	  green_op.tau += p.beta;
	}	
	//	green_op.show_g();
      }
    }
    
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
      }
    }
    
    double n_g2 = green_op.weight();    
    vector<double> n_gt2(active_list_R2.size());
    double n_gt_sum2 = 0.;
    vector<green_operator> green_op_R2_new(n_gt2.size(),green_op);
    for (int i=0; i<n_gt2.size(); i++) {
      green_op_R2_new[i].add(active_list_R2[i][0],active_list_R2[i][1],0);
      n_gt2[i] = green_op_R2_new[i].weight()*p.t;
      n_gt_sum2 += n_gt2[i];
    }
    vector<double> n_tg2(active_list_L2.size());
    double n_tg_sum2 = 0.;
    vector<green_operator> green_op_L2_new(n_tg2.size(),green_op);
    for (int i=0; i<n_tg2.size(); i++) {
      green_op_L2_new[i].add(active_list_L2[i][0],active_list_L2[i][1],0);
      n_tg2[i] = green_op_L2_new[i].weight()*p.t;
      n_tg_sum2 += n_tg2[i];   
    }
    
    double V_L2 = 0., V_R2 = 0.;
    for (int i=0; i<model.interaction.size(); i++){
      if((psiR[model.interaction[i][0]] == 1)
	 && (psiR[model.interaction[i][1]] == 1)){
	V_R2 += model.v[i];
      }
      if((psiL[model.interaction[i][0]] == 1)
	 && (psiL[model.interaction[i][1]] == 1)){
	V_L2 += model.v[i];
      }
      if(psiR[model.interaction[i][0]] == 1) V_R2 -= model.mu[i];
      if(psiL[model.interaction[i][0]] == 1) V_L2 -= model.mu[i];
    }
    
    double r_L2 = V_L2 + n_gt_sum2/n_g2;
    double r_R2 = V_R2 + n_tg_sum2/n_g2;
    double R_f = r_L2 + r_R2;
    R_rate = R_f;
    V_expec = V_L2;
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
  void add_psi(int annihilation_num,int creation_num,vector<int> psi){
    if ((psi[annihilation_num]==1)&&(psi[creation_num]==0)){
      psi[annihilation_num] -= 1;
      psi[creation_num] += 1;
    } else {
      for(int i=0;i<p.nsite;i++){
	psi[i] =0;
      }
    }
  }
  double get_dd_correlation(int i,int j){
    double dd_correlation = 0.0;
    if( (psiR[i]==1) && (psiR[j]==1) ) dd_correlation = 1.0;
    return dd_correlation;
  }
  vector<double> get_ne() {
    vector<double> ne_result(p.nsite,0.0);
    double tau1 = green_op.tau;
    if (vertex_list.size() == 0) {
      for (int i=0; i<p.nsite; i++) {
	ne_result[i] = (double) psiR[i];
      }
    } else {
      vector<int> psi(psiR);
      double tau2;
      for (list<vertex_element>::reverse_iterator itr = vertex_list.rbegin();
	   itr != vertex_list.rend(); ++itr) {
	// CAUTION: tau is now increasing.
	tau2 = itr->tau;
	double delta_tau = tau2 - tau1;
	if (delta_tau < 0) delta_tau += p.beta;	
	for (int i=0; i<p.nsite; i++) {
	  ne_result[i] += delta_tau/p.beta*psi[i];
	}
	psi[itr->site_annihilation] = 0;
	psi[itr->site_creation] = 1;
	tau1 = tau2;
      }
      tau2 = green_op.tau;
      double delta_tau = tau2 - tau1;
      if (delta_tau < 0) delta_tau += p.beta;	
      for (int i=0; i<p.nsite; i++) {
	ne_result[i] += delta_tau/p.beta*psi[i];
      }
    }
    return ne_result;
  }
  vector<double> get_Cjj() {
    vector<double> Cjj_result(p.omega_ncut,0.0);
    for (int n=0; n<p.omega_ncut; n++) {
      double omega_n = 2.0*M_PI/p.beta*(double) n;
      double jr = 0.0;
      double ji = 0.0;    
      for (list<vertex_element>::reverse_iterator itr = vertex_list.rbegin();
	   itr != vertex_list.rend(); ++itr) {
	// CAUTION: tau is now increasing.
	double sign;
	if (((itr->site_creation - itr->site_annihilation + p.nsite) % p.nsite) == 1) {
	  sign = 1.0;
	} else {
	  sign = -1.0;
	}
	double tau = itr->tau;
	jr += sign*cos(omega_n*tau);
	ji += sign*sin(omega_n*tau);
	/*
	cout << "vertex : " << itr->site_creation << ' ' << itr->site_annihilation << ' ' << tau << ' '
	     << jr << ' ' << ji << endl;
	*/
      }
      //      cout << "$$$$$$" << endl;
      Cjj_result[n] = jr*jr + ji*ji;
      //cout << n << ' ' << Cjj_result[n] << ' ' << jr << ' ' << ji << endl;
    }
    //    exit(0);
    return Cjj_result;
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
  bool is_diagonal() {
    bool flag = false;
    if ((green_op.p == 0) && (green_op.q == 0)) {
      flag = true;
    }
    return flag;
  }
};

class observable {
public:
  double sum,sum2,sum_R;
  int n;
  observable() {
    n = 0;
    sum = 0.;
    sum2 = 0.;
    sum_R = 0.;
  }
  void add(double x) {
    sum += x;
    sum2 += x*x;
    n++;
  }
  void add(double x,double R) {
    sum += x/R;
    sum2 += sqr(x)/R;
    sum_R += 1./R;
    n++;
  }
  double get_av(){
    return sum/sum_R;
  }
  double get_av_sq() {
    return sum2/sum_R;
  }  
  double get_err() {
    double av = get_av();
    double av2 = get_av_sq();
    return sqrt((av2 - av*av)/(double) n);
  }
};

struct mpi1darray {
  int nsize,nproc,id;
  double *array;
  vector<vector<double> > data;
  mpi1darray(int np, int id_in, vector<double> d) {
    nproc = np;
    id = id_in;
    nsize = d.size();    
    data.resize(np);
    for (int k=0; k<np; k++) {
      data[k].resize(nsize);
    }
    array = new double[nsize];
    for (int j=0; j<nsize; j++) {
      array[j] = d[j];
    } 
  }
  ~mpi1darray() {
    delete[] array;
  }
  void send_receive() {
    if (id == 0) {
      for (int i=0; i<nsize; i++) {
	data[0][i] = array[i];
      }
#ifndef NOMPI
      for (int k=1; k<nproc; k++) {
	int tag = 1001;
	MPI_Status status;
	MPI_Recv(array,nsize,MPI_DOUBLE,k,tag,
		 MPI_COMM_WORLD,&status);
	for (int j=0; j<nsize; j++) {
	  data[k][j] = array[j];
	}
      }
#endif
    } else {
#ifndef NOMPI
      int tag = 1001;
      MPI_Send(array,nsize,MPI_DOUBLE,0,tag,MPI_COMM_WORLD);
#endif
    }
  }
  double get_data(int k, int j) {
    return data[k][j];
  }
};

int main(int argc, char **argv) {

  int id,nproc;
#ifndef NOMPI
  // for parallel computing
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);
#else
  nproc = 1;
  id = 0;
#endif

  parameters p;
  read_parameter_file param;
  param.add("nseed","seed for random number generator",p.nseed);
  param.add("nsite","size of a chain",p.nsite);
  param.add("np","number of hardcore bosons",p.np);
  //  param.add("v","nearest-neighbor interaction",p.v);
  //  param.add("v_i","initial site of v",p.v_i);
  //  param.add("v_f","final site of v",p.v_f);
  param.add("t","hopping",p.t);
  param.add("mu","chemical potential",p.mu);
  param.add("beta","inverse temperature",p.beta);  
  param.add("nthermal","number of mcs for thermalization",p.nthermal);  
  param.add("nmeasure","number of measurement",p.nmeasure);  
  param.add("nbin","number of bins",p.nbin);  
  param.add("omegan_max","Matsubara frequency cut-off",p.omega_ncut);  
  param.add("header","header of output files",p.header);
  param.add("pot","impurity potential",p.pot);
  param.add("pot_i","initial site of impurity potential",p.pot_i);
  param.add("pot_f","final site of impurity potential",p.pot_f);
  param.parse(argc,argv);
  p.nseed += id*1231;

  stringstream ss;
  ss << "_output.dat";
  string filename = p.header + ss.str();
  ofstream ofs(filename.c_str());
  
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
  
  model_struct model(p.nsite*2,p.nsite); // hardcore boson model
  for (int i=0; i<p.nsite; i++) {
    //hopping lett to right.
    model.hopping[i][0] = i;
    model.hopping[i][1] = (i + 1) % p.nsite; // periodic boundary
    //hopping right toleft.
    model.hopping[i+p.nsite][0] = i % p.nsite;    
    model.hopping[i+p.nsite][1] = (i + p.nsite - 1)  % p.nsite;
    model.interaction[i][0] = i;
    model.interaction[i][1] = (i + 1) % p.nsite;
    model.v[i] = 0.0;
    model.mu[i] = p.mu;
  }
  //impurity potential
  for(int i=p.pot_i;i<p.pot_f;i++){
    model.mu[i] -= p.pot;
  }
  
  stochastic_green_MC_method mc(p,model,psi_initial);
  //mc.change_parameter(p);
  for (int k=0; k<p.nthermal; k++) {
    mc.update();
  }
  // observable kin;
  vector<observable> ne(p.nsite);
  vector<observable> Cjj(p.omega_ncut);
  for (int j=0; j<p.nmeasure*p.nbin; j++) { // to be implemented about BIN sampling
    mc.update();
    // measurement of n_i    
    /*
    if ( mc.is_diagonal() ) {
      double R_tmp = mc.get_R();
      vector<double> ne_tmp = mc.get_ne();
      for (int i=0; i<p.nsite; i++) {
	ne[i].add(ne_tmp[i],R_tmp);
      }
    }
    */
    // measurement of C_jj
    if ( mc.is_diagonal() && j>1000) {
      double R_tmp = mc.get_R();
      vector<double> Cjjtmp = mc.get_Cjj();
      for (int i=0; i<p.omega_ncut; i++) {
	Cjj[i].add(Cjjtmp[i],R_tmp);
      }
    }
  }
  /*
  for(int i=0;i<p.nsite;i++){
    ofs << i << ' ' << ne[i].get_av() << ' ' << ne[i].get_err() << ' ' << ne[i].n << endl;
  }
  ofs << endl; 
  */
  vector<double> result(p.omega_ncut,0.);
  for(int i=0; i<p.omega_ncut; i++){
    result[i] = Cjj[i].get_av();
  }

  mpi1darray result_mpi(nproc,id,result);
  result_mpi.send_receive();
  if (id == 0) {
    for (int i=0; i<p.omega_ncut; i++) {
      cout << i << ' ';
      for (int k=0; k<nproc; k++) {
	cout << result_mpi.get_data(k,i) << ' ';
      }
      cout << endl;
    }
  }
#ifndef NOMPI
  MPI_Finalize();
#endif
}
