#include <cstdlib>
#include <cstdio>

#include "emulator/ssa_emit_context.h"
#include "emulator/guest_process.h"

#define EXPORT __attribute__((visibility("default")))

static void* base_plus_va(void* context, uint64_t virtual_address)
{
    return (char*)context + virtual_address;
}

static void base_plus_va_jit(void* memory, ssa_emit_context* ir, ir_operand destination, ir_operand source)
{
    ir_operation_block* ctx = ir->ir;

    ir_operation_block::emitds(ctx, ir_add, destination, ir_operand::create_con((uint64_t)memory), source);
}

struct export_guest_process
{
    guest_process   process;
    jit_context     jit;
};

extern "C"
{
    EXPORT void* create_arm_process(void* guest_memory, aarch64_context_offsets* offsets)
    {
        export_guest_process* result = new export_guest_process();

        jit_context::create(&result->jit, 1024 * 1024 * 1024, get_abi());
        guest_process::create(&result->process, {guest_memory, base_plus_va, base_plus_va_jit}, &result->jit, *offsets );

        return result;
    }

    EXPORT void destroy_arm_process(export_guest_process* process)
    {
        jit_context::destroy(&process->jit);

        delete process;
    }

    EXPORT uint64_t execute(export_guest_process* process, void* guest_context, uint64_t virtual_address)
    {
        guest_process* guest_process = &process->process;

        return guest_process::jit_function(guest_process, virtual_address, guest_context);
    }
}