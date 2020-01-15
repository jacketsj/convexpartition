#include <bits/stdc++.h>
using namespace std;
#include "Annealer.h"
#include "Graph.h"

int main() {
  graph g;
  g.read("../in/stars-0000010.in");
  annealer ann(g);
  ann.anneal();
  g.write("../out/stars-0000010.out");
}
