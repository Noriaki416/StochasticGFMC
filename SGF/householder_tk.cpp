#include<iostream>
#include<vector>
#include<cmath>
using namespace std;

extern "C" {
    void dsyev_(char *jobz,char *uplo,int *Np,double *A,
	      int *lda,double *w,double *work,int *lwork,int *info);
  void dgemv_(char *trans,int *column,int *row,double *alpha,double *A,int *lda,double *x,int *incx,double *beta,double *y,int *incy);
  double ddot_(int *Np,double *A,int *incx,double *B,int *incy);
}

int main() {
  double v = 1.0;

  int n = 2*2*2*2*2*2; 
  vector<int> list1; // k ---> configration
  vector<int> list2(n,-1); // configration --> k
  vector<int> tmp(6);  
  for (int i=0; i<n; i++) {
    int itmp = i;
    int np = 0;
    for (int j=0; j<6; j++) {
      tmp[j] = itmp % 2;
      if (tmp[j] == 1) np++;
      itmp = itmp/2;
    }
    if (np == 3) {
      list1.push_back(i);
      list2[i] = list1.size()-1;
    }
  }

  int nbase = list1.size();

  double H[nbase*nbase];
  for (int i=0; i<nbase*nbase; i++) {
    H[i] = 0.;
  }
  vector<vector<int> > element(nbase);
  
  for (int i=0; i<nbase; i++) {
    //    cout << "i: " << i << endl;
    int itmp = list1[i];
    for (int j=0; j<6; j++) {
      tmp[j] = itmp % 2;
      itmp = itmp/2;
    }
    // diagonal
    double sum_v = 0.;
    for (int j=0; j<6; j++) {
      sum_v += v*tmp[j]*tmp[(j+1) % 6];
    }
    H[i*nbase + i] = sum_v;
    // offdiagonal
    for (int j=0; j<6; j++) {
      // right
      if ((tmp[j] == 1) && (tmp[(j+1) % 6] == 0)) {
	vector<int> tmp2(tmp);
	tmp2[j] = 0;
	tmp2[(j+1)%6] = 1;
	int c = tmp2[5];
	for (int l=4; l>=0; l--) {	  
	  c *= 2;
	  c += tmp2[l];
	}
	int k = list2[c];
	if (k == -1) { cout << "ERROR" << endl; }
	H[i*nbase + k] = -1.;
	element[i].push_back(k);
      }
      // left
      if ((tmp[j] == 1) && (tmp[(j+5) % 6] == 0)) {
	vector<int> tmp2(tmp);
	tmp2[j] = 0;
	tmp2[(j+5)%6] = 1;
	int c = 0;
	for (int l=5; l>=0; l--) {	  
	  c *= 2;
	  c += tmp2[l];
	}
	int k = list2[c];
	if (k == -1) { cout << "ERROR" << endl; }
	H[i*nbase + k] = -1.;
	element[i].push_back(k);
      }
    }
  }
  int lda = nbase;
  int lwork = 3*nbase - 1;
  int info;
  double w[nbase],work[lwork];
  char jobz='V',uplo='L';
  dsyev_(&jobz,&uplo,&nbase,H,&lda,w,work,&lwork,&info);

  for (double temp = 0.02; temp < 10.0; temp += 0.05) {
    double beta = 1./temp;
    double z = 0.;
    double kin = 0.;
    for (int i=0; i<nbase; i++) {
      z += exp(-beta*w[i]);
      for (int j=0; j<nbase; j++) {
	for (int k=0; k<element[j].size(); k++) {
	  int m = element[j][k];
	  kin += H[i*nbase + j]*H[i*nbase + m]*exp(-beta*w[i]);
	}
      }
    }
    cout << temp << ' ' << kin/z << endl;
  }
}
