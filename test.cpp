#include <chrono>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "heap_timer.h"

#define ASSERT(x) \
    if (!(x)) { \
        std::cout << "Assertion failed " << #x << " on line " << __LINE__ << std::endl; \
        exit(1); \
    }

int test() {
    HeapTimer t;
    auto t1 = t.AddAt(HeapTimer::Clock::now() + std::chrono::milliseconds(1));
    auto t2 = t.AddAt(HeapTimer::Clock::now() + std::chrono::milliseconds(2));
    auto t3 = t.AddAt(HeapTimer::Clock::now() + std::chrono::hours(1));
    ASSERT(t.Size() == 3);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto ret = t.Update();
    ASSERT(ret.size() == 2);
    ASSERT(ret[0] == t1);
    ASSERT(ret[1] == t2);
    ASSERT(t.Size() == 1);

    auto ok = t.Del(t3);
    ASSERT(ok);
    ASSERT(t.Size() == 0);
    ASSERT(!t.Del(t3));
    ret = t.Update();
    ASSERT(ret.size() == 0);

    HeapTimer expired;
    auto z1 = expired.Add(0);
    auto z2 = expired.Add(0);
    auto z3 = expired.Add(0);
    ASSERT(expired.Del(z2));
    auto fired = expired.Update();
    ASSERT(fired.size() == 2);
    ASSERT(fired[0] == z1 || fired[0] == z3);
    ASSERT(expired.Size() == 0);

    HeapTimer order;
    const auto base = HeapTimer::Clock::now();
    std::vector<HeapTimer::TimerId> ids;
    for (int i = 0; i < 20; i++) {
        ids.push_back(order.AddAt(base + std::chrono::milliseconds(20 - i)));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    auto ordered = order.Update();
    ASSERT(ordered.size() == 20);
    for (int i = 0; i < 20; i++) {
        ASSERT(ordered[i] == ids[19 - i]);
    }

    HeapTimer src;
    auto id = src.Add(10000);
    HeapTimer dst = std::move(src);
    ASSERT(src.Size() == 0);
    ASSERT(dst.Size() == 1);
    ASSERT(!src.Del(id));
    ASSERT(dst.Del(id));
    ASSERT(dst.Size() == 0);

    auto after_move = src.Update();
    ASSERT(after_move.empty());
    ASSERT(src.Size() == 0);

    return 0;
}

int benchmark() {
    HeapTimer t;
    std::unordered_set<HeapTimer::TimerId> ids;

    auto begin = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000000; i++) {
        auto id = t.Add(1000);
        ids.insert(id);
    }
    ASSERT(t.Size() == 1000000);
    std::cout << "add 1000000 timers cost " << std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - begin).count() << "ms" << std::endl;

    HeapTimer del_bench;
    std::vector<HeapTimer::TimerId> del_ids;
    del_ids.reserve(1000000);
    for (int i = 0; i < 1000000; i++) {
        del_ids.push_back(del_bench.Add(1000));
    }
    begin = std::chrono::steady_clock::now();
    for (auto id: del_ids) {
        auto ok = del_bench.Del(id);
        ASSERT(ok);
    }
    ASSERT(del_bench.Size() == 0);
    std::cout << "del 1000000 timers cost " << std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - begin).count() << "ms" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    begin = std::chrono::steady_clock::now();
    auto ret = t.Update();
    std::cout << "update cost " << std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - begin).count() << "ms" << std::endl;

    for (auto id: ret) {
        ASSERT(ids.find(id) != ids.end());
        ids.erase(id);
    }
    ASSERT(ids.empty());
    ASSERT(t.Size() == 0);

    std::unordered_map<HeapTimer::TimerId, std::chrono::steady_clock::time_point> id_time;
    int count = 0;
    std::map<int64_t, int> diffs;
    while (true) {
        if (count >= 100000) {
            if (id_time.empty()) {
                break;
            }
        } else {
            auto random_time = rand() % 100000 + 100;
            auto now = std::chrono::steady_clock::now();
            auto id = t.Add(random_time);
            id_time[id] = now + std::chrono::milliseconds(random_time);
        }
        count = count + 1;

        auto expired = t.Update();
        auto now = std::chrono::steady_clock::now();
        for (auto id: expired) {
            auto it = id_time.find(id);
            ASSERT(it != id_time.end());
            auto expect_time_ms = it->second;
            auto diff = now - expect_time_ms;
            diffs[std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()]++;
            if (id % 10000 == 0) {
                std::cout << "tid " << id << " diff " << std::chrono::duration_cast<std::chrono::milliseconds>(diff)
                        .count() << "ms" << std::endl;
            }
            ASSERT(now >= expect_time_ms);
            id_time.erase(it);
        }
    }

    for (auto& it: diffs) {
        std::cout << "diff " << it.first << "ms count " << it.second << std::endl;
    }

    std::cout << "benchmark passed" << std::endl;
    return 0;
}

int main(int argc, char** argv) {
    test();
    std::cout << "unit tests passed" << std::endl;

    // Default CI/smoke path skips the long-running benchmark.
    // Run: ./heap_timer_test benchmark
    if (argc > 1 && std::string(argv[1]) == "benchmark") {
        return benchmark();
    }
    return 0;
}
