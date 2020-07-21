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

#include "set.hpp"
#include <unordered_map>

namespace nbit
{

    template <std::size_t N = DEFAULT_BLOCK_SIZE,
              class MAP = std::unordered_map<std::uint64_t, fixed_set<N>>>
    class sparse_set
    {
        using iterator = typename MAP::iterator;
        using const_iterator = typename MAP::const_iterator;

    public:
        ///@{@name Constructor

        /// Default constructor, construct an empty sparse_set
        sparse_set() = default;

        /// Copy constructor
        sparse_set(const sparse_set &other) : data(other.data) {}

        /// Move constructor
        sparse_set(sparse_set &&other) noexcept
        {
            data.swap(other.data);
        }

        /// Creates a bit set from a list of integer values.
        template <typename T>
        sparse_set(std::initializer_list<T> list) noexcept : sparse_set()
        {
            insert(list.begin(), list.end());
        }

        /// Create a bit set from a container of integers.
        template <typename Container>
        explicit sparse_set(const Container &c) : sparse_set()
        {
            static_assert(std::is_integral<typename Container::value_type>::value);
            insert(c.begin(), c.end());
        }

        /// Creates a bit set from a list of integer values.
        template <typename T>
        sparse_set &operator=(std::initializer_list<T> list)
        {
            static_assert(std::is_integral<T>::value);
            insert(list.begin(), list.end());
            return *this;
        }

        sparse_set &operator=(const sparse_set &other) = default;
        sparse_set &operator=(sparse_set &&other) noexcept = default;

        ///@} Constructor

        ///@{@name Destructor

        /// Default destructor
        ~sparse_set() = default;
        ///@}

        ///@{@name Modifiers

        /// Clears all data, without resizing the bit set
        void clear() noexcept
        {
            for (auto it = begin(); it != end(); it++)
                it->second.clear();
        }

        /// Inserts a single element into the bit set
        inline void insert(const std::size_t key) noexcept
        {
            insert_single(key);
        }

        /// Inserts elements from range [first, last) into the bit set
        template <typename Iter>
        void insert(const Iter first, const Iter last)
        {
            static_assert(std::is_integral<typename std::iterator_traits<Iter>::value_type>::value);
            for (Iter it = first; it != last; ++it)
                insert_single(*it);
        }

        /// Inserts sorted elements from range [first, last) into the bit map.
        /// @note Might be inefficient if the origin range is not sorted
        template <typename Iter>
        void insert_sorted(const Iter first, const Iter last)
        {
            static_assert(std::is_integral<typename std::iterator_traits<Iter>::value_type>::value);
            for (Iter it = first; it != last;)
            {
                std::size_t group = *it >> exp;

                auto next = std::partition_point(it, last, [&](auto el) {
                    return std::size_t(el >> exp) == group;
                });

                fixed_set<N> &bitset = data[group];
                std::for_each(it, next, [&](auto &key) {
                    std::uint64_t short_key = key & (N - 1);
                    bitset.insert(short_key);
                });
                it = next;
            }
        }

        /// Erase a single element from the set.
        void erase(const std::size_t key) noexcept { erase_single(key); };

        /// Requests the sparse bit set to reduce its capacity to fit its size.
        /// Remove all empty bitsets
        void shrink_to_fit()
        {
            auto it = begin();
            while (it != end())
            {
                if (it->second.empty())
                    it = data.erase(it);
                else
                    it++;
            }
        }

        ///@}

        ///@{@name Capacity

        /// Returns the number of elements the set can hold, the current capacity.
        [[nodiscard]] std::size_t max_size() const { return N * data.size(); }

        /// Returns the number of elements the set hold.
        [[nodiscard]] std::size_t size() const { return count(); }

        /// Returns the number of elements the sparse set hold (number of set bits).
        [[nodiscard]] inline std::size_t count() const noexcept
        {
            auto lambda = [&](std::uint64_t t, auto &map) {
                return t + map.second.count();
            };
            return static_cast<size_t>(std::accumulate(cbegin(), cend(), 0, lambda));
        }

        /// Test whether the sparse bit set is empty
        [[nodiscard]] bool empty() const noexcept { return (lower_bound() == cend()); }

        /// Returns the index of the last set (1) bit
        /// @note If the set is empty the return value is undefined (NBIT_UNDEFINED)
        [[nodiscard]] std::int64_t maximum() const
        {
            if (empty())
                return NBIT_UNDEFINED;

            auto cmp = [](const auto &a, const auto &b) { return a.first < b.first; };
            const_iterator it = std::max_element(data.begin(), data.end(), cmp);
            return (it->first * N) + it->second.maximum();
        }

        /// Returns the index of the first set (1) bit
        /// @note If the set is empty the return value is undefined (NBIT_UNDEFINED)
        [[nodiscard]] std::int64_t minimum() const
        {
            if (empty())
                return NBIT_UNDEFINED;

            auto cmp = [](const auto &a, const auto &b) { return a.first < b.first; };
            const_iterator it = std::min_element(data.begin(), data.end(), cmp);
            return (it->first * N) + it->second.minimum();
        }

