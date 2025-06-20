#pragma once

#include <inttypes.h>
#include "guest_compiler_optimization_flags.h"
#include "guest_function.h"

struct translate_request_data;

typedef guest_function (*translate_guest_function)(translate_request_data*, guest_compiler_optimization_flags);

struct translate_request_data {
    void* process;
    uint64_t address;
    translate_guest_function translate_function;
};
