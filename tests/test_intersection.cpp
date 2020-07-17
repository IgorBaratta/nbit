#include "../src/sparse_set.hpp"
#include "test_data.hpp"

#include <gtest/gtest.h>
#include <map>
#include <random>

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

    TYPED_TEST(BitSetTest, TestIntersectionEmpty)
    {
        // Copy empty bitset
        auto set1 = this->set;
        auto set2 = this->set;

        // Test intersection of empty sets
        EXPECT_EQ(set1, set2);
        auto set = set1 & set2;
        EXPECT_TRUE(set.empty());
    }

    TYPED_TEST(BitSetTest, TestIntersectionRandom)
    {

        // Copy empty bitset
        auto set1 = this->set;
        auto set2 = this->set;

        std::vector<int> v1 = generate_data<int>(2 * N, 10 * N);
        std::vector<int> v2 = generate_data<int>(2 * N, 5 * N);
        EXPECT_NE(v1, v2);

        // Test intersection of large random sets with different sizes
        std::vector<int> v_intersection;
        std::sort(v1.begin(), v1.end());
        std::sort(v2.begin(), v2.end());
        std::set_intersection(v1.begin(), v1.end(),
                              v2.begin(), v2.end(),
                              std::back_inserter(v_intersection));

        v_intersection.erase(std::unique(v_intersection.begin(), v_intersection.end()),
                             v_intersection.end());

        set1.insert_sorted(v1.begin(), v1.end());
        set2.insert_sorted(v2.begin(), v2.end());
        auto set = set1 & set2;
        auto decode = set.template decode<int>();

        EXPECT_EQ(decode, v_intersection);

        set.clear();
        EXPECT_FALSE(set1.empty());
        EXPECT_FALSE(set2.empty());
        EXPECT_TRUE(set.empty());

        // intersection of non-empty with an empty set is also an empty set
        set1 &= set;
        EXPECT_TRUE(set1.empty());
        auto new_set = set1 & set;

        EXPECT_TRUE(new_set.empty());
        new_set = set1 & set2;
        EXPECT_TRUE(new_set.empty());
    }

    TYPED_TEST(BitSetTest, TestIntersectionSelf)
    {
        std::vector<int> vec{100'000, 288, 925, 10};

        // Copy empty bitset
        auto set1 = this->set;
        set1.insert(vec.begin(), vec.end());

        auto set2 = set1 & set1;
        EXPECT_EQ(set1, set2);

        set2 &= set1;
        EXPECT_EQ(set1, set2);

        auto decode = set1.template decode<int>();

        std::sort(vec.begin(), vec.end());
        EXPECT_EQ(decode, vec);
    }

} // namespace
