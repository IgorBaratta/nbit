#include "../src/set.hpp"
#include "catch2/catch.hpp"

TEST_CASE("Test insert dynamic resizing")
{
  std::vector<std::int64_t> vec{1, 4, 2, 15, 25, 20, 1, 64};

  // create dynamic set and include vector
#if __cplusplus >= 201703L
  nbit::set dynamic_set;
#else
  nbit::set<true> dynamic_set;
#endif

  REQUIRE(dynamic_set.empty());
  dynamic_set.insert(vec.begin(), vec.end());
  REQUIRE_FALSE(dynamic_set.empty());

  // erase single value from set
  dynamic_set.erase(15);

  // check number of unique values in the set
  REQUIRE(dynamic_set.size() == 6);
  // check for capacity without resizing
  REQUIRE(dynamic_set.max_size() == 128);

  // decode bitset into a vector of ordered unique values
  std::vector<int> unique_values = dynamic_set.decode<int>();
  REQUIRE((unique_values == std::vector<int>({1, 2, 4, 20, 25, 64})));
}

TEST_CASE("Create bitset from container and clear and modify content")
{
  std::vector<std::int64_t> vec{1, 4, 2, 15, 25, 20, 1, 64};

  // create dynamic set and include vector
#if __cplusplus >= 201703L
  nbit::set dynamic_set(vec);
#else
  nbit::set<true> dynamic_set(vec);
#endif

  dynamic_set.insert(1023);
  REQUIRE(dynamic_set.size() == 8);
  REQUIRE(dynamic_set.max_size() == 1024);

  dynamic_set.clear();
  REQUIRE(dynamic_set.size() == 0);
  REQUIRE(dynamic_set.max_size() == 1024);
  REQUIRE(dynamic_set.empty());

  dynamic_set.resize(2047);
  REQUIRE(dynamic_set.max_size() == 2048);

  dynamic_set.insert(13);
  dynamic_set.shrink_to_fit();
  REQUIRE(dynamic_set.max_size() == 64);

  // decode bitset into a vector, output vector should be empty
  std::vector<int> unique_values = dynamic_set.decode<int>();
  REQUIRE((unique_values == std::vector<int>({13})));
}

TEST_CASE("Test assignment and comparison")
{
  std::vector<std::int64_t> vec{1, 4, 2, 15, 25, 20, 1, 64};
  nbit::set set1(vec);
  nbit::set set2(set1);
  REQUIRE(set1 == set2);
  set2.erase(64);
  set2.shrink_to_fit();
  REQUIRE((set1 != set2));
}