#!/bin/bash
LLVM_VERSION=9

sudo apt-get update -y

# Install dependencies
sudo apt-get install -y \
    llvm-9 \
    clang-9 \
    libconfig-dev \
    libnuma-dev \
    libsnappy-dev \
    cmake \

# # Build cache-line-pass and rdtsc-pass
pushd src/cache-line-pass
bash setup-pass.sh
popd

pushd src/rdtsc-pass
bash setup-pass.sh
popd

# Build Concord lib
pushd src/lib
make
popd

# Build Benchmarks
pushd benchmarks
bash setup.sh
popd