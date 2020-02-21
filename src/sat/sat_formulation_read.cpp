/*
	This code corresponds to sat_formulation.cpp,
	allowing one to read the output of the SAT solver.
	Running this code will require out/ and sat/ folders present in the local directory.
*/
#include <bits/stdc++.h>
#include "../Point.h"
using namespace std;

#define int long long

typedef int num;
typedef long double fl;

struct linseg
{
	pt a, b;
	linseg(pt a, pt b) : a(a), b(b) {}
	linseg(num x1, num y1, num x2, num y2) : a(pt(x1,y1)), b(pt(x2,y2)) {}
};

struct edge
{
	linseg ln;
	int e_index;
	edge(pt a, pt b, int e) : ln(a,b), e_index(e) {}
	pt vec() const
	{
		return ln.b-ln.a;
	}
	void print() const
	{
		cerr << "edge: e_index=" << e_index
			<< ", a=(" << ln.a.x << ',' << ln.a.y << "),"
			<< ", b=(" << ln.b.x << ',' << ln.b.y << ")," << '\n';
	}
};

string read_problem_file()
{
	string s; cin >> s;
	return s;
}

#undef int
int main()
{
#define int long long
	// start by reading in problem file name
	string s = read_problem_file();

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

	// now read in out file from sat solver
	assert(freopen(("sat/"+s+".sat").c_str(),"r",stdin) != NULL);

	string sat;
	//do
	//{
	//	//cin >> sat;
	//}
	while (sat != "v" && cin >> sat) {};
	assert(sat=="v");

	// find which edges are included in the solution
	vector<edge> incl_edges;
	vector<vector<int>> incl_adj(n);
	for (int i = 0; i < m; ++i)
	{
		int f; cin >> f;
		//f *= -1;
		if (f > 0)
		{
			edge &e = edges[f-1];
			assert(e.e_index==f); // verify indexing
			incl_edges.push_back(e);
			int v = e.ln.a.i, u = e.ln.b.i;
			incl_adj[v].push_back(u);
			incl_adj[u].push_back(v);
		}
	}

	// now output instance format of plane graph
	assert(freopen(("out/"+s+".out").c_str(),"w",stdout) != NULL);
	cout << n << '\n';
	for (int i = 0; i < n; ++i)
		cout << i << ' ' << points[i].x << ' ' << points[i].y << '\n';
	for (int i = 0; i < n; ++i)
	{
		cout << incl_adj[i].size();
		for (int j : incl_adj[i])
			cout << ' ' << j;
		cout << '\n';
	}
}
