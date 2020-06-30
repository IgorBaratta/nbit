#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#define IS_POWER_OF_TWO(N) (__builtin_popcountll(N) == 1)
#define LOG2_OF_P2(N) (__builtin_ctzll(N))
#define DEFAULT_BLOCK_SIZE 65536
#define UNDEFINED_BLOCK -1

namespace nbit
{
    template <bool DynamicResize = true>
    class set
    {
    protected:
        char _group_size = 64;
        char _exp = 6;
        std::size_t _num_groups;
        std::vector<std::uint64_t> _mask;
        std::size_t _counter;

    public:
        // Default constructor, construct and empty
        set() : _num_groups{0}, _mask(0), _counter{0} {}

        // -----------------------------------------------------
        // Create a bit map supporting at least max_value elements
        set(std::size_t max_value)
            : _num_groups{(max_value / _group_size) + 1},
              _mask(_num_groups, 0),
              _counter{0} {}

        // -----------------------------------------------------
        ~set() {} // default destructor

        /// Inserts a single element into the bit map,
        /// Resize cointainer to fit
        inline void insert(const std::size_t k) noexcept
        {
            insert_single(k);
        }

        /// Inserts elements from range [first, last) into the bit map
        template <typename Iter>
        inline void insert(const Iter begin, const Iter end)
        {
            for (Iter it = begin; it != end; ++it)
                insert_single(*it);
        }

        /// Clears all data, without resizing the bitmask.
        /// @complexity: Linear in the size of the bitmask.
        void clear() noexcept
        {
            std::fill(_mask.begin(), _mask.end(), 0);
            _counter = 0;
        }

        /// returns the number of elements the set hold
        /// @parameters: (none)
        std::size_t size() const { return _counter; }

        /// returns the number of elements the set can hold
        /// @parameters: (none)
        std::size_t max_size() const { return _group_size * _num_groups; }

        /// decode bitmap into a vector of indices
        template <typename T = std::uint64_t>
        std::vector<T> decode(std::size_t offset = 0)
        {
            return decode_simple<T>(offset);
        }

        void resize(std::size_t new_max)
        {
            _num_groups = std::max((new_max / _group_size) + 1, _num_groups);
            _mask.resize(_num_groups);
        }

        /// returns the maximum set value in the bit map
        /// @note: if the map is empty the return value is undefined
        std::uint64_t maximum() const
        {
            if (!empty())
            {
                auto last_nz = std::find_if(_mask.rbegin(), _mask.rend(), [](std::uint64_t x) { return x; });
                std::size_t group = (_num_groups - 1) - std::distance(_mask.rbegin(), last_nz);
                int last_bit = _group_size - __builtin_clzll(*last_nz) - 1;
                return (group * _group_size) + last_bit;
            }
            return -1;
        }

        /// returns the minimum set value in the bit map
        /// @note: if the map is empty the return value is undefined
        std::uint64_t minimum() const
        {
            if (!empty())
            {
                auto first_nz = std::find_if(_mask.begin(), _mask.end(), [](std::uint64_t x) { return x; });
                std::size_t group = std::distance(_mask.begin(), first_nz);
                int first_bit = __builtin_ctzll(*first_nz);
                return (group * _group_size) + first_bit;
            }
            return -1;
        }

        template <typename T>
        void erase(const T &key){};

        bool empty() const noexcept { return (_counter == 0); }

        void shrink_to_fit()
        {
            auto last_nz = std::find_if(_mask.rbegin(), _mask.rend(), [](std::uint64_t x) { return x; });
            _num_groups = _num_groups - std::distance(_mask.rbegin(), last_nz);
            _mask.resize(_num_groups);
        }

        /// Inserts sorted elements from range [first, last) into the bit map
        /// @note: Might be innefficient if the origin range of elements is not sorted
        template <typename Iter>
        void insert_sorted(const Iter begin, const Iter end)
        {
            for (Iter it = begin; it != end;)
            {
                std::size_t working_group = *it >> _exp;
                std::size_t current_group = working_group;
                std::uint64_t &current = _mask[working_group];
                std::size_t mask = 0;
                int start_count = __builtin_popcountll(current);
                while (working_group == current_group && it != end)
                {
                    mask |= (1UL << *it & (_group_size - 1));
                    current_group = *it >> _exp;
                    it++;
                }
                current |= mask;
                int final_count = __builtin_popcountll(current);
                _counter += (final_count - start_count);
            }
        }

