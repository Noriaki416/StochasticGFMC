// MPI version
// compile on jupiter3
// MPICH : mpiicpc SSE_SpinlessFermion.cpp 
// single: icc -DNOMPI SSE_SpinlessFermion.cpp 
#ifndef NOMPI
#include<mpi.h>
#endif
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include "MersenneTwister.h"
using namespace std;

inline double sqr(double x) {
  return x*x;
}

template <typename T> void swap03( T & a, T & b ) {
  T temp = a ;
  a = b ;
  b = temp ;
}

struct parameters {
  string header;
  int nthermal,nbin,nmeasure; // MC sampling parameter
  int l_init; // initial size of vertex_list
  int nsite; // # of sites
  int seed;
  int nomega; // number of Matsubara freq. to be measured
  int ndiv;
  // Sandvik 1999, J=1 
  vector<double> t;  // hopping (usually set as 1.0)
  vector<double> v;  // nearest neighbor Coulomb interaction 
  vector<double> h;
  vector<double> C;
  double beta;
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
	    //    cout << keys[i] << ' ' << str_keyword << endl;
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
};

struct vertex {
  int kind; 
  // 0 ... Identity Operator
  // 1 ... : |   2... | :   3... : |  4... | :  5... | |  6... : :  
  //       ---        ---        ---       ---       ---       ---
  //       : |        | :        | :       : |       | |       : :
  //   (":"= 0, "|"= 1  <-- charge state)
  int bond; // spatial position of a vertex  bond = site:bond~site:bond+1
  //  vector<int> link, leg, sz; // temporal position of a vertex connected with leg i
  // leg asignment
  //     0 | | 1
  //       --- 
  //     2 | | 3
  vertex() {
    bond = -1;
    kind = 0;
  }    
};

