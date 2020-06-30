// Let Catch provide main():
#define CATCH_CONFIG_MAIN

#include "../src/nbit.hpp"
#include "catch.hpp"

TEST_CASE("Test insert repeated")
{
  nbit::set set(1000);
  set.insert(3);
  set.insert(2);
  set.insert(3);
  REQUIRE(set.size() == 2);

  nbit::nset<16> new_set;
  new_set.insert(3);
  new_set.insert(2);
  new_set.insert(3);
  REQUIRE(new_set.size() == 2);

  REQUIRE(new_set.max_size() == 16);
}