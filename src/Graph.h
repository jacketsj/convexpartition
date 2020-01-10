#pragma once

#include <bits/stdc++.h>
#include "Point.h"
using namespace std;

struct graph {
  vector<pt> points;
  struct pt_cmp {
    pt o;
    const vector<pt>& v;
    pt_cmp(const pt& base, const vector<pt>& pts): o(base), v(pts) {}
    bool operator()(const int a, const int b) const {
      return cp(v[a], v[b], o) < 0;
    }
  };
  vector<set<int, pt_cmp>> adj;
  void add_vertex(pt i) {
    assert(i.i == points.size());
    points.push_back(i);
    adj.emplace_back(i, points);
  }
};
