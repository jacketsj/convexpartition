#include<bits/stdc++.h>
#include "Graph.h"

using namespace std;

const int INF = 1e9;

map<int, vector<int>> xs;

void triangulate_upper(graph& g, vector<int>& v) {
  vector<int> ch(v.size());
  int top = 0, bot = 1;
  for(int i=0;i<v.size();i++) {
    while(top > bot && cp(g.points[ch[top-1]]-g.points[ch[top-2]], g.points[v[i]]-g.points[ch[top-2]]) <= 0) {
      g.add_edge(v[i], ch[top-1]);
      top--;
    }
    if (top) g.add_edge(v[i], ch[top-1]);
    ch[top++] = v[i];
  }
}

int main() {
  ifstream basenames("../base_names.txt");
  string base;
  while(basenames >> base) {
    cerr << "Vertifying " << base << endl;
    graph g;
    g.read("../in/"+base+".in");
    for(auto &[i, x, y]: g.points) {
      xs[x].push_back(i);
    }
    vector<int> upper;
    vector<int> lower;
    for(auto &[x, v]: xs) {
      sort(v.begin(), v.end(), [&g](int i, int j) {
            return g.points[i].y < g.points[j].y;
          });
      int last = -1;
      for(int i: v) {
        if (last != -1) g.add_edge(i, last);
        last = i;
      }
      upper.push_back(v.back());
      lower.push_back(v.front());
    }
    swap(upper, lower);
    triangulate_upper(g, upper);
    reverse(lower.begin(), lower.end());
    triangulate_upper(g, lower);
    g.print_matlab();
    g.write("../verticals/" + base + ".out");
    return 0;
  }
}
