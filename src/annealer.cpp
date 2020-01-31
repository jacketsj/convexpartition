#include <bits/stdc++.h>
using namespace std;
#include "Annealer.h"
#include "Graph.h"

void localsearch(graph &g) {
	cerr << "running local search!" << endl;
  int m = g.get_edge_num();
	annealer ann(g);
	// random local search
	while (ann.g.removable_edges.size() > 0)
	{
		auto edge = ann.sample_removable_edges();
		g.remove_edge(edge.first,edge.second,1);
	}
	if (ann.g.get_edge_num() < m)
		cerr << "improved result with greedy: " << m << " to " << g.get_edge_num() << endl;
	else
		cerr << "no improvements with greedy" << endl;
}

int main() {
  ifstream in("../base_names.txt");
  //string filename = "mona-lisa-1000000";
  string filename = "euro-night-0060000";
//  while(in >> filename) {
//    if (filename < "uniform-0090000-2") continue;
    graph g;
    g.read("../temp/"+filename+".out");
    //g.read("../triangulations/"+filename+".tri");
//    g.print_matlab();
    int m0 = g.get_edge_num();
		localsearch(g);
    int m = g.get_edge_num();
    annealer ann(g);
		ann.MAXIT=1000000;
		ann.MAXT=10000;
		//ann.MAXIT+=10000;
		//ann.MAXT+=40000;
    ann.anneal();
    cerr << filename << " started annealing with " << m << " edges and finished with " << g.get_edge_num() <<endl;
		localsearch(g);
		cerr << filename << " started greedy-anneal-greedy with " << m0 << " edges" << endl;
    g.write("../temp/"+filename+".out");
//    g.print_matlab();
  //}
}
