#include <algorithm>
#include <random>
#include <vector>

template <typename T = std::uint64_t>
std::vector<T> generate_data(std::size_t vec_size, std::size_t max_value)
{
    std::random_device rd;
    std::mt19937 engine(rd());

    // generate random data sampled from a uniform distribution
    std::uniform_int_distribution<T> distribution(0, max_value);
    auto generator = [&engine, &distribution]() { return distribution(engine); };
    std::vector<T> vec(vec_size);
    std::generate(vec.begin(), vec.end(), generator);
    return vec;
}