class SSE_method {
private:
  parameters p;
  vector<vertex> v; // a list of vertices
  vector<int> ch; // 0 or 1 (charge configration at tau=0)
  vector<int> ch_tmp; // 0 or 1 (charge configration during diagonal update)
  int nham;
  //mt19937 mt;
  MTRand rng;
  //uniform_real_distribution<double> rnd;
  vector<vector<double> > weight; // prob for diagonal update
  vector<vector<vector<double> > > prob1,prob2; // prob for offdiagonal update
  vector<vector<int> > vkind_after_flipped1;
  vector<vector<int> > vkind_after_flipped2;
  vector<vector<int> > vkind_after_flipped3;
  vector<vector<int> > leg_after_flipped1;
  vector<vector<int> > leg_after_flipped2;
  vector<vector<int> > leg_after_flipped3;
  // vkind_after_flipped1[ikind][in_leg] vertex kind after flip for rnd() < prob1
  // vkind_after_flipped2[ikind][in_leg] vertex kind after flip for prob1 < rnd() < prob2
  // vkind_after_flipped3[ikind][in_leg] vertex kind after flip for prob2 < rnd() 
  vector<int> ir;
  vector<double> log_factorial;
  vector<double> log_tau_list;
  vector<int> ich_to_vkind;
  vector<int> link_list; // assigning tau*4 + ileg --> tau*4 + ileg
  vector<int> ch_list; // assigning tau*4 + ileg --> a site for flipping sping
  vector<int> link_tmp; // used for making a link list
  vector<bool> is_vertex_connected;
  vector<vector<int> > Cjj_count_table;
  vector<vector<vector<double> > > integ_table;
public:
  SSE_method(parameters p_in) :
    p(p_in), 
    rng(p_in.seed), ch(p_in.nsite), ch_tmp(p_in.nsite), ir(p_in.nsite),
    weight(7,vector<double>(p_in.nsite,0.0)),
    prob1(7,vector<vector<double> >(4,vector<double>(p_in.nsite,0.0))),
    prob2(7,vector<vector<double> >(4,vector<double>(p_in.nsite,0.0))),
    vkind_after_flipped1(7,vector<int>(4,0)),
    vkind_after_flipped2(7,vector<int>(4,0)),
    vkind_after_flipped3(7,vector<int>(4,0)),
    leg_after_flipped1(7,vector<int>(4,0)),
    leg_after_flipped2(7,vector<int>(4,0)),
    leg_after_flipped3(7,vector<int>(4,0)),
    ich_to_vkind(4),
    v(p_in.l_init),
    link_tmp(p_in.nsite),
    is_vertex_connected(p_in.nsite),
    link_list(p_in.l_init*4),
    ch_list(p_in.l_init*4),
    log_factorial(p_in.l_init),
    log_tau_list(p_in.ndiv) {
    nham = 0;
    if (p.nsite > 0) {    
      for (int i=0; i<p.nsite; i++) {
	ir[i] = i + 1;	
      }
      ir[p.nsite-1] = 0;
    }
    // for correlation function measurement
    log_factorial[0] = 0.0;
    for (int i=1; i<v.size(); i++) {
      log_factorial[i] = log_factorial[i-1] + log((double) i);
    }
    log_tau_list[0] = 0.0;
    for (int i=1; i<p.ndiv; i++) {
      double x = (double)i/(double) p.ndiv;
      log_tau_list[i] = log(x);
    }    
    // 1 ... : |   2... | :   3... : |  4... | :  5... | |  6... : :  
    //       ---        ---        ---       ---       ---       ---
    //       : |        | :        | :       : |       | |       : :
    ich_to_vkind[0 + 2*0] = 6;
    ich_to_vkind[1 + 2*0] = 2;
    ich_to_vkind[0 + 2*1] = 1;
    ich_to_vkind[1 + 2*1] = 5;
        
    // set intial charge config.      
    for (int j=0; j<p.nsite; j++) {
      //      ch[j] = j % 2;
      ch[j] = (int)(rng()+0.5); // 0 or 1, randomly chosen
    }
    for (int tau=0; tau<v.size(); tau++) {
      v[tau].kind = 0; // identity operator
      v[tau].bond = -1;
    }
    // make a flip probability
    // for identity operator
    for (int ileg=0; ileg<4; ileg++) {
      vkind_after_flipped1[0][ileg] = 0;
      vkind_after_flipped2[0][ileg] = 0;
      vkind_after_flipped3[0][ileg] = 0;
      for (int i=0; i<p.nsite; i++) {
	prob1[0][ileg][i] = 1.0;
	prob2[0][ileg][i] = 1.0;
      }
    }
    // for the other operators
    #include "prob_table_SpinlessFermionPot.cpp"    
  }
  void line_update() {
    for (int i=0; i<p.nsite; i++) {
      is_vertex_connected[i] = false;
    }
    for (int tau=0; tau<v.size(); tau++) {
      if (v[tau].bond >= 0) {
	is_vertex_connected[v[tau].bond] = true;
	is_vertex_connected[ir[v[tau].bond]] = true;
      }
    }
    for (int i=0; i<p.nsite; i++) {
      if (!is_vertex_connected[i]) {	
	ch[i] = (int)(2*rng());
      }
    }
  }
  void vertex_doubled() {
    // adjust length of operator string
    vector<vertex> v_new(2*v.size());
    for (unsigned int tau=0; tau<v.size(); tau++) {
      v_new[2*tau] = v[tau];
      v_new[2*tau+1].kind = 0;
      v_new[2*tau+1].bond = -1;	
    }
    swap03(v,v_new);
    link_list.resize(v.size()*4);
    ch_list.resize(v.size()*4);
    log_factorial.resize(v.size());
    log_factorial[0] = 0.0;
    for (int i=1; i<v.size(); i++) {
      log_factorial[i] = log_factorial[i-1] + log((double) i);
    }
  }
  void diagonal_update() {
    // initialization
    for (unsigned int i=0; i<p.nsite; i++) {
      ch_tmp[i] = ch[i];  // ch[i] = 0 or 1;
    }
    // search
    for (unsigned int tau=0; tau<v.size(); tau++) {
      if (v[tau].kind == 0) {
	// identitiy --> diagonal
	int j_tmp = (int)(rng()*(double)p.nsite);
	int ich_tmp = ch_tmp[j_tmp] + 2*ch_tmp[ir[j_tmp]];	
	int vkind_tmp = ich_to_vkind[ich_tmp];
	double prob = (double) p.nsite*p.beta/(double)(v.size()-nham)
	  *weight[vkind_tmp][j_tmp];
	if (rng() < prob) {
	  v[tau].kind = vkind_tmp;
	  v[tau].bond = j_tmp;
	  nham++;
	}
      } else if (weight[v[tau].kind][v[tau].bond] > 0.0) {
	// diagonal --> identity
	// int ich_tmp = ch[v[tau].bond] + 2*ch[ir[v[tau].bond]];
	int kind_tmp = v[tau].kind;
	int j_tmp = v[tau].bond;
	double prob = (double)(v.size()-nham+1)
	  /(double)p.nsite/p.beta/weight[kind_tmp][j_tmp];
	if (rng() < prob) {
	  v[tau].kind = 0;
	  v[tau].bond = -1;
	  nham--;
	}
      } else if ((v[tau].kind == 3) || (v[tau].kind == 4)) {
	// offdiagonal
	ch_tmp[v[tau].bond] = 1 - ch_tmp[v[tau].bond];
	ch_tmp[ir[v[tau].bond]] = 1 - ch_tmp[ir[v[tau].bond]];
      }
    }
  }
  int offdiagonal_update(int nloop,int it) {
    if (nham == 0) return 0;

    // make link list
    for (int tau=0; tau<v.size(); tau++) {
      for (int leg=0; leg<4; leg++) {
	ch_list[tau*4+leg] = -1;
	link_list[tau*4+leg] = -1;
      }
    }
    for (int i=0; i<p.nsite; i++) {
      link_tmp[i] = -1;
    }
    // forward searck (backward link)
    for (int tau=0; tau<v.size(); tau++) {
      if (v[tau].kind == 0) { // identity operator
	link_list[tau*4 + 0] = -2;
	link_list[tau*4 + 1] = -2;
	link_list[tau*4 + 2] = -2;
	link_list[tau*4 + 3] = -2;
      } else { // vertex
	link_list[tau*4 + 2] = link_tmp[v[tau].bond];
	link_list[tau*4 + 3] = link_tmp[ir[v[tau].bond]];
	link_tmp[v[tau].bond] = tau*4 + 0;
	link_tmp[ir[v[tau].bond]] = tau*4 + 1;	
      }
    }
    for (int tau=0; tau<v.size(); tau++) {      
      if (link_list[tau*4 + 2] == -1) {
	link_list[tau*4 + 2] = link_tmp[v[tau].bond];
	ch_list[tau*4 + 2] = v[tau].bond;
      }
      if (link_list[tau*4 + 3] == -1) {      
	link_list[tau*4 + 3] = link_tmp[ir[v[tau].bond]];
	ch_list[tau*4 + 3] = ir[v[tau].bond];
      }
    }
    // backward search
    for (int tau=0; tau<v.size(); tau++) {
      if (link_list[tau*4] > -2) { 
	int link1 = link_list[tau*4 + 2];
	int link2 = link_list[tau*4 + 3];
	int tau1 = link1 / 4;
	int leg1 = link1 % 4;
	int tau2 = link2 / 4;
	int leg2 = link2 % 4;
	link_list[tau1*4 + leg1] = tau*4 + 2;
	link_list[tau2*4 + leg2] = tau*4 + 3;
	ch_list[tau1*4 + leg1] = ch_list[tau*4 + 2];
	ch_list[tau2*4 + leg2] = ch_list[tau*4 + 3];
      }
    }
    int count_vertex = 0;
    if (nloop <= 0) {
      int loop_count = 0;
      while (count_vertex < nham) {
	loop_count++;
	count_vertex += worm_search();
      }
      return loop_count;
    } else {
      for (int i=0; i<nloop; i++) {
	worm_search();
      }
      return nloop;
    }
  }
  inline int worm_search() {
    int worm_tau_init;
    do {
      worm_tau_init = (int)(rng()*v.size());
    } while (v[worm_tau_init].kind <= 0);
    int worm_leg_init = (int)(rng()*4);
    int worm_tau = worm_tau_init;
    int worm_leg = worm_leg_init;      
    int count_vertex = 0;
    while (true) {
      count_vertex++;
      double r = rng();
      int ktmp = v[worm_tau].kind;
      int jtmp = v[worm_tau].bond;
      int leg_in = worm_leg;
      if (r<prob1[ktmp][worm_leg][jtmp]) {
	v[worm_tau].kind = vkind_after_flipped1[ktmp][worm_leg];
	worm_leg = leg_after_flipped1[ktmp][worm_leg];
      } else if (r<prob2[ktmp][worm_leg][jtmp]) {
	v[worm_tau].kind = vkind_after_flipped2[ktmp][worm_leg];
	worm_leg = leg_after_flipped2[ktmp][worm_leg];
      } else {
	v[worm_tau].kind = vkind_after_flipped3[ktmp][worm_leg];
	worm_leg = leg_after_flipped3[ktmp][worm_leg];
      }	
      int tau_plus_leg = 4*worm_tau + worm_leg; //leg positon for outgoing    
      worm_tau = link_list[tau_plus_leg]/4; // neat tau
      worm_leg = link_list[tau_plus_leg]%4; // next leg
      int flipsite = ch_list[tau_plus_leg]; // charge flip at tau = 0
      if (flipsite >=0) {
	ch[flipsite] = 1 - ch[flipsite];
      } 
      // finish ?
      if ((worm_tau == worm_tau_init) && (worm_leg == worm_leg_init)) {
	break;
      }  
    }
    return count_vertex;
  }
  int get_l() {
    return v.size();
  }
  int get_nham() {
    return nham;
  }   
  int get_ntot() {
    int s = 0;
    for (int i=0; i<p.nsite; i++) {
      s += ch[i];
    }
    return s;
  }
  vector<double> get_ne() {
    vector<int> ne(p.nsite,0.0);
    for (int i=0; i<p.nsite; i++) {
      ch_tmp[i] = ch[i];
    }
    for (int tau=0; tau<v.size(); tau++) {
      if (v[tau].kind > 0) {
	for (int i=0; i<p.nsite; i++) {
	  ne[i] += ch_tmp[i];
	}
      }
      if ((v[tau].kind == 3) || (v[tau].kind == 4)) {
	ch_tmp[v[tau].bond] = 1 - ch_tmp[v[tau].bond];
	ch_tmp[ir[v[tau].bond]] = 1 - ch_tmp[ir[v[tau].bond]];
      }
    }
    vector<double> result(p.nsite);
    for (int i=0; i<p.nsite; i++) {
      result[i] = (double)ne[i]/nham;
    }
    return result;    
  }
  int get_nkin() {
    int count = 0;
    for (int tau=0; tau<v.size(); tau++) {
      if ((v[tau].kind == 3) || (v[tau].kind == 4)) {
	count++;
      }
    }
    return count;
  }
  int get_w() { 
    int count1 = 0;
    int count2 = 0;
    for (int tau=0; tau<v.size(); tau++) {
      if (v[tau].kind == 3) count1++;
      if (v[tau].kind == 4) count2++;
    }
    return (count1-count2)*(count1-count2) - count1 - count2;
  }
  void start_measurement_Cjj() {
    Cjj_count_table.resize(v.size());
    for (int i=0; i<v.size(); i++) {
      Cjj_count_table[i].resize(v.size());
      for (int j=0; j<v.size(); j++) {
	Cjj_count_table[i][j] = 0;
      }
    }
    integ_table.resize(p.nomega);
    for (int nn=0; nn<p.nomega; nn++) {
      integ_table[nn].resize(v.size());
      for (int n=0; n<v.size(); n++) {
	integ_table[nn][n].resize(v.size());
	for (int m=0; m<=n-2; m++) {
	  integ_table[nn][n][m] = integ(n,m,nn);
	}
      }
    }
  }  
  void count_Cjj() {
    vector<int> v_new(nham,0);
    int ntot = 0;
    for (int i=0; i<p.nsite; i++) {
      ntot += ch[i];
    }
    // 1 ... : |   2... | :   3... : |  4... | :  5... | |  6... : :  
    //       ---        ---        ---       ---       ---       ---
    //       : |        | :        | :       : |       | |       : :
    int c = 0;
    int cc = 0;
    if (ntot % 2 == 1) {
      for (int tau=0; tau<v.size(); tau++) {
	if (v[tau].kind == 3) {
	  v_new[c] = 1;
	  c++;
	  cc++;
	} else if (v[tau].kind == 4) {
	  v_new[c] = -1;
	  c++;
	  cc++;
	} else if (v[tau].kind > 0) {
	  c++;
	}
      } 
    } else {
      for (int tau=0; tau<v.size(); tau++) {
	if (v[tau].kind == 3) {
	  v_new[c] = 1;
	  if (v[tau].bond == p.nsite-1) {
	    v_new[c] = 1;
	  }
	  c++;
	  cc++;
	} else if (v[tau].kind == 4) {
	  v_new[c] = -1;	
	  if (v[tau].bond == p.nsite-1) {
	    v_new[c] = -1;
	  }
	  c++;
	  cc++;
	} else if (v[tau].kind > 0) {
	  c++;
	}
      }
    }
    //    if (c != nham) { cout << "ERROR" << c << ' ' << nham << endl; exit(0); }
    for (int m=0; m<=nham-2; m++) {
      int count = 0;
      for (int p=0; p<nham; p++) {
	count += v_new[p]*v_new[(p+m+1)%nham];
      }
      Cjj_count_table[nham][m] += count;
    }    
    Cjj_count_table[nham][nham-1] += cc;
  }
  vector<double> get_Cjj_mean() {
    vector<double> Cjj(p.ndiv,0.0);
    double sum = 0.;
    for (int n=2; n<v.size(); n++) {
      sum += (double)(nham - 1)*Cjj_count_table[n][n-1];
    }
    Cjj[0] = sum/(double)p.beta/(double)p.beta;

    for (int i=1; i<p.ndiv; i++) {
      double sum = 0.;
      for (int n=2; n<v.size(); n++) {
	for (int m=0; m<=n-2; m++) {
	  double fexp = (double)m*log_tau_list[i]
	    + (double)(n-m-2)*log_tau_list[p.ndiv-i]
	    + log_factorial[n-1]
	    - log_factorial[n-m-2]
	    - log_factorial[m];
	  sum += exp(fexp)*(double)Cjj_count_table[n][m];	
	}
      }
      Cjj[i] = sum/(p.beta*p.beta);      
    }
    return Cjj;    
  }  
  vector<double> get_Cjjomega() {
    vector<double> Cjjomega(p.nomega);
    for (int nn=0; nn<p.nomega; nn++) {    
      double sum = 0.0;
      for (int n=2; n<v.size(); n++) {
	for (int m=0; m<=n-2; m++) {
	  sum += integ_table[nn][n][m]*(double)Cjj_count_table[n][m];	
	}
      }
      Cjjomega[nn] = sum/p.beta;
    }
    return Cjjomega;    
  }  
  void reset_Cjj() {
    for (int i=0; i<v.size(); i++) {
      for (int j=0; j<v.size(); j++) {
	Cjj_count_table[i][j] = 0;
      }
    }
  }
  int get_sign() {
    int n_tot = 0;
    for (unsigned int i=0; i<p.nsite; i++) {
      n_tot += ch[i];      
    }
    int sign = 1;
    if (n_tot % 2 == 0) {
      for (unsigned int tau=0; tau<v.size(); tau++) {
	if (v[tau].bond == p.nsite - 1) {
	  int kind = v[tau].kind;
	  if ((kind == 3) || (kind == 4)) {
	    sign *= -1;
	  }
	}
      }
    }
    return sign;
  }   
  inline double integ(int n, int m, int nn) {
    int nd = 400; // division number
    double h = 2.5/(double) nd;
    double integ = 0.;
    for (int k=-nd; k<=nd; k++) { // DE formula
      double t = h*(double) k;
      double x = 0.5*(1.0+tanh(M_PI/2.*sinh(t)));
      double exponent = m*log(x) + (n-m-2)*log(1.0-x) 
	+ log_factorial[n-1]
	- log_factorial[m] - log_factorial[n-m-2];      
      integ += exp(exponent)/sqr(cosh(M_PI/2.0*sinh(t)))
	*M_PI/4.*cosh(t)*h*cos(2.0*M_PI*x*(double)nn);
    }
    return integ;
  }
  void output_vertex() {
    for (int i=0; i<p.nsite; i++) {
      ch_tmp[i] = ch[i];
    }        
    for (int tau=0; tau<v.size(); tau++) {
      cout << "         ";
      for (int i=0; i<p.nsite; i++) {
	if (ch_tmp[i] == 0) {
	  cout << ": ";
	} else {
	  cout << "| ";
	}
      }
      cout << endl;
      cout << setw(2) << left
	   << tau << ' '
	   << setw(2) << left
	   << v[tau].kind << ' '
	   << setw(2) << left
	   << v[tau].bond << ' ';
      for (int i=0; i<p.nsite; i++) {
	if (ch_tmp[i] == 0) {
	  cout << ":";
	} else {
	  cout << "|";
	}
	if ((v[tau].kind > 0) && (v[tau].bond == i)) {
	  cout << "-";
	} else{
	  cout << " ";
	}
      }      
      cout << endl;
      if ((v[tau].kind == 3) || (v[tau].kind == 4)) {
	ch_tmp[v[tau].bond] = 1 - ch_tmp[v[tau].bond];
	ch_tmp[ir[v[tau].bond]] = 1 - ch_tmp[ir[v[tau].bond]];
      }
    }
  }
};					  

