#include "../src/nbit.hpp"
#include <cassert>
#include <iostream>
#include <numeric>
#include <omp.h>
#include <unordered_set>
#include <vector>

int main()
{
    std::size_t sz = 1UL << 16;
    std::size_t cc = 10000;
    std::vector<std::int64_t> vec(sz);
    std::iota(vec.begin(), vec.end(), 0);

    // create fixed size bit set supporting integers up to 1024

    double start = omp_get_wtime();
    nbit::nset<1UL << 16> fixed_bitset;
    for (size_t i = 0; i < cc; i++)
        fixed_bitset.insert(vec.begin(), vec.end());
    double elapsed = omp_get_wtime() - start;
    std::cout << elapsed;

    start = omp_get_wtime();
    nbit::set dynamic_bitset;
    for (size_t i = 0; i < cc; i++)
        dynamic_bitset.insert(vec.begin(), vec.end());
    elapsed = omp_get_wtime() - start;
    std::cout << "\n"
              << elapsed;

    start = omp_get_wtime();
    std::unordered_set<int> set;
    for (size_t i = 0; i < cc; i++)
        set.insert(vec.begin(), vec.end());
    elapsed = omp_get_wtime() - start;
    std::cout << "\n"
              << elapsed << "\n";

    return 0;
}