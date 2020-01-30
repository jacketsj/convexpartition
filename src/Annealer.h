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
  int64_t flip_cnt, add_cnt, remove_cnt;

  annealer(graph& _g) : //rng(0xF), 
												rng(chrono::high_resolution_clock::now().time_since_epoch().count()), 
                        g(_g), dis(0.0L, 1.0L) {
    // n log^2 n should be enough whp to visit all edges
    MAXT = 0.5*g.n*log2(g.n)*log2(g.n); 
		cerr << "MAXT=" << MAXT << endl;
    MAXIT = MAXT+3*g.n*log(g.n)+100000;
    temperature = 1;
    flip_cnt = add_cnt = remove_cnt = success = 0;
  }

	int64_t atleast0(int64_t a)
	{
		if (a < 0)
			return 0;
		return a;
	}

  void update_temperature() {
    // should start and end low
    //temperature = max(0.L, exp((ld) -3*it / MAXT - 0.5L) + 0.01);//1 - (ld)it / MAXT);
    static const ld EPS = 3;
    //temperature = max(0.05L, 0.4 * exp(-sqr(EPS * ((ld) it / MAXT - 0.5L))));
    temperature = max(0.02L, 0.4 * exp(-sqr(EPS * ((ld) (atleast0(it-int64_t(80000))) / MAXT - 0.5L))));
    //temperature = max(0.L, 0.3 * (1 - 4*sqr((ld) it / MAXT - 0.5L)));
  }

  pair<int,int> sample_inner_edge() { 
    // This is slow! we sample via pbds order stat tree
    return *g.inner_edges.find_by_order(rng()%g.inner_edges.size());
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
    // 3. Add an edge with probability temperature
    // 4. If failed, remove if possible with probability 1-temperature.
    // 5. If failed, rotate/flip a random edge
    if (dis(rng) <= temperature) { // try adding an edge
      pair<int, int> edge = sample_inner_edge();
      int a = edge.first;
      int b = edge.second;
      if (rng()%2) swap(a,b);
      int e1, e2;
      if (g.triangulate_halfedge(a,b,e1,e2)) success++, add_cnt++;
    }
    else if (dis(rng) >= temperature && g.removable_edges.size()) { // removal probability
      pair<int, int> edge = sample_removable_edges();
      int a = edge.first;
      int b = edge.second;
      assert(g.adj[a].count(b)); assert(g.adj[b].count(a));
      if (g.can_remove(a, b)) {
        g.remove_edge(a, b, 1);
        success++, remove_cnt++;
      }
    }
    else {
      pair<int, int> edge = sample_good_edge();
      int a = edge.first;
      int b = edge.second;
      assert(g.adj[a].count(b)); assert(g.adj[b].count(a));
      if (rng()%2) swap(a,b);
      if (g.flip(a, b, 1)) success++, flip_cnt++;
      else if (rng()%2) {
        if (g.rot_cw(a, b, 1)) success++, flip_cnt++;
        else //rotate a bunch! we're stuck
					while (g.rot_ccw(a, b, 1)) {
						success++, flip_cnt++;
						if (rng()%8==0)
							break;
					}
      }
      else {
        if (g.rot_ccw(a, b, 1)) success++, flip_cnt++;
        else //rotate a bunch! we're stuck
					while (g.rot_cw(a, b, 1)) {
						success++, flip_cnt++;
						if (rng()%8==0)
							break;
					}
      }
    }
  }
  
  void anneal() {
    static const int64_t PRINT_ITER = 1e4; // NOTE SHOULD CHANGE THIS
    cerr << setprecision(3) << fixed;
    cerr << "ANNEALING FOR " << MAXIT << " iterations" <<endl;
    for(it=1;it<=MAXIT;it++) { // run some extra steps for safety
      if (it % PRINT_ITER == 0) {
        cerr << "IT = " << it << " TEMP = " << temperature << " SUCCESS% = " << (ld) success / PRINT_ITER<< " FLIP% = " << (ld) flip_cnt / PRINT_ITER <<
					" ADD% = " << (ld) add_cnt / PRINT_ITER << " REMOVE% = " << (ld) remove_cnt / PRINT_ITER << " Current edges: " << g.get_edge_num() << endl;
        flip_cnt = add_cnt = remove_cnt = success = 0;
      }
      update_temperature();
      anneal_step();
    }
  }
};
