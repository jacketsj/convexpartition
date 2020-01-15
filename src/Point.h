#pragma once

#include <bits/stdc++.h>
using namespace std;

struct pt {
  int x, y, i;
  pt operator+(const pt& o) const {
    return {x+o.x, y+o.y, -1};
  }
  pt operator-(const pt& o) const {
    return {x-o.x, y-o.y, -1};
  }
  bool operator<(const pt& o) const {
    return tie(y, x) < tie(o.y, o.x);
  }

  friend int64_t cp(const pt& a, const pt& b) {
    return (int64_t)a.x*b.y - (int64_t)a.y*b.x;
  }
  friend int64_t cp(const pt& a, const pt& b, const pt& o) {
    return cp(a-o, b-o);
  }
  // is angle abc >= pi?
  friend bool is_reflex(const pt& a, const pt& b, const pt& c) {
    return cp(a,c,b) >= 0;
  }
};
