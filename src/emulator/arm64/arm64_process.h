#ifndef ARM_PROCESS_H
#define ARM_PROCESS_H

#include "emulator/guest_memory.h"
#include "emulator/guest_function_store.h"
#include "jit/jit_context.h"

#include "arm64_context_offsets.h"

struct arm64_process
{
    guest_memory            guest_memory_context;
    jit_context*            host_jit_context;
    guest_function_store    guest_functions;
    arm64_context_offsets   arm_guest_data;

    static void             create(arm64_process* result, guest_memory guest_memory_context, jit_context* host_jit_context, arm64_context_offsets arm_guest_data);
    static uint64_t         execute_function(arm64_process* process, uint64_t guest_function, void* arm_context);
    static uint64_t         interperate_function(arm64_process* process, uint64_t guest_function, void* arm_context);
};

#endif