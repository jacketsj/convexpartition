#pragma once

#include <bits/stdc++.h>
using namespace std;

struct thread_pool {
  static const int THREAD = 60;
	static const int SZ = THREAD;
	thread ts[SZ];
	queue<packaged_task<void()>> q;
	mutex m;
  condition_variable qempty;
	bool active;
	thread_pool() {
		active = 1;
		auto loop = [&](const int i) {
			while (active) {
				bool has = 0;
				packaged_task<void()> p;
				{
					lock_guard<mutex> l(m);
					if (!q.empty()) {
						has = 1;
						p = move(q.front());
						q.pop();
            qempty.notify_one();
					}
				}
				if (has) p();
				else this_thread::yield();
			}
		};
		for (int i = 0; i < SZ; i++) {
			ts[i] = thread(bind(loop, i));
		}
	}
	void add(packaged_task<void()>&& p) {
		lock_guard<mutex> l(m);
		q.push(move(p));
	}
	~thread_pool() {
    {
      unique_lock l(m);
      qempty.wait(l, [this]() { return q.empty(); });
    }
		active = 0;
		for (auto& t : ts) {
			if (t.joinable()) t.join();
		}
	}
};

