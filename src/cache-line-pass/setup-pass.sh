#!/bin/bash
LLVM_VERSION=${LLVM_VERSION:9}

mkdir -p build
cd build
cmake -DLLVM_DIR=/usr/lib/llvm-${LLVM_VERSION}/lib/cmake/ ..
make
cd ..

