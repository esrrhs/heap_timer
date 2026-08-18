# HeapTimer
C++ 4-ary heap timer, inspired by [Golang 4-ary heap timer](https://github.com/golang/go/blob/release-branch.go1.3/src/pkg/runtime/time.goc).
header only, no dependencies.

`HeapTimer` is not copyable (nodes are not shared across instances) and not thread-safe.
Deadlines use `std::chrono::steady_clock`.

## Usage
```cpp
HeapTimer t;

// add two timer with 1000ms
auto t1 = t.Add(1000);
auto t2 = t.Add(1000);

// delete the second timer
t.Del(t2); 

// wait for 1000ms
std::this_thread::sleep_for(std::chrono::milliseconds(1000));

// update the timer, return the expired timers t1
auto ret = t.Update();
```

## Build
```bash
mkdir build
cd build
cmake ..
make
```
