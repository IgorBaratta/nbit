#include "../src/sparse_set.hpp"
#include <gtest/gtest.h>
#include <map>
#include <unordered_map>

namespace
{

    template <class T>
    class BitSetTest : public testing::Test
    {
    protected:
        BitSetTest() = default;
        ~BitSetTest() override = default;
        T set;
    };

    using testing::Types;

    typedef Types<nbit::set<true>,
                  nbit::fixed_set<1024>,
                  nbit::fixed_set<2048>,
                  nbit::fixed_set<4096>,
                  nbit::fixed_set<65536>,
                  nbit::sparse_set<1024>,
                  nbit::sparse_set<2048>,
                  nbit::sparse_set<4096>,
                  nbit::sparse_set<65536>>
        Implementations;

    TYPED_TEST_SUITE(BitSetTest, Implementations);

    TYPED_TEST(BitSetTest, TestInsertAndCount)
    {
        std::vector<int> vec{1, 2, 17, 56, 899};
        int inital_size = vec.size();

        // bit sets are initially empty
        EXPECT_TRUE(this->set.empty());

        this->set.insert(vec.begin(), vec.end());
        EXPECT_EQ(this->set.size(), inital_size);

        // inserting repeatead values does not change the bitset size
        this->set.insert(vec.begin(), vec.end());
        EXPECT_EQ(this->set.count(), inital_size);

        // erase single value from set
        this->set.erase(17);
        EXPECT_EQ(this->set.count(), inital_size - 1);

        EXPECT_EQ(this->set.minimum(), 1);
        EXPECT_EQ(this->set.maximum(), 899);

        this->set.clear();
        EXPECT_TRUE(this->set.empty());
        EXPECT_EQ(this->set.size(), 0);

        EXPECT_EQ(this->set.maximum(), NBIT_UNDEFINED);
        EXPECT_EQ(this->set.minimum(), NBIT_UNDEFINED);
    }

} // namespace
