#include <bits/stdc++.h>
using namespace std;
#include "Annealer.h"
#include "Graph.h"

int main() {
  graph g;
  g.read("???");
  annealer ann(g);
  ann.anneal();
}
