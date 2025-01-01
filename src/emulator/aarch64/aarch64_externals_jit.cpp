#include "aarch64_impl.h"
#include "aarch64_emit_context.h"

//Memory 

ir_operand translate_address_jit(ssa_emit_context* ctx, ir_operand address)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;
    guest_process* process = actx->process;

    ir_operand result = ssa_emit_context::create_local(ctx, int64);

    process->guest_memory_context.emit_translate_address(process->guest_memory_context.base, ctx, result, address);

    return result;
}

//Registers 

ir_operand _x_jit(ssa_emit_context* ctx, uint64_t reg_id)
{
    return aarch64_emit_context::get_x_raw((aarch64_emit_context*)ctx->context_data, reg_id);
}

void _x_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value)
{
    aarch64_emit_context::set_x_raw((aarch64_emit_context*)ctx->context_data, reg_id, value);
}

ir_operand _sys_jit(ssa_emit_context* ctx, uint64_t reg_id)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;
    aarch64_context_offsets offsets = actx->process->guest_context_offset_data;

    switch (reg_id)
    {
        case 0: return aarch64_emit_context::get_context_reg_raw(actx,offsets.n_offset); break;
        case 1: return aarch64_emit_context::get_context_reg_raw(actx,offsets.z_offset); break;
        case 2: return aarch64_emit_context::get_context_reg_raw(actx,offsets.c_offset); break;
        case 3: return aarch64_emit_context::get_context_reg_raw(actx,offsets.v_offset); break;
        default: throw 0;
    }
}

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

ir_operand V_jit(ssa_emit_context* ctx, uint64_t reg_id)
{
    return aarch64_emit_context::get_v_raw((aarch64_emit_context*)ctx->context_data, reg_id);
}

void V_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value)
{
    aarch64_emit_context::set_v_raw((aarch64_emit_context*)ctx->context_data, reg_id, value);
}

//Branching

void _branch_long_jit(ssa_emit_context* ctx, ir_operand location)
{
    aarch64_emit_context::branch_long((aarch64_emit_context*)ctx->context_data, location);
}

void _branch_short_jit(ssa_emit_context* ctx, uint64_t location)
{
    aarch64_emit_context::branch_long((aarch64_emit_context*)ctx->context_data, ir_operand::create_con(location));
}

void _branch_conditional_jit(ssa_emit_context* ctx, uint64_t condition_pass, uint64_t condition_fail, ir_operand condition)
{
    ir_operand end = ir_operation_block::create_label(ctx->ir);
    ir_operand yes = ir_operation_block::create_label(ctx->ir);

    ir_operation_block::jump_if(ctx->ir,yes, condition);

    {
        aarch64_emit_context::branch_long((aarch64_emit_context*)ctx->context_data, ir_operand::create_con(condition_fail));
    }
    
    ir_operation_block::jump(ctx->ir,end);
    ir_operation_block::mark_label(ctx->ir, yes);

    {
        aarch64_emit_context::branch_long((aarch64_emit_context*)ctx->context_data, ir_operand::create_con(condition_pass));
    }

    ir_operation_block::mark_label(ctx->ir, end);
}

//Misc

uint64_t _get_pc_jit(ssa_emit_context* ctx)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;

    return actx->current_instruction_address;
}

void undefined_jit(ssa_emit_context* ctx){throw 0;};