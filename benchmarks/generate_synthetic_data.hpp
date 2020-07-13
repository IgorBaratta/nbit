#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

/// Sample nint integers from an uniform distribution in the range [0, nint/10^-exp)
std::vector<std::uint64_t> uniform_data(std::size_t nint, std::size_t exp)
{
    double density = std::pow(10., -1 * (double)exp);
    std::size_t max_value = nint / density;

    std::random_device rd;
    std::mt19937 engine(rd());

    std::uniform_int_distribution<std::uint64_t> distribution(0, max_value);
    auto generator = [&engine, &distribution]() { return distribution(engine); };
    std::vector<std::uint64_t> data_vec(nint);
    std::generate(data_vec.begin(), data_vec.end(), generator);

    return data_vec;
}

/// Sample nint integers from an binomial discrete distribution in the range of
/// [0, nint/10^-exp), each with a probability of success equal to 0,75.
template <typename T>
std::vector<T> binomial_data(std::size_t nint, std::size_t exp)
{
    double density = std::pow(10., -1 * (double)exp);
    std::size_t max_value = nint / density;

    std::random_device rd;
    std::mt19937 engine(rd());

    std::binomial_distribution<> distribution(max_value, 0.75);

    auto generator = [&engine, &distribution]() { return distribution(engine); };
    std::vector<std::uint64_t> data_vec(nint);
    std::generate(data_vec.begin(), data_vec.end(), generator);

    return data_vec;
}
