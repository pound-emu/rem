#include "emulator/guest_process.h"
#include "emulator/ssa_emit_context.h"

#if (defined __linux__) || (defined __APPLE__)
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

    ir_operation_block::emitds(ctx, ir_add, destination, source, ir_operand::create_con((uint64_t)memory));
}

extern "C"
{
    EXPORT void* create_rem_context(void* memory, aarch64_context_offsets* context_offsets, void* svc, void* counter)
    {
        external_context* result = new external_context;

        jit_context::create(&result->memory, 1ULL * 1024 * 1024 * 1024, get_abi());
        guest_process::create(&result->process, {memory, base_plus_va, base_plus_va_jit}, &result->memory, *context_offsets);

        result->process.svc_function = svc;
        result->process.counter_function = counter;

        return result;
    }
    
    EXPORT void destroy_rem_context(external_context* context)
    {
        jit_context::destroy(&context->memory);

        delete context;
    }

    EXPORT uint64_t interperate_until_long_jump(external_context* context, uint64_t virtual_address, void* guest_context)
    {
        return guest_process::interperate_function(&context->process, virtual_address, guest_context);
    }

    EXPORT uint64_t jit_until_long_jump(external_context* context, uint64_t virtual_address, void* guest_context)
    {
        while (1)
        {
            virtual_address = guest_process::jit_function(&context->process, virtual_address, guest_context);
        }
    }
}