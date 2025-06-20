#pragma once

#include <inttypes.h>

#include "guest_compiler_optimization_flags.h"

struct guest_function {
    uint64_t times_executed;
    guest_compiler_optimization_flags optimizations;
    uint32_t jit_offset;
    void (*raw_function)(void* arguments);
};
