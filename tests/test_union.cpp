#include "../src/sparse_set.hpp"
#include "test_data.hpp"

#include <algorithm>
#include <gtest/gtest.h>
#include <map>
#include <random>
#include <unordered_map>
#include <unordered_set>

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

    /// Test union of empty sets
    TYPED_TEST(BitSetTest, TestUnionEmpty)
    {
        // Copy empty bitset
        auto set1 = this->set;
        auto set2 = this->set;

        // Test intersection of empty sets
        EXPECT_EQ(set1, set2);
        auto set = set1 | set2;
        EXPECT_TRUE(set.empty());

        set.insert(10);
        EXPECT_EQ(set.size(), 1);

        set1 = set; //copy non-empty set to set1
        set |= set2;
        EXPECT_EQ(set.size(), 1);
        EXPECT_EQ(set, set1);

        auto vec = set2.template decode<int>();
        EXPECT_TRUE(vec.empty());
    }

    TYPED_TEST(BitSetTest, TestUnionSimple)
    {
        // Copy empty bitset
        auto set1 = this->set;
        auto set2 = this->set;

        set1.insert(10);
        set1.insert(100);
        set1.insert(1'000);

        set2.insert(100'000);
        set2.insert(1'000'000);

        auto set = set1 | set2;
        auto dec = set.template decode<int>();
        EXPECT_EQ(dec, std::vector<int>({10, 100, 1'000, 100'000, 1'000'000}));

        set &= set2;

        EXPECT_EQ(set.template decode<int>(), set2.template decode<int>());
    }

    TYPED_TEST(BitSetTest, TestIntersectionRandom)
    {
        // Copy empty bitset
        auto set1 = this->set;
        auto set2 = this->set;

        std::vector<std::uint32_t> v1 = generate_data<std::uint32_t>(N, 10 * N);
        std::vector<std::uint32_t> v2 = generate_data<std::uint32_t>(2 * N, 5 * N);

        EXPECT_NE(v1, v2);

        // Test union of "large" random sets with different sizes
        std::vector<std::uint32_t> v_union;
        std::sort(v1.begin(), v1.end());
        std::sort(v2.begin(), v2.end());

        std::set_union(v1.begin(), v1.end(),
                       v2.begin(), v2.end(),
                       std::back_inserter(v_union));

        v_union.erase(std::unique(v_union.begin(), v_union.end()),
                      v_union.end());

        set1.insert_sorted(v1.begin(), v1.end());
        set2.insert_sorted(v2.begin(), v2.end());

        auto union_set = set1 | set2;
        auto decode_union = union_set.template decode<std::uint32_t>();

        EXPECT_EQ(decode_union, v_union);

        union_set.clear();
        EXPECT_FALSE(set1.empty());
        EXPECT_FALSE(set2.empty());
        EXPECT_TRUE(union_set.empty());
    }

} // namespace
