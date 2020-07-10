// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2020 Igor Baratta
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <numeric>
#include <type_traits>
#include <vector>

#define NBIT_UNDEFINED -1
#define DEFAULT_BLOCK_SIZE 65536
#define IS_POWER_OF_TWO(N) (__builtin_popcountll(N) == 1)

namespace nbit
{
    namespace parallel
    {
        //forward declaration
        class par_set;
    } // namespace parallel

    //forward declaration
    template <std::size_t>
    class fixed_set;

    template <bool DynamicResize = true>
    class set
    {
        using iterator = std::vector<std::uint64_t>::iterator;
        using const_iterator = std::vector<std::uint64_t>::const_iterator;
        friend class parallel::par_set;

    public:
        ///@{@name Constructor

        /// Default constructor, construct an empty set
        set() = default;

        /// Creates a bit set supporting at least max_value elements
        explicit set(const std::size_t max_value) : data((max_value / group_size) + 1, 0) {}

        /// Copy constructor
        set(const set &other) : data(other.data) {}

        /// Move constructor
        set(set &&other) noexcept { data.swap(other.data); }

        /// Creates a bit set from a list of integer values.
        template <typename T,
                  bool U = DynamicResize,
                  typename = typename std::enable_if<U>::type>
        set(std::initializer_list<T> list) noexcept : set()
        {
            static_assert(std::is_integral<T>::value);
            insert(list.begin(), list.end());
        }

        /// Assignment operator with a list of integer values.
        template <typename T>
        set &operator=(std::initializer_list<T> list)
        {
            static_assert(std::is_integral<T>::value);
            insert(list.begin(), list.end());
        }

        /// Create a bit set from a container of integers.
        template <typename Container>
        explicit set(const Container &c) : set()
        {
            static_assert(std::is_integral<typename Container::value_type>::value);
            insert(c.begin(), c.end());
        }

        set &operator=(const set &other) = default;
        set &operator=(set &&other) noexcept = default;

        ///@}

        ///@{@name Destructor

        /// Default destructor
        ~set() = default;
        ///@}

        ///@{@name Modifiers

        /// Clears all data, without resizing the bit set
        void clear() noexcept { std::fill(begin(), end(), 0); }

        /// Inserts a single element into the bit set
        template <bool U = DynamicResize>
        inline void insert(const std::size_t key) noexcept
        {
            if constexpr (U)
                resize_to_fit(key);
            insert_single(key);
        }

        /// Resize the bit set to support a new max index.
        template <bool U = DynamicResize,
                  typename = typename std::enable_if<U>::type>
        void resize(std::size_t new_max)
        {
            std::size_t num_groups = (new_max / group_size) + 1;
            data.resize(num_groups, 0);
        }

        /// Inserts a single element into the bit set after the application of a
        /// user-specified transformation.
        template <bool U = DynamicResize, typename Func>
        inline void insert(const std::uint64_t key, Func fn) noexcept
        {
            std::uint64_t mapped_key = fn(key);
            if constexpr (U)
                resize_to_fit(mapped_key);
            insert_single(mapped_key);
        }

        /// Inserts elements from range [first, last) into the bit set
        template <bool U = DynamicResize, typename Iter>
        void insert(const Iter first, const Iter last)
        {
            if constexpr (U)
                resize_to_fit(*std::max_element(first, last));
            for (Iter it = first; it != last; ++it)
                insert_single(*it);
        }

        /// Inserts sorted elements from range [first, last) into the bit map.
        /// @note Might be inefficient if the origin range is not sorted
        template <bool U = DynamicResize, typename Iter>
        void insert_sorted(const Iter first, const Iter last)
        {
            if constexpr (U)
                resize_to_fit(*std::next(last, -1));
            for (Iter it = first; it != last;)
            {
                std::size_t group = *it >> exp;
                auto next = std::partition_point(it, last, [&](auto el) {
                    return (el >> exp) == group;
                });
                data[group] = std::accumulate(it, next, data[group], [&](auto mask, auto element) {
                    return mask | (1UL << (element & (group_size - 1)));
                });
                it = next;
            }
        }

