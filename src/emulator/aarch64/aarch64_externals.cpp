#include "aarch64_impl.h"
#include "aarch64_emit_context.h"

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

void _branch_long_interpreter(interpreter_data* ctx, uint64_t location)
{
    ctx->branch_type = branch_type::long_branch;
    ctx->current_pc = location;
}

uint64_t _get_pc_interpreter(interpreter_data* ctx)
{
    return ctx->current_pc;
}

uint64_t _sys_interpreter(interpreter_data* ctx, uint64_t reg_id){ throw 0; }

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


ir_operand _x_jit(ssa_emit_context* ctx, uint64_t reg_id)
{
    return aarch64_emit_context::get_x_raw((aarch64_emit_context*)ctx->context_data, reg_id);
}

void _x_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value)
{
    aarch64_emit_context::set_x_raw((aarch64_emit_context*)ctx->context_data, reg_id, value);
}

ir_operand _sys_jit(ssa_emit_context* ctx, uint64_t reg_id){ throw 0; }

void _sys_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;
    aarch64_context_offsets offsets = actx->process->guest_context_offset_data;
    
    switch (reg_id)
    {
        case 0: aarch64_emit_context::set_context_reg_raw(actx,offsets.n_offset, value); break;
        case 1: aarch64_emit_context::set_context_reg_raw(actx,offsets.z_offset, value); break;
        case 2: aarch64_emit_context::set_context_reg_raw(actx,offsets.c_offset, value); break;
        case 3: aarch64_emit_context::set_context_reg_raw(actx,offsets.v_offset, value); break;
        default: throw 0;
    }
}

void _branch_long_jit(ssa_emit_context* ctx, ir_operand location)
{
    aarch64_emit_context::branch_long((aarch64_emit_context*)ctx->context_data, location);
}

uint64_t _get_pc_jit(ssa_emit_context* ctx)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;

    return actx->current_instruction_address;
}