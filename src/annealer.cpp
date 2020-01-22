#include <bits/stdc++.h>
using namespace std;
#include "Annealer.h"
#include "Graph.h"

void run(const string& filename) {
  graph g;
  g.read("../triangulations/"+filename+".tri");
  int m = g.get_edge_num();
  annealer ann(g);
  ann.anneal();
  cerr << filename << " started with " << m << " edges and finished with " << g.get_edge_num() <<endl;
  g.write("../min_from_triangulation/"+filename+".out");
}

int main() {
  ifstream in("../base_names.txt");
  string filename = "euro-night-0010000";
  vector<thread> work;
  while(in >> filename) {
    work.emplace_back(run, filename);
  }
  for (auto& t : work) {
    t.join();
  }
}