    private:
        // -----------------------------------------------------
        inline void insert_single(const std::uint64_t key) noexcept
        {
            if (key >= max_size())
                resize(key);
            std::size_t group = key >> _exp;
            std::size_t pos = key & (_group_size - 1);
            std::uint64_t mask = (1UL << pos);
            if (!(_mask[group] & (mask)))
            {
                _mask[group] |= mask;
                ++_counter;
            }
        }

        // -----------------------------------------------------
        template <typename T>
        std::vector<T> decode_simple(std::size_t offset = 0)
        {
            std::vector<T> out(_counter);
            std::size_t pos = 0;
            for (std::size_t i = 0; i < _num_groups; i++)
            {
                std::uint64_t bitset = _mask[i];
                while (bitset != 0)
                {
                    T result = __builtin_ctzll(bitset);
                    out[pos] = ((i * _group_size) + result) + offset;
                    bitset ^= (bitset & -bitset);
                    pos++;
                }
            }
            return out;
        }
    };

    //=====================================================================//
    // Compile time fixed size map
    template <std::size_t N = DEFAULT_BLOCK_SIZE>
    class nset : public set<true>
    {
    public:
        nset() : set(N - 1) {}
        ~nset() {} // default destructor
    };

    //=====================================================================//
    /// Dynamic compressed bit map
    template <std::size_t N = DEFAULT_BLOCK_SIZE>
    class compressed_set
    {
    public:
        std::size_t _block_size;
        std::uint16_t _exp;
        std::vector<std::int64_t> _indices;
        std::vector<std::pair<nset<N>, std::size_t>> _map_array;
        std::size_t _num_blocks{0};

    public:
        compressed_set() {}

        compressed_set(std::size_t max_value, std::size_t min_blocks = 1)
            : _block_size(N),
              _exp{LOG2_OF_P2(N)},
              _indices((max_value / _block_size) + 1, UNDEFINED_BLOCK)
        {
            _map_array.reserve(min_blocks);
            static_assert(IS_POWER_OF_TWO(N), "Template parameter should be power of two.");
        }

        ~compressed_set() {} // default destructor

        // attempt to preallocate enough memory for nblocks
        void reserve(std::size_t nblocks)
        {
            _map_array.reserve(nblocks);
        }

        /// Inserts a single element into the bit map,
        inline void insert(const std::size_t key) noexcept
        {
            insert_single(key);
        }

        /// Inserts elements from range [first, last) into the bit map
        template <typename Iter>
        inline void insert(const Iter begin, const Iter end)
        {
            for (Iter it = begin; it != end; ++it)
                insert_uncked(*it);
        }

        /// count nonzeros
        std::size_t count()
        {
            std::size_t counter = 0;
            for (size_t i = 0; i < _num_blocks; i++)
                counter += _map_array[i].first.size();
            return counter;
        }

        template <typename T>
        std::vector<T> decode()
        {
            std::vector<T> output;
            std::size_t nz = count();
            output.reserve(nz);
            for (size_t i = 0; i < _num_blocks; i++)
            {
                std::size_t offset = _map_array[i].second * N;
                std::vector<T> vec = _map_array[i].first.decode(offset);
                output.insert(output.end(), vec.begin(), vec.end());
            }
            return output;
        }

    private:
        inline std::size_t check_and_resize(const std::size_t block_num)
        {
            if (block_num >= _indices.size())
                _indices.resize(block_num + 1, -1);

            if (_indices[block_num] == -1)
            {
                _indices[block_num] = _num_blocks;
                _num_blocks += 1;
                _map_array.push_back({nset<N>(), block_num});
            }
            return _indices[block_num];
        }

        inline void insert_single(const std::size_t key)
        {
            std::uint16_t short_key = key & (N - 1);
            std::size_t block_num = key >> _exp;
            std::size_t block_pos = check_and_resize(block_num);
            _map_array[block_pos].first.insert(short_key);
        }

        inline void insert_uncked(const std::size_t key)
        {
            std::uint16_t short_key = key & (N - 1);
            std::size_t block_num = key >> _exp;
            std::size_t block_pos = _indices[block_num];
            _map_array[block_pos].first.insert(short_key);
        }
    };
} // namespace nbit