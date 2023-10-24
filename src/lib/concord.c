#define _GNU_SOURCE
#include "concord.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <x86intrin.h>

#define DISPATCHER_CORE 4
#define PAGE_SIZE 4096

#define FUNC_ACTION CONCORD_ACT_NONE
#define PIN_DISPATCHER 0

#ifndef ACCURACY_TEST
__thread uint64_t concord_preempt_after_cycle = 8600000000000000;
#else
__thread uint64_t concord_preempt_after_cycle = 16000;
#endif

pthread_t dispatcher_thread;
uint8_t finish_dispatcher = 0;

__thread int concord_preempt_now = 0;
__thread uint64_t concord_start_time;

int concord_timer_reset = 0;
int concord_lock_counter = 0;

void *dispatcher();
void initial_setup();

uint64_t concord_timestamps[1000000];
uint64_t concord_timestamps_counter = 0;
uint64_t concord_timestamp_break_flag = 0;

unsigned long long *mmap_file;

void measurement_init() {
    FILE *fp = fopen(PATH, "w+");
    if (fp == 0) assert(0 && "fopen failed");
    fseek(fp, (1000000 * sizeof(long)) - 1, SEEK_SET);
    fwrite("", 1, sizeof(char), fp);
    fflush(fp);
    fclose(fp);

    int fd = open(PATH, O_RDWR);
    if (fd < 0) assert(0 && "open failed");
    mmap_file = (long long *)mmap(NULL, 1000000 * sizeof(long long), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmap_file == MAP_FAILED) assert(0 && "mmap failed");
    close(fd);
    long gap = PAGE_SIZE / sizeof(long long);
    // Touch each page to load them to the TLB.
    for (long i = 0; i < 1000000; i += gap) mmap_file[i] = 0;
}

int first_time_init = 1;
int concord_enable_log = 1;

void concord_rdtsc_func() {
    if (unlikely(!concord_enable_log)) {
        return;
    }

    if (unlikely(first_time_init)) {
        measurement_init();
        first_time_init = 0;
    }

    concord_start_time = __rdtsc();
    mmap_file[concord_timestamps_counter++] = concord_start_time;
}

void concord_func() {
#if FUNC_ACTION == CONCORD_ACT_LOG
    if (unlikely(!concord_enable_log)) {
        return;
    }

    if (unlikely(first_time_init)) {
        measurement_init();
        first_time_init = 0;
    }

    concord_start_time = __rdtsc();
    mmap_file[concord_timestamps_counter++] = concord_start_time;
#endif

    return;
}

void concord_register_dispatcher() {
    printf("Registering concord dispatcher\n");
    initial_setup();

#if PIN_DISPATCHER == 1
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(DISPATCHER_CORE, &cpuset);
    pthread_create(&dispatcher_thread, NULL, dispatcher, NULL);
    pthread_setaffinity_np(dispatcher_thread, sizeof(cpu_set_t), &cpuset);
#endif
}

void concord_unregister_dispatcher() {
    finish_dispatcher = 1;
    // pthread_join(dispatcher_thread, NULL);
    printf("Dispatcher unregistered\n");
}

void concord_set_preempt_flag(int flag) { concord_preempt_now = flag; }

void concord_disable() {
    // printf("Disabling concord\n");
    concord_lock_counter -= 1;
}

void concord_enable() {
    // printf("Enabling concord\n");
    concord_lock_counter += 1;
}

static inline long long get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

void __attribute__((optimize("O0"))) initial_setup() {
    for (size_t i = 0; i < 300; i++) {
        asm volatile("nop");
        get_time();
        int k = __rdtsc();
    }
}

void *dispatcher() {
    uint64_t last_time;

    while (1) {
        while (concord_preempt_now) {
            asm volatile("nop");
        }
    timer:
        last_time = __rdtsc();
        while (((__rdtsc() - last_time) / 2.5) < 5000) {
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");

            if (concord_timer_reset) {
                printf("Resetting timer\n");
                concord_timer_reset = 0;
                goto timer;
            }
        };

        concord_preempt_now = 1;

        if (unlikely(finish_dispatcher == 1)) {
            break;
        }
    }

    printf("Dispatcher finished\n");

    return NULL;
}