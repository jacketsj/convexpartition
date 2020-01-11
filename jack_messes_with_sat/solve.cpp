#include <bits/stdc++.h>
using namespace std;

typedef long double ld;

struct pt
{
	ld x, y;
	pt() : x(0), y(0) {}
	pt(ld x, ld y) : x(x), y(y) {}
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

int main()
{
	int n; cin >> n;
	vector<pt> points(n);
	for (int i = 0; i < n; ++i)
	{
		int k;
		ld x, y; cin >> k >> x >> y;
		points[i] = (x,y);
	}
}
