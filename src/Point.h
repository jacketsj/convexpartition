#pragma once

#include <bits/stdc++.h>
using namespace std;
typedef long double ld;

struct pt {
  int64_t i, x, y;
  pt() : i(-1), x(0), y(0) {}
  pt(int64_t x, int64_t y) : i(-1), x(x), y(y) {}
  pt(int i, int64_t x, int64_t y) : i(i), x(x), y(y) {}
  pt operator+(const pt& o) const {
    return pt(x+o.x,y+o.y);
  }
	pt subtract(const pt& o) const {
    return pt(x-o.x,y-o.y);
	}
  pt operator-(const pt& o) const {
    return pt(x-o.x,y-o.y);
  }
  bool operator<(const pt& o) const {
    return tie(y, x) < tie(o.y, o.x);
  }
  bool operator==(const pt& o) const {
    return tie(y, x) == tie(o.y, o.x);
  }
	ld angle(const pt &o) const
	{
		pt n = subtract(o);
		return atan2(n.y,n.x);
	}
	ld angle() const
	{
		return angle(pt(0,0));
	}
  int64_t normsqr() const {
    int64_t a(x), b(y);
    return b*b+a*a;
  }
  pt operator/(int d) const {
    assert(d!=0);
    return pt(x/d,y/d);
  }

  friend ld cpld(const pt& a, const pt& b) {
    return (ld)a.x*b.y - (ld)a.y*b.x;
  }
  friend int64_t cp(const pt& a, const pt& b) {
    return (int64_t)a.x*b.y - (int64_t)a.y*b.x;
  }
  friend int64_t cp(const pt& a, const pt& b, const pt& o) {
    return cp(a-o, b-o);
  }
  // is angle abc >= pi?
  friend bool is_reflex(const pt& a, const pt& b, const pt& c) {
    return cp(a,c,b) < 0;
  }
  friend int64_t distsqr(const pt& a, const pt& b) {
    return (a-b).normsqr();
  }
  friend int64_t dot(const pt& a, const pt &b) {
    return (int64_t)a.x*b.x+(int64_t)a.y*b.y;
  }
  friend ld dist(const pt& a, const pt& b) {
    return sqrt(distsqr(a,b));
  }
};
