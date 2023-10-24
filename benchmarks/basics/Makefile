
LLVM_VERSION=9
CC=clang-$(LLVM_VERSION)
OPT=opt-$(LLVM_VERSION)
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

CFLAGS=-O3 -lpthread
PASS_FLAGS=-c -emit-llvm

OPT_CONFIG=-postdomtree -mem2reg -indvars -loop-simplify -branch-prob -scalar-evolution 

CONCORD_PASS=$(ROOT_DIR)/../../src/cache-line-pass/build/src/libConcordPass.so
CONCORD_RDTSC_PASS=$(ROOT_DIR)/../../src/rdtsc-pass/build/src/libConcordPass.so

CONCORD_INC=-I$(ROOT_DIR)/../../src/lib/

BENCH := hello

all: clean hello hello_rdtsc

# bench:
# 	$(CC) $(CONCORD_LIB) $(LEVELDB_INC)  $(CFLAGS) $(PASS_FLAGS) $(BENCH).c -o $(BENCH).bc $(CONCORD_INC)
# 	$(OPT) -S $(OPT_CONFIG) < $(BENCH).bc > $(BENCH).opt.bc
# 	$(OPT) -S -load $(CONCORD_PASS) -yield < $(BENCH).opt.bc > $(BENCH).pass.opt.bc
# 	$(CC) $(CFLAGS) $(BENCH).pass.opt.bc $(CONCORD_INC) -o $(BENCH).out

hello: hello.opt.bc
	$(OPT) -S -load $(CONCORD_PASS) -yield < $< > $<.pass.opt.bc
	$(CC) $(CFLAGS) $<.pass.opt.bc $(CONCORD_INC) -o $@.out

hello_rdtsc: hello_rdtsc.opt.bc
	$(OPT) -S -load $(CONCORD_RDTSC_PASS) -yield < $< > $<.pass.opt.bc
	$(CC) $(CFLAGS) $<.pass.opt.bc $(CONCORD_INC) -o $@.out

# general rule to create opt.bc from c file
%.opt.bc: %.c
	$(CC) $(CONCORD_LIB) $(LEVELDB_INC)  $(CFLAGS) $(PASS_FLAGS) $< -o $@ $(CONCORD_INC)
	$(OPT) -S $(OPT_CONFIG) < $@ > $@.opt.bc

clean:
	$(RM) *.o *.bc *.ll *.opt.ll *.out