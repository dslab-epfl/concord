CC	:= gcc
MODULE_LIB := /lib/modules/$(shell uname -r)/build

SCRIPT_DIR := $(shell pwd)
DUNE_DIR = $(SCRIPT_DIR)/../../concord-shinjuku/deps/dune
LDFLAGS := -ldune -lpthread
CFLAGS	:= -Wall -g -MD -O2 -I$(DUNE_DIR)/


# === BENCH params ===

SCHED_QUANTUM := 5000

# 0 ipi
# 1 concord
# 2 rdtsc
# 3 clean
ifeq ($(BENCH),1)
	LOOP_LIB = $(SCRIPT_DIR)/../../benchmarks/looplib/lib/concord_loop.a
else ifeq ($(BENCH),2)
	LOOP_LIB = $(SCRIPT_DIR)/../../benchmarks/looplib/lib/concord_loop_rdtsc.a
else
	LOOP_LIB = $(SCRIPT_DIR)/../../benchmarks/looplib/lib/concord_loop_clear.a
	BENCH = 0
endif

# === BENCH params ===


all: fig2

fig2: fig2.o $(DUNE_DIR)/libdune/libdune.a
	$(CC) $(CFLAGS) -static $(<) -L$(DUNE_DIR)/libdune $(LDFLAGS) -o $(@) $(LOOP_LIB)

fig2.o: fig2.c
	$(CC) $(CFLAGS) -c $(<) -o $(@) -DBENCH_TYPE=$(BENCH) -DSCHED_QUANTUM=$(SCHED_QUANTUM)

clean:
	rm -f *.o test *.d $(tests_files)
	make -C $(MODULE_LIB) M=$(PWD) clean

-include *.d
