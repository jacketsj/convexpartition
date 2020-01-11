#pragma once

#include <bits/stdc++.h>
using namespace std;
using ld = long double;
#include "Graph.h"

struct annealer {
  mt19937 rng;
  ld temperature;
  graph g;
  void flip(int u, int v) {
  }
  void remove(int u, int v) {
  }
};
