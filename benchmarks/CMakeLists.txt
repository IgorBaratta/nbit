cmake_minimum_required(VERSION 3.10)

project(nbit_bench LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set c++ flags
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

CHECK_CXX_COMPILER_FLAG("-O3" COMPILER_SUPPORTS_O3)
if(COMPILER_SUPPORTS_O3)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -pthread")

find_package(benchmark REQUIRED)

add_executable(insert benchmark_insert.cpp)
target_link_libraries(insert PRIVATE benchmark::benchmark)

add_executable(intersect benchmark_intersect.cpp)
target_link_libraries(intersect PRIVATE benchmark::benchmark)

add_executable(union benchmark_union.cpp)
target_link_libraries(union PRIVATE benchmark::benchmark)
