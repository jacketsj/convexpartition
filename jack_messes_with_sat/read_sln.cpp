#include <bits/stdc++.h>
using namespace std;

typedef int num;
typedef long double fl;

struct pt
{
	int i;
	num x, y;
	pt() : i(-1), x(0), y(0) {}
	pt(num x, num y) : i(-1), x(x), y(y) {}
	pt(int i, num x, num y) : i(i), x(x), y(y) {}
	pt subtract(const pt &o) const
	{
		return pt(x-o.x,y-o.y);
	}
	pt operator-(const pt &o) const
	{
		return subtract(o);
	}
	num normsqr() const
	{
		return x*x+y*y;
	}
	num distsqr(const pt &o) const
	{
		return subtract(o).normsqr();
	}
	//pt normalize() const
	//{
	//	ld ds = sqrt(normsqr());
	//	assert(ds!=0);
	//	return pt(x/ds,y/ds);
	//}
	fl angle(const pt &o) const
	{
		//pt n = subtract(o).normalize();
		pt n = subtract(o);
		return atan2(n.y,n.x);
	}
	fl angle() const
	{
		return angle(pt(0,0));
	}
	num cross(const pt &o) const
	{
		return x*o.y-o.x*y;
	}
};

struct linseg
{
	pt a, b;
	linseg(pt a, pt b) : a(a), b(b) {}
	linseg(num x1, num y1, num x2, num y2) : a(pt(x1,y1)), b(pt(x2,y2)) {}
	bool isectray(const linseg &o) const
	{
		return ((b.x-a.x)*(o.a.y-b.y)-(b.y-a.y)*(o.a.x-b.x))
			* ((b.x-a.x)*(o.b.y-b.y)-(b.y-a.y)*(o.b.x-b.x))
			< 0;
	}
	bool isect(const linseg &o) const
	{
		return isectray(o) && o.isectray(*this);
	}
};

struct edge
{
	linseg ln;
	int e_index;
	edge(pt a, pt b, int e) : ln(a,b), e_index(e) {}
	pt vec() const
	{
		return ln.a-ln.b;
	}
	fl angle() const
	{
		return vec().angle();
	}
	bool operator<(const edge &other) const
	{
		return angle() < other.angle();
	}
	bool to_the_left(const edge &other) const
	{
		fl a1 = angle(), a2 = other.angle();
		fl diff = a1-a2;
		while(diff < 0)
			diff += 2*M_PI;
		return diff <= M_PI;
	}
};

void print_cnf(int m, vector<vector<int>> ors, vector<vector<int>> nands)
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

int main()
{
	// start by reading in problem file
	freopen("uniform.0000100.2.pts","r",stdin);
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
	freopen("uniform.0000100.2.v6.out","r",stdin);

	string sat; cin >> sat;
	assert(sat=="SAT");

	// find which edges are included in the solution
	vector<edge> incl_edges;
	vector<vector<int>> incl_adj(n);
	for (int i = 0; i < m; ++i)
	{
		int f; cin >> f;
		if (f > 0)
		{
			edge &e = edges[f-1];
			incl_edges.push_back(e);
			int v = e.ln.a.i, u = e.ln.b.i;
			incl_adj[v].push_back(u);
			incl_adj[u].push_back(v);
		}
	}

	// now output instance format of plane graph
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
