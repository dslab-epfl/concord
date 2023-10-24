#!/bin/sh
SCRIPT_DIR=$(dirname $0)
SHINJUKU_DIR=$SCRIPT_DIR/../../concord-shinjuku
DUNE_DIR=$SHINJUKU_DIR/deps/dune

bash $SHINJUKU_DIR/deps/fetch-deps.sh

pushd $DUNE_DIR
rmmod dune
sudo make -s -j16
sudo insmod kern/dune.ko

# Run the test program
make -C test
sudo ./test/hello
popd

# Build simpleloop
pushd $SCRIPT_DIR/../../benchmarks/looplib/lib
make    
popd

# Run the benchmark
bash run.sh