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
		//return isectray(o) && o.isectray(*this);
		auto c = o.a, d = o.b;
		return ((d-a).cross(b-a)) * ((c-a).cross(b-a)) < 0
			&& ((a-c).cross(d-c)) * ((b-c).cross(d-c)) < 0;
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

void add_rand_pt(int n, vector<pt> &points, int k, vector<vector<int>> &pt_cl, vector<vector<int>> &or_cl, int &m)
{
	if (n == 1) // unnecessary probably
		return;

	// choose a random interior point
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

vector<vector<int>> read_ch(int n, vector<pt> points, int k)
{
	vector<bool> input(n*k);
	string sat;
	do
	{
		cin >> sat;
	}
	while (sat != "v");
	assert(sat=="v");
	int tr = 0;
	//for (int i = 0; i < 2*n*k+k*(k-1)/2; ++i)
	int f;
	while (cin >> f)
	{
		//int f; cin >> f;
		if (f > 0) // default is false, so only need this case
		{
			input[f-1]=true;
			++tr;
		}
	}
	cerr << tr << " values are true" << endl;

	vector<vector<pt>> sets(k);
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < k; ++j)
			if (input[i*k+j])
				sets[j].push_back(points[i]);
	// now, do an angle sort on each point set
	for (int i = 0; i < k; ++i)
		if (sets[i].size() > 0)
		{
			pt& fp = sets[i][0]; // fixed-point in set
			sort(sets[i].begin()+1,sets[i].end(), // skip fp
				[&](const pt& a, const pt& b) {
					return (a-fp).cross(b-fp) < 0; //either direction ok
				});
		}
	// now, our point sets are ordered around the convex hull
	// find the edges
	vector<vector<int>> adj(n);
	for (int i = 0; i < k; ++i)
	{
		int sz = sets[i].size();
		for (int j = 1; j <= sz; ++j)
		{
			// edge u->v
			int u = sets[i][j-1].i;
			int v = sets[i][j%sz].i;
			cerr << "creating edge (" << u << ',' << v << ")" << endl;
			adj[u].push_back(v);
			adj[v].push_back(u);
		}
	}
	return adj;
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
	//vector<vector<int>> p_hu; // point on ch
	vector<vector<int>> ch_isect_pt; // points inside ch (none!)
	for (int i = 0; i < n; ++i)
		for (int j = 0; j < k; ++j)
		{
			//psi[i][j] = ++m;
			++m;

			or_cl[i].push_back(m);
			pt_cl.push_back({j,m,2,points[i].x,points[i].y});
		}

	for (int i = 0; i < n; ++i)
		for (int j = 0; j < k; ++j)
		{
			// point not allowed to be inside ch
			ch_isect_pt.push_back({j,++m,1,2,points[i].x,points[i].y});
			units.push_back(-m);
			//p_hu.push_back({j,m,m}); // might be able to just do this
			//p_hu.push_back({j,m,++m});
			//or_cl.push_back({-m,m-1}); // point on hull iff point in set
			//or_cl.push_back({m,-(m-1)});
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
	// for (auto &u : p_hu)
	// {
	// 	cout << "point_on_hull";
	// 	for (auto i : u)
	// 		cout << ' ' << i;
	// 	cout << '\n';
	// }
	for (auto &u : ch_isect_pt)
	{
		cout << "convex_hull_intersects_polygon";
		for (auto i : u)
			cout << ' ' << i;
		cout << '\n';
	}
}

#undef int
int main(int argc, char* argv[])
{
	ios::sync_with_stdio(0);
	bool read = false;
	if (argc == 2)// && arg2 == "-r")
		read = true;
	else if (argc > 2)
	{
		cerr << "Usage: " << argv[0] << " [-r]" << endl;
		return 1;
	}

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

	if (!read)
	{
		cout << "writing to gnf file" << endl;
		assert(freopen(("gnf/"+s+".gnf").c_str(),"w",stdout) != NULL);
		encode_ch(n,points,k);
	}
	else
	{
		cout << "reading from gsat file" << endl;
		assert(freopen(("gsat/"+s+".gsat").c_str(),"r",stdin) != NULL);

		vector<vector<int>> adj = read_ch(n,points,k);
		cout << "writing to out file" << endl;
		assert(freopen(("gout/"+s+".out").c_str(),"w",stdout) != NULL);
		cout << n << '\n';
		for (int i = 0; i < n; ++i)
			cout << i << ' ' << points[i].x << ' ' << points[i].y << '\n';
		for (int i = 0; i < n; ++i)
		{
			cout << adj[i].size();
			for (int j : adj[i])
				cout << ' ' << j;
			cout << '\n';
		}
	}
}
