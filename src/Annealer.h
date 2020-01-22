#pragma once

#include <bits/stdc++.h>
using namespace std;
using ld = long double;
#include "Graph.h"

struct annealer {
  mt19937 rng;
  graph& g;
  uniform_real_distribution<ld> dis;
  int64_t it, MAXT, MAXIT, success;
  ld temperature;

  annealer(graph& _g) : rng(0xF), //rng(chrono::high_resolution_clock::now().time_since_epoch().count()), 
                        g(_g), dis(0.0L, 1.0L) {
    // n log^2 n should be enough whp to visit all edges
    MAXT = 10*g.n*log2(g.n)*log2(g.n); 
    MAXIT = MAXT+3*g.n*log(g.n)+100000;
    temperature = 1;
  }

  void update_temperature() {
    temperature = max(0.L, 1 - (ld)it / MAXT);
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
    if (dis(rng) > temperature) { // removal probability
      if (g.can_remove(a,b)) {
        g.remove_edge(a,b);
        success++;
      }
    }
    else {
      /* actions to try:
      g.flip(a,b);
      g.rot_cw(a,b);
      g.rot_ccw(a,b);
      g.rot_cw(b,a);
      g.rot_ccw(b,a);

      */
      if (g.flip(a,b)) success++;
      else if (rng()%2) {
        if (g.rot_cw(a,b)) success++;
        else if (g.rot_ccw(a,b)) success++;
      }
      else {
        if (g.rot_ccw(a,b)) success++;
        else if (g.rot_cw(a,b)) success++;
      }
    }
    ++it;
  }
  
  void anneal() {
    cerr << "ANNEALING FOR " << MAXIT << " iterations" <<endl;
    for(it=0;it<MAXIT;it++) { // run some extra steps for safety
      update_temperature();
      anneal_step();
    }
  }
};
