#include <bits/stdc++.h>
using namespace std;
#include "Graph.h"
#include "extern/delaunator.hpp"

int m;

void delaunate(graph& g) {
  vector<double> dinput;
  for(pt &p: g.points) {
    dinput.push_back(p.x);
    dinput.push_back(p.y);
  }

  // all the work is done in constructor for some reason
  delaunator::Delaunator d(dinput);
  m = d.triangles.size()/3;
  for(int i=0;i<m;i++) {
    g.add_edge(d.triangles[3*i+0], d.triangles[3*i+1]);
    g.add_edge(d.triangles[3*i+1], d.triangles[3*i+2]);
    g.add_edge(d.triangles[3*i+2], d.triangles[3*i+0]);
  }
}

int main() {
  ifstream base_names_file("../base_names.txt");
  string filename;
  while(base_names_file >> filename) {
    graph g;
    g.read("../in/"+filename+".in");
    delaunate(g);
    g.write("../triangulations/"+filename+".tri");
    cerr << filename << " was triangulated and had " << m << " faces " <<endl;
  }
}
