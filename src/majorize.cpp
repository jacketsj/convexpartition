#include<bits/stdc++.h>
#include "Graph.h"
using namespace std;
typedef long long ll;

void triangulate_upper(graph &g, vector<pt> &points, vector<int>& v) {
  vector<int> ch(v.size());
  int top = 0, bot = 1;
  for(int i=0;i<v.size();i++) {
    while(top > bot && cp(points[ch[top-1]]-points[ch[top-2]], points[v[i]]-points[ch[top-2]]) <= 0) {
      g.add_edge(v[i], ch[top-1]);
      top--;
    }
    if (top) g.add_edge(v[i], ch[top-1]);
    ch[top++] = v[i];
  }
}

pair<ll, ll> reduce(ll num, ll dem) {
  // assume not 0 / 0
  if (dem == 0) num = 1;
  else {
    if (dem < 0) dem*=-1, num*=-1;
    ll g = gcd(abs(num),abs(dem));
    if (!g) {
      assert(g);
    }
    num/=g;
    dem/=g;
  }
  return {num,dem};
}

mt19937 rng(0x3f);

pair<int, int> find_slope(vector<pt> &v) {
  int IT = 1e7;
  int n = v.size();
  map<pair<int, int>, int> slopes; //change this to unordered map (hash possibly negative numbers)
  for(int it=0;it<IT;it++) {
    int id1 = rng()%n;
    int id2 = rng()%n;
    if (id1==id2) continue;
    ll num = v[id1].x - v[id2].x;
    ll dem = v[id1].y - v[id2].y;
    slopes[reduce(num, dem)]++;
  }
  vector<pair<int, pair<int,int>>> vcnt;
  for(auto [h, c]: slopes) {
    vcnt.emplace_back(c,h);
  }
  sort(vcnt.rbegin(), vcnt.rend());
  int id = 0;
  // this following line skips horizontal and vertical lines
  while(id<v.size() && ((vcnt[id].second.first == 0) || (vcnt[id].second.second ==0))) id++;
  cerr << "Found occ of: " << vcnt[id].first << " ";
  return vcnt[id].second;
}

int main() {
  ifstream basenames("../base_names.txt");
  string base;
  while(basenames >> base) {
    graph g;
    if (base.find("paris") == string::npos) continue;
    g.read("../in/"+base+".in");
    vector<pt> points = g.points; // make a copy (this is necessary)
    assert(points.size());
    pair<ll, ll> slope = find_slope(points);
    ll a = slope.first;
    ll b = slope.second;
    cerr << "Majorize " << base << " WITH RESPECT TO SLOPE " << a << " / " << b << endl;
    for(auto &[i, x, y]: points) {
      // transform points with same slope {a, b} to have same x
      // rotation
      // suppose (x1 - x2)*b = (y1 - y2)*a
      // Then x1*b - y1*a = x2*b - y2*a
      // x <- xb - ya
      // y <- xa + yb
      ll newx = x*b - y*a;
      ll newy = x*a + y*b;
      tie(x,y) = tie(newx, newy);
    }
    map<int, vector<int>> xs;
    for(auto &[i, x, y]: points) {
      xs[x].push_back(i);
    }
    vector<int> upper;
    vector<int> lower;
    for(auto &[x, v]: xs) {
      sort(v.begin(), v.end(), [&points](int i, int j) {
            return points[i].y < points[j].y;
          });
      int last = -1;
      for(int i: v) {
        if (last != -1) g.add_edge(i, last);
        last = i;
      }
      upper.push_back(v.back());
      lower.push_back(v.front());
    }
    //g.print_matlab();
    swap(upper, lower);
    triangulate_upper(g, points, upper);
    reverse(lower.begin(), lower.end());
    triangulate_upper(g, points, lower);
    g.print_matlab();
    g.write("../majorized/" + base + "_" + to_string(a) + "-" + to_string(b) + ".out");
    return 0;
  }
}


