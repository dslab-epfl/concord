
LLVM_VERSION=9
CC=clang-$(LLVM_VERSION)
OPT=opt-$(LLVM_VERSION)

CFLAGS=-O3 -lpthread
PASS_FLAGS=-c -emit-llvm

OPT_CONFIG=-postdomtree -mem2reg -indvars -loop-simplify -branch-prob -scalar-evolution 

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

CONCORD_PASS=$(ROOT_DIR)/../../src/cache-line-pass/build/src/libConcordPass.so
CONCORD_LIB=$(ROOT_DIR)/../../src/lib/concord.a
CONCORD_INC=-I$(ROOT_DIR)/../../src/lib/

BENCH=looplib

all: clean loop-benchmark build-test

loop-benchmark:
	$(CC) $(CONCORD_LIB) $(LEVELDB_INC)  $(CFLAGS) $(PASS_FLAGS) looplib.c -o $(BENCH).bc $(CONCORD_INC)
	$(OPT) -S $(OPT_CONFIG) < $(BENCH).bc > $(BENCH).opt.bc
	$(OPT) -S -load $(CONCORD_PASS) -yield < $(BENCH).opt.bc > $(BENCH).pass.opt.bc
	$(CC) $(BENCH).pass.opt.bc -c  $(CFLAGS) $(CONCORD_INC) -o looplib.a $(CONCORD_LIB)

build-test:
	# compile test.c with LEVELDB_LIB and LEVELDB_INC which are c++ libraries
	$(CC) -Wall -fPIC $(CFLAGS) -o test.o test.c $(CONCORD_INC) looplib.a  $(CONCORD_LIB)

clean:
	$(RM) *.o *.bc *.ll *.opt.ll