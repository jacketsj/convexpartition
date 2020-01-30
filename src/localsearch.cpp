#include <bits/stdc++.h>
using namespace std;
#include "Annealer.h"
#include "Graph.h"

int main() {
  //ifstream in("../base_names.txt");
	//while(in >> filename) {
	//if (filename < "uniform-0090000-2") continue;
  //}

  string filename = "rop0011619";
  graph g0;
  g0.read("../triangulations/"+filename+".tri");
	int m_best = g0.get_edge_num();
	graph g_best(g0);
	for (int i = 0; i < 100; ++i)
	{
		graph g(g0);
  	int m = g.get_edge_num();
  	annealer ann(g);
		// random local search
		while (ann.g.removable_edges.size() > 0)
		{
			auto edge = ann.sample_removable_edges();
			g.remove_edge(edge.first,edge.second,1);
			--m;
		}
		if (m < m_best)
		{
			m_best = m;
			g_best = graph(g);
			cerr << "improved best result (iteration=" << i << "): m_best=" << m << endl;
		}
	}
  g_best.write("../temp/"+filename+".out");
}
