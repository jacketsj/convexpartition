#include <bits/stdc++.h>
using namespace std;
#include "Annealer.h"
#include "Graph.h"

int main() {
  ifstream in("../base_names.txt");
  graph g;
  string filename = "london-0000010";
  while(in >> filename) {
    g.read("../triangulations/"+filename+".tri");
//    g.print_matlab();
    int m = g.get_edge_num();
    annealer ann(g);
    ann.anneal();
    cerr << filename << " started with " << m << " inner edges and finished with " << g.get_edge_num() <<endl;
    g.write("../min_from_triangulation/"+filename+".out");
//    g.print_matlab();
  }
}
