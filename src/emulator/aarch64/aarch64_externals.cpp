#include "aarch64_impl.h"

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


ir_operand _x_jit(ssa_emit_context* ctx, uint64_t reg_id){ throw 0; }
void _x_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value){ throw 0; }
ir_operand _sys_jit(ssa_emit_context* ctx, uint64_t reg_id){ throw 0; }
void _sys_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value){ throw 0; }
void _branch_long_jit(ssa_emit_context* ctx, ir_operand location){ throw 0; }
uint64_t _get_pc_jit(ssa_emit_context* ctx){ throw 0; }