cmake_minimum_required(VERSION 3.10)

project(nbit_test LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(GTest REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")

add_executable(nbit_test test.cpp)

target_compile_options(nbit_test PUBLIC -fprofile-arcs -ftest-coverage)
target_link_libraries(nbit_test PUBLIC gtest gtest_main pthread -fprofile-arcs -ftest-coverage)


# collect all source files from the chosen include dir
file(GLOB_RECURSE SOURCE_FILES src/set.hpp)


add_custom_target(lcov
    COMMAND lcov --directory . --capture --output-file coverage.info # parse coverage data
    COMMAND lcov -e coverage.info ${SOURCE_FILES} --output-file coverage.info # filter
    COMMAND coveralls-lcov coverage.info  # upload results
)

add_test(NAME nbit_test COMMAND nbit_test)
