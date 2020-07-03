#include "../src/set.hpp"
#include "catch2/catch.hpp"

TEST_CASE("Test fixed size bit sets")
{
    std::vector<std::int64_t> vec{1, 4, 2, 15, 25, 20, 1, 64};

    // create fixed size set and include vector
    nbit::fixed_set<1024> set;

    REQUIRE(set.empty());
    set.insert(vec.begin(), vec.end());
    REQUIRE_FALSE(set.empty());

    // erase single value from set
    set.erase(15);

    // check number of unique values in the set
    REQUIRE(set.size() == 6);
    // check for capacity without resizing
    REQUIRE(set.max_size() == 1024);

    // decode bitset into a vector of sorted unique values
    std::vector<int> unique_values = set.decode<int>();
    REQUIRE((unique_values == std::vector<int>({1, 2, 4, 20, 25, 64})));
}