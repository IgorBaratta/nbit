#include "../src/sparse_set.hpp"
#include "test_data.hpp"

#include <gtest/gtest.h>
#include <map>
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

    TYPED_TEST(BitSetTest, TestInsert)
    {
        std::vector<std::uint64_t> vec = generate_data(N, 10 * N);
        auto &set = this->set;

        std::unordered_set<std::uint64_t> verification_set;
        verification_set.insert(vec.begin(), vec.end());
        int set_size = verification_set.size();

        // bit sets are initially empty
        EXPECT_TRUE(set.empty());

        set.insert(vec.begin(), vec.end());
        EXPECT_EQ(set.size(), set_size);

        // inserting repeatead values does not change the bitset size
        set.insert(vec.begin(), vec.end());
        EXPECT_EQ(set.count(), set_size);

        // erase single value from set
        set.erase(*std::max_element(vec.begin(), vec.end()));
        EXPECT_EQ(set.count(), set_size - 1);

        EXPECT_EQ(set.minimum(), *std::min_element(vec.begin(), vec.end()));
        EXPECT_NE(set.maximum(), *std::max_element(vec.begin(), vec.end()));

        // insert the value that has been removed
        set.insert(*std::max_element(vec.begin(), vec.end()));

        set.clear();
        EXPECT_TRUE(set.empty());
        EXPECT_EQ(set.size(), 0);

        EXPECT_EQ(set.maximum(), NBIT_UNDEFINED);
        EXPECT_EQ(set.minimum(), NBIT_UNDEFINED);

        vec.clear();
        EXPECT_TRUE(vec.empty());

        set.insert(vec.begin(), vec.end());
        EXPECT_TRUE(set.empty());
        EXPECT_EQ(set.size(), 0);
    }

    TYPED_TEST(BitSetTest, TestConstructos)
    {
        auto set = this->set;

        /// Initializer_list
        decltype(set) new_set{100, 255, 3, 4000};

        /// Copy constructor
        decltype(set) new_set1(new_set);

        /// Move constructor
        decltype(set) new_set2 = decltype(set)(new_set);

        EXPECT_EQ(new_set, new_set1);
        EXPECT_EQ(new_set, new_set2);
    }

} // namespace
