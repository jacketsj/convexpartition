#pragma once

#include <bits/stdc++.h>
#include "Point.h"
using namespace std;

struct graph {
  int n;
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

  void add_vertex(pt p) {
    assert(p.i == (int)points.size());
    points.push_back(p);
    adj.emplace_back(pt_cmp(p, points));
  }
  void add_edge(int i, int j) {
    adj[i].insert(j);
    adj[j].insert(i);
  }
  void remove_edge(int i, int j) {
    adj[i].erase(j);
    adj[j].erase(i);
  }

  void reset() {
    n = 0;
    points.clear();
    adj.clear();
  }
  void read(string filename) { // clear graph and read in new graph
    reset();
    ifstream in(filename);
    in >> n;
    for(int i=0;i<n;i++) {
      int id, x, y;
      in >> id >> x >> y;
      add_vertex({id, x, y});
    }
    for(int i=0;i<n;i++) {
      int ki, a;
      in >> ki;
      for(int j=0;j<ki;j++) {
        in >> a;
        add_edge(i, a);
      }
    }
  }
  void write(string filename) {
    ofstream out(filename);
    out << n << '\n';
    for(int i=0;i<n;i++) {
      out << points[i].i << " " << points[i].x << " " << points[i].y << '\n';
    }
    for(int i=0;i<n;i++) {
      out << adj[i].size() << " ";
      for(int j: adj[i]) {
        out << j << " ";
      }
      cout << '\n';
    }
  }
};
