#include <bits/stdc++.h>
using namespace std;
#include "Graph.h"
#include "extern/delaunator.hpp"

void delaunate(graph& g) {
  vector<double> dinput;
  for(pt &p: g.points) {
    dinput.push_back(p.x);
    dinput.push_back(p.y);
  }

  // all the work is done in constructor for some reason
  delaunator::Delaunator d(dinput);
  int m = d.triangles.size()/3;
  for(int i=0;i<m;i++) {
    g.add_edge(d.triangles[3*i+0], d.triangles[3*i+1]);
    g.add_edge(d.triangles[3*i+1], d.triangles[3*i+2]);
    g.add_edge(d.triangles[3*i+2], d.triangles[3*i+0]);
  }
}
int main() {
  // temporary for testing 
  // TODO: actually test this piece of garbage once data conversion is done.
  string filename = "../challenge_instances/data/images/stars-0000100.instance.json";
  //graph g;
  //g.read(filename);
  //delaunate(g);
}
