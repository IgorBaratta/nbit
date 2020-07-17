#include "set.hpp"
#include <execution>

using namespace nbit;

namespace nbit::parallel
{
    class par_set : public set<true>
    {
        std::execution::parallel_unsequenced_policy policy = std::execution::par_unseq;

    public:
        /// Inserts sorted elements from range [first, last) into the bit map.
        /// @note Might be inefficient if the origin range is not sorted
        template <typename Iter>
        void insert_sorted(const Iter first, const Iter last)
        {
            resize_to_fit(*std::max_element(policy, first, last));
            for (Iter it = first; it != last;)
            {
                std::size_t group = *it >> exp;
                std::uint64_t &el = data[group];
                auto next = std::find_if_not(it, last, [=](std::uint64_t el) {
                    return (el >> exp) == group;
                });
                el = std::transform_reduce(it, next, el, std::bit_or<>(), [=](std::uint64_t v) {
                    return 1UL << (v & (group_size - 1));
                });
                it = next;
            }
        }

        /// Returns the number of elements the set hold (number of set bits).
        [[nodiscard]] inline std::size_t count() const noexcept
        {
            return std::transform_reduce(
                policy, cbegin(), cend(), 0, std::plus<>(),
                [](std::uint64_t v) { return __builtin_popcountll(v); });
        }

        ///@{@name Operators

        bool operator==(const par_set &other) const
        {
            return std::equal(std::execution::par_unseq, this->cbegin(), this->cend(),
                              other.cbegin(), other.cend());
        }

        bool operator!=(const par_set &other) const { return !(*this == other); }

        par_set &operator&=(const par_set &other)
        {
            std::size_t limit = std::min(data.size(), other.data.size());
            auto it = std::next(begin(), limit);
            std::transform(policy, begin(), it, other.cbegin(),
                           begin(), std::bit_and<>());
            if (it < end())
                std::fill(policy, it, end(), 0);

            return *this;
        }

        par_set &operator|=(const par_set &other)
        {
            resize_to_fit(static_cast<size_t>(other.maximum()));
            std::transform(policy, other.cbegin(), other.cend(),
                           this->cbegin(), this->begin(),
                           std::bit_or<>());
            return *this;
        }

        par_set &operator^=(const par_set &other)
        {
            resize_to_fit(static_cast<size_t>(other.maximum()));
            std::transform(policy, this->cbegin(), this->cend(),
                           other.cbegin(), this->begin(),
                           std::bit_xor<>());
            return *this;
        }

        par_set operator&(const par_set &other) { return par_set(*this) &= other; }
        par_set operator|(const par_set &other) { return par_set(*this) |= other; }
        par_set operator^(const par_set &other) { return par_set(*this) ^= other; }

        ///@}
    };
} // namespace nbit::parallel