// observable class
class observable {
  double sum_av, sum_sq;
  unsigned int ncount;
public:
  observable() {
    ncount = 0;
    sum_av = 0.0;
    sum_sq = 0.0;
  }
  ~observable() {}
  void operator<<(double x) { 
    sum_av += x;
    sum_sq += sqr(x);
    ncount++;
  }
  double mean() {
    return (ncount > 0) ? sum_av/(double) ncount : 99999.0;
   }
  double err() {
    return (ncount > 1) ? 
      sqrt(( sum_sq/(double)ncount-sqr(mean()) )/(double)(ncount - 1)) : 99999.0;
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

int main(int argc, char** argv) {
  clock_t start_time = clock();

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
  double t,v,pot,mu;
  int potw;
  param.add("header","header of output",p.header);  
  param.add("nsite","size of a chain",p.nsite);
  param.add("t","hopping",t);
  param.add("v","nearest-neighbor interaction",v);
  param.add("mu","chemical potential",mu);
  param.add("beta","inverse temperature",p.beta);  
  param.add("pot","impurity potential",pot);
  param.add("potw","initial site of impurity potential",potw);
  param.add("nthermal","number of mcs for thermalization",p.nthermal);  
  param.add("nmeasure","number of measurement",p.nmeasure);  
  param.add("nbin","number of bins",p.nbin);  
  param.add("nseed","seed for random number generator",p.seed);
  param.add("nomega","number of Matsubara frequencies to be measured.",p.nomega);  
  for (int i=0; i<nproc; i++) {
    if (i == id) param.parse(argc,argv);
#ifndef NOMPI
    MPI_Barrier(MPI_COMM_WORLD);
#endif
  }
  p.seed += id*1231;
  p.v = vector<double>(p.nsite,v); // =v
  p.t = vector<double>(p.nsite,t); // hopping = 1.0
  p.C = vector<double>(p.nsite,t); // =p.t default
  p.l_init = 100;
  p.ndiv = 100; // not used
  p.h = vector<double>(p.nsite,-mu);
  for (int i=0; i<potw; i++) {
    p.h[i] += pot; // potential
  }

  SSE_method sse(p);
  observable nloop;
  for (int k=0; k<p.nthermal; k++) {
    if (sse.get_nham() > 0.8*sse.get_l()) {
      sse.vertex_doubled();
    }	
    sse.diagonal_update();
    sse.line_update();
    int tmp = (double) sse.offdiagonal_update(0,k);
    nloop << (double) tmp;
  }  
  //    observable ntot,ene,kin,sign,wind;
  //    vector<observable> ne(p.nsite);
  observable sign,wind;
  vector<observable> Cjjomega(p.nomega);
  sse.start_measurement_Cjj();
  for (int k=0; k<p.nmeasure; k++) {
    // observable ntot2,ene2,kin2,sign2;
    //      vector<observable> ne2(p.nsite);
    observable sign2,wind2;
    for (int kk=0; kk<p.nbin; kk++) {    
      //	cout << k << ' ' << kk << endl;
      sse.diagonal_update();      
      sse.line_update();
      sse.offdiagonal_update((int)(nloop.mean())+1,k);	
      int sign_tmp = sse.get_sign();
      sign2 << (double)sign_tmp;
      sse.count_Cjj();
      //	int nham_tmp = sse.get_nham(); 
      //int ntot_tmp = sse.get_ntot();
      //int kin_tmp = sse.get_nkin();
      int wind_tmp = sse.get_w();
      //vector<double> ne_tmp = sse.get_ne();
      //	ene2 << (double)(sign_tmp*nham_tmp);
      //kin2 << (double)(sign_tmp*kin_tmp);
      //ntot2 << (double)(sign_tmp*ntot_tmp);
      wind2 << (double)(sign_tmp*wind_tmp);
      //	cout << wind_tmp << endl;
      //	for (int i=0; i<p.nsite; i++) {
      //  ne2[i] << (double)(ne_tmp[i]*sign_tmp);	  
      //}	
    }
    //      ene << ene2.mean();
    //kin << kin2.mean();
    //ntot << ntot2.mean();
    sign << sign2.mean();
    wind << -wind2.mean()/(double)p.nsite/(double)p.nsite/p.beta;
    vector<double> Cjjtmp = sse.get_Cjjomega();
    for (int nn=0; nn<p.nomega; nn++) {
      Cjjomega[nn] << Cjjtmp[nn]/(double)p.nbin/(double)p.nsite/(double)p.nsite;
    } 
    sse.reset_Cjj();
    //for (int i=0; i<p.nsite; i++) {
    //	ne[i] << ne2[i].mean();
    //}   
  }
  
  double b = sign.mean();
  //    if (id == 0) cout << b << endl;
  //    double db = sign.err();
  double c = wind.mean();
  // double dc = wind.err();
  /*
    cout << temp << ' '
    << c/b << ' '
    << c/b*(dc/c+db/b) << ' ';
  */ 
  //    for (int nn=0; nn<p.nomega; nn++) {
  //      result[nn] = Cjjomega[nn].mean();
  //     double a = Cjjomega[nn].mean();
  //      double da = Cjjomega[nn].err();
  //      cout << a/b << ' '
  //   << a/b*(da/a+db/b) << ' ';
  //}
  //cout << endl;
  vector<double> result(p.nomega+2,0.);    
  result[0] = b;
  result[1] = c; 
  for(int i=0; i<p.nomega; i++){
    result[i+2] = Cjjomega[i].mean();
    if (i == 0) result[i+2] *= -1.;
  }

  mpi1darray result_mpi(nproc,id,result);
  result_mpi.send_receive();
  if (id == 0) {
    string filename = p.header + ".dat";
    ofstream ofs(filename.c_str());
    // cout << temp << ' ';
    for (int nn=0; nn<p.nomega+2; nn++) {
      double sum = 0.;
      double sum2 = 0.;
      for (int k=0; k<nproc; k++) {
	double tmp = result_mpi.get_data(k,nn); 
	sum += tmp;
	sum2 += tmp*tmp;
      }
      sum /= (double) nproc;
      sum2 /= (double) nproc;
      ofs << sum << ' ' << sqrt(sum2-sum*sum)/sqrt(nproc) << ' ';
    }
    clock_t end_time = clock();
    ofs << "# time: " << (double)(end_time - start_time)/CLOCKS_PER_SEC << " [s]" << endl; 
    ofs.close();
    //    cout << endl;
  }
  
  /*
    for (int i=0; i<=100; i++) {
    double x = 0.01*(double) i*p.beta;
    double d = c + a*cos(2.0*M_PI*x/p.beta)/p.beta;
    double dd = dc + da*cos(2.0*M_PI*x/p.beta)/p.beta;
    cout << x << ' '
    << d/b << ' '
    << d/b*(dd/d+db/b) << endl;
    }
  */
  
  /*
    vector<double> Cjj_av(p.ndiv+1);
    for (int i=0; i<p.ndiv; i++) { 
    Cjj_av[i] = Cjj[i].mean()/b;
    }
    Cjj_av[p.ndiv] = Cjj_av[0];
    double sum = 0.;
    double dtau = p.beta/(double) p.ndiv;
    for (int i=0; i<p.ndiv; i++) {
      double x1 = (double)i/(double)p.ndiv;
      double c1 = cos(2.0*M_PI*x1);
      double x2 = (double)(i+1)/(double)p.ndiv;
      double c2 = cos(2.0*M_PI*x2);
      sum += 0.5*(Cjj_av[i]*c1 + Cjj_av[i+1]*c2)*dtau;
    }
    cout << temp << ' ' << sum << endl;
    */
    //    cout << temp << ' ';
    // totan particle number
    /*
    a = ntot.mean()/(double) p.nsite;
    da = ntot.err()/(double) p.nsite;
    double aa = a;
    double dda = da;
    cout << a/b << ' ' << a/b*(da/a+db/b) << ' ';
    */
    // total energy
    /*
    a = ene.mean()/(double) p.nsite/(double)p.beta;
    da = ene.err()/(double) p.nsite/(double)p.beta;
    double sum = 0.;
    for (int i=0; i<p.nsite; i++) {
      sum += 0.25*p.v[i] + p.C[i] + 0.5*p.h[i];
    }
    sum /= (double)p.nsite;
    cout << -a/b + sum + mu*(aa-0.5) << ' '
	 << a/b*(da/a+db/b) + mu*dda << ' ';
    */
    // kinetic energy
    /*
    a = kin.mean()/(double) p.nsite/(double) p.beta;
    da = kin.err()/(double) p.nsite/(double) p.beta;
    cout << -a/b << ' ' << a/b*(da/a+db/b) << ' ';
    cout << b << ' ' << db << ' ';
    */
    // winding number
    /*
    a = wind.mean()*(double) p.nsite/(double) p.beta;
    da = wind.err()*(double) p.nsite/(double) p.beta;
    cout << a/b << ' ' << a/b*(da/a+db/b) << ' ';
    cout << b << ' ' << db << ' ';   
    cout << endl;
    */
#ifndef NOMPI
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
#endif
}


