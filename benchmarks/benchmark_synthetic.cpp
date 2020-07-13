#include "../src/set.hpp"
#include "../src/sparse_set.hpp"
#include "generate_synthetic_data.hpp"
#include <benchmark/benchmark.h>
#include <map>
#include <set>
#include <unordered_set>

constexpr std::size_t N = 65536;
using map = std::map<std::uint64_t, nbit::fixed_set<N>>;
using unordered_map = std::map<std::uint64_t, nbit::fixed_set<N>>;

static void CustomArguments(benchmark::internal::Benchmark *b)
{

    std::vector<int> sizes{100'000, 1'000'000, 10'000'000};
    std::vector<int> densities{1, 2, 3};
    for (auto size : sizes)
        for (auto d : densities)
            b->Args({size, d});
}

template <typename SetType>
static void InsertUniformData(benchmark::State &state)
{
    auto vec = uniform_data(state.range(0), state.range(1));
    for (auto _ : state)
    {
        SetType set;
        set.insert(vec.begin(), vec.end());
        set.clear();
    }
}

BENCHMARK_TEMPLATE(InsertUniformData, nbit::set<true>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertUniformData, nbit::sparse_set<2048>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertUniformData, nbit::sparse_set<N, map>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertUniformData, nbit::sparse_set<N, unordered_map>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertUniformData, std::set<uint64_t>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertUniformData, std::unordered_set<uint64_t>)->Apply(CustomArguments);

BENCHMARK_MAIN();
