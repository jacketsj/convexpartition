#include <bits/stdc++.h>
using namespace std;

#define int long long

typedef int num;
typedef long double fl;

struct pt
{
	int i;
	num x, y;
	pt() : i(-1), x(0), y(0) {}
	pt(num x, num y) : i(-1), x(x), y(y) {}
	pt(int i, num x, num y) : i(i), x(x), y(y) {}
	pt add(const pt &o) const
	{
		return pt(x+o.x,y+o.y);
	}
	pt operator+(const pt &o) const
	{
		return add(o);
	}
	pt subtract(const pt &o) const
	{
		return pt(x-o.x,y-o.y);
	}
	pt operator-(const pt &o) const
	{
		return subtract(o);
	}
	pt operator/(num d) const
	{
		assert(d!=0);
		return pt(x/d,y/d);
	}
	num normsqr() const
	{
		return x*x+y*y;
	}
	num distsqr(const pt &o) const
	{
		return subtract(o).normsqr();
	}
	fl angle(const pt &o) const
	{
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
	num dot(const pt &o) const
	{
		return x*o.x+y*o.y;
	}
};

string read_problem_file()
{
	string s; cin >> s;
	return s;
}

void add_rand_pt(int n, vector<pt> &points, int k, vector<vector<int>> &pt_cl, vector<vector<int>> &or_cl, int &m)
{
	if (n == 1) // unnecessary
		return;

	int i = rand()%n;
	int j = rand()%n;
	while (j == i)
		j = rand()%n;
	pt interp = (points[i]+points[j])/2;
	vector<int> cur;
	for (int i = 0; i < k; ++i)
	{
		++m;
		pt_cl.push_back({i,m,2,interp.x,interp.y});
		cur.push_back(m);
	}
	or_cl.push_back(cur);
}

void encode_ch(int n, vector<pt> points, int k)
{
	// idea: add k convex hulls around our existing point set
	// to make sure they partition: add O(n) random interpolated points
	// every interpolated point must be inside some point set
	// (later: put these points around our existing set)

	int m = 0; // note: sat vars start at 1
	//vector<vector<int>> psi(n,vector<int>(k)); // point/set index
	vector<int> units; // unit clauses
	vector<vector<int>> or_cl(n); // or clauses
	vector<vector<int>> pt_cl; // point clauses
	vector<vector<int>> ch_coll; // ch's collide
	vector<vector<int>> p_hu; // point on ch
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < k; ++j)
		{
			//psi[i][j] = ++m;
			++m;

			or_cl[i].push_back(m);
			pt_cl.push_back({k,m,2,points[i].x,points[i].y});

			//p_hu.ush_back({k,m,m}); // might be able to just do this
			p_hu.push_back({k,m,++m});
			or_cl.push_back({-m,m-1}); // point on hull iff point in set
			or_cl.push_back({m,-(m-1)});
		}
	// make sure convex hulls do not collide
	for (int i = 0; i < k; ++i)
		for (int j = i+1; j < k; ++j)
		{
			ch_coll.push_back({i,j,++m});
			units.push_back(-m);
		}
	// add random interior points
	for (int i = 0; i < 3*n; ++i)
		add_rand_pt(n, points, k, pt_cl, or_cl, m);

	// print
	int num_clauses = units.size() + or_cl.size();
	cout << "p cnf " << m << " " << num_clauses << '\n';
	for (int u : units)
		cout << u << " 0\n";
	for (auto &u : or_cl)
	{
		for (auto i : u)
			cout << i << ' ';
		cout << "0\n";
	}
	for (auto &u : pt_cl)
	{
		cout << "point";
		for (auto i : u)
			cout << ' ' << i;
		cout << '\n';
	}
	for (auto &u : ch_coll)
	{
		cout << "convex_hulls_collide";
		for (auto i : u)
			cout << ' ' << i;
		cout << '\n';
	}
	for (auto &u : p_hu)
	{
		cout << "point_on_convex_hull";
		for (auto i : u)
			cout << ' ' << i;
		cout << '\n';
	}
}

#undef int
int main()
{
	ios::sync_with_stdio(0);

#define int long long
	// start by reading in problem file name
	string s = read_problem_file();

	cout << "reading problem file: " << s << endl;
	assert(freopen(("../in/"+s+".in").c_str(),"r",stdin) != NULL);
	int n; cin >> n;
	vector<pt> points(n);
	for (int i = 0; i < n; ++i)
	{
		int k;
		num x, y; cin >> k >> x >> y;
		points[i] = pt(i,x,y);
	}

	int k = n-2; // number of convex hulls to try for

	cout << "writing to gnf file" << endl;
	assert(freopen(("../gnf/"+s+".gnf").c_str(),"w",stdout) != NULL);
	encode_ch(n,points,k);
}
