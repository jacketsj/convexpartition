#include <bits/stdc++.h>
using namespace std;
#include "Annealer.h"
#include "Graph.h"

int main() {
  ifstream in("../base_names.txt");
  string filename = "euro-night-0010000";
  while(in >> filename) {
    if (filename < "uniform-0090000-2") continue;
    graph g;
    g.read("../triangulations/"+filename+".tri");
//    g.print_matlab();
    int m = g.get_edge_num();
    annealer ann(g);
    ann.anneal();
    cerr << filename << " started with " << m << " edges and finished with " << g.get_edge_num() <<endl;
    g.write("../min_from_triangulation/"+filename+".out");
//    g.print_matlab();
  }
}
