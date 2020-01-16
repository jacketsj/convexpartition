#include <bits/stdc++.h>
using namespace std;
#include "Annealer.h"
#include "Graph.h"

int main() {
  graph g;
  string filename = "euro-night-0000100";
  g.read("../triangulations/"+filename+".tri");
  int m = g.get_edge_num();
  annealer ann(g);
  ann.anneal();
  cerr << filename << " started with " << m << " inner edges and finished with " << g.get_edge_num() <<endl;
  g.write("../out/"+filename+".out");
  //g.print_matlab();
}
