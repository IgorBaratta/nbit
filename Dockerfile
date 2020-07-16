FROM ubuntu:latest as base

WORKDIR /tmp

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get -qq update && \
    apt-get -y install \
    curl \
    libtbb-dev\
    cmake \
    g++ \
    clang-10 lldb-10 lld-10\
    libomp-dev\
    git \
    lcov && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN git clone https://github.com/google/googletest.git &&\
    cd googletest &&\
    mkdir build  &&\
    cd build &&\
    cmake .. &&\
    make install

RUN git clone https://github.com/google/benchmark.git &&\
    git clone https://github.com/google/googletest.git benchmark/googletest &&\
    cd benchmark/ &&\
    mkdir build  &&\
    cd build &&\
    cmake -DCMAKE_BUILD_TYPE=Release .. &&\
    make install
