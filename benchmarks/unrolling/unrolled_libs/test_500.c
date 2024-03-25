#include <stdio.h>
#include "concord-loop.h"
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <x86intrin.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <time.h> 

int preempt_array[1000000];
int preempt_iter = 0;

#define NUM_KEYS 15000

__thread int concord_preempt_now;
__thread uint64_t concord_preempt_after_cycle = 5 * 3 * 1000; // 5us for a 3GHz machine
__thread uint64_t concord_start_time;

void concord_disable()
{
    concord_preempt_now = -1;
}

void concord_enable()
{
    concord_preempt_now = 0;
}

static inline uint64_t get_ns()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_nsec;
}

void concord_func()
{
    printf("concord_func\n");
    preempt_array[preempt_iter++] = get_ns();
    concord_preempt_now = 0;
}

void concord_rdtsc_func()
{
    printf("concord_func\n");
    concord_start_time = __rdtsc();
}


int main()
{

    unsigned long long start, end;

    // access concord_preempt_now to make sure it is in the cache
    concord_preempt_now = 0;

    start = __rdtsc();
    for (size_t i = 0; i < 100; i++)
    {
        simpleloop(200);
    }
    end = __rdtsc();
    printf("Total cycles: %llu\n", end - start);

    return(0);
}