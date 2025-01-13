#ifndef GUEST_FUNCTION_H
#define GUEST_FUNCTION_H

#include <inttypes.h>

struct guest_function
{
    uint64_t    times_executed;
    uint32_t    jit_offset;
    void        (*raw_function)(void* arguments);
};

#endif