#include "../src/nbit.hpp"
#include <cassert>
#include <iostream>
#include <vector>

int main()
{
    std::vector<std::int64_t> vec{1, 4, 2, 15, 25, 20, 1, 64};

    // create dynamic set and include vector
    nbit::set dynamic_set;
    dynamic_set.insert(vec.begin(), vec.end());

    // check number of unique values in the set
    assert(dynamic_set.size() == 7);

    // check for capacity without resizing
    assert(dynamic_set.max_size() == 128);

    // decode bitset into a vector of ordered unique values
    std::vector<int> unique_values = dynamic_set.decode<int>();

    assert((unique_values == std::vector<int>({1, 2, 4, 15, 20, 25, 64})));

    return 0;
}