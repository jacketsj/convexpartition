#pragma once

#include <bits/stdc++.h>
using namespace std;
using ld = long double;
#include "Graph.h"

struct annealer {
  mt19937 rng;
  graph& g;
  uniform_real_distribution<ld> dis;
  int it, maxit;
  ld temperature;

  annealer(graph& _g) : g(_g), dis(0.0L, 1.0L) {
    it = 0;
    maxit = g.n*100;
    temperature = 1;
  }

  void update_temperature() {
    temperature = max(0.L, 1 - (ld)it / maxit);
  }

  pair<int,int> bad_sample_halfedge() { 
    // This is slow! we sample via pbds order stat tree
    return *g.inner_edges.find_by_order(rng()%g.inner_edges.size());
  }

  void anneal_step() {
    // Rule: 
    // 1. sample random edge
    // 2. flip with probability temperature
    // 3. remove if possible with probability 1-temperature.
    pair<int, int> halfedge = bad_sample_halfedge();
    int a = halfedge.first;
    int b = halfedge.second;
    if (dis(rng) > temperature) {
      if (g.can_remove(a,b)) {
        g.remove_edge(a,b);
      }
    }
    else {
      int e00=-1, e01=-1, e10=-1, e11=-1;
      bool t1 = g.triangulate_halfedge(a, b, e00, e01);
      bool t2 = g.triangulate_halfedge(b, a, e10, e11);
      if (!g.flip(a,b)) {
        // if we failed to flip, put back removed edges(?)
        // TODO: Figure out what we want to do here
        if (t1) g.remove_edge(e00,e01);
        if (t2) g.remove_edge(e10,e11);
      }
    }
    assert(g.adj[a].size() >=2);
    assert(g.adj[b].size() >=2);
    ++it;
  }
  
  void anneal() {
    for(it=0;it<maxit*1.1L;it++) {
      update_temperature();
      anneal_step();
    }
  }
};
