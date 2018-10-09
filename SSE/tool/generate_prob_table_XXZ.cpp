#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<algorithm>
#include<functional>
using namespace std;

struct vertex {
  vector<int> s;
  string weight;
  int priority;
  vertex() {
    s.resize(4);
  }
};

struct result_element {
  int vkind;
  int leg;
  int priority;
  string weight;
};  

bool compare(const result_element e1, const result_element e2) {
  return e1.priority < e2.priority;
}

int main() {
  vector<vertex> v(7);
  v[0].weight = "0.0";
  v[1].s[0] = 0; v[1].s[1] = 1; 
  v[1].s[2] = 0; v[1].s[3] = 1;
  v[1].weight = "p.C+0.5*(p.Jz+p.h)"; v[1].priority = 2; 
  v[2].s[0] = 1; v[2].s[1] = 0; 
  v[2].s[2] = 1; v[2].s[3] = 0; 
  v[2].weight = "p.C+0.5*(p.Jz+p.h)"; v[2].priority = 2;
  v[3].s[0] = 0; v[3].s[1] = 1; 
  v[3].s[2] = 1; v[3].s[3] = 0;
  v[3].weight = "0.5*p.Jxy"; v[3].priority = 1;
  v[4].s[0] = 1; v[4].s[1] = 0; 
  v[4].s[2] = 0; v[4].s[3] = 1;
  v[4].weight = "0.5*p.Jxy"; v[4].priority = 1;
  v[5].s[0] = 1; v[5].s[1] = 1; 
  v[5].s[2] = 1; v[5].s[3] = 1;
  v[5].weight = "p.C+p.h"; v[5].priority = 3;
  v[6].s[0] = 0; v[6].s[1] = 0; 
  v[6].s[2] = 0; v[6].s[3] = 0;
  v[6].weight = "p.C"; v[6].priority = 4;

  /*
  const string header =
    "#include<vector>\n"
    "using namespace std;\n"
    "// probability table generator for the XXZ model under external field\n"
    "// 0 ... Identity Operator\n"
    "// 1 ... : |   2... | :   3... : |  4... | :  5... | |  6... : :\n"
    "//       ---        ---        ---       ---       ---       ---\n"
    "//       : |        | :        | :       : |       | |       : :\n"
    "//   (':'= 0, '|'= 1  <-- charge state)\n"
    "//  The weight of 6 is set as zero, and is not used.\n\n"
    "// leg asignment\n"
    "//     0 | | 1\n"
    "//       ---  \n"
    "//     2 | | 3\n"    
    "inline void make_prob_table (vector<vector<int> > &vkind_after_flipped1,\n"
    "                             vector<vector<int> > &vkind_after_flipped2,\n"
    "                             vector<vector<int> > &vkind_after_flipped3,\n"
    "                             vector<vector<double> > &prob1,\n"
    "                             vector<vector<double> > &prob2,\n"
    "                             parameters p) {\n"
    "  h = p.h\n"
    "  delta = p.Delta\n";
  cout << header;
  */

  ofstream ofs;
  ofs.open("prob_table_XXZ.cpp");  
  ofs << "  weight[0] = 0.0;" << endl
      << "  weight[1] = " << v[1].weight << ";" << endl
      << "  weight[2] = " << v[2].weight << ";" << endl
      << "  weight[3] = 0.0;" << endl
      << "  weight[4] = 0.0;" << endl
      << "  weight[5] = " << v[5].weight << ";" << endl
      << "  weight[6] = " << v[6].weight << ";" << endl;

  for (int i=1; i<=6; i++) {
    for (int ileg=0; ileg<4; ileg++) {
      vector<result_element> result;
      int count_nonzero_weight = 0;
      for (int oleg=0; oleg<4; oleg++) {
	vertex vtmp(v[i]);
	vtmp.s[ileg] = 1 - vtmp.s[ileg];
	vtmp.s[oleg] = 1 - vtmp.s[oleg];
	int vkind_after_flip = 0;
	int vkind = 1;
	while (vkind<7) {
	  if ((vtmp.s[0]==v[vkind].s[0]) && (vtmp.s[1]==v[vkind].s[1]) 
	      && (vtmp.s[2]==v[vkind].s[2]) && (vtmp.s[3]==v[vkind].s[3])) {
	    vkind_after_flip = vkind;
	    break;
	  }
	  vkind++;
	}
	ofs << "  // vkind: " << i << " inleg: " << ileg << " outleg: " << oleg << " vkind_after_flipp: " << vkind_after_flip << endl;
	ofs << "  // " << v[i].s[0] << ' ' << v[i].s[1] << " -> " << vtmp.s[0] << ' ' << vtmp.s[1] << endl;
	ofs << "  // " << v[i].s[2] << ' ' << v[i].s[3] << " -> " << vtmp.s[2] << ' ' << vtmp.s[3] 
	     << "    weight: " << v[vkind_after_flip].weight << endl;
	if (vkind_after_flip != 0) {
	  result_element rtmp;
	  rtmp.vkind = vkind_after_flip;
	  rtmp.weight = v[vkind_after_flip].weight;
	  rtmp.priority = v[vkind_after_flip].priority;
	  rtmp.leg = oleg;
	  result.push_back(rtmp);
	}
	if (v[vkind_after_flip].weight != "0.0") {
	  count_nonzero_weight++;
	}
      }
      if (result.size() != 3) { 
	cout << "ERROR." << endl;
	exit(0);
      }
      sort(result.begin(),result.end(),compare);

      string ssum;
      for (int j=0; j<3; j++) {
	ofs << "  vkind_after_flipped" << j+1 << "[" << i << "][" << ileg << "] = " << result[j].vkind << ";" << endl;
	ofs << "  leg_after_flipped" << j+1 << "[" << i << "][" << ileg << "] = " << result[j].leg << ";" << endl;
	ssum = ssum + result[j].weight + "+";
      }
      ssum.erase(ssum.size()-1);
      ofs << "  prob1[" << i << "][" << ileg << "] = (" << result[0].weight << ")/(" << ssum << ");" << endl;
      if (count_nonzero_weight == 3) {
	ofs << "  prob2[" << i << "][" << ileg << "] = prob1[" << i << "][" << ileg << "] + ("<< result[1].weight << ")/(" << ssum << ");" << endl;
      } else {
	ofs << "  prob2[" << i << "][" << ileg << "] = 1.0;" << endl;
      }
    }
  }
}


