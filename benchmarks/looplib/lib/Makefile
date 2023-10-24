
LLVM_VERSION=9

CC=clang-$(LLVM_VERSION)
OPT=opt-$(LLVM_VERSION)

CFLAGS=-O3 -g
PASS_FLAGS=-c -emit-llvm

OPT_CONFIG=-postdomtree -mem2reg -indvars -loop-simplify -branch-prob -scalar-evolution 
BENCH=leveldb

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

CONCORD_PASS=$(ROOT_DIR)/../../../src/cache-line-pass/build/src/libConcordPass.so
CONCORD_INC=-I$(ROOT_DIR)/../../../src/lib/

RDTSC_PASS=$(ROOT_DIR)/../../../src/rdtsc-pass/build/src/libConcordPass.so


all: clean concord-leveldb build-test build-test-ubench

concord-leveldb:
	$(CC) $(CONCORD_LIB) $(LEVELDB_INC)  $(CFLAGS) $(PASS_FLAGS) concord-loop.c -o $(BENCH).bc $(CONCORD_INC)
	$(OPT) -S $(OPT_CONFIG) < $(BENCH).bc > $(BENCH).opt.bc

	$(OPT) -S -load $(CONCORD_PASS) -yield < $(BENCH).opt.bc > $(BENCH).pass.opt.bc
	$(OPT) -S -load $(RDTSC_PASS) -yield < $(BENCH).opt.bc > $(BENCH)_rdtsc.pass.opt.bc
	
	$(CC) $(CFLAGS) $(BENCH).pass.opt.bc $(CFLAGS) -c -o concord_loop.a
	$(CC) $(CFLAGS) $(BENCH).opt.bc $(CFLAGS) -c -o concord_loop_clear.a 
	$(CC) $(CFLAGS) $(BENCH)_rdtsc.pass.opt.bc $(CFLAGS) -c -o concord_loop_rdtsc.a

build-test:
	# compile test.c with LEVELDB_LIB and LEVELDB_INC which are c++ libraries
	$(CC) -Wall -fPIC $(CFLAGS) -o test.o test.c concord_loop.a

build-test-ubench:
	# compile test.c with LEVELDB_LIB and LEVELDB_INC which are c++ libraries
	$(CC) -Wall -fPIC $(CFLAGS) -o test_ubench.o concord_loop.a test.c

clean:
	$(RM) *.o *.bc *.ll *.opt.ll *.a *.so