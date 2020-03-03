/*
	This code generates an O(n^4) size SAT formulation for n points.
	It works (and is very fast) for all instances of size <=50.
	
	The output should be solved with a MaxSAT solver, such as UWrMaxSAT.

	Running this code will require a cnf/ folder present in the local directory.
*/
#include <bits/stdc++.h>
#include "../Graph.h"
#include "../Point.h"
#include "../extern/delaunator.hpp"
#include "segments.h"
using namespace std;

#define int long long

typedef int num;
typedef long double fl;

void print_cnf(int m, vector<vector<int>> ors, vector<vector<int>> nands,
		vector<int> nots)
{
	cout << "p cnf " << m << ' ' << ors.size()+nands.size() << '\n';
	for (auto &vi : ors)
	{
		for (auto i : vi)
			cout << i << ' ';
		cout << "0\n";
	}
	for (auto &vi : nands)
	{
		for (auto i : vi)
			cout << '-' << i << ' ';
		cout << "0\n";
	}
}

void print_wcnf(int m, vector<vector<int>> ors, vector<vector<int>> nands,
		vector<int> nots)
{
	int required = m + 1;
	cout << "p wcnf " << m << ' ' << ors.size()+nands.size() << ' ' << required << '\n';
	for (auto &vi : ors)
	{
		cout << required << ' ';
		for (auto i : vi)
			cout << i << ' ';
		cout << "0\n";
	}
	for (auto &vi : nands)
	{
		cout << required << ' ';
		for (auto i : vi)
			cout << '-' << i << ' ';
		cout << "0\n";
	}
	/* unnecessary
	for (auto i : nots)
	{
		cout << required << ' ';
		//cout << i << ' ';
		cout << '-' << i << ' ';
		cout << "0\n";
	}
	*/
	for (int i = 1; i <= m; ++i)
	{
		cout << "1 " << -i << " 0\n";
	}
}

string read_problem_file()
{
	string s; cin >> s;
	return s;
}

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

#undef int
int main()
{
#define int long long
	// start by reading in problem file name
	string s = read_problem_file();

	cout << "reading problem file: " << s << endl;
  graph g;
  g.read("../in/"+s+".in");
  delaunate(g);
	assert(freopen(("../in/"+s+".in").c_str(),"r",stdin) != NULL);
	int n; cin >> n;
	vector<pt> points(n);
	for (int i = 0; i < n; ++i)
	{
		int k;
		num x, y; cin >> k >> x >> y;
		points[i] = pt(i,x,y);
	}
	vector<edge> edges;
	vector<vector<edge>> adj(n);
	int m = 0; //sat vars start at 1
	for (int i = 0; i < n; ++i)
	{
		for (int j = i+1; j < n; ++j)
		{
			edges.push_back(edge(points[i],points[j],++m));
			adj[i].push_back(edge(points[i],points[j],m));
			adj[j].push_back(edge(points[j],points[i],m));
		}
		sort(adj[i].begin(),adj[i].end());
	}

	auto has_edge = [&](int i){
		return g.has_edge(edges[i].ln.a.i,edges[i].ln.b.i);
	};

	vector<vector<int>> nands, ors;
	vector<int> nots;
	// line intersection
	for (int i = 0; i < m; ++i)
	{
		for (int j = i+1; j < m; ++j)
		{
			if (edges[i].ln.isect(edges[j].ln))
			{
				nands.push_back({edges[i].e_index,edges[j].e_index});
			}
		}
		for (int j = 0; j < n; ++j)
			if (edges[i].ln.isect(points[j]))
			{
				nots.push_back(edges[i].e_index);
				break;
			}
	}

	for (int v = 0; v < n; ++v)
	{
		int k = adj[v].size();
		for (int i = 0; i < k; ++i)
		{
			vector<int> or_cur, or_cur_rev;
			for (int j = 0; j < k; ++j)
			{
				auto res = cp(adj[v][i].vec(),(adj[v][j].vec()));
				if (j != i)
				{
					if (res <= 0)
						or_cur.push_back(adj[v][j].e_index);
					if (res >= 0)
						or_cur_rev.push_back(adj[v][j].e_index);
				}
			}
			if (!or_cur.empty())
				ors.push_back(or_cur);
			if (!or_cur_rev.empty())
				ors.push_back(or_cur_rev);
		}
	}

	cout << "reading file" << endl;
	assert(freopen(("cnf/"+s+".cnf").c_str(),"w",stdout) != NULL);

	//print_cnf(m,ors,nands,nots);
	print_wcnf(m,ors,nands,nots);
}
