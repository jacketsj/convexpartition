#pragma once

#include <bits/stdc++.h>
#include "Point.h"
// pbds for edge sampler
#include <ext/pb_ds/assoc_container.hpp>
using namespace __gnu_pbds;
typedef tree<pair<int,int>, null_type, less<int>, rb_tree_tag, tree_order_statistics_node_update> edge_ost;
// end pbds

using namespace std;

struct graph {
  int n;
  vector<pt> points;
  struct pt_cmp {
    pt o;
    const vector<pt>& v;
    pt_cmp(const pt& base, const vector<pt>& pts): o(base), v(pts) {}
    bool operator()(const int a, const int b) const {
      bool aorig = (o < a); bool borig = (o < b);
      if (aorig ^ borig) return aorig;
      return cp(v[a], v[b], o) > 0;
    }
  };
  vector<set<int, pt_cmp>> adj;
  edge_ost inner_edges;

  // Functions to grab adjacent edges
  int halfedge_next(int a, int b) {
    // Return next edge ccw 
    assert(adj[a].count(b));
    auto it = next(adj[a].find(b));
    return (it == adj[a].end() ? *adj[a].begin() : *it);
  }
  int halfedge_prev(int a, int b) {
    // Return next edge cw 
    assert(adj[a].count(b));
    auto it = adj[a].find(b);
    return (it == adj[a].begin() ? *adj[a].rbegin() : *prev(it));
  }

  void init_inner_edges() {
    // remove outer edges from inner_edges start with point definitely on hull
    int mni = 0;
    for(int i=0;i<n;i++){
      if (point < points[mni]) {
        mni = i;
      }
    }
    // mni is lowest index coordinate point
    int cur = mni;
    int nex = adj[mni].begin();
    do {
      inner_edges.erase(cur, nex);
      inner_edges.erase(nex, cur);
      tie(cur,nex) = tie(nex, halfedge_next(nex, cur));
    } while(nex!=mni);
  }

  // Geometric functions
  bool is_triangle(int a, int b) { 
    // return true if halfedge a->b is on triangle (faces oriented ccw)
    return halfedge_prev(b, a) == halfedge_next(a, b);
  }
  bool can_remove(int a, int b) {
    if (!is_triangle(a, b) || !is_triangle(b, a)) return false;
    int c = halfedge_next(a, b);
    int d = halfedge_next(b, a);
    // check convexity of points
    return !is_reflex(d, b, c) && !is_reflex(c, a, d);
  }
  void flip(int a, int b) {
    assert(can_remove(a, b));
    int c = halfedge_next(a, b);
    int d = halfedge_next(b, a);
    remove_edge(a, b);
    insert_edge(c, d);
  }

  // Functions to add vertices/edges
  void add_vertex(pt p) {
    assert(p.i == (int)points.size()); 
    points.push_back(p);
    adj.emplace_back(pt_cmp(p, points));
  }
  void add_edge(int i, int j) {
    adj[i].insert(j);
    adj[j].insert(i);
    inner_edges.insert(i,j);
    inner_edges.insert(j,i);
  }
  void remove_edge(int i, int j) {
    adj[i].erase(j);
    adj[j].erase(i);
    inner_edges.erase(i,j);
    inner_edges.erase(j,i);
  }
  void reset() {
    n = 0;
    points.clear();
    adj.clear();
  }
  // Functions to load/save graphs
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
      if(!(in >> ki)) break; // read if there is anything left to read
      for(int j=0;j<ki;j++) {
        in >> a;
        add_edge(i, a);
      }
    }
    init_inner_edges();
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
