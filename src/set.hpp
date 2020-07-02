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
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
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

namespace nbit
{
    template <bool DynamicResize = true>
    class set
    {
    protected:
        char _group_size = 64;
        char _exp = 6;
        std::vector<std::uint64_t> _array;

    public:
        /// Default constructor, construct an empty set
        set() : _array(0) {}

        /// Create a bit map supporting at least max_value elements
        set(const std::size_t max_value) : _array((max_value / _group_size) + 1, 0) {}

        /// copy consctructor
        set(const set &x) : _array(x._array) {}

        /// move constructor
        set(set &&x) { _array.swap(x._array); }

        /// Create a bitset from a container of intergers.
        template <typename Container>
        set(const Container &c) : set()
        {
            static_assert(std::is_integral<typename Container::value_type>::value);
            insert(c.begin(), c.end());
        }

        ~set() {} // default destructor

        /// Inserts a single element into the bit set
        template <bool U = DynamicResize>
        inline void insert(const std::size_t key) noexcept
        {
            if constexpr (U)
                resize_to_fit(key);
            insert_single(key);
        }

        /// Inserts a single element into the bit set after the applciation of a
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
        inline void insert(const Iter begin, const Iter end)
        {
            if constexpr (U)
                resize_to_fit(*std::max_element(begin, end));
            for (Iter it = begin; it != end; ++it)
                insert_single(*it);
        }

        /// Returns the number of elements the set can hold
        /// @parameters: (none)
        std::size_t max_size() const { return _group_size * _array.size(); }

        /// returns the number of elements the set hold
        /// @parameters: (none)
        std::size_t size() const { return count(); }

        /// returns the number of elements the set hold
        inline std::size_t count() const noexcept
        {
            return std::accumulate(cbegin(), cend(), 0,
                                   [&](std::uint64_t t, std::uint64_t item) {
                                       return t + __builtin_popcountll(item);
                                   });
        }

        /// Test whether the bit set is empty
        bool empty() const noexcept { return (nz_begin() >= nz_end()); }

        /// Clears all data, without resizing the bit set.
        /// @complexity: Linear in the range of nonzeros of the the bit set.
        void clear() noexcept { std::fill(begin(), end(), 0); }

        // resize bit set to support indices up to new_max
        template <bool U = DynamicResize>
        typename std::enable_if<U, void>::type resize(std::size_t new_max)
        {
            std::size_t num_groups = (new_max / _group_size) + 1;
            _array.resize(num_groups, 0);
        }

        bool operator==(const set &other) const
        {
            return std::equal(this->cbegin(), this->cend(),
                              other.cbegin(), other.cend());
        }

        bool operator!=(const set &other) const
        {
            return !(*this == other);
        }

        set &operator&=(const set &other)
        {
            std::transform(this->cbegin(), this->cend(), other.cbegin(),
                           this->begin(), std::bit_and<std::uint64_t>());
            return *this;
        }

        set &operator|=(const set &other)
        {
            resize_to_fit(other.maximum());
            std::transform(other.cbegin(), other.cend(), this->cbegin(),
                           this->begin(), std::bit_or<std::uint64_t>());
            return *this;
        }

        set operator&(const set &other)
        {
            std::size_t upper_bound = std::min(this->maximum(), other.maximum());
            set<DynamicResize> new_set(upper_bound);
            std::size_t num_groups = new_set._array.size();
            std::transform(this->begin(), this->begin() + num_groups, other.begin(), new_set.begin(),
                           std::bit_and<std::uint64_t>());
            return new_set;
        }

        set operator|(const set &other)
        {
            if (this->maximum() > other.maximum())
            {
                set<DynamicResize> new_set = *this;
                std::transform(other.cbegin(), other.cend(), new_set.begin(), new_set.begin(),
                               std::bit_or<std::uint64_t>());
                return new_set;
            }
            else
            {
                set<DynamicResize> new_set = other;
                std::transform(this->cbegin(), this->cend(), new_set.begin(), new_set.begin(),
                               std::bit_or<std::uint64_t>());
                return new_set;
            }
        }

        set operator^(const set &other)
        {
            if (this->maximum() > other.maximum())
            {
                set<DynamicResize> new_set = *this;
                std::transform(other.cbegin(), other.cend(), new_set.begin(), new_set.begin(),
                               std::bit_xor<std::uint64_t>());
                return new_set;
            }
            else
            {
                set<DynamicResize> new_set = other;
                std::transform(this->cbegin(), this->cend(), new_set.begin(), new_set.begin(),
                               std::bit_xor<std::uint64_t>());
                return new_set;
            }
        }

        /// returns the maximum index in the bit set
        /// @note: if the map is empty the return value is undefined
        std::uint64_t maximum() const
        {
            if (empty())
                return NBIT_UNDEFINED;
            std::size_t group = std::distance(cbegin(), nz_end()) - 1;
            int last_bit = (_group_size - 1) - __builtin_clzll(*(nz_begin() - 1));
            return (group * _group_size) + last_bit;
        }