        ///@}

        ///@{@name Operators

        /// Returns true if all of the elements in both bit sets match.
        bool operator==(const sparse_set &other) const
        {
            return data == other.data;
        }

        /// Returns true if at least one of the elements in both bit sets mismatch.
        bool operator!=(const sparse_set &other) const { return !(*this == other); }

        /// Performs bitwise AND, and assign to the current set.
        /// @see operator&
        sparse_set &operator&=(const sparse_set &other) noexcept
        {
            iterator it = begin();
            while (it != end())
            {
                const_iterator other_it = other.data.find(it->first);
                if (other_it != other.cend())
                {
                    it->second &= other_it->second;
                    if (it->second.empty())
                        it = data.erase(it);
                    else
                        it = std::next(it);
                }
                else
                    it = data.erase(it);
            }
            return *this;
        }

        /// Performs bitwise OR, and assign to the current set.
        /// @see operator|
        sparse_set &operator|=(const sparse_set &other) noexcept
        {
            for (auto const &[key, bitset] : other.data)
                data[key] |= bitset;
            return *this;
        }

        /// Performs bitwise XOR, and assign to the current set.
        /// @see operator^
        sparse_set &operator^=(const sparse_set &other) noexcept
        {
            for (auto const &[key, bitset] : other.data)
                data[key] ^= bitset;
            return *this;
        }

        /// Compute difference between this set and the other, and assign to the current set.
        /// @see operator-
        sparse_set &operator-=(const sparse_set &other) noexcept
        {
            *this ^= (*this & other);
            return *this;
        }

        /// Performs bitwise AND, intersection operator.
        /// Returns a new set formed only by the elements that are present in both sets.
        sparse_set operator&(const sparse_set &other) { return sparse_set(*this) &= other; }

        /// Performs bitwise OR, union operator.
        /// Returns a new set formed by the elements that are present in any of the sets.
        sparse_set operator|(const sparse_set &other) { return sparse_set(*this) |= other; }

        /// Performs bitwise XOR, symmetric difference operator.
        /// Returns a new set formed by the elements that are present in one of the sets,
        /// but not in the other.
        sparse_set operator^(const sparse_set &other) { return sparse_set(*this) ^= other; }

        /// Performs the difference operation.
        /// Returns a new set formed by the elements that are present in the first set,
        /// but not in the second one.
        sparse_set operator-(const sparse_set &other) { return sparse_set(*this) ^= (*this & other); }

        // /// Decodes the bit set into a vector of indices (of 1 bits).
        // template <typename T = std::uint64_t>
        // std::vector<T> decode()
        // {
        //     std::vector<T> output;
        //     output.reserve(count());
        //     for (auto it = begin(); it != end(); it++)
        //     {
        //         std::uint64_t offset = N * it->first;
        //         set<false> &bitset = it->second;
        //         if (!bitset.empty())
        //         {
        //             auto vec = bitset.decode<T>([&](auto x) {
        //                 return x + offset;
        //             });
        //             output.insert(output.end(), vec.begin(), vec.end());
        //         }
        //     }
        //     return output;
        // }

        /// Decodes the bit set into a vector of sorted indices (of 1 bits).
        template <typename T>
        std::vector<T> decode()
        {
            std::vector<T> output;
            output.reserve(count());

            // sort so the output can be also keys
            std::vector<std::uint64_t> keys;
            keys.reserve(data.size());
            for (auto &it : data)
                keys.push_back(it.first);
            std::sort(keys.begin(), keys.end());

            for (auto &k : keys)
            {
                std::uint64_t offset = N * k;
                set<false> &bitset = data[k];
                if (!bitset.empty())
                {
                    auto vec = bitset.decode<T>([&offset](auto x) {
                        return x + offset;
                    });
                    output.insert(output.end(), vec.begin(), vec.end());
                }
            }
            return output;
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

        /// Return a const iterator pointing to first nonzero local bitset in the sparse set.
        /// If the sparse set is empty, the returned iterator will be equal to cend().
        [[nodiscard]] const_iterator lower_bound() const noexcept
        {
            return std::find_if_not(cbegin(), cend(), [](auto x) {
                return x.second.empty();
            });
        }
        ///@}

    public:
        char exp = __builtin_ctzll(N);
        MAP data;

    private:
        /// Inserts a single key in the bit set.
        inline void insert_single(const std::uint64_t key) noexcept
        {
            std::size_t block = key >> exp;
            std::size_t short_key = key & (N - 1);
            data[block].insert(short_key);
        }

        /// Inserts a single key in the bit set.
        inline void erase_single(const std::uint64_t key) noexcept
        {
            std::size_t block = key >> exp;
            std::size_t short_key = key & (N - 1);
            data[block].erase(short_key);
            if (data[block].empty())
                data.erase(block);
        }
    };

} // namespace nbit
