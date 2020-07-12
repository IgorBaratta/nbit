#include "../src/sparse_set.hpp"
#include <cassert>
#include <chrono>
#include <iostream>
#include <set>
#include <unordered_map>

int main()
{

    std::vector<int> vec(1000);
    std::iota(vec.begin(), vec.end(), 0);

    constexpr std::size_t N = 256;
    using mymap = std::unordered_map<std::uint64_t, nbit::fixed_set<N>>;
    nbit::sparse_set<N, mymap> set1;

    set1.insert(vec.begin(), vec.end());
}