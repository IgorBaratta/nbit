#!bin/bash

# compiler="g++ clang++"
# standard="c++11 c++14 c++17 c++2a"
compiler="g++"
standard="c++17"

export CATCH_CONFIG_FAST_COMPILE
for cpp in $compiler
do
    for std in $standard
    do
        echo "Using $cpp std $std"
        $cpp -g -std=$std tests_main.cpp -c
        $cpp -g -std=$std tests_dynamic.cpp -c
        $cpp -g -std=$std tests_main.o tests_dynamic.o -o tests && ./tests --success
    done
done