#include <bits/stdc++.h>
using namespace std;
#include "Annealer.h"
#include "Graph.h"
#include "ThreadPool.h"
const int RESTART = 5;

mutex lk;
unordered_set<string> active;
unordered_map<string, int> noimprove;

bool can_restart(const string& filename) {
  return 0;
  return filename.find("rop") != string::npos;
}

void run(const string& filename) {
  {
    lock_guard<mutex> l(lk);
    if (active.count(filename)) return;
    active.insert(filename);
  }
  graph g;
  bool restart = 0;
  {
    lock_guard<mutex> l(lk);
    if (can_restart(filename) && noimprove[filename] >= RESTART) {
      cerr << "restarting " << filename << endl;
      noimprove[filename] = 0;
    }
  }
  if (restart) {
    g.read("../triangulations/"+filename+".tri");
  } else {
    g.read("../min_from_triangulation/"+filename+".out");
  }
  if (g.n <= 100) return; // file stays in active, so never runs
  int m = g.get_edge_num();
  annealer ann(g);
  ann.anneal();
  if (g.get_edge_num() < m) {
    cerr << filename << " started with " << 1+m-g.n << " faces and finished with " << 1+g.get_edge_num()-g.n;
    graph h;
    h.read("../best/"+filename+".out");
    if (h.get_edge_num() > g.get_edge_num()) {
      cerr << " new best" << endl;
      g.write("../best/"+filename+".out");
    } else cerr << endl;
    if (can_restart(filename)) {
      lock_guard<mutex> l(lk);
      noimprove[filename] = 0;
    }
  } else if (can_restart(filename)) {
    lock_guard<mutex> l(lk);
    noimprove[filename]++;
  }
  g.write("../min_from_triangulation/"+filename+".out");
  {
    lock_guard<mutex> l(lk);
    active.erase(filename);
  }
}

int main() {
  ifstream in("../big.txt");
  string filename = "euro-night-0010000";
  thread_pool tp(12);
  vector<string> files;
  while (in >> filename) {
    //if (filename.find("rop") == string::npos && filename.find("paris") == string::npos && filename.find("euro") == string::npos) continue;
    //if (filename.find("rop") != string::npos || filename.find("ortho") != string::npos) continue;
    files.push_back(filename);
  }
  while (1) {
    //if (filename < "us-night-0000010") continue;
    //if (filename.find("rop") == string::npos) continue;
    for (const auto& file : files) {
      {
        unique_lock l(tp.m);
        tp.qfull.wait(l, [&tp]() { return tp.q.size() < tp.QSZ; });
      }
      packaged_task<void()> p(bind(run, file));
      tp.add(move(p));
    }
  }
}
