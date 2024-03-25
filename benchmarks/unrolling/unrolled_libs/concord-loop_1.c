#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "concord-loop.h"


#if 0
#define PRE_PROTECTCALL { asm volatile ("cli" :::); }
#define POST_PROTECTCALL { asm volatile ("sti" :::); }
#else 
#define PRE_PROTECTCALL { }
#define POST_PROTECTCALL { }
#endif

#ifdef __cplusplus
extern "C"
{
#endif
int simpleloop(int k);

#ifdef __cplusplus
}
#endif


int simpleloop(int k)
{
    for (int i = 0; i < k; i+=1)
    {
        asm volatile("nop");
    }

    return 1;
}