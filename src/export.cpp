#include "emulator/guest_process.h"
#include "emulator/ssa_emit_context.h"

#if defined (__linux__) || defined (__APPLE__)
    #define EXPORT __attribute__((visibility("default")))
#elif (defined _WIN32)
    #define EXPORT __declspec(dllexport)
#endif

struct external_context
{
    guest_process   process;
    jit_context     memory;
};

static void* base_plus_va(void* context, uint64_t virtual_address)
{
    return (char*)context + virtual_address;
}

static void base_plus_va_jit(void* memory, ssa_emit_context* ir, ir_operand destination, ir_operand source)
{
    ir_operation_block* ctx = ir->ir;

    ir_operation_block::emitds(ctx, ir_add, destination, source, ir->memory_base);
}

extern "C"
{
    EXPORT void* create_rem_context(void* memory, aarch64_context_offsets* context_offsets, void* svc, void* counter, void* undefined_instruction)
    {
        external_context* result = new external_context;

        result->process.log_native = nullptr;

        jit_context::create(&result->memory, 5ULL * 1024 * 1024 * 1024, get_abi());

        guest_memory guest_memory_context = {memory, base_plus_va, base_plus_va_jit};

        guest_process::create_guest_process(&result->process, guest_memory_context, &result->memory, context_offsets, sizeof(aarch64_context_offsets), cpu_type::arm, cpu_size::_64_bit, memory_order::little_endian);

        result->process.undefined_instruction = undefined_instruction;
        result->process.svc_function = svc;
        result->process.counter_function = counter;

        return result;
    }

    EXPORT void set_log_native(external_context* context, void* log_native)
    {
        context->process.log_native = log_native;
    }
    
    EXPORT void destroy_rem_context(external_context* context)
    {
        jit_context::destroy(&context->memory);

        delete context;
    }

    EXPORT uint64_t interperate_until_long_jump(external_context* context, uint64_t virtual_address, void* guest_context, bool* is_running)
    {
        return guest_process::interperate_function(&context->process, virtual_address, guest_context, is_running);
    }

    EXPORT uint64_t jit_until_long_jump(external_context* context, uint64_t virtual_address, void* guest_context, bool* is_running, void* log_native)
    {
        context->process.log_native = log_native;

        while (*is_running)
        {
            virtual_address = guest_process::jit_function(&context->process, virtual_address, guest_context);
        }

        return virtual_address;
    }

    EXPORT void invalidate_jit_region(external_context* context, uint64_t address, uint64_t size)
    {
        context->process.guest_functions.main_translate_lock.lock();

        for (uint64_t i = 0; i < size; i += 4)
        {
            uint64_t working_address = address + i;

            fast_function_table::insert_function(&context->process.guest_functions.native_function_table, working_address, -1);
            context->process.guest_functions.functions.erase(working_address);
        }

        context->process.guest_functions.main_translate_lock.unlock();
    }
}