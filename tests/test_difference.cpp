#include "../src/sparse_set.hpp"
#include "test_data.hpp"

#include <gtest/gtest.h>
#include <map>
#include <random>
#include <set>

namespace
{
    using testing::Types;

    constexpr std::size_t N = 65536;
    using map = std::map<std::uint64_t, nbit::fixed_set<N>>;

    template <class T>
    class BitSetTest : public testing::Test
    {
    protected:
        BitSetTest() = default;
        ~BitSetTest() override = default;
        T set;
    };

    typedef Types<nbit::set<true>,
                  nbit::sparse_set<N, map>,
                  nbit::sparse_set<1024>,
                  nbit::sparse_set<2048>,
                  nbit::sparse_set<4096>,
                  nbit::sparse_set<65536>>
        Implementations;

    TYPED_TEST_SUITE(BitSetTest, Implementations);

    TYPED_TEST(BitSetTest, TestDifferenceEmpty)
    {
        // Copy empty bitset
        auto set1 = this->set;
        auto set2 = this->set;

        // Test intersection of empty sets
        EXPECT_EQ(set1, set2);
        auto set = set1 - set2;
        EXPECT_TRUE(set.empty());

        set1.insert(3);
        set = set1 - set2;
        EXPECT_EQ(set, set1);

        set = set2 - set1;
        EXPECT_TRUE(set.empty());
    }

    TYPED_TEST(BitSetTest, TestDifferenceRandom)
    {

        // Copy empty bitset
        auto set1 = this->set;
        auto set2 = this->set;

        std::vector<int> v1 = generate_data<int>(2 * N, 10 * N);
        std::vector<int> v2 = generate_data<int>(2 * N, 5 * N);
        EXPECT_NE(v1, v2);

        // Test intersection of large random sets with different sizes
        std::set<int> s1(v1.begin(), v1.end());
        std::set<int> s2(v2.begin(), v2.end());
        std::vector<int> v_difference;
        std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                            std::back_inserter(v_difference));

        set1.insert(v1.begin(), v1.end());
        set2.insert(v2.begin(), v2.end());
        auto set = set1 - set2;
        auto decode = set.template decode<int>();

        EXPECT_EQ(decode, v_difference);

        auto set3 = set2 - set1;
        EXPECT_NE(set, set3);
    }
} // namespace
