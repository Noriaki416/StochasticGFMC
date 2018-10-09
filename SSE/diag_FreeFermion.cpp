// diag_FreeFermion3.cpp ... output of Cjj(omega1)
// compilie on jupiter3
// icc diag_FreeFermion.cpp -I ~/lib/cpplapack/include/  -Wl,--start-group ${MKLROOT}/lib/intel64/libmkl_intel_lp64.a ${MKLROOT}/lib/intel64/libmkl_sequential.a ${MKLROOT}/lib/intel64/libmkl_core.a -Wl,--end-group -lpthread -lm -ldl
#include<iostream>
#include<cmath>
#include<complex>
#include<fstream>
#include<cstdlib>
#include<vector>
#include<string>
#include<cpplapack.h>
using namespace std;
using namespace CPPL;

inline double sqr(double x) {
  return x*x;
}

struct parameters { 
  int nsite; // # of sites
  double t; // hopping
  int nomega;
};

class diag_method {
private:
  parameters p;
  vector<double> pot;
  dsymatrix Ham;
  vector<double> e;
  vector<vector <double> > ev; //eigen vectors
public:
  diag_method(const parameters p_in, const vector<double> pot_in)
    : p(p_in), pot(pot_in), Ham(p_in.nsite)
    , ev(p_in.nsite, vector<double> 
	 (p_in.nsite, 0.0)) {}
  void change_parameter(const parameters p_in, const vector<double> pot_in) {
    p = p_in;
    pot = pot_in;
  }
  void diagonalization() {
    for (int l=0; l<p.nsite; l++) {
      for (int ll=0; ll<p.nsite; ll++) {
	Ham(l,ll) = 0.0;
      }
    }
    // diagonal elements
    for (int l=0; l<p.nsite; l++) {
      Ham(l,l) = pot[l];
    }
    // hopping
    for (int l=0; l<p.nsite-1; l++) {
      Ham(l+1,l) = -p.t;
      Ham(l,l+1) = -p.t;
    }
    Ham(0,p.nsite-1) = -p.t;
    Ham(p.nsite-1,0) = -p.t;    
    // diagonalizatoin
    //vector<zcovector> eigen_vectors;
    vector<dcovector> v;
    Ham = Ham.dsyev(e,v);

    for (int n=0; n<p.nsite; n++) {
      double sum = 0.0;
      for (int l=0; l<p.nsite; l++) {
	//	zvec(l) = Ham(l,n);
	ev[l][n] = v[n](l); // normalization ?????
	sum += ev[l][n]*ev[l][n];
      }
      sum = 1./sqrt(sum);
      if (ev[p.nsite/2 + 1][n] < 0) {
	sum *= -1.;
      }
      for (int l=0; l<p.nsite; l++) {
	ev[l][n] *= sum;
      }       
    }    
  }
  double get_ene(double temp, double mu) {
    double sum = 0.;
    for (int i=0; i<p.nsite; i++) {
      sum += e[i]/(exp((e[i]-mu)/temp)+1.0);
    }
    return sum/(double) p.nsite;
  }
  double get_kin(double temp, double mu) {
    double sum = 0.;
    for (int n=0; n<p.nsite; n++) {
      double sum2 = 0.;
      for (int l=0; l<p.nsite; l++) {
	int ll = (l + 1) % p.nsite;
	sum2 -= ev[ll][n]*ev[l][n] + ev[l][n]*ev[ll][n];	
      }
      sum += sum2/(exp((e[n]-mu)/temp)+1.0);
    }
    return sum/(double) p.nsite;
  }
  inline double FermiDist(double e,double temp, double mu) {
    return 1./(exp((e-mu)/temp)+1.0);    
  }
  inline double FermiDist1(double e,double temp,double mu) {
    return -1./temp*exp((e-mu)/temp)/sqr(exp((e-mu)/temp)+1.0);    
  }
  double get_w(double temp, double mu) { 
    double sum = 0.;
    for (int n=0; n<p.nsite; n++) {
      for (int nn=0; nn<p.nsite; nn++) {
	double J = 0.0;
	for (int l=0; l<p.nsite; l++) {
	  int l_next = (l+1) % p.nsite;
	  J += ev[l_next][n]*ev[l][nn] - ev[l][n]*ev[l_next][nn];
	}
	J /= (double) p.nsite;
	if (n != nn) {
	  sum += -J*J*(FermiDist(e[n],temp,mu)
		      -FermiDist(e[nn],temp,mu))/(e[n]-e[nn]);
	} else {
	  sum += -J*J*FermiDist1(e[n],temp,mu);
	}
      }
    }
    return sum;
  }
  vector<double> get_Cjjomega(double temp, double mu) {
    vector<double> Cjjomega(p.nomega);
    for (int k=0; k<p.nomega; k++) {
      double omega_n = 2.0*M_PI*temp*(double) k;
      complex<double> sum(0.,0.);
      complex<double> ci(0.,1.);
      for (int n=0; n<p.nsite; n++) {
	for (int nn=0; nn<p.nsite; nn++) {
	  double J = 0.0;
	  for (int l=0; l<p.nsite; l++) {
	    int l_next = (l+1) % p.nsite;
	    J += ev[l_next][n]*ev[l][nn] - ev[l][n]*ev[l_next][nn];
	  }
	  J /= (double) p.nsite;
	  sum += -J*J*(FermiDist(e[n],temp,mu)-FermiDist(e[nn],temp,mu))/(ci*omega_n + e[n]-e[nn]);
	}
      }
      Cjjomega[k] = real(sum);
    }
    return Cjjomega;
  }
  /*
  double get_kin_test(double temp, double mu) {
    double sum = 0.0;
    for (int n=0; n<p.nsite; n++) {
      double k = 2.0*M_PI*n/(double) p.nsite;
      double ene = -2.0*cos(k);
      sum += ene/(exp((ene+2.0-mu)/temp)+1.0);     
    }
    return sum/(double) p.nsite;
  }
  */
  double get_ntot(double temp, double mu) {
    double sum = 0.;
    for (int i=0; i<p.nsite; i++) {
      sum += 1.0/(exp((e[i]-mu)/temp)+1.0);
    }
    return sum/(double)p.nsite;
  }
  vector<double> get_ne(double temp, double mu) {
    vector<double> w(p.nsite);
    for (int i=0; i<p.nsite; i++) {
      w[i] = 1.0/(exp((e[i]-mu)/temp)+1.0);
    }
    vector<double> result(p.nsite);
    for (int i=0; i<p.nsite; i++) {
      double sum = 0.;
      for (int j=0; j<p.nsite; j++) {
	sum += w[j]*ev[i][j]*ev[i][j];
      }
      result[i] = sum;
    }
    return result;    
  }  
  double get_Cjj_tau(double tau,double temp, double mu) {
    double sum = 0.;
    for (int n=0; n<p.nsite; n++) {
      for (int nn=0; nn<p.nsite; nn++) {
	double J1 = 0.0;
	double J2 = 0.0;
	for (int l=0; l<p.nsite; l++) {
	  int l_next = (l+1) % p.nsite;
	  J1 += ev[l_next][n]*ev[l][nn] - ev[l][n]*ev[l_next][nn];
	  J2 += ev[l_next][nn]*ev[l][n] - ev[l][nn]*ev[l_next][n];
	}
	J1 /= (double) p.nsite;
	J2 /= (double) p.nsite;
	sum += -J1*J2*FermiDist(e[n],temp,mu)*(1.0-FermiDist(e[nn],temp,mu))
	  *exp(tau*(e[n]-e[nn]));
      }
    }
    return sum;
  }
};

int main(){
  parameters p;
  p.nsite = 8;
  p.t = 1.0;
  p.nomega = 5;

  vector<double> pot(p.nsite,0.0);
  for (int i=2; i<=4; i++) {
    pot[i] = 1.0001;
  }

  diag_method dm(p,pot);
  dm.diagonalization();

  /*
  int ndiv = 100;
  double beta = 1/0.2;
  double mu = -1.0;
  for (int i=0; i<=ndiv; i++) {
    double tau = beta/(double)ndiv*(double) i;
    cout << tau << ' ' << dm.get_Cjj_tau(tau,1./beta,mu) << endl;
  }
  */

  for (double temp = 0.01; temp<=2.00001; temp+=0.01) {
    double mu = -1.0;
    vector<double> tmp = dm.get_Cjjomega(temp,mu);
    cout << temp << ' ' 
	 << dm.get_w(temp,mu) << ' ';    
    for (int k=1; k<p.nomega; k++) {
      cout << tmp[k] << ' ';
    }
    cout << endl;
  }
}
