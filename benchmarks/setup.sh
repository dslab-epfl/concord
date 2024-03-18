#!/bin/bash

BENCHS=("looplib")

for bench in "${BENCHS[@]}"; do
    echo "Running $bench"
    pushd $bench
    ./setup.sh
    popd
done