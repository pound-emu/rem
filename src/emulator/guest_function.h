#ifndef GUEST_FUNCTION_H
#define GUEST_FUNCTION_H

#include <inttypes.h>

struct guest_function
{
    uint64_t    times_executed;
    void        (*raw_function)(void* arguments);
};

#endif