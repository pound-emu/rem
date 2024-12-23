#include "arm64_process.h"
#include "jit/jit_context.h"

void arm64_process::create(arm64_process* result, guest_memory guest_memory_context, jit_context* host_jit_context, arm64_context_offsets arm_guest_data)
{
    result->guest_memory_context = guest_memory_context;
    result->host_jit_context = host_jit_context;
    result->arm_guest_data = arm_guest_data;
}

uint64_t arm64_process::execute_function(arm64_process* process, uint64_t guest_function_address, void* arm_context)
{
    guest_function function_to_execute = guest_function_store::get_or_translate_function(&process->guest_functions, guest_function_address);

    void* arguments[] = { arm_context };

    return jit_context::call_jitted_function(process->host_jit_context, (void*)function_to_execute.raw_function, (uint64_t*)arguments);
}

uint64_t arm64_process::interperate_function(arm64_process* process, uint64_t guest_function, void* arm_context)
{
    throw 0;
}