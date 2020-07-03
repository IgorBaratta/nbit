# nbit
A fast C++ 17 header-only (compressed) bit-set library.

~~~~~~~~~~~~~{.cpp}
#include "../src/set.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <vector>

int main()
{
    std::uint64_t offset = 10000;

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

~~~~~~~~~~~~~
