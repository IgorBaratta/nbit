#include "../src/set.hpp"
#include "catch2/catch.hpp"

TEST_CASE("Test insert dynamic resizing")
{
  std::vector<std::int64_t> vec{1, 4, 2, 15, 25, 20, 1, 64};

  // create dynamic set and include vector
  nbit::set dynamic_set;

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
  nbit::set dynamic_set(vec);

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

TEST_CASE("Test operator&, intersection")
{
  nbit::set new_set{1, 5, 7, 42, 555, 1700};
  nbit::set new_set2{1, 5, 188};
  new_set &= new_set2;
  auto dec = new_set.decode<int>();
  REQUIRE(dec == std::vector<int>({1, 5}));
}

TEST_CASE("Test operator|, union")
{
  nbit::set new_set{1};
  nbit::set new_set2{1, 5, 188};
  new_set |= new_set2;
  auto dec = new_set.decode<int>();
  REQUIRE(dec == std::vector<int>({1, 5, 188}));

  new_set2.insert(17);
  nbit::set new_set3{800};
  new_set2 |= new_set3;
  dec = new_set2.decode<int>();
  REQUIRE(dec == std::vector<int>({1, 5, 17, 188, 800}));

  nbit::set set1{2, 155};
  nbit::set set2{4, 16, 25};
  nbit::set set3 = set1 | set2;
  dec = set3.decode<int>();
  REQUIRE(dec == std::vector<int>({2, 4, 16, 25, 155}));
}

TEST_CASE("Test operator xor")
{
  nbit::set set1{1};
  nbit::set set2{1, 2, 10000};
  set1 ^= set2;
  auto dec = set1.decode<int>();
  REQUIRE(dec == std::vector<int>({2, 10000}));

  nbit::set set3 = set1 ^ set2;
  set3.insert(5);
  dec = set3.decode<int>();
  REQUIRE(dec == std::vector<int>({1, 5}));
}