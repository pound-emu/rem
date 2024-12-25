#include "aarch64_process.h"
#include "jit/jit_context.h"
#include "aarch64_impl.h"

void aarch64_process::create(aarch64_process* result, guest_memory guest_memory_context, jit_context* host_jit_context, aarch64_context_offsets arm_guest_data)
{
    result->guest_memory_context = guest_memory_context;
    result->host_jit_context = host_jit_context;
    result->guest_context_offset_data = arm_guest_data;

    init_aarch64_decoder(result);
}

uint64_t aarch64_process::jit_function(aarch64_process* process, uint64_t guest_function_address, void* arm_context)
{
    guest_function function_to_execute = guest_function_store::get_or_translate_function(&process->guest_functions, guest_function_address);

    void* arguments[] = { arm_context };

    return jit_context::call_jitted_function(process->host_jit_context, (void*)function_to_execute.raw_function, (uint64_t*)arguments);
}

uint64_t aarch64_process::interperate_function(aarch64_process* process, uint64_t guest_function, void* arm_context)
{
    interpreter_data interpreter;

    interpreter.process_context = process;
    interpreter.register_data = arm_context;
    interpreter.current_pc = guest_function;

    while (true)
    {
        void* physical_memory = process->guest_memory_context.translate_address(process->guest_memory_context.base,interpreter.current_pc);

        uint32_t instruction = *(uint32_t*)physical_memory;

        auto table = fixed_length_decoder<uint32_t>::decode_slow(&process->decoder, instruction);

        if (table == nullptr)
        {
            std::cout << std::hex << "undefined instruction " << instruction << std::endl;

            throw 0;
        }

        interpreter.branch_type = branch_type::no_branch;

        ((void(*)(interpreter_data*, uint32_t))table->interpret)(&interpreter, instruction);

        if (interpreter.branch_type & branch_type::long_branch)
        { 
            break;
        }
        else if (interpreter.branch_type == branch_type::no_branch)
        {
            interpreter.current_pc += 4;
        }
    }

    return interpreter.current_pc;
}