#include<iostream>
#include<cmath>
#include<fstream>
#include<random>
#include<cstdlib>
#include<vector>
#include<list>
#include<string>
#include<bitset>
using namespace std;

int nCr(int n,int r){
  if ( r > n ) return 0;
  if ( r * 2 > n ) r = n - r;
  if ( r == 0 ) return 1;

  int result = n;
  for(int i=2;i<=r;i++){
      result *= (n-i+1);
      result /=i;
    }
  return result;
}

extern "C" {
    void dsyev_(char *jobz,char *uplo,int *Np,double *A,
	      int *lda,double *w,double *work,int *lwork,int *info);
  void dgemv_(char *trans,int *column,int *row,double *alpha,double *A,int *lda,double *x,int *incx,double *beta,double *y,int *incy);
  double ddot_(int *Np,double *A,int *incx,double *B,int *incy);
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
  double t; //hopping term
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

class matrix_element{
  private:
  parameters p;
  model_struct model;
  vector<vector<int> > basis_all;
  vector<vector<int> > basis_num;
  double T_sum_expec;
  double Ene_sum_expec;
  double V_sum_expec;
  public:
  matrix_element(const parameters p_in,const model_struct model_in):p(p_in),model(model_in){
    vector<vector<int> >  basis(pow(2,p.nsite)+1,vector<int>(pow(2,p.nsite)+1,0));// all number basis 
    vector<vector<int> > basis_np;//specified number basis
    for(int i=0;i<basis.size();i++){
      int sum = 0;
      for(int j=0;j<p.nsite;j++){
	basis[i][j] = static_cast<bitset<12> >(i)[j]; //example 2 = 000010 = (0,0,0,0,1,0)
	sum += basis[i][j];
      }
      if(sum == p.np) basis_np.push_back(basis[i]); //for specified number of particles
    }
    basis_all = basis;
    basis_num = basis_np;
    
    for(int i=0;i<basis_num.size();i++){
      //cout << i << "    " ;
      for(int j=0;j<p.nsite;j++){
	//cout << basis_num[i][j] ;
      }
      //cout << endl;
    }
    //cout << basis_num.size() << endl;
  }
  void exact_diag(){ 
    vector<vector<double> > V_element(basis_num.size(),vector<double>(basis_num.size(),0.0));    
    for(int i=0;i<basis_num.size();i++){
      double V_int=0.0;
      for(int j=0; j<p.nsite; j++){
	if( (basis_num[i][model.interaction[j][0]]==1) && (basis_num[i][model.interaction[j][1]]==1) ) V_int +=p.v;
      }
      V_element[i][i] = V_int;
      //cout << "V_element[" << i << "][" << i << "]: " << V_element[i][i] << endl;
    }
    /*
    cout << "V_element" << endl;
    for(int i=0;i<basis_num.size();i++){
      for(int j=0;j<basis_num.size();j++){
	cout << V_element[i][j] << " ";
      }
      cout << endl;
    }
    cout << endl;
    */
    
    //cout << "T_element" << endl;
    vector<vector<double> > T_element(basis_num.size(),vector<double>(basis_num.size(),0.0));    
    for(int i=0;i<basis_num.size();i++){
      for(int j=0;j<basis_num.size();j++){
	double T_hop=0.0;
	for(int k=0;k<p.nsite*2;k++){
	  vector<int> basis_num_new = basis_num[j];
	  if( ((basis_num_new[model.hopping[k][0]] == 1 ) && (basis_num_new[model.hopping[k][1]] == 0 )) ){
	    basis_num_new[model.hopping[k][0]] = 0;
	    basis_num_new[model.hopping[k][1]] = 1;
	    if(basis_num_new == basis_num[i]) T_hop += p.t;
	  }
	}
	  T_element[i][j] = T_hop;
	  //cout << T_element[i][j] << ' ';
      }
      //cout << endl;
    }
    
    vector<vector<double> > T2_element(basis_num.size(),vector<double>(basis_num.size(),0));
    for(int i=0;i<basis_num.size();i++){
      for(int j=0;j<basis_num.size();j++){
	T2_element[i][j] = T_element[j][i];
      }
    }
    /*
    if(T2_element == T_element) cout << "symmetric" << endl;
    else {cout << "not symmetric" << endl;}
    */
    vector<vector<double> > H_element(basis_num.size(),vector<double>(basis_num.size(),0));
    for(int i=0;i<basis_num.size();i++){
      for(int j=0;j<basis_num.size();j++){
	H_element[i][j] = V_element[i][j] - T_element[i][j];
      }
    }
    //cout << "H_element: " << endl;
     for(int i=0;i<basis_num.size();i++){
      for(int j=0;j<basis_num.size();j++){
	//cout << H_element[i][j] << ' ' ;
      }
      //cout << endl;
     }
     //cout << endl; 
     int N=H_element.size(),ipiv[N],lda=N,info,lwork=3*N-1;
     double Hami[N*N],Hami2[N*N],T_hami[N*N],V_hami[N*N],w[N],work[lwork];
     for(int j=0;j<N;j++){
       for(int i=0;i<N;i++){
	 Hami[i+j*N] = H_element[i][j];
	 Hami2[i+j*N] = H_element[i][j];
	 T_hami[i+j*N] = T_element[i][j];
	 V_hami[i+j*N] = V_element[i][j];
       }
     }
     char jobz='V',uplo='L';
     dsyev_(&jobz,&uplo,&N,Hami,&lda,w,work,&lwork,&info);
     //for check
     for(int j=0;j<N;j++){
       for(int i=0;i<N;i++){
	 //cout << Hami2[j+i*N] << ' ' ;
       }
       //cout << endl;
     }
     /*
     for(int i=0;i<N;i++){
       cout << "eigen_value: "<< w[i] << endl;
       cout << "eigen_vector = (" ;
	 for(int j=0;j<N;j++){
	   cout << Hami[i*N+j] << "," ;
	 }
	 cout << ")" << endl;
	 cout << endl;
     }
     */
     for(int i=0;i<N;i++){
       //cout <<"eigen_value : " << w[i] << endl;
     }
     
     vector<double> Hami_expec(H_element.size(),0.0),T_expec(T_element.size(),0.0),V_expec(V_element.size(),0.0);
     char trans='N';
     double alpha=1.0,beta=1.0,y_T[N],y_Hami[N],y_V[N],eigen_vec[N],eigen_vec2[N],eigen_vec3[N];
     int incx=1,incy=1;
     //cout << endl;
     for(int i=0;i<N;i++){
       for(int j=0;j<N;j++){
	 eigen_vec[j] = Hami[i*N+j];
	 //eigen_vec2[j] = eigen_vec[j];
	 //eigen_vec3[j] = eigen_vec[j];
	 y_T[j] = 0.0;
	 y_Hami[j] = 0.0;
	 y_V[j] =0.0;
       }
     dgemv_(&trans,&N,&N,&alpha,T_hami,&lda,eigen_vec,&incx,&beta,y_T,&incy);
     dgemv_(&trans,&N,&N,&alpha,Hami2,&lda,eigen_vec,&incx,&beta,y_Hami,&incy);
     dgemv_(&trans,&N,&N,&alpha,V_hami,&lda,eigen_vec,&incx,&beta,y_V,&incy);
     /*
     for(int k=0;k<N;k++){
       cout << y_T[k] << " ";
     }
     cout << endl;
     */
     T_expec[i] = ddot_(&N,eigen_vec,&incx,y_T,&incy);
     Hami_expec[i] = ddot_(&N,eigen_vec,&incx,y_Hami,&incy);
     V_expec[i] = ddot_(&N,eigen_vec,&incx,y_V,&incy);
     //cout << "Hami_expec: " << Hami_expec[i] << endl;
     //cout << "T_expec: " << T_expec[i] << endl;
     //cout << endl;
     }
     double T_sum=0.,H_sum=0.,V_sum=0.;
     for(int i=0;i<N;i++){
       T_sum += exp(-p.beta*w[i])*T_expec[i];
       H_sum += exp(-p.beta*w[i])*w[i];
       V_sum += exp(-p.beta*w[i])*V_expec[i];
     }
     //cout <<"T_sum: " << T_sum << endl;
     //cout <<"Hami_sum: " << H_sum << endl;
     double Z_partition=0.0;
     for(int i=0;i<N;i++){
       Z_partition += exp(-p.beta*w[i]);
     }
     //cout << "Z_partition: " << Z_partition << endl;
     T_sum_expec = T_sum/Z_partition;
     Ene_sum_expec = H_sum/Z_partition;
     V_sum_expec = V_sum/Z_partition;
     //cout << "T_sum: " << T_sum << endl;
  }
  void change_parameter(parameters p_in) {
    p = p_in;
  }
  double get_kin(){
    return T_sum_expec;
  }
    double get_energy(){
    return Ene_sum_expec;
  }
      double get_V(){
    return V_sum_expec;
  }
};



int main(){
  parameters p;
  p.nsite = 12;
  p.np = 8;
  p.v = 1.0;
  p.beta = 1.0;
  p.t = 1.0; 

  model_struct model(p.nsite*2,p.nsite); // hardcore boson model
  for (int i=0; i<p.nsite; i++) {
    model.hopping[i][0] = i;
    model.hopping[i][1] = (i + 1) % p.nsite; // periodic boundary
    model.hopping[i+p.nsite][0] = i ;//% p.nsite;    
    model.hopping[i+p.nsite][1] = (i + p.nsite - 1)  % p.nsite;
    model.interaction[i][0] = i;
    model.interaction[i][1] = (i + 1) % p.nsite;
    model.v[i] = p.v;
  }
  /*
  for(int i=0;i<model.hopping.size();i++){
    cout << model.hopping[i][0] << " " << model.hopping[i][1] << endl;
  }
  */

  matrix_element matmat(p,model);
  //ofstream ofs1("T_expec.data");

  for(double temp=0.01; temp <=10.0; temp +=0.5){
    p.beta = 1./temp;
    matmat.change_parameter(p);
    matmat.exact_diag();
    cout << temp << ' '  <<   matmat.get_energy() << ' ' << matmat.get_kin() << ' ' << matmat.get_V() << endl;
  }
  
}
