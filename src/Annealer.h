#pragma once

#include <bits/stdc++.h>
using namespace std;
using ld = long double;
#include "Graph.h"

struct annealer {
  mt19937 rng;
  int it, maxit;
  ld temperature;
  graph& g;

  annealer(graph& _g) : g(_g) {
    it = 0;
    maxit = g.n*100;
  }

  void update_temperature() {
    temperature = 1 - (ld)it / maxit;
  }

  pair<int,int> bad_sample_halfedge() {
    return g.edges.find_by_order(rng()%g.edges.size());
  }

  void anneal() {
    pair<int, int> halfedge = bad_sample_halfedge();
  }
};
