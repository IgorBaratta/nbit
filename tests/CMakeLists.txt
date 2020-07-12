cmake_minimum_required(VERSION 3.10)

project(nbit_test CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(GTest REQUIRED)

add_executable(nbit_test test.cpp)

target_link_libraries(nbit_test PUBLIC gtest gtest_main pthread nbit)

add_test(NAME nbit_test COMMAND nbit_test --gtest_repeat=5)