        /// Erase a single element from the set.
        void erase(const std::size_t key) noexcept { erase_single(key); };

        /// Requests the bit set to reduce its capacity to fit its size.
        template <bool U = DynamicResize,
                  typename = typename std::enable_if<U>::type>
        void shrink_to_fit()
        {
            auto num_groups = static_cast<size_t>(std::distance(cbegin(), upper_bound()));
            data.resize(num_groups);
        }

        ///@}

        ///@{@name Capacity

        /// Returns the number of elements the set can hold, the current capacity.
        [[nodiscard]] std::size_t max_size() const { return group_size * data.size(); }

        /// Returns the number of elements the set hold.
        [[nodiscard]] std::size_t size() const { return count(); }

        /// Returns the number of elements the set hold (number of set bits).
        [[nodiscard]] inline std::size_t count() const noexcept
        {
            auto lambda = [&](auto t, auto item) {
                return t + __builtin_popcountll(item);
            };
            return static_cast<size_t>(std::accumulate(cbegin(), cend(), 0, lambda));
        }

        /// Test whether the bit set is empty
        [[nodiscard]] bool empty() const noexcept { return (lower_bound() == cend()); }

        /// Returns the index of the last set (1) bit
        /// @note If the set is empty the return value is undefined (NBIT_UNDEFINED)
        [[nodiscard]] std::int64_t maximum() const
        {
            if (empty())
                return NBIT_UNDEFINED;
            auto group = static_cast<size_t>(std::distance(cbegin(), upper_bound()) - 1);
            int last_bit = (group_size - 1) - __builtin_clzll(*(upper_bound() - 1));
            return (group * group_size) + last_bit;
        }

        /// Returns the index of the first set (1) bit
        /// @note If the set is empty the return value is undefined (NBIT_UNDEFINED)
        [[nodiscard]] std::int64_t minimum() const
        {
            if (empty())
                return NBIT_UNDEFINED;
            auto group = static_cast<size_t>(std::distance(cbegin(), lower_bound()));
            int first_bit = __builtin_ctzll(*lower_bound());
            return (group * group_size) + first_bit;
        }

        ///@}

        ///@{@name Operators

        /// Returns true if all of the elements in both bit sets match.
        bool operator==(const set &other) const
        {
            return std::equal(this->cbegin(), this->cend(),
                              other.cbegin(), other.cend());
        }

        /// Returns true if at least one of the elements in both bit sets mismatch.
        bool operator!=(const set &other) const { return !(*this == other); }

        /// Performs bitwise AND, and assign to the current set.
        /// @see operator&
        set &operator&=(const set &other)
        {
            std::size_t limit = std::min(data.size(), other.data.size());
            auto it = std::next(begin(), limit);
            std::transform(begin(), it, other.cbegin(), begin(),
                           std::bit_and<>());
            if (it < end())
                std::fill(it, end(), 0);

            return *this;
        }

        /// Performs bitwise OR, and assign to the current set.
        /// @see operator|
        set &operator|=(const set &other)
        {
            resize_to_fit(static_cast<size_t>(other.maximum()));
            std::transform(other.cbegin(), other.cend(), this->cbegin(),
                           this->begin(), std::bit_or<>());
            return *this;
        }

        /// Performs bitwise XOR, and assign to the current set.
        /// @see operator^
        set &operator^=(const set &other)
        {
            resize_to_fit(static_cast<size_t>(other.maximum()));
            std::transform(this->cbegin(), this->cend(), other.cbegin(),
                           this->begin(), std::bit_xor<>());
            return *this;
        }

        /// Performs bitwise AND, intersection operator.
        /// Returns a new set formed only by the elements that are present in both sets.
        set operator&(const set &other) { return set(*this) &= other; }

        /// Performs bitwise OR, union operator.
        /// Returns a new set formed by the elements that are present in any of the sets.
        set operator|(const set &other) { return set(*this) |= other; }

        /// Performs bitwise XOR, symmetric differenc operator.
        /// Returns a new set formed by the elements that are present in one of the sets,
        /// but not in the other.
        set operator^(const set &other) { return set(*this) ^= other; }

