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
    // This comparator thinks collinear points are the same 
    // THIS FUNCTIONALITY IS EXPECTED.
    bool operator()(const int a, const int b) const {
      bool aorig = (o < v[a]); bool borig = (o < v[b]);
      if (aorig ^ borig) return aorig;
      return cp(v[a], v[b], o) > 0;
    }
  };
  vector<set<int, pt_cmp>> adj;
  set<pair<int,int>> chull_edges;
  edge_ost good_edges, removable_edges;
  int edge_cnt;
  ld tot_edge_len;

  int get_edge_num() {
    return edge_cnt;
  }

  // MUST call this function after ADDING/REMOVING edges
  void update_status(int a, int b) {
    if (a>b) swap(a, b);
    if (chull_edges.count({a, b})) {
      return;
    }
    if (can_remove(a,b)) removable_edges.insert({a,b});
    else removable_edges.erase({a,b});
    if (can_flip(a, b) || can_rot(a, b) || can_rot(b, a)) good_edges.insert({a, b});
    else good_edges.erase({a, b});
  }

  // Functions to add vertices/edges
  void add_vertex(pt p) {
    assert(p.i == (int)points.size()); 
    points.push_back(p);
    adj.emplace_back(pt_cmp(p, points));
  }

  void remove_edge_and_update_status(int i, int j) {
    if (i > j) swap(i,j);
    int c, d, e, f;
    c = halfedge_next(i,j);
    d = halfedge_prev(i,j);
    e = halfedge_next(j,i);
    f = halfedge_prev(j,i);
    remove_edge(i,j);
    removable_edges.erase({i, j});
    good_edges.erase({i, j});
    update_status(i,c);
    update_status(i,d);
    update_status(j,e);
    update_status(j,f);
  }
  bool add_edge_and_update_status(int i, int j) {
    if(!add_edge(i,j)) return false;
    update_status(i,j);
    int c, d, e, f;
    c = halfedge_next(i,j);
    d = halfedge_prev(i,j);
    e = halfedge_next(j,i);
    f = halfedge_prev(j,i);
    update_status(i,c);
    update_status(i,d);
    update_status(j,e);
    update_status(j,f);
    return true;
  }
  bool add_edge(int i, int j, bool update=0) {
    if (update) return add_edge_and_update_status(i, j);
    assert(i!=j);
    if (i>j) swap(i,j);
    if (adj[i].count(j)) return false;
    if (adj[j].count(i)) return false;
    adj[i].insert(j);
    adj[j].insert(i);
    edge_cnt++;
    tot_edge_len += sqrt(distsqr(points[i], points[j]));
    return true;
  }
  void remove_edge(int i, int j, bool update=0) {
    if (update) {
      remove_edge_and_update_status(i, j);
      return;
    }
    assert(adj[i].count(j));
    assert(adj[j].count(i));
    if (i>j) swap(i,j);
    adj[i].erase(j);
    adj[j].erase(i);
    edge_cnt--;
    tot_edge_len -= sqrt(distsqr(points[i], points[j]));
  }
  void reset() {
    n = 0;
    edge_cnt= 0;
    tot_edge_len = 0;
    points.clear();
    adj.clear();
    good_edges.clear();
    chull_edges.clear();
  }


  // Functions to grab adjacent edges
  int halfedge_next(int a, int b) {
    // Return next edge ccw 
    // if (!adj[a].count(b)) { assert(adj[a].count(b)); }
    auto it = next(adj[a].find(b));
    return (it == adj[a].end() ? *adj[a].begin() : *it);
  }
  int halfedge_prev(int a, int b) {
    // Return next edge cw 
    // if (!adj[a].count(b)) { assert(adj[a].count(b)); }
    auto it = adj[a].find(b);
    return (it == adj[a].begin() ? *adj[a].rbegin() : *prev(it));
  }

  void init_good_edges() {
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
      chull_edges.insert({min(cur, nex), max(cur, nex)});
      int nexnex = halfedge_next(nex, cur);
      tie(cur,nex) = tie(nex, nexnex);
    } while(cur!=mni);

    // update status of all edges
    for(int i=0;i<n;i++) {
      vector<int> adji; // cast to vector to avoid iteration problems
      for(int j:adj[i]) adji.push_back(j);
      for(int j:adji) {
        if (i<j) update_status(i,j);
      }
    }
  }

  // Geometric functions
  bool is_triangle(int a, int b) { 
    // return true if halfedge a->b is on triangle (faces oriented ccw)
    // if degenerate, can't be side of triangle
    if (adj[a].size() == 2 || adj[b].size() == 2) return false;
    return halfedge_prev(b, a) == halfedge_next(a, b);
  }
  bool can_remove_half(int a, int b) {
    // inner edges of degree 2 arise from colinear points
    int c = halfedge_next(a, b);
    int d = halfedge_prev(a, b);
    if (is_reflex(points[d], points[a], points[c])){
      return false;
    }
    return true;
  }
  bool can_remove(int a, int b) {
    if (a > b) swap(a, b);
    if (adj[a].size()==2 || adj[b].size() == 2) return false; 
    return !chull_edges.count({a,b}) && can_remove_half(a, b) && can_remove_half(b, a);
  }


  bool rot(int a, int &b, int nb, bool update=0) {
    // assert(adj[a].count(b));
    if (adj[a].size()==2 || adj[b].size() == 2) return false; 
    // can rotate edge a->b towards nb around a
    // try a -> b to become a -> nb
    if (adj[a].count(nb)) return false;
    // rotating it might create colinear points
    if (!add_edge(a, nb, update)) return false;
    if (can_remove(a, b)){
      remove_edge(a, b, update);
      b = nb;
      return true;
    }
    remove_edge(a, nb, update);
    return false;
  }

  bool rot_ccw(int a, int &b, bool update=0) {
    int nb = halfedge_next(b, a);
    return rot(a, b, nb, update);
  }

  bool rot_cw(int a, int &b, bool update=0) {
    int pb = halfedge_prev(b, a);
    return rot(a, b, pb, update);
  }

  // directed rotations
  bool can_rot(int a, int b) {
    if (rot_ccw(a, b)) {
      assert(rot_cw(a,b));
      return true;
    }
    if (rot_cw(a, b)){
      assert(rot_ccw(a,b));
      return true;
    }
    return false;
  }

  bool flip(int& a, int& b, bool update=0) {
    // return whether flip worked
    if (!is_triangle(a, b) || !is_triangle(b, a)) return false;
    if (!can_remove(a, b)) return false;
    int c = halfedge_next(a, b);
    int d = halfedge_next(b, a);
    remove_edge(a, b, update);
    if(!add_edge(c, d, update)){
      add_edge(a, b, update);
      return false;
    }
    a = c; b = d;
    return true;
  }
  // flip flop to check
  bool can_flip(int a, int b) {
    if (!flip(a,b)) return false;
    else return flip(a,b);
  }

  // commented out because unused
  /*
  bool triangulate_halfedge(int a, int b, int& e1, int& e2) {
    // ensure a->b is a triangle
    if (is_triangle(a,b)) return false;
    int c = halfedge_next(a,b);
    // assert(!adj[b].count(c));
    add_edge(b, c);
    e1 = b, e2 = c;
    return true;
  }
  */

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
    init_good_edges();
    //cerr << "DONE READING GRAPH " << filename <<endl;
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
