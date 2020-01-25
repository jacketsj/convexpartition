#pragma once

#include <bits/stdc++.h>
using namespace std;
using ld = long double;
#include "Graph.h"

ld sqr(ld x) { return x*x; }

struct annealer {
  mt19937 rng;
  graph& g;
  uniform_real_distribution<ld> dis;
  int64_t it, MAXT, MAXIT, success;
  ld temperature;

  annealer(graph& _g) : rng(0xF), //rng(chrono::high_resolution_clock::now().time_since_epoch().count()), 
                        g(_g), dis(0.0L, 1.0L) {
    // n log^2 n should be enough whp to visit all edges
    MAXT = 2*g.n*log2(g.n)*log2(g.n);  // Reduced 
    MAXIT = MAXT+3*g.n*log(g.n)+100000;
    temperature = 1;
    success = 0;
  }

  void update_temperature() {
    // should start and end low
    //temperature = max(0.L, exp((ld) -3*it / MAXT - 0.5L) + 0.01);//1 - (ld)it / MAXT);
    static const ld EPS = 3;
    temperature = max(0.05L, 0.4 * exp(-sqr(EPS * ((ld) it / MAXT - 0.5L))));
    //temperature = max(0.L, 0.3 * (1 - 4*sqr((ld) it / MAXT - 0.5L)));
  }

  pair<int,int> sample_good_edge() { 
    // This is slow! we sample via pbds order stat tree
    return *g.good_edges.find_by_order(rng()%g.good_edges.size());
  }
  pair<int,int> sample_removable_edges() { 
    // This is slow! we sample via pbds order stat tree
    return *g.removable_edges.find_by_order(rng()%g.removable_edges.size());
  }


  void anneal_step() {
    // Rule: 
    // 1. sample random edge
    // 2. flip with probability temperature
    // 3. remove if possible with probability 1-temperature.
    if (dis(rng) >= temperature && g.removable_edges.size()) { // removal probability
      pair<int, int> edge = sample_removable_edges();
      int a = edge.first;
      int b = edge.second;
      assert(g.adj[a].count(b)); assert(g.adj[b].count(a));
      if (g.can_remove(a, b)) {
        g.remove_edge(a, b, 1);
        success++;
      }
    }
    else {
      pair<int, int> edge = sample_good_edge();
      int a = edge.first;
      int b = edge.second;
      assert(g.adj[a].count(b)); assert(g.adj[b].count(a));
      if (rng()%2) swap(a,b);
      if (g.flip(a, b, 1)) success++;
      else if (rng()%2) {
        if (g.rot_cw(a, b, 1)) success++;
        else if (g.rot_ccw(a, b, 1)) success++;
      }
      else {
        if (g.rot_ccw(a, b, 1)) success++;
        else if (g.rot_cw(a, b, 1)) success++;
      }
    }
  }
  
  void anneal() {
    static const int64_t PRINT_ITER = 1e5;
    cerr << "ANNEALING FOR " << MAXIT << " iterations" <<endl;
    for(it=1;it<=MAXIT;it++) { // run some extra steps for safety
      if (it % PRINT_ITER == 0) cerr << "ANNEALING ITERATION " << it << " TEMP = " << temperature << " SUCCESS% = " << (ld) success / it << endl;
      update_temperature();
      anneal_step();
    }
  }
};
