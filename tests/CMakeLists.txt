cmake_minimum_required(VERSION 3.10)

project(nbit_test LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(GTest REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")

add_executable(test_insert test_insert.cpp)
add_executable(test_intersection test_intersection.cpp)
add_executable(test_union test_union.cpp)
add_executable(test_difference test_difference.cpp)
add_executable(test_symmetric_difference test_symmetric_difference.cpp)

target_link_libraries(test_insert  PUBLIC gtest gtest_main pthread)
target_link_libraries(test_intersection PUBLIC gtest gtest_main pthread)
target_link_libraries(test_union PUBLIC gtest gtest_main pthread)
target_link_libraries(test_difference PUBLIC gtest gtest_main pthread)
target_link_libraries(test_symmetric_difference PUBLIC gtest gtest_main pthread)

add_test(NAME test_insert COMMAND test_insert)
add_test(NAME test_intersection COMMAND test_intersection)
add_test(NAME test_union COMMAND test_union)
add_test(NAME test_difference COMMAND test_difference)
add_test(NAME test_symmetric_difference COMMAND test_symmetric_difference)

