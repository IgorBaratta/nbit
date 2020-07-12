#include "../src/set.hpp"
#include "../src/sparse_set.hpp"
#include "generate_synthetic_data.hpp"
#include <benchmark/benchmark.h>
#include <map>

static void CustomArguments(benchmark::internal::Benchmark *b)
{
    for (int i = 3; i <= 4; ++i)
        for (int j = 2; j <= 10; j++)
            b->Args({(int)std::pow(10, i), j});
}

static void InsertSet(benchmark::State &state)
{
    auto vec = uniform_data(state.range(0), state.range(1));
    nbit::set set;
    for (auto _ : state)
        set.insert(vec.begin(), vec.end());
}

static void InsertFixedSet(benchmark::State &state)
{
    auto vec = uniform_data(state.range(0), state.range(1));
    nbit::set<false> set(*std::max_element(vec.begin(), vec.end()));
    for (auto _ : state)
        set.insert(vec.begin(), vec.end());
}

static void InsertSparseSet(benchmark::State &state)
{
    auto vec = uniform_data(state.range(0), state.range(1));
    nbit::sparse_set set;
    for (auto _ : state)
        set.insert(vec.begin(), vec.end());
}

static void InsertSparseSetWithMap(benchmark::State &state)
{
    constexpr std::size_t N = 65536;
    using mymap = std::map<std::uint64_t, nbit::fixed_set<N>>;
    auto vec = uniform_data(state.range(0), state.range(1));
    nbit::sparse_set<N, mymap> set;
    for (auto _ : state)
        set.insert(vec.begin(), vec.end());
}

static void AndSet(benchmark::State &state)
{
    auto vec1 = uniform_data(state.range(0), state.range(1));
    auto vec2 = uniform_data(state.range(0), state.range(1));
    nbit::set set1, set2;
    set1.insert(vec1.begin(), vec1.end());
    set2.insert(vec2.begin(), vec2.end());
    for (auto _ : state)
        auto set3 = set1 & set2;
}

static void AndSparseSet(benchmark::State &state)
{
    auto vec1 = uniform_data(state.range(0), state.range(1));
    auto vec2 = uniform_data(state.range(0), state.range(1));
    nbit::sparse_set set1, set2;
    set1.insert(vec1.begin(), vec1.end());
    set2.insert(vec2.begin(), vec2.end());
    for (auto _ : state)
        auto set3 = set1 & set2;
}

static void AndSparseSet2048(benchmark::State &state)
{
    auto vec1 = uniform_data(state.range(0), state.range(1));
    auto vec2 = uniform_data(state.range(0), state.range(1));
    nbit::sparse_set<2048> set1, set2;
    set1.insert(vec1.begin(), vec1.end());
    set2.insert(vec2.begin(), vec2.end());
    for (auto _ : state)
        auto set3 = set1 & set2;
}

static void AndSparseSetWithMap(benchmark::State &state)
{
    auto vec1 = uniform_data(state.range(0), state.range(1));
    auto vec2 = uniform_data(state.range(0), state.range(1));
    constexpr std::size_t N = 65536;
    using mymap = std::map<std::uint64_t, nbit::fixed_set<N>>;
    nbit::sparse_set<2048> set1, set2;
    set1.insert(vec1.begin(), vec1.end());
    set2.insert(vec2.begin(), vec2.end());
    for (auto _ : state)
        auto set3 = set1 & set2;
}

//Register the function as a benchmark
BENCHMARK(InsertSet)->Apply(CustomArguments);
BENCHMARK(InsertFixedSet)->Apply(CustomArguments);
BENCHMARK(InsertSparseSet)->Apply(CustomArguments);
BENCHMARK(InsertSparseSetWithMap)->Apply(CustomArguments);
BENCHMARK(AndSet)->Apply(CustomArguments);
BENCHMARK(AndSparseSet)->Apply(CustomArguments);
BENCHMARK(AndSparseSet2048)->Apply(CustomArguments);
BENCHMARK(AndSparseSetWithMap)->Apply(CustomArguments);

BENCHMARK_MAIN();