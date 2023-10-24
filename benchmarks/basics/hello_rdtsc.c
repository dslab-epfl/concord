#include <stdint.h>
#include <stdio.h>

__thread uint64_t concord_start_time;
__thread uint64_t concord_preempt_after_cycle = 16000;

unsigned long long exit_timestamps[1000000];
unsigned long long exit_timestamps_counter = 0;

void concord_rdtsc_func() {
    exit_timestamps[exit_timestamps_counter++] = __rdtsc();
    concord_start_time = __rdtsc();
    return;
}

void hello(int count) {
    for (int i = 0; i < count; i++) {
        asm("nop");
        asm("nop");
    }
}

int main(int argc, char **argv) {
    int count = argc > 1 ? atoi(argv[1]) : 10000;
    hello(count);

    for (int i = 0; i < exit_timestamps_counter - 1; i++) {
        printf("%llu\n", exit_timestamps[i + 1] - exit_timestamps[i]);
    }

    return 0;
}
