#include <bits/stdc++.h>
#include "../../src/Point.h"
//#include "../src/Graph.h"
#include "../../src/extern/delaunator.hpp"
using namespace std;

string read_problem_file()
{
	string s; cin >> s;
	return s;
}

void encode_smt(int n, const vector<pt> &points, int k)
{
	// k edges
	// n nodes
	vector<string> u(k), v(k);
	vector<string> vars;
	vector<string> assertions;

	auto literal = [](int i) {
		return string("(_ bv" + to_string(i) + " 64)");
	};

	for (int i = 0; i < k; ++i)
	{
		u[i] = "u_" + to_string(i);
		v[i] = "v_" + to_string(i);
		vars.push_back(u[i]);
		vars.push_back(v[i]);
		vars.push_back(u[i] + "_x");
		vars.push_back(u[i] + "_y");
		vars.push_back(v[i] + "_x");
		vars.push_back(v[i] + "_y");
		// u[i] < v[i], unsigned comparison
		assertions.push_back("(bvult " + u[i] + ' ' + v[i] + ")");
		// v[i] < n
		assertions.push_back("(bvult " + v[i] + " " + literal(n) + ")");
		// u[i-1] < u[i], unsigned comparison
		if (i > 0)
			assertions.push_back("(bvult " + u[i-1] + " " + u[i] + ")");

		// define what u[i]_x, u[i]_y, v[i]_x, and v[i]_y must be
		for (int p = 0; p < points.size(); ++p)
		{
			assertions.push_back("(=> (= " + u[i] + " " + literal(p) + ") "
					+ "(and (= " + u[i] + "_x " + literal(points[p].x) + ") "
					+ "(= " + u[i] + "_y " + literal(points[p].y) + ")))");
			assertions.push_back("(=> (= " + v[i] + " " + literal(p) + ") "
					+ "(and (= " + v[i] + "_x " + literal(points[p].x) + ") "
					+ "(= " + v[i] + "_y " + literal(points[p].y) + ")))");
		}
	}

	// we use cross products both for intersections and for angle constraints in the adjacency list
	auto cross = [](const string &x, const string &y, const string &ox, const string &oy) {
		return string("(bvsub (bvmul " + x + " " + oy + ") (bvmul " + ox + " " + y + "))");
	};
	// cross product indicates <=180 degrees
	auto cross_max = [&](const string &x, const string &y, const string &ox, const string &oy) {
		// signed <=
		return string("(bvsle " + cross(x,y,ox,oy) + literal(0) + ")");
	};

	// no two lines can intersect
	for (int i = 0; i < k; ++i)
		for (int j = i+1; j < k; ++j)
		{
			// seg x seg
			// here we do signed comparisons of everything
			auto use_cross = [&](const string &a, const string &b, const string &c) {
				string ax, ay, bx, by, cx, cy; // how do we get these? just append _x or _y for now
				ax = a + "_x";
				ay = a + "_y";
				bx = b + "_x";
				by = b + "_y";
				cx = c + "_x";
				cy = c + "_y";
				string x = "(bvsub " + ax + " " + bx + ")";
				string y = "(bvsub " + ay + " " + by + ")";
				string ox = "(bvsub " + cx + " " + bx + ")";
				string oy = "(bvsub " + cy + " " + by + ")";
				return cross(x,y,ox,oy);
			};
			auto isectray = [&](const string &a, const string &b, const string &c, const string &d)
			{
				string prod = "(bvmul " + use_cross(a,b,c) + " " + use_cross(d,b,c) + ")";
				// signed less than 0
				return string("(bvslt " + prod + " " + literal(0) + ")");
			};
			// variable names by order of appearance
			string a = u[i];
			string b = v[i];
			string c = u[j];
			string d = v[j];
			assertions.push_back(isectray(d,a,b,c));
			assertions.push_back(isectray(a,c,d,b));
		}

	// create a permutation of all 2k half-edges
	vector<string> p(2*k);
	for (int i = 0; i < 2*k; ++i)
		p[i] = "p_" + to_string(i);
	for (int i = 0; i < 2*k; ++i)
	{
		vars.push_back(p[i]);
		// append _dx and _dy to get vectors
		vars.push_back(p[i] + "_dx");
		vars.push_back(p[i] + "_dy");
		vars.push_back(p[i] + "_v"); // adjacent (closer) vertex
		// p[i] < 2*k
		assertions.push_back("(bvult " + p[i] + " " + literal(2*k) + ")");
		// p[i][v] < n
		assertions.push_back("(bvult " + p[i] + "_v " + literal(n) + ")");
		// define what p_i_dx (signed), p_i_dy (signed), and p_i_v must be
		for (int j = 0; j < k; ++j)
		{
			assertions.push_back("(=> (= " + literal(j) + " " + p[i] + ") "
					+ "(= (bvsub " + u[j] + "_x " + v[j] + "_x) "
					+ p[i] + "_dx))");
			assertions.push_back("(=> (= " + literal(j) + " " + p[i] + ") "
					+ "(= (bvsub " + u[j] + "_y " + v[j] + "_y) "
					+ p[i] + "_dy))");
			assertions.push_back("(=> (= " + literal(j) + " " + p[i] + ") "
					+ "(= " + v[j] + " " + p[i] + "_v))");
			assertions.push_back("(=> (= (bvsub " + literal(j) + " " + literal(k) + ") "
						+ p[i] + ") "
					+ "(= (bvsub " + v[j] + "_x " + u[j] + "_x) "
					+ p[i] + "_dx))");
			assertions.push_back("(=> (= (bvsub " + literal(j) + " " + literal(k) + ") "
						+ p[i] + ") "
					+ "(= (bvsub " + v[j] + "_y " + u[j] + "_y) "
					+ p[i] + "_dy))");
			assertions.push_back("(=> (= (bvsub " + literal(j) + " " + literal(k) + ") "
						+ p[i] + ") "
					+ "(= " + v[j] + " " + p[i] + "_v))");
		}
		if (i > 0)
		{
			// p_(i-1)_v <= p_i_v, and p_(i-1)_v + 1 >= p_i_v
			assertions.push_back("(bvule " + p[i-1] + "_v " + p[i] + "_v)");
			assertions.push_back("(bvuge " + p[i-1] + "_v " + p[i] + "_v)");
			if (i < n-1)
			{
				// p_i_v = p_(i-1)_v or p_i_v = p_(i+1)_v (at least two for every vertex)
				assertions.push_back("(or (= " + p[i] + "_v " + p[i-1] + "_v) "
						+ "(= " + p[i] + "_v " + p[i+1] + "_v))");
			}
			// if p_(i-1)_v = p_i_v, then p_(i-1) must have p_i less than 180 degrees away clockwise (?)
			assertions.push_back("(=> (= " + p[i-1] + "_v " + p[i] + "_v) "
					+ cross_max(p[i-1] + "_dx", p[i-1] + "_dy", p[i] + "_dx", p[i] + "_dy") + ")");
		}
	}
	// force it to actually be a permutation (probably unnecessary, but might speed things up?)
	for (int i = 0; i < 2*k; ++i)
		for (int j = i+1; j < 2*k; ++j)
			assertions.push_back("(not (= " + p[i] + " " + p[j] + "))");

	// first two must be v=0, last two must be v=n-1
	assertions.push_back("(= " + p[0] + "_v " + literal(0) + ")");
	assertions.push_back("(= " + p[1] + "_v " + literal(0) + ")");
	assertions.push_back("(= " + p[2*k-1] + "_v " + literal(n-1) + ")");
	assertions.push_back("(= " + p[2*k-2] + "_v " + literal(n-1) + ")");

	// add the cross product rule for the last to first edge of every adjacency list too
	for (int i = 0; i < 2*k; ++i)
		for (int j = i+1; j < 2*k; ++j)
		{
			if (i > 0 && j < 2*k-1)
				assertions.push_back("(=> (and (= " + p[i] + "_v " + p[j] + "_v) "
							+ "(and (not (= " + p[j] + "_v " + p[j+1] + "_v)) "
								+ "(not (= " + p[i] + "_v " + p[i-1] + "_v)))) "
						+ cross_max(p[j] + "_dx", p[j] + "_dy", p[i] + "_dx", p[i] + "_dy") + ")");
			else if (i > 0)
				assertions.push_back("(=> (and (= " + p[i] + "_v " + p[j] + "_v) "
							+ "(not (= " + p[i] + "_v " + p[i-1] + "_v))) "
						+ cross_max(p[j] + "_dx", p[j] + "_dy", p[i] + "_dx", p[i] + "_dy") + ")");
			else if (j < 2*k-1)
				assertions.push_back("(=> (and (= " + p[i] + "_v " + p[j] + "_v) "
							+ "(not (= " + p[j] + "_v " + p[j+1] + "_v))) "
						+ cross_max(p[j] + "_dx", p[j] + "_dy", p[i] + "_dx", p[i] + "_dy") + ")");
			else
				assertions.push_back("(=> (= " + p[i] + "_v " + p[j] + "_v) "
						+ cross_max(p[j] + "_dx", p[j] + "_dy", p[i] + "_dx", p[i] + "_dy") + ")");
		}


	//cout << "(set-option :print-success true)" << '\n';
	cout << "(set-option :pp.bv-literals false)" << '\n'; // change default display to decimal (?)

	for (string &s : vars)
		cout << "(declare-const " << s << " (_ BitVec 64))" << '\n';

	for (string &s : assertions)
		cout << "(assert " << s << ")\n";

	cout << "(check-sat)" << '\n';

	// might want to use (display ...) instead of get-model
	// note that this defaults to hex or binary, not decimal
	cout << "(get-model)" << '\n';
}

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

	// start by reading in problem file name
	string s = read_problem_file();

	cerr << "reading problem file: " << s << endl;
	assert(freopen(("../../in/"+s+".in").c_str(),"r",stdin) != NULL);
	int n; cin >> n;
	vector<pt> points(n);
	for (int i = 0; i < n; ++i)
	{
		int k;
		int x, y; cin >> k >> x >> y;
		points[i] = pt(i,x,y);
	}

	int k = 21; // number of convex hulls to try for
	cerr << "using k=" << k << endl;

	if (!read)
	{
		cerr << "writing to smt file" << endl;
		assert(freopen(("smt/"+s+".smt").c_str(),"w",stdout) != NULL);
		//encode_ch(n,points,k);
		encode_smt(n,points,k);
	}
	else
	{
		//cout << "reading from  file" << endl;
		//assert(freopen(("gsat/"+s+".gsat").c_str(),"r",stdin) != NULL);

		//vector<vector<int>> adj = read_ch(n,points,k);
		//cout << "writing to out file" << endl;
		//assert(freopen(("gout/"+s+".out").c_str(),"w",stdout) != NULL);
		//cout << n << '\n';
		//for (int i = 0; i < n; ++i)
		//	cout << i << ' ' << points[i].x << ' ' << points[i].y << '\n';
		//for (int i = 0; i < n; ++i)
		//{
		//	cout << adj[i].size();
		//	for (int j : adj[i])
		//		cout << ' ' << j;
		//	cout << '\n';
		//}
	}
}
