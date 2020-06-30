#!bin/bash

compiler="g++ clang++"
standard="c++11 c++14 c++17"
# compiler="g++"
# standard="c++17"

export CATCH_CONFIG_FAST_COMPILE
for cpp in $compiler
do
    for std in $standard
    do
        echo "==================================================="
        echo "Using $cpp -std=$std"
        $cpp -g -std=$std tests_main.cpp -c
        $cpp -g -std=$std tests_dynamic.cpp -c
        $cpp -g -std=$std tests_main.o tests_dynamic.o -o tests.out && ./tests.out --reporter compact --success
    done
done