# nbit


![C++ CI](https://github.com/IgorBaratta/nbit/workflows/C++%20CI/badge.svg)
![CI Docker images](https://img.shields.io/docker/cloud/build/igorbaratta/nbit)

## Overview 
_nbit_ fast C++ 17 header-only (compressed) bit-set library.

## Installation
 - Just copy _set.hpp_ (and _sparse_set.hpp_) to your C++ project.

## Tests
Running the tests is simple, and should take approximately 1 minute:

```bash
mkdir build && cd build
cmake ..
make
make test
```
Requires [Google Test](https://github.com/google/googletest).

## Benchmarks

```bash
mkdir build && cd build
cmake ..
make
```
There are currently main 3 benchmark:
```
./benchmarks/insert
./benchmarks/intersect
./benchmarks/union
```
that can be filtered by input data or size:

| Benchmark   |      CPU      |  Iterations |
|----------|-------------:|------:|
| nbit::set< true>/1000000/1 |   5248268 ns | 125 |
| std::set<uint64_t>/1000000/1|    827615144 ns   |   1 |



Requires [googletest](https://github.com/google/googletest) and [google/benchmark](https://github.com/google/benchmark).


## Example

```c++
#include "../src/set.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <vector>

int main()
{
    std::uint64_t offset = 10'000'000'000;

    // Create vector and s
    std::vector<std::int64_t> vec(64);
    std::iota(vec.begin(), vec.end(), offset);
    std::random_shuffle(vec.begin(), vec.end());

    nbit::set<true> dynamic_set;
    auto input_map = [=](std::uint64_t a) -> std::uint64_t { return a - offset; };
    auto output_map = [=](std::uint64_t a) -> std::uint64_t { return a + offset; };

    for (auto value : vec)
        dynamic_set.insert(value, input_map);

    auto sorted_vec = dynamic_set.decode<std::uint64_t>(output_map);
    assert(offset == sorted_vec[0]);
}

```
