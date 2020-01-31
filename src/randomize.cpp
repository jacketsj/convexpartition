#include <bits/stdc++.h>
using namespace std;
#include "Annealer.h"
#include "Graph.h"

void random_flip(graph &g, annealer &ann)
{
	vector<pair<int,int>> edges;
	for (pair<int,int> edge : g.good_edges)
		if (ann.rng()%8)
			edges.push_back(edge);
	int success = 0;
	for (auto &e : edges)
	{
		int a = e.first, b = e.second;
		int most = ann.rng()%50+1;
		if (ann.rng()%2)
			while (g.rot_cw(a,b,1) && most > 0)
				success++, --most;
		else
			while (g.rot_ccw(a,b,1) && most > 0)
				success++, --most;
	}
	//cerr << "randomly repeat-rotated " << success << " times" << endl;
}

int main() {
  //ifstream in("../base_names.txt");
	//while(in >> filename) {
	//if (filename < "uniform-0090000-2") continue;
  //}

  //string filename = "mona-lisa-1000000";
  string filename = "euro-night-0060000";
  graph g;
  g.read("../temp/"+filename+".out");
	int m_best = g.get_edge_num();
  annealer ann(g);
	for (int i = 0; i < 10; ++i)
	{
		random_flip(g,ann);
  	int m = g.get_edge_num();
		// random local search
		while (ann.g.removable_edges.size() > 0)
		{
			auto edge = ann.sample_removable_edges();
			g.remove_edge(edge.first,edge.second,1);
			--m;
		}
		if (m <= m_best)
		{
			if (m < m_best)
				cerr << "improved best result (iteration=" << i << "): m_best=" << m << endl;
			m_best = m;
		}
	}
  g.write("../temp/"+filename+".out");
}
