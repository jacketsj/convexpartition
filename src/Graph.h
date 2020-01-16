#pragma once

#include <bits/stdc++.h>
#include "Point.h"
// pbds for edge sampler
#include <ext/pb_ds/assoc_container.hpp>
using namespace __gnu_pbds;
typedef tree<pair<int,int>, null_type, less<pair<int,int>>, rb_tree_tag, tree_order_statistics_node_update> edge_ost;
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
      bool aorig = (o < v[a]); bool borig = (o < v[b]);
      if (aorig ^ borig) return aorig;
      return cp(v[a], v[b], o) > 0;
    }
  };
  vector<set<int, pt_cmp>> adj;
  edge_ost inner_edges;
  int chull_edge_num=0;

  int get_edge_num() {
    return chull_edge_num + inner_edges.size()/2;
  }

  // Functions to add vertices/edges
  void add_vertex(pt p) {
    assert(p.i == (int)points.size()); 
    points.push_back(p);
    adj.emplace_back(pt_cmp(p, points));
  }
  void add_edge(int i, int j) {
    assert(i!=j);
    if (adj[i].count(j)) return;
    adj[i].insert(j);
    adj[j].insert(i);
    inner_edges.insert({i,j});
    inner_edges.insert({j,i});
  }
  void remove_edge(int i, int j) {
    assert(adj[i].count(j));
    assert(adj[j].count(i));
    adj[i].erase(j);
    adj[j].erase(i);
    inner_edges.erase({i,j});
    inner_edges.erase({j,i});
  }
  void reset() {
    n = 0;
    points.clear();
    adj.clear();
    inner_edges.clear();
  }


  // Functions to grab adjacent edges
  int halfedge_next(int a, int b) {
    // Return next edge ccw 
    if (!adj[a].count(b)) {
      assert(adj[a].count(b));
    }
    auto it = next(adj[a].find(b));
    return (it == adj[a].end() ? *adj[a].begin() : *it);
  }
  int halfedge_prev(int a, int b) {
    // Return next edge cw 
    if (!adj[a].count(b)) {
      assert(adj[a].count(b));
    }
    auto it = adj[a].find(b);
    return (it == adj[a].begin() ? *adj[a].rbegin() : *prev(it));
  }

  void init_inner_edges() {
    // remove outer edges from inner_edges start with point definitely on hull
    int mni = 0;
    for(int i=0;i<n;i++){
      if (points[i] < points[mni]) {
        mni = i;
      }
    }
    // mni is lowest index coordinate point
    int cur = mni;
    int nex = *adj[mni].begin();
    do {
      chull_edge_num++;
      inner_edges.erase({cur, nex});
      inner_edges.erase({nex, cur});
      int nexnex = halfedge_next(nex, cur);
      tie(cur,nex) = tie(nex, nexnex);
    } while(cur!=mni);
  }

  // Geometric functions
  bool is_triangle(int a, int b) { 
    // return true if halfedge a->b is on triangle (faces oriented ccw)
    return halfedge_prev(b, a) == halfedge_next(a, b);
  }
  bool can_remove(int a, int b) {
    int c = halfedge_next(a, b);
    int d = halfedge_prev(a, b);
    if (is_reflex(points[d], points[a], points[c])){
      return false;
    }
    swap(a, b);
    c = halfedge_next(a, b);
    d = halfedge_prev(a, b);
    if (is_reflex(points[d], points[a], points[c])) {
      return false;
    }
    return true;
  }
  bool flip(int a, int b) {
    // return whether flip worked
    if (!is_triangle(a, b) || !is_triangle(b, a)) return false;
    if (!can_remove(a, b)) return false;
    int c = halfedge_next(a, b);
    int d = halfedge_next(b, a);
    remove_edge(a, b);
    add_edge(c, d);
    return true;
  }
  bool triangulate_halfedge(int a, int b, int& e1, int& e2) {
    // ensure a->b is a triangle
    if (is_triangle(a,b)) return false;
    int c = halfedge_next(a,b);
    assert(!adj[b].count(c));
    add_edge(b, c);
    e1 = b, e2 = c;
    return true;
  }

  // Functions to load/save graphs
  void read(string filename) { // clear graph and read in new graph
    reset();
    ifstream in(filename);
    in >> n;
    for(int i=0;i<n;i++) {
      int id, x, y;
      in >> id >> x >> y;
      add_vertex(pt(id, x, y));
    }
    for(int i=0;i<n;i++) {
      int ki, a;
      if(!(in >> ki)) return; // read if there is anything left to read
      for(int j=0;j<ki;j++) {
        in >> a;
        add_edge(i, a);
      }
    }
    // If we finished reading the graph, initialize things
    init_inner_edges();
    cerr << "DONE READING GRAPH " << filename <<endl;
  }

  void print_matlab() {
    cout << "g = graph([";
    for(int i=0;i<n;i++) {
      for(int j: adj[i]) {
        if (i>j) continue;
        cout << i+1 << " ";
      }
    }
    cout << "], [";
    for(int i=0;i<n;i++) {
      for(int j: adj[i]) {
        if (i>j) continue;
        cout << j+1 << " ";
      }
    }
    cout << "])" << endl;
    cout << "plot(g, 'XData', [";
    for(int i=0;i<n;i++) cout << points[i].x << " "; 
    cout << "], 'YData', [";
    for(int i=0;i<n;i++) cout << points[i].y << " "; 
    cout << "])" <<endl;
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
      out << '\n';
    }
  }
};
