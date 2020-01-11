#include <bits/stdc++.h>
using namespace std;

typedef long double ld;

struct pt
{
	int i;
	ld x, y;
	pt() : i(-1), x(0), y(0) {}
	pt(ld x, ld y) : i(-1), x(x), y(y) {}
	pt subtract(const pt &o) const
	{
		return pt(x-o.x,y-o.y);
	}
	pt operator-(const pt &o) const
	{
		return subtract(o);
	}
	ld normsqr() const
	{
		return x*x+y*y;
	}
	ld distsqr(const pt &o) const
	{
		return subtract(o).normsqr();
	}
	pt normalize() const
	{
		ld ds = sqrt(normsqr());
		assert(ds!=0);
		return pt(x/ds,y/ds);
	}
	ld angle(const pt &o) const
	{
		pt n = subtract(o).normalize();
		return atan2(n.y,n.x);
	}
	ld angle() const
	{
		return angle(pt(0,0));
	}
	ld cross(const pt &o) const
	{
		return x*o.y-o.x*y;
	}
};

struct linseg
{
	pt a, b;
	linseg(pt a, pt b) : a(a), b(b) {}
	linseg(ld x1, ld y1, ld x2, ld y2) : a(pt(x1,y1)), b(pt(x2,y2)) {}
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
	ld angle() const
	{
		return vec().angle();
	}
	bool operator<(const edge &other) const
	{
		return angle() < other.angle();
	}
	bool to_the_left(const edge &other) const
	{
		ld a1 = angle(), a2 = other.angle();
		ld diff = a1-a2;
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
	int n; cin >> n;
	vector<pt> points(n);
	for (int i = 0; i < n; ++i)
	{
		int k;
		ld x, y; cin >> k >> x >> y;
		points[i] = pt(x,y);
	}
	vector<edge> edges;
	vector<vector<edge>> adj(n);
	int m = 1; //sat vars start at 1
	for (int i = 0; i < n; ++i)
	{
		for (int j = i+1; j < n; ++j)
		{
			edges.push_back(edge(points[i],points[j],m));
			adj[i].push_back(edge(points[i],points[j],m));
			adj[j].push_back(edge(points[j],points[i],m++));
		}
		sort(adj[i].begin(),adj[i].end());
	}
	vector<vector<int>> nands, ors;
	//temporary, to be replaced with bentley-ottmann
	for (int i = 0; i < m; ++i)
		for (int j = i+1; j < m; ++j)
			if (edges[i].ln.isect(edges[j].ln))
				nands.push_back({edges[i].e_index,edges[j].e_index});

	for (int v = 0; v < n; ++v)
	{
		int k = adj[v].size();
		for (int i = 0; i < k; ++i)
		{
			vector<int> or_cur = {adj[v][i].e_index};
			//for (int j = i+1; adj[v][i].vec().cross(adj[v][j].vec()) > 0; j=(j+1)%k)
			//for (int j = i+1; adj[v][i].to_the_left(adj[v][j]) > 0 && j!=i; j=(j+1)%k)
			for (int j = i+1; j != i; j=(j+1)%k)
				if (adj[v][i].vec().cross(adj[v][j].vec()) < 0)
					or_cur.push_back(adj[v][j].e_index);
			ors.push_back(or_cur);
		}
	}

	print_cnf(m,ors,nands);
}
