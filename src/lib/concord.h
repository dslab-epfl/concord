#ifndef CONCORD_H
#define CONCORD_H

#include <stdlib.h>
#include <stdatomic.h>

#define __atribute__(concord_skip) __attribute__((annotate("concord_skip")))

// Concord Function Actions
#define CONCORD_ACT_NONE 0
#define CONCORD_ACT_LOG  1

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#ifdef __cplusplus
extern "C"{
#endif 
// Register dispatcher
void concord_register_dispatcher();

// Unregister dispatcher
void concord_unregister_dispatcher();

// Concord start timer
void concord_set_preempt_flag(int);

void concord_unlock();
void concord_lock();

void concord_enable();
void concord_disable();
#ifdef __cplusplus
}
#endif

#endif