#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

std::vector<std::uint64_t>
uniform_data(std::size_t size, std::size_t exp)
{
    double density = 1. / (1 << exp);
    std::size_t max_value = size / density;

    std::random_device rd;
    std::mt19937 engine(rd());

    std::uniform_int_distribution<std::uint64_t> distribution(0, max_value);
    auto generator = [&engine, &distribution]() { return distribution(engine); };
    std::vector<std::uint64_t> data_vec(size);
    std::generate(data_vec.begin(), data_vec.end(), generator);

    return data_vec;
}

template <typename T>
std::vector<T> binomial_data(std::size_t size, std::size_t exp)
{
    double density = 1. / (1 << exp);
    std::size_t max_value = size / density;

    std::random_device rd;
    std::mt19937 engine(rd());

    std::binomial_distribution<> distribution(max_value, 0.75);

    auto generator = [&engine, &distribution]() { return distribution(engine); };
    std::vector<std::uint64_t> data_vec(size);
    std::generate(data_vec.begin(), data_vec.end(), generator);

    return data_vec;
}
