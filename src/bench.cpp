#include <bits/stdc++.h>
const char nl = '\n';
using namespace std;
#include "Annealer.h"
#include "Graph.h"
#include "ThreadPool.h"

const unordered_set<string> horiz = {
  "euro-night-0040000",
  "us-night-0040000",
  "euro-night-0070000",
  "us-night-0080000",
  "euro-night-0080000",
  "us-night-0090000",
  "euro-night-0090000",
  "us-night-0100000",
  "euro-night-0100000"
};

const int T = 2*60*60; // time limit

mutex lk;

int iters[T];
int faces[T];
ld score[T];
void run(const string& filename) {
  graph g;
  g.read("../triangulations/"+filename+".tri");
  annealer ann(g, 1);
  auto get_score = [&]() {
    int c = g.chull_edges.size();
    int s = 3*(g.n-1) - c - g.get_edge_num();
    return (ld) s / (3*(g.n-1) - c);
  };
  //cerr << (ld) clock() / CLOCKS_PER_SEC << nl;
  for (int i = 0; i < T; i++) {
    ann.anneal();
    iters[i] = (i ? iters[i-1] : 0) + ann.it;
    faces[i] = 1+g.get_edge_num()-g.n;
    score[i] = get_score();
  }
  //cerr << (ld) clock() / CLOCKS_PER_SEC << nl;
  /*
  if (filename.find("rop") != string::npos) {
    g.read("../verticals/"+filename+".out");
    annealer ann(g, T/2);
    ann.anneal();
    graph h;
    h.read("../horizontals/"+filename+".out");
    annealer ann2(h, T/2);
    ann2.anneal();
    if (h.get_edge_num() < g.get_edge_num()) swap(g, h);
  } else {
    if (horiz.find(filename) != horiz.end()) {
      g.read("../horizontals/"+filename+".out");
    } else {
      g.read("../triangulations/"+filename+".tri");
    }
    annealer ann(g, T);
    ann.anneal();
  }*/
  graph h;
  h.read("../best/"+filename+".out");
  cerr << filename << " finished with " << 1+g.get_edge_num()-g.n
       << " vs best: " << 1+h.get_edge_num()-h.n << endl;
}

int main() {
  ifstream in("../base_names.txt");
  string filename = "ortho_rect_union_47381";
  run(filename);
  /*
  {
  thread_pool tp(61);
  vector<string> files;
  while (in >> filename) {
    //if (filename.find("rop") == string::npos && filename.find("paris") == string::npos && filename.find("euro") == string::npos) continue;
    //if (filename.find("rop") == string::npos && filename.find("ortho") == string::npos) continue;
    //if (filename.find("rop") == string::npos) continue;
    //files.push_back(filename);
    packaged_task<void()> p(bind(run, filename));
    tp.add(move(p));
    //noimprove[filename] = RESTART;
  }
  }*/
  //cout << fixed << setprecision(20) << score << endl;
  for (int i = 0; i < T; i++) {
    cout << iters[i] << " ";
  }
  cout << nl;
  for (int i = 0; i < T; i++) {
    cout << faces[i] << " ";
  }
  cout << nl;
  for (int i = 0; i < T; i++) {
    cout << fixed << setprecision(10) << score[i] << " ";
  }
  cout << nl;
}
