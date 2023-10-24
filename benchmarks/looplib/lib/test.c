#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <x86intrin.h>

#include "concord-loop.h"

int preempt_array[1000000];
int preempt_iter = 0;

#define NUM_KEYS 15000

__thread int concord_preempt_now;
__thread uint64_t concord_preempt_after_cycle = 5000 * 3.3;
__thread uint64_t concord_start_time;

void concord_disable() { concord_preempt_now = -1; }

void concord_enable() { concord_preempt_now = 0; }

static inline uint64_t get_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_nsec;
}

void concord_func() {
    printf("concord_func\n");
    preempt_array[preempt_iter++] = get_ns();
    concord_preempt_now = 0;
}

void concord_rdtsc_func() {
    printf("concord_func\n");
    concord_start_time = __rdtsc();
}

int main() {
    for (size_t i = 0; i < 100; i++) {
        simpleloop(1000000);
    }

    for (size_t i = 1; i < preempt_iter; i++) {
        printf("%d ,", preempt_array[i] - preempt_array[i - 1]);
    }
    printf("\n");

    return (0);
}