#pragma once

#include <cassert>
#include <stdint.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstddef>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <memory>

class HeapTimer {
public:
    HeapTimer() = default;

    ~HeapTimer() = default;

    uint32_t Add(uint32_t delay_ms) {
        auto now = std::chrono::system_clock::now();
        if (heap_size_ >= static_cast<int>(heap_.size())) {
            // Grow slice.
            size_t n = 16;
            if (n <= heap_.size()) {
                n = heap_.size() * 3 / 2;
            }
            heap_.resize(n);
        }
        auto t = std::make_shared<TimerNode>();
        t->i = heap_size_++;
        t->when = now + std::chrono::milliseconds(delay_ms);
        t->id = timer_id_++;
        heap_[t->i] = t;
        SiftUp(t->i);
        timer_map_[t->id] = t;
        return t->id;
    }

    bool Del(uint32_t id) {
        auto it = timer_map_.find(id);
        if (it == timer_map_.end()) {
            return false;
        }
        auto t = it->second;
        timer_map_.erase(it);

        auto i = t->i;
        if (i < 0 || i >= heap_size_ || heap_[i] != t) {
            return false;
        }

        heap_size_--;
        if (i == heap_size_) {
            heap_[i] = nullptr;
        } else {
            heap_[i] = heap_[heap_size_];
            heap_[heap_size_] = nullptr;
            heap_[i]->i = i;
            SiftUp(i);
            SiftDown(i);
        }

        return true;
    }

    std::vector<uint32_t> Update() {
        auto now = std::chrono::system_clock::now();
        std::vector<uint32_t> ret;
        while (true) {
            if (heap_size_ == 0) {
                break;
            }

            auto t = heap_[0];
            if (t->when > now) {
                break;
            }

            // remove from heap
            heap_[0] = heap_[--heap_size_];
            heap_[0]->i = 0;
            SiftDown(0);

            t->i = -1; // mark as removed
            timer_map_.erase(t->id);

            ret.push_back(t->id);
        }

        return ret;
    }

    size_t Size() const {
        return heap_size_;
    }

private:
    void SiftUp(int32_t i) {
        auto when = heap_[i]->when;
        auto tmp = heap_[i];
        while (i > 0) {
            auto p = (i - 1) / 4; // parent
            if (when >= heap_[p]->when) {
                break;
            }
            heap_[i] = heap_[p];
            heap_[i]->i = i;
            heap_[p] = tmp;
            tmp->i = p;
            i = p;
        }
    }

    void SiftDown(int32_t i) {
        auto when = heap_[i]->when;
        auto tmp = heap_[i];
        while (true) {
            auto c = i * 4 + 1; // left child
            auto c3 = c + 2; // mid child
            if (c >= heap_size_) {
                break;
            }
            auto w = heap_[c]->when;
            if (c + 1 < heap_size_ && heap_[c + 1]->when < w) {
                w = heap_[c + 1]->when;
                c++;
            }
            if (c3 < heap_size_) {
                auto w3 = heap_[c3]->when;
                if (c3 + 1 < heap_size_ && heap_[c3 + 1]->when < w3) {
                    w3 = heap_[c3 + 1]->when;
                    c3++;
                }
                if (w3 < w) {
                    w = w3;
                    c = c3;
                }
            }
            if (w >= when) {
                break;
            }
            heap_[i] = heap_[c];
            heap_[i]->i = i;
            heap_[c] = tmp;
            tmp->i = c;
            i = c;
        }
    }

private:
    struct TimerNode {
        std::chrono::time_point<std::chrono::system_clock> when;
        uint32_t id = 0;
        int32_t i = 0;
    };

    typedef std::shared_ptr<TimerNode> TimerNodePtr;
    uint32_t timer_id_ = 0;
    std::vector<TimerNodePtr> heap_;
    int heap_size_ = 0;
    std::unordered_map<uint32_t, TimerNodePtr> timer_map_;
};
