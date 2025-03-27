#ifndef GUEST_PROCESS_H
#define GUEST_PROCESS_H

#include "emulator/guest_memory.h"
#include "emulator/guest_function_store.h"
#include "jit/jit_context.h"

#include "aarch64/aarch64_context_offsets.h"
#include "emulator/fixed_length_decoder.h"

#include "guest_compiler_optimization_flags.h"

#include "cpu_type.h"

struct guest_process
{
    guest_memory                    guest_memory_context;
    jit_context*                    host_jit_context;
    guest_function_store            guest_functions;
    fixed_length_decoder<uint32_t>  fixed_length_decoder_context;

    uint8_t                         guest_context_data[1024];

    void*                           svc_function;
    void*                           counter_function;
    void*                           undefined_instruction;
    void*                           log_native;

    bool                            debug_mode;

    void*                           interperate_function_reference;
    void*                           jit_function_reference;

    cpu_type                        process_type;
    cpu_size                        process_size;
    memory_order                    process_memory_order;

    void*                           process_data;

    static uint64_t                 jit_function(guest_process* process, uint64_t guest_function, void* arm_context);
    static uint64_t                 interperate_function(guest_process* process, uint64_t guest_function, void* arm_context, bool* is_running, bool exit_on_long_branch = false);

    static guest_function           translate_function(translate_request_data* data, guest_compiler_optimization_flags flags);

    static void                     create(guest_process* result,guest_memory memory,jit_context* jit, cpu_type process_type, cpu_size process_size, memory_order process_memory_order);
    static void                     destroy(guest_process* process);

    static void                     create_guest_process(guest_process* result, guest_memory guest_memory_context, jit_context* host_jit_context, void* context_data, int context_data_size, cpu_type cpu, cpu_size size, memory_order order);
};

#endif