        /// returns the minimum set value in the bit map
        /// @note: if the map is empty the return value is undefined
        std::uint64_t minimum() const
        {
            if (empty())
                return NBIT_UNDEFINED;
            std::size_t group = std::distance(cbegin(), nz_begin());
            int first_bit = __builtin_ctzll(*nz_begin());
            return (group * _group_size) + first_bit;
        }

        /// Erase a single element from the set.
        void erase(const std::size_t key) noexcept { erase_single(key); };

        /// decode bit set into a vector of indices using a output map function
        template <typename T = std::uint64_t, typename Function>
        std::vector<T> decode(Function fn)
        {
            return decode_simple<T>(fn);
        }

        /// decode bit set into a vector of indices using a output map function
        template <typename T>
        std::vector<T> decode()
        {
            auto lambda = [](T x) { return x; };
            return decode_simple<T>(lambda);
        }

        /// Requests the container to reduce its capacity to fit its size.
        template <bool U = DynamicResize>
        typename std::enable_if<U, void>::type shrink_to_fit()
        {
            std::size_t num_groups = std::distance(cbegin(), nz_begin()) + 1;
            _array.resize(num_groups);
        }

        /// Inserts sorted elements from range [first, last) into the bit map
        /// @note: Might be innefficient if the origin range of elements is not sorted
        template <typename Iter>
        void insert_sorted(const Iter begin, const Iter end)
        {
            for (Iter it = begin; it != end;)
            {
                Iter tempit = it;
                std::size_t group = *it >> _exp;
                std::size_t current = group;
                while (current == group && tempit != end)
                    current = *(++tempit) >> _exp;
                for (; it < tempit; ++it)
                    _array[group] |= (1UL << (*it & (_group_size - 1)));
            }
        }

        std::vector<std::uint64_t>::iterator begin() { return _array.begin(); }

        std::vector<std::uint64_t>::iterator end() { return _array.end(); }

        /// Returns const_iterator to beginning
        std::vector<std::uint64_t>::const_iterator cbegin() const noexcept
        {
            return _array.cbegin();
        }

        /// Returns a const_iterator to end
        std::vector<std::uint64_t>::const_iterator cend() const noexcept
        {
            return _array.cend();
        }

        /// Return a const iterator poiting to first nonzero group in the set
        std::vector<std::uint64_t>::const_iterator nz_begin() const noexcept
        {
            return std::find_if(cbegin(), cend(), [](std::uint64_t x) { return x; });
        }

        /// Return a const iterator poiting to past-last nonzero group in the set
        std::vector<std::uint64_t>::const_iterator nz_end() const noexcept
        {
            auto reserse_last = std::find_if(_array.rbegin(), _array.rend(), [](std::uint64_t x) { return x; });
            return reserse_last.base();
        }

    private:
        template <bool U = DynamicResize>
        void resize_to_fit(std::size_t new_max)
        {
            if constexpr (U)
                if (max_size() <= new_max)
                    resize(new_max);
        }

        inline void insert_single(const std::uint64_t key) noexcept
        {
            std::size_t group = key >> _exp;
            std::size_t pos = key & (_group_size - 1);
            std::uint64_t mask = (1UL << pos);
            _array[group] |= mask;
        }

        inline void erase_single(const std::uint64_t key) noexcept
        {
            if (key < max_size())
            {
                std::size_t group = key >> _exp;
                std::size_t pos = key & (_group_size - 1);
                std::uint64_t mask = (1UL << pos);
                if ((_array[group] & (mask)))
                {
                    _array[group] ^= mask;
                }
            }
        }

        template <typename T, typename Function>
        std::vector<T> decode_simple(Function fn)
        {
            static_assert(std::is_integral<T>::value, "Decode type is not integral type");
            std::vector<T> out(count());
            std::size_t pos = 0;
            for (auto it = nz_begin(); it != nz_end(); it++)
            {
                std::uint64_t bitset = *it;
                std::size_t group = std::distance(cbegin(), it);
                std::size_t loop_size = __builtin_popcountll(bitset);
                for (size_t j = 0; j < loop_size; j++)
                {
                    T result = __builtin_ctzll(bitset);
                    out[pos + j] = fn((group * _group_size) + result);
                    bitset ^= (bitset & -bitset);
                }
                pos += loop_size;
            }
            return out;
        }
    };

    // Specialization for fixed size bit set
    template <std::size_t N = DEFAULT_BLOCK_SIZE>
    class fixed_set : public set<false>
    {
        constexpr bool is_power_of_two() { return __builtin_popcountll(N) == 1; };

    public:
        fixed_set() : set(N - 1) { static_assert(is_power_of_two()); }
        ~fixed_set() {} // default destructor
    };

} // namespace nbit