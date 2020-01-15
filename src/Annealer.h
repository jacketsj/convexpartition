#pragma once

#include <bits/stdc++.h>
using namespace std;
using ld = long double;
#include "Graph.h"

struct annealer {
  mt19937 rng;
  uniform_real_distribution<ld> dis;
  int it, maxit;
  ld temperature;
  graph& g;

  annealer(graph& _g) : g(_g), dis(0.0L, 1.0L) {
    it = 0;
    maxit = g.n*100;
  }

  void update_temperature() {
    temperature = max(0.L, 1 - (ld)it / maxit);
  }

  pair<int,int> bad_sample_halfedge() { 
    // This is slow! we sample via pbds order stat tree
    return g.inner_edges.find_by_order(rng()%g.inner_edges.size());
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
        g.remove(a,b);
      }
    }
    else {
      int e00, e01, e10, e11;
      bool t1 = g.triangulate_halfedge(a, b, e00, e01);
      bool t2 = g.triangulate_halfedge(b, a, e10, e11);
      if (!g.flip(a,b)) {
        if (t1) g.remove_edge(e00,e01);
        if (t2) g.remove_edge(e10,e11);
      }
    }
  }
  
  void anneal() {
    for(it=0;it<maxit*2;it++) {
      update_temperature();
      anneal_step();
    }
  }
};
