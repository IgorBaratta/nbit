#include "../src/sparse_set.hpp"
#include "catch2/catch.hpp"

TEST_CASE("Test insert sparse set")
{
    std::vector<std::int64_t> vec{1, 4, 2, 15, 25, 20, 1, 64};
    // create dynamic set and include vector

    nbit::sparse_set<16> set;

    REQUIRE(set.empty());
    set.insert(vec.begin(), vec.end());
    REQUIRE_FALSE(set.empty());

    // erase single value from set
    set.erase(15);

    // check number of unique values in the set
    REQUIRE(set.size() == 6);

    // decode bitset into a vector of ordered unique values
    std::vector<int> unique_values = set.decode<int>();
    REQUIRE((unique_values == std::vector<int>({1, 2, 4, 20, 25, 64})));
}

TEST_CASE("Create sparse bitset from container and clear and modify content")
{
    std::vector<std::int64_t> vec{1000004, 1000005};

    // create dynamic set and include vector
    nbit::sparse_set<2048> sparse_set(vec);

    sparse_set.insert(1000006);
    REQUIRE(sparse_set.size() == 3);
    REQUIRE(sparse_set.max_size() == 2048);

    sparse_set.insert(13);
    REQUIRE(sparse_set.max_size() == 4096);
    std::vector<int> unique_values = sparse_set.decode<int>();
    REQUIRE((unique_values == std::vector<int>({13, 1000004, 1000005, 1000006})));

    sparse_set.erase(13);
    REQUIRE(sparse_set.max_size() == 4096);
    sparse_set.shrink_to_fit();
    REQUIRE(sparse_set.max_size() == 2048);

    sparse_set.clear();

    /// decode sparse bitset into a vector, output vector should be empty
    unique_values = sparse_set.decode<int>();
    REQUIRE(unique_values.empty());
}

TEST_CASE("Test sparse set assignment and comparison")
{
    std::vector<std::int64_t> vec{1, 4, 2, 15, 25, 20, 1, 64};
    nbit::sparse_set set1(vec);
    nbit::sparse_set set2(set1);
    REQUIRE(set1 == set2);
    set2.erase(64);
    set2.shrink_to_fit();
    REQUIRE((set1 != set2));
}

TEST_CASE("Sparse set test operator&, intersection")
{
    nbit::sparse_set new_set{1, 5, 7, 42, 555, 1700};
    nbit::sparse_set new_set2{1, 5, 188};
    new_set &= new_set2;
    auto dec = new_set.decode<int>();
    REQUIRE(dec == std::vector<int>({1, 5}));
    nbit::sparse_set new_set3{1700};
    new_set3 &= new_set;
    REQUIRE(new_set3.empty());
}

TEST_CASE("Sparse set test operator|, union")
{
    nbit::sparse_set new_set{1};
    nbit::sparse_set new_set2{1, 5, 188};
    new_set |= new_set2;
    auto dec = new_set.decode<int>();
    REQUIRE(dec == std::vector<int>({1, 5, 188}));

    new_set2.insert(17);
    nbit::sparse_set new_set3{800};
    new_set2 |= new_set3;
    dec = new_set2.decode<int>();
    REQUIRE(dec == std::vector<int>({1, 5, 17, 188, 800}));

    nbit::sparse_set set1{2, 155};
    nbit::sparse_set set2{4, 16, 25};
    nbit::sparse_set set3 = set1 | set2;
    dec = set3.decode<int>();
    REQUIRE(dec == std::vector<int>({2, 4, 16, 25, 155}));
}

TEST_CASE("Sparse set test operator xor, symmetric difference")
{
    nbit::sparse_set set1{1};
    nbit::sparse_set set2{1, 2, 10000};
    set1 ^= set2;
    auto dec = set1.decode<int>();
    REQUIRE(dec == std::vector<int>({2, 10000}));

    nbit::sparse_set set3 = set1 ^ set2;
    set3.insert(5);
    dec = set3.decode<int>();
    REQUIRE(dec == std::vector<int>({1, 5}));
}

TEST_CASE("Sparse set test difference")
{
    nbit::sparse_set set1{1};
    nbit::sparse_set set2{1, 2, 10000};
    set1 ^= set2;
    auto dec = set1.decode<int>();
    REQUIRE(dec == std::vector<int>({2, 10000}));

    nbit::sparse_set set3 = set1 ^ set2;
    set3.insert(5);
    dec = set3.decode<int>();
    REQUIRE(dec == std::vector<int>({1, 5}));
}