#include <stdio.h>

__thread int concord_preempt_now = 0;

void concord_func() {
    printf("Program Modified !! \n");
    return;
}

void hello(int count) {
    for (int i = 0; i < count; i++) {
        printf("Hello, world!\n");
    }
}

int main(int argc, char **argv) {
    int count = argc > 1 ? atoi(argv[1]) : 1;

    concord_preempt_now = 1;

    hello(count);

    return 0;
}
