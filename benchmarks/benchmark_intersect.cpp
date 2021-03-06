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

    std::vector<int> sizes{10'000'000};
    std::vector<int> densities{1};
    for (auto size : sizes)
        for (auto d : densities)
            b->Args({size, d});
}

static void AndUniformDataStd(benchmark::State &state)
{
    auto vec1 = uniform_data(state.range(0), state.range(1));
    auto vec2 = uniform_data(state.range(0), state.range(1));

    std::set<std::uint64_t> set1;
    set1.insert(vec1.begin(), vec1.end());
    std::set<std::uint64_t> set2;
    set2.insert(vec2.begin(), vec2.end());
    std::vector<int> v_intersection;
    for (auto _ : state)
    {
        std::set_intersection(set1.begin(), set1.end(),
                              set2.begin(), set2.end(),
                              std::back_inserter(v_intersection));

        v_intersection.erase(std::unique(v_intersection.begin(), v_intersection.end()),
                             v_intersection.end());
    }
}

template <typename SetType>
static void AndUniformData(benchmark::State &state)
{
    auto vec1 = uniform_data(state.range(0), state.range(1));
    auto vec2 = uniform_data(state.range(0), state.range(1));

    SetType set1;
    set1.insert(vec1.begin(), vec1.end());
    SetType set2;
    set2.insert(vec2.begin(), vec2.end());

    for (auto _ : state)
    {
        set1 &= set2;
    }
}

template <typename SetType>
static void AndBinomialData(benchmark::State &state)
{
    auto vec1 = binomial_data(state.range(0), state.range(1));
    auto vec2 = binomial_data(state.range(0), state.range(1));

    SetType set1;
    set1.insert(vec1.begin(), vec1.end());
    SetType set2;
    set2.insert(vec2.begin(), vec2.end());

    for (auto _ : state)
    {
        set1 &= set2;
    }
}

static void AndBinomialDataStd(benchmark::State &state)
{
    auto vec1 = uniform_data(state.range(0), state.range(1));
    auto vec2 = uniform_data(state.range(0), state.range(1));

    std::set<std::uint64_t> set1;
    set1.insert(vec1.begin(), vec1.end());
    std::set<std::uint64_t> set2;
    set2.insert(vec2.begin(), vec2.end());
    std::vector<int> v_intersection;
    for (auto _ : state)
    {
        std::set_intersection(set1.begin(), set1.end(),
                              set2.begin(), set2.end(),
                              std::back_inserter(v_intersection));

        v_intersection.erase(std::unique(v_intersection.begin(), v_intersection.end()),
                             v_intersection.end());
    }
}

// Intersect benchmarks
BENCHMARK_TEMPLATE(AndUniformData, nbit::set<true>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(AndUniformData, nbit::sparse_set<N, map>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(AndUniformData, nbit::sparse_set<N, unordered_map>)->Apply(CustomArguments);
BENCHMARK(AndUniformDataStd)->Apply(CustomArguments);

// Intersect benchmarks
BENCHMARK_TEMPLATE(AndBinomialData, nbit::set<true>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(AndBinomialData, nbit::sparse_set<N, map>)->Apply(CustomArguments);
BENCHMARK_TEMPLATE(AndBinomialData, nbit::sparse_set<N, unordered_map>)->Apply(CustomArguments);
BENCHMARK(AndBinomialDataStd)->Apply(CustomArguments);

BENCHMARK_MAIN();
