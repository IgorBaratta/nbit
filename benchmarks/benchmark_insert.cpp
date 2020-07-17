#include "../src/set.hpp"
#include "../src/sparse_set.hpp"
#include "generate_synthetic_data.hpp"
#include <benchmark/benchmark.h>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

constexpr std::size_t N = 65536;

using map = std::map<std::uint64_t, nbit::fixed_set<N>>;
using unordered_map = std::unordered_map<std::uint64_t, nbit::fixed_set<N>>;

static void CustomArguments(benchmark::internal::Benchmark *b)
{

    std::vector<int> sizes{1'000'000};
    std::vector<int> densities{1};
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

static void CreateBitSet(benchmark::State &state)
{
    std::size_t size = 1'000'000UL;
    for (auto _ : state)
    {
        nbit::set<false> set(size);
        benchmark::DoNotOptimize(set);
    }
}

static void CreateVectorBool(benchmark::State &state)
{
    std::size_t size = 1'000'000UL;
    for (auto _ : state)
    {
        std::vector<bool> set(size);
        benchmark::DoNotOptimize(set);
    }
}

template <typename SetType>
static void InsertUniformSortedData(benchmark::State &state)
{
    auto vec = uniform_data(state.range(0), state.range(1));
    std::sort(vec.begin(), vec.end());

    for (auto _ : state)
    {
        SetType set;
        set.insert_sorted(vec.begin(), vec.end());
        set.clear();
    }
}

template <typename SetType>
static void InsertBinomialData(benchmark::State &state)
{
    auto vec = binomial_data(state.range(0), state.range(1));
    for (auto _ : state)
    {
        SetType set;
        set.insert(vec.begin(), vec.end());
        set.clear();
    }
}

// Insert Random Integers from and Uniform Distribution
BENCHMARK_TEMPLATE(InsertUniformData, nbit::set<true>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertUniformData, nbit::sparse_set<2048>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertUniformData, nbit::sparse_set<N, map>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertUniformData, nbit::sparse_set<N, unordered_map>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertUniformData, std::set<uint64_t>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertUniformData, std::unordered_set<uint64_t>)->Apply(CustomArguments);

BENCHMARK_TEMPLATE(InsertUniformSortedData, nbit::set<true>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertUniformSortedData, nbit::sparse_set<2048>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertUniformSortedData, nbit::sparse_set<N, map>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertUniformSortedData, nbit::sparse_set<N, unordered_map>)->Apply(CustomArguments);

// Insert Random Integers from and Binomial Distribution
BENCHMARK_TEMPLATE(InsertBinomialData, nbit::set<true>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertBinomialData, nbit::sparse_set<2048>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertBinomialData, nbit::sparse_set<N, map>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertBinomialData, nbit::sparse_set<N, unordered_map>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertBinomialData, std::set<uint64_t>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(InsertBinomialData, std::unordered_set<uint64_t>)->Apply(CustomArguments);

BENCHMARK(CreateBitSet);
BENCHMARK(CreateVectorBool);

BENCHMARK_MAIN();
