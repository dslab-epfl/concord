
LLVM_VERSION=9

CC=clang-$(LLVM_VERSION)
OPT=opt-$(LLVM_VERSION)

CFLAGS=-O3 -lpthread -g
PASS_FLAGS=-c -emit-llvm

OPT_CONFIG=-postdomtree -mem2reg -indvars -loop-simplify -branch-prob -scalar-evolution 
BENCH=leveldb

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

CONCORD_PASS=$(ROOT_DIR)/../../../src/cache-line-pass/build/src/libConcordPass.so
CONCORD_INC=-I$(ROOT_DIR)/../../../src/lib/

LEVELDB_LIB=$(ROOT_DIR)/../leveldb/concord_libleveldb.a
LEVELDB_LIB_CLEAR=$(ROOT_DIR)/../leveldb/concord_libleveldb_clear.a
LEVELDB_LIB_RDTSC=$(ROOT_DIR)/../leveldb/concord_libleveldb_rdtsc.a

LEVELDB_INC=-I$(ROOT_DIR)/../leveldb/include

RDTSC_PASS=$(ROOT_DIR)/../../../src/rdtsc-pass/build/src/libConcordPass.so


all: clean concord-leveldb build-test build-test-ubench build-test-clear build-test-rdtsc build-dl-test

concord-leveldb:
	$(CC) $(CONCORD_LIB) $(LEVELDB_INC)  $(CFLAGS) $(PASS_FLAGS) concord-leveldb.c -o $(BENCH).bc $(CONCORD_INC)
	$(OPT) -S $(OPT_CONFIG) < $(BENCH).bc > $(BENCH).opt.bc

	$(OPT) -S -load $(CONCORD_PASS) -yield < $(BENCH).opt.bc > $(BENCH).pass.opt.bc
	$(OPT) -S -load $(RDTSC_PASS) -yield < $(BENCH).opt.bc > $(BENCH)_rdtsc.pass.opt.bc
	
	$(CC) $(BENCH).pass.opt.bc $(CFLAGS) -c -o concord_apileveldb.a
	$(CC) $(BENCH).bc $(CFLAGS) -c -o concord_apileveldb_clear.a 
	$(CC) $(BENCH)_rdtsc.pass.opt.bc $(CFLAGS) -c -o concord_apileveldb_rdtsc.a 
	$(CC) $(BENCH)_rdtsc.pass.opt.bc $(CFLAGS) $(LEVELDB_LIB_RDTSC) -shared -o concord_apileveldb_rdtsc.so 

build-test:
	# compile test.c with LEVELDB_LIB and LEVELDB_INC which are c++ libraries
	$(CC) -Wall -fPIC $(CFLAGS) $(LEVELDB_LIB) -o test.o test.c concord_apileveldb.a $(LEVELDB_INC) $(CONCORD_INC) -lsnappy -lpthread -lrt -lm -lstdc++ -ldl

build-test-ubench:
	# compile test.c with LEVELDB_LIB and LEVELDB_INC which are c++ libraries
	$(CC) -Wall -fPIC $(CFLAGS) $(LEVELDB_LIB_CLEAR) -o test_ubench.o test.c concord_apileveldb.a $(LEVELDB_INC) $(CONCORD_INC) -lsnappy -lpthread -lrt -lm -lstdc++

build-test-clear:
	# compile test.c with LEVELDB_LIB and LEVELDB_INC which are c++ libraries
	$(CC) -Wall -fPIC $(CFLAGS) $(LEVELDB_LIB_CLEAR) -o test_clear.o test.c concord_apileveldb_clear.a $(LEVELDB_INC) $(CONCORD_INC) -lsnappy -lpthread -lrt -lm -lstdc++
	
build-test-rdtsc:
	# compile test.c with LEVELDB_LIB and LEVELDB_INC which are c++ libraries
	$(CC) -Wall -fPIC $(CFLAGS) $(LEVELDB_LIB_RDTSC) -o test_rdtsc.o test.c concord_apileveldb_rdtsc.a $(LEVELDB_INC) $(CONCORD_INC) -lsnappy -lpthread -lrt -lm -lstdc++

build-dl-test:
	$(CC) -Wall -fPIC $(CFLAGS) $(LEVELDB_LIB_CLEAR) -o dl_test.o dl-test.c concord_apileveldb_clear.a $(LEVELDB_INC) $(CONCORD_INC) -lsnappy -lpthread -lrt -lm -lstdc++ -ldl -rdynamic

clean:
	$(RM) *.o *.bc *.ll *.opt.ll *.a *.so
