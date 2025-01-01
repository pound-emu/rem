#include "aarch64_impl.h"
#include "aarch64_emit_context.h"

//Memory

uint64_t translate_address_interpreter(interpreter_data* ctx, uint64_t address)
{
    guest_process* process = ctx->process_context;

    return (uint64_t)process->guest_memory_context.translate_address(process->guest_memory_context.base, address);
}

//Registers 

uint64_t _x_interpreter(interpreter_data* ctx, uint64_t reg_id)
{
    aarch64_context_offsets* offsets = &ctx->process_context->guest_context_offset_data;

    assert(reg_id >= 0 && reg_id <= 32);
    
    return ((uint64_t*)((char*)ctx->register_data + offsets->x_offset))[reg_id];
}

void _x_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value)
{
    aarch64_context_offsets* offsets = &ctx->process_context->guest_context_offset_data;

    assert(reg_id >= 0 && reg_id <= 32);

    ((uint64_t*)((char*)ctx->register_data + offsets->x_offset))[reg_id] = value;
}

uint64_t _sys_interpreter(interpreter_data* ctx, uint64_t reg_id)
{
    aarch64_context_offsets* offsets = &ctx->process_context->guest_context_offset_data;

    switch (reg_id)
    {
        case 0: return *(uint64_t*)((char*)ctx->register_data + offsets->n_offset) & 1;break;
        case 1: return *(uint64_t*)((char*)ctx->register_data + offsets->z_offset) & 1;break;
        case 2: return *(uint64_t*)((char*)ctx->register_data + offsets->c_offset) & 1;break;
        case 3: return *(uint64_t*)((char*)ctx->register_data + offsets->v_offset) & 1;break;
        default: throw 0;
    }
}

void _sys_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value)
{
    aarch64_context_offsets* offsets = &ctx->process_context->guest_context_offset_data;

    switch (reg_id)
    {
        case 0: *(uint64_t*)((char*)ctx->register_data + offsets->n_offset) = value & 1;break;
        case 1: *(uint64_t*)((char*)ctx->register_data + offsets->z_offset) = value & 1;break;
        case 2: *(uint64_t*)((char*)ctx->register_data + offsets->c_offset) = value & 1;break;
        case 3: *(uint64_t*)((char*)ctx->register_data + offsets->v_offset) = value & 1;break;
        default: throw 0;
    }
}

uint128_t V_interpreter(interpreter_data* ctx, uint64_t reg_id)
{
    aarch64_context_offsets* offsets = &ctx->process_context->guest_context_offset_data;

    assert(reg_id >= 0 && reg_id <= 32);

    return ((uint128_t*)((char*)ctx->register_data + offsets->q_offset))[reg_id];
}

void V_interpreter(interpreter_data* ctx, uint64_t reg_id, uint128_t value)
{
    aarch64_context_offsets* offsets = &ctx->process_context->guest_context_offset_data;

    assert(reg_id >= 0 && reg_id <= 32);

    ((uint128_t*)((char*)ctx->register_data + offsets->q_offset))[reg_id] = value;
}

//Branching

void _branch_long_interpreter(interpreter_data* ctx, uint64_t location)
{
    ctx->branch_type = branch_type::long_branch;
    ctx->current_pc = location;
}

void _branch_short_interpreter(interpreter_data* ctx, uint64_t location)
{
    ctx->branch_type = branch_type::short_branch;
    ctx->current_pc = location;
}

void _branch_conditional_interpreter(interpreter_data* ctx, uint64_t yes, uint64_t no, uint64_t condition)
{
    ctx->branch_type = branch_type::short_branch;

    if (condition)
    {
        ctx->current_pc = yes;
    }
    else
    {
        ctx->current_pc = no;
    }
}

//Misc

uint64_t _get_pc_interpreter(interpreter_data* ctx)
{
    return ctx->current_pc;
}

void undefined_interpreter(interpreter_data* ctx){throw 0;};