#include <bits/stdc++.h>
using namespace std;
#include "Annealer.h"
#include "Graph.h"

int main() {
  graph g;
  string filename = "stars-0000020";
  g.read("../triangulations/"+filename+".tri");
  int m = g.inner_edges.size();
  annealer ann(g);
  ann.anneal();
  cerr << filename << " started with " << m << " inner edges and finished with " << g.inner_edges.size() <<endl;
  g.write("../out/"+filename+".out");
}
