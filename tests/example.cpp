#include "../src/sparse_set.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <set>
#include <unordered_set>
#include <vector>

int main(int argc, char *argv[])
{
    std::string arg = argv[1];
    std::size_t pos;
    int size = std::stoi(arg, &pos);
    std::vector<std::uint64_t> vec(size);
    // std::generate(vec.begin(), vec.end(), rand);
    std::iota(vec.begin(), vec.end(), 0);

    std::chrono::time_point<std::chrono::system_clock> start, end;

    if (atoi(argv[2]) == 0)
    {
        std::cout << "Using std::unordered_set" << std::endl;
        start = std::chrono::system_clock::now();
        std::unordered_set<std::uint64_t> new_set;
        new_set.insert(vec.begin(), vec.end());
        end = std::chrono::system_clock::now();
    }
    else if (atoi(argv[2]) == 1)
    {
        std::cout << "Using std::set" << std::endl;
        start = std::chrono::system_clock::now();
        std::set<std::uint64_t> new_set;
        new_set.insert(vec.begin(), vec.end());
        end = std::chrono::system_clock::now();
    }
    else
    {
        std::cout << "Using nbit::set" << std::endl;
        nbit::set new_set2;
        new_set2.insert_sorted(vec.begin(), vec.end());
        start = std::chrono::system_clock::now();
        // nbit::set new_set;
        // new_set.insert(vec.begin(), vec.end());
        bool b = (new_set2.count() == size);
        end = std::chrono::system_clock::now();
    }

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";
}