        /// Performs the difference operation.
        /// Returns a new set formed by the elements that are present in the first set,
        /// but not in the second one.
        set operator-(const set &other) { return set(*this) ^= (*this & other); }

        /// Decodes the bit set into a vector of indices (of 1 bits) using a
        /// output map function.
        template <typename T = std::uint64_t, typename Function>
        std::vector<T> decode(Function fn)
        {
            return decode_simple<T>(fn);
        }

        /// Decodes the bit set into a vector of indices (of 1 bits).
        template <typename T>
        std::vector<T> decode()
        {
            auto lambda = [](T x) { return x; };
            return decode_simple<T>(lambda);
        }

        ///@}

        ///@{@name Iterators

        /// Returns an iterator to beginning.
        [[nodiscard]] iterator begin() noexcept { return data.begin(); }

        /// Returns an iterator to the end of bit set (past-last element of the array).
        [[nodiscard]] iterator end() noexcept { return data.end(); }

        /// Returns a const_iterator to beginning.
        [[nodiscard]] const_iterator cbegin() const noexcept { return data.cbegin(); }

        /// Returns a const_iterator to end os the bit set.
        [[nodiscard]] const_iterator cend() const noexcept { return data.cend(); }

        /// Return a const iterator pointing to first nonzero group in the set.
        [[nodiscard]] const_iterator lower_bound() const noexcept
        {
            return std::find_if(cbegin(), cend(), [](auto x) { return x; });
        }

        /// Return a const iterator pointing to past-last nonzero group in the set.
        [[nodiscard]] const_iterator upper_bound() const noexcept
        {
            auto reverse = std::find_if(data.rbegin(), data.rend(), [](auto x) { return x; });
            return reverse.base();
        }
        ///@}

    private:
        /// Number of bits per group
        char group_size = 64;

        /// Log2(group_size)
        char exp = 6;

        /// Array of integers that compose the bit set.
        std::vector<std::uint64_t> data;

        /// Resizes the container so it can hold a new max index.
        template <bool U = DynamicResize>
        void resize_to_fit(std::size_t new_max)
        {
            if constexpr (U)
                if (max_size() <= new_max)
                    resize(new_max);
        }

        /// Inserts a single key in the bit set.
        inline void insert_single(const std::uint64_t key) noexcept
        {
            std::size_t group = key >> exp;
            std::size_t pos = key & (group_size - 1);
            std::uint64_t mask = (1UL << pos);
            data[group] |= mask;
        }

        /// Erases a single key from the bit set.
        inline void erase_single(const std::uint64_t key) noexcept
        {
            if (key <= max_size())
            {
                std::size_t group = key >> exp;
                std::size_t pos = key & (group_size - 1);
                std::uint64_t mask = (1UL << pos);
                data[group] ^= mask;
            }
        }

        template <typename T, typename Function>
        std::vector<T> decode_simple(Function fn)
        {
            static_assert(std::is_integral<T>::value,
                          "Decode type is not integral type");

            std::vector<T> out(count());
            std::size_t pos = 0;
            for (auto it = lower_bound(); it != upper_bound(); it++)
            {
                std::uint64_t bitset = *it;
                auto group = static_cast<size_t>(std::distance(cbegin(), it));
                std::size_t loop_size = __builtin_popcountll(bitset);
                for (size_t j = 0; j < loop_size; j++)
                {
                    T result = __builtin_ctzll(bitset);
                    out[pos + j] = fn((group * group_size) + result);
                    bitset ^= (bitset & -bitset);
                }
                pos += loop_size;
            }
            return out;
        }
    };

    /// Bit set specialization for fixed size bit sets
    /// the template parameter N should be a power of two.
    template <std::size_t N = DEFAULT_BLOCK_SIZE>
    class fixed_set : public set<false>
    {
    public:
        fixed_set() : set(N - 1) { static_assert(IS_POWER_OF_TWO(N)); }
        ~fixed_set() = default; // default destructor
    };

} // namespace nbit