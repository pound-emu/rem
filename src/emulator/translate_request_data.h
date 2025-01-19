#ifndef TRANSLATE_REQUEST_DATA_H
#define TRANSLATE_REQUEST_DATA_H

#include <inttypes.h>
#include "guest_function.h"
#include "guest_compiler_optimization_flags.h"

struct translate_request_data;

typedef guest_function (*translate_guest_function)(translate_request_data*, guest_compiler_optimization_flags);

struct translate_request_data
{
    void*                       process;
    uint64_t                    address;
    translate_guest_function    translate_function;
};

#endif