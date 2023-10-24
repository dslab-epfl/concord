#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <x86intrin.h>

#include "libdune/cpu-x86.h"
#include "libdune/dune.h"

#define NUM_THREADS 10
#define TEST_VECTOR 0xF2

#define WORKER_CORE 2

#ifndef BENCH_TYPE
#define BENCH_TYPE 0
#endif

#ifndef SCHED_QUANTUM
#define SCHED_QUANTUM 5000
#endif

bool bench_finished = false;
bool worker_initialized = false;
uint64_t packet_processed = 0;

// Concord
__thread uint64_t concord_preempt_now = 0;
uint64_t *preempt_points[NUM_THREADS] = {0};

// Concord-rdtsc
__thread uint64_t concord_preempt_after_cycle = SCHED_QUANTUM * 2.5;
__thread uint64_t concord_start_time = 0;

// Extern simple loop
extern int simpleloop(int k);

void concord_rdtsc_func() {
    concord_start_time = __rdtsc();
    return;
}

void concord_func() {
    concord_preempt_now = 0;
    return;
}

static void test_handler(struct dune_tf *tf) { dune_apic_eoi(); }

void *worker(void *arg) {
#if BENCH_TYPE == 0
    printf("Worker thread running on core %d\n", sched_getcpu());

    volatile int ret = dune_enter();
    if (ret) {
        printf("posted_ipi: failed to enter dune in thread 2\n");
        return NULL;
    }
    dune_register_intr_handler(TEST_VECTOR, test_handler);
#elif BENCH_TYPE == 1
    preempt_points[WORKER_CORE] = &concord_preempt_now;
#endif

    printf("Worker going to start\n");
    worker_initialized = true;

    while (!bench_finished) {
        simpleloop(6200);
        packet_processed++;
    };

    return NULL;
}

void dispatcher() {
    uint64_t bench_run_time = 2 * 1e9;
    uint64_t bench_start_time = __rdtsc();
    uint64_t last_time;

    while (((__rdtsc() - bench_start_time) / 2.5) < bench_run_time) {
#if BENCH_TYPE != 3
        last_time = __rdtsc();

        while (((__rdtsc() - last_time) / 2.5) < SCHED_QUANTUM) {
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
        };

#if BENCH_TYPE == 0
        dune_apic_send_posted_ipi(TEST_VECTOR, WORKER_CORE);
#elif BENCH_TYPE == 1
        *(preempt_points[WORKER_CORE]) = 1;
#endif

#endif
    }

    bench_finished = true;
    printf("Dispatcher finished\n");
}

int main(int argc, char *argv[]) {
    volatile int ret;

    printf("Starting benchmark\n");
    printf("SCHED_QUANTUM: %d\n", SCHED_QUANTUM);
    printf("BENCH_TYPE: %d\n", BENCH_TYPE);

    ret = dune_init_and_enter();
    if (ret) {
        printf("failed to initialize dune\n");
        return ret;
    }

    pthread_t worker_t;
    pthread_attr_t attr;
    cpu_set_t cpus;
    pthread_attr_init(&attr);
    CPU_ZERO(&cpus);
    CPU_SET(WORKER_CORE, &cpus);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
    pthread_create(&worker_t, &attr, worker, NULL);

    while (!worker_initialized) {
        asm volatile("nop");
    };

    // Start dispatcher
    printf("Starting dispatcher\n");
    dispatcher();

    // End worker
    pthread_join(worker_t, NULL);

    // Print results
    printf("Packets processed: %lu\n", packet_processed);

    return 0;
}
