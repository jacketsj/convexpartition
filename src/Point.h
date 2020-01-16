#pragma once

#include <bits/stdc++.h>
using namespace std;

struct pt {
  int i, x, y;
  pt() : i(-1), x(0), y(0) {}
  pt(int x, int y) : i(-1), x(x), y(y) {}
  pt(int i, int x, int y) : i(i), x(x), y(y) {}
  pt operator+(const pt& o) const {
    return pt(x+o.x,y+o.y);
  }
  pt operator-(const pt& o) const {
    return pt(x-o.x,y-o.y);
  }
  bool operator<(const pt& o) const {
    return tie(y, x) < tie(o.y, o.x);
  }
  int64_t normsqr() const {
    int64_t x(x), y(y);
    return x*x+y*y;
  }
  pt operator/(int d) const {
    assert(d!=0);
    return pt(x/d,y/d);
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
};
