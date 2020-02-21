/*
	This code generates an O(n^4) size SAT formulation for n points.
	It works (and is very fast) for all instances of size <=50.
*/
#include <bits/stdc++.h>
#include "../src/Graph.h"
#include "../src/Point.h"
#include "../src/extern/delaunator.hpp"
using namespace std;

#define int long long

typedef int num;
typedef long double fl;

// some code from the UBC Sports Programming Club code archive
namespace ubc {
typedef long double ld;
#define EPS 1e-7
template<class T> struct cplx {
  T x, y; cplx() {x = 0.0; y = 0.0;}
  cplx(T nx, T ny=0) {x = nx; y = ny;}
  cplx operator+(const cplx &c) const {return {x + c.x, y + c.y};}
  cplx operator-(const cplx &c) const {return {x - c.x, y - c.y};}
  cplx operator*(const cplx &c) const {return {x*c.x - y*c.y, x*c.y + y*c.x};}
  cplx& operator*=(const cplx &c) { return *this={x*c.x-y*c.y, x*c.y+y*c.x}; }
  // Only supports right scalar multiplication like p*c
  template<class U> cplx operator*(const U &c) const {return {x*c,y*c};}
  template<class U> cplx operator/(const U &c) const {return {x/c,y/c};} };
#define polar(r,a)  (cpt){r*cos(a),r*sin(a)}
typedef cplx<ld> cpt;
typedef vector<cpt> pol;
cpt operator*(ld c, const cpt p) { return {p.x*c,p.y*c};} // for left mult. c*p
// useful for debugging
ostream&operator<<(ostream &o,const cpt &p){o<<"("<<p.x<<","<<p.y<<")";return o;}
ld cp(const cpt& a, const cpt& b) { return a.x*b.y - b.x*a.y; }
ld dp(const cpt& a, const cpt& b) { return a.x*b.x + a.y*b.y; }
inline ld abs(const cpt &a) {return sqrt(a.x*a.x + a.y*a.y);}
inline ld arg(const cpt &a) {return atan2(a.y,a.x);}
ld ang(cpt &a, cpt &b, cpt &c) { return atan2(cp(a-b,b-c),dp(a-b,b-c)); }
namespace std{
template<class T>inline bool operator<(const cplx<T>& a,const cplx<T>& b){
  return a.x<b.x || (a.x == b.x && a.y<b.y); } };
inline bool cmp_lex(const cpt& a, const cpt& b) {
  return a.x<b.x-EPS||(a.x<b.x+EPS&&a.y<b.y-EPS);}
inline bool cmp_lex_i(const cpt& a, const cpt& b) {
  return a.y<b.y-EPS||(a.y<b.y+EPS&&a.x<b.x-EPS);}
cpt convert(const pt &a)
{
	return cpt(a.x, a.y);
}
cpt conj(const cpt &a)
{
	return cpt(a.x,-a.y);
}
// dist(const cpt& a, const cpt& b) ==> abs(a-b)
inline bool eq(const cpt &a, const cpt &b) { return abs(a-b) < EPS; }
inline ld sgn(const ld& x) { return abs(x) < EPS ? 0 : x/abs(x); }
bool seg_x_seg(cpt a1, cpt a2, cpt b1, cpt b2) {
	if (eq(a1,a2) || eq(b1,b2))
		return false; // uncomment to exclude endpoints
	ld za = abs(a2-a1), zb = abs(b2-b1);
	za=za>EPS?1/za:0; zb=zb>EPS?1/zb:0;
	int s1 = sgn(cp(a2-a1, b1-a1)*za), s2 = sgn(cp(a2-a1, b2-a1)*za);
	int s3 = sgn(cp(b2-b1, a1-b1)*zb), s4 = sgn(cp(b2-b1, a2-b1)*zb);
	if(!s1 && !s2 && !s3)
	{ // collinear
	  if (cmp_lex(a2, a1))
			swap(a1, a2);
		if (cmp_lex(b2, b1))
			swap(b1, b2);
	  return cmp_lex(a1, b2) && cmp_lex(b1, a2);//uncomment to exclude endpoints
	  //return !cmp_lex(b2, a1) && !cmp_lex(a2, b1);
	}
	return s1*s2 < 0 && s3*s4 < 0;
	//return s1*s2 <= 0 && s3*s4 <= 0;
} //change to < to exclude endpoints
bool pt_x_seg(const cpt& p, const cpt& a, const cpt& b)
{
	return abs(cp(b-a,p-a))<EPS && dp(p-a,b-a) > 0 && dp(p-b,a-b) > 0;
}
}

struct linseg
{
	pt a, b;
	linseg(pt a, pt b) : a(a), b(b) {}
	linseg(num x1, num y1, num x2, num y2) : a(pt(x1,y1)), b(pt(x2,y2)) {}
	bool isect(const linseg &o) const
	{
		// exclude intersections at direct endpoints, this is covered by another case (not true in general)
		if (a == o.a || a == o.b || b == o.a || b == o.b) {
			return false;
		}
		auto colinear = [](const pt &a, const pt &b, const pt &c) {
			return cp(b-a,c-a) == 0;
		};
		// if colinear
		if (colinear(a,b,o.a) && colinear(a,b,o.b)) {
			//return false;
			// check if a.x is inside (o.a.x,o.b.x)
			if (o.a.x < a.x && a.x < o.b.x)
				return true;
			// check if a.y is inside (o.a.y,o.b.y)
			if (o.a.y < a.y && a.y < o.b.y)
				return true;
			// check if b.x is inside (o.a.x,o.b.x)
			if (o.a.x < b.x && b.x < o.b.x)
				return true;
			// check if b.y is inside (o.a.y,o.b.y)
			if (o.a.y < b.y && b.y < o.b.y)
				return true;
			// check if o.b.y is inside (a.y,b.y)
			if (a.y < o.b.y && o.b.y < b.y)
				return true;
			// check if o.b.x is inside (a.x,b.x)
			if (a.x < o.b.x && o.b.x < b.x)
				return true;
			return false;
		}
		else if (colinear(a,b,o.a) || colinear(a,b,o.b))
			return false;

		auto c = o.a, d = o.b;
		// use ubc code archive to solve edge cases
		return ubc::seg_x_seg(ubc::convert(a),ubc::convert(b),ubc::convert(o.a),ubc::convert(o.b));
	}
	bool isect(const pt &p) const
	{
		return cp(b-a,p-a) == 0 && dot(p-a,b-a) > 0 && dot(p-b,a-b) > 0;
	}
};

struct edge
{
	linseg ln;
	int e_index;
	edge(pt a, pt b, int e) : ln(a,b), e_index(e) {}
	pt vec() const
	{
		//return ln.a-ln.b;
		return ln.b-ln.a;
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
		fl a1 = angle(), a2 = other.angle();
		fl diff = a1-a2;
		while(diff < 0)
			diff += 2*M_PI;
		return diff <= M_PI;
	}
	void print() const
	{
		cerr << "edge: e_index=" << e_index
			<< ", a=(" << ln.a.x << ',' << ln.a.y << "),"
			<< ", b=(" << ln.b.x << ',' << ln.b.y << ")," << '\n';
	}
};

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
