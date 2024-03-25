#!/bin/bash

LLVM_VERSION=9
CC=clang-$LLVM_VERSION
OPT=opt-$LLVM_VERSION
CFLAGS="-O3"
PASS_FLAGS="-c -emit-llvm"
OPT_CONFIG="-postdomtree -mem2reg -indvars -loop-simplify -branch-prob -scalar-evolution"
BENCH="leveldb"

ROOT_DIR=$(dirname $(realpath ${BASH_SOURCE[0]}))

CONCORD_PASS=$ROOT_DIR/../../src/cache-line-pass/build/src/libConcordPass.so
CONCORD_INC="-I$ROOT_DIR/../../src/lib/"
RDTSC_PASS=$ROOT_DIR/../../src/rdtsc-pass/build/src/libConcordPass.so

clean() {
    rm -f *.o *.bc *.ll *.opt.ll *.a *.so
}

# args
# 1: unroll factor
build_manual_unrolled() {
    $CC -Wall -fPIC $CONCORD_LIB $LEVELDB_INC $CFLAGS $PASS_FLAGS unrolled_libs/concord-loop_${1}.c -o $BENCH.bc $CONCORD_INC
    $OPT -S $OPT_CONFIG < $BENCH.bc > ${BENCH}_pass_${1}.opt1.bc
    $OPT -S -load $CONCORD_PASS -yield -loop_body_size $1 -c_instrument 0 < ${BENCH}_pass_${1}.opt1.bc > ${BENCH}_pass_${1}.opt.bc
    $CC -Wall -fPIC ${BENCH}_pass_${1}.opt.bc $CFLAGS -c -o manual_concord_loop_${1}.a
    $CC -Wall -fPIC $CFLAGS -o manual_test_${1}.o unrolled_libs/test_${1}.c manual_concord_loop_${1}.a
}

# args
# 1: unroll factor
concord_leveldb() {
    $CC $CONCORD_LIB $LEVELDB_INC $CFLAGS $PASS_FLAGS concord-loop.c -o $BENCH.bc $CONCORD_INC
    $OPT -S $OPT_CONFIG < $BENCH.bc > $BENCH.opt.bc
    $OPT -S -load $CONCORD_PASS -yield -loop_body_size $1  -c_instrument 1 < $BENCH.opt.bc > ${BENCH}_pass_${1}.opt.bc
    $CC ${BENCH}_pass_${1}.opt.bc $CFLAGS -c -o concord_loop_${1}.a
}

# 1: unroll factor
build_automatic_unrolled() {
    concord_leveldb $1
    $CC $CFLAGS -o automatic_test_${1}.o test.c concord_loop_${1}.a
}

test_all(){
    for i in 50 100 200 500
    do
        # Build manual unrolled
        build_manual_unrolled "$i"

        # Build automatically unrolled
        build_automatic_unrolled $i
    done

    for i in 50 100 200 500
    do
        echo "Testing total time for unroll factor $i"
        echo "****************************************"
        echo "Time taken by manually unrolled loop"
        ./manual_test_${i}.o
        echo "Time taken by automatically unrolled loop"
        ./automatic_test_${i}.o
        echo ""
    done
}

case "$1" in
    clean)
        clean
        ;;
    test)
        clean
        test_all
        clean
        ;;
    *)
        echo "Usage: $0 {clean|test}"
        exit 1
        ;;
esac

exit 0
