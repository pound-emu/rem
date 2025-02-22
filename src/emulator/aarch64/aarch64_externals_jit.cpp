#include "aarch64_impl.h"
#include "aarch64_emit_context.h"
#include "debugging.h"
#include "tools/numbers.h"
#include "iomanip"
#include <tuple>
#include "aarch64_soft_float.h"

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
        case nzcv_n:            return ir_operand::copy_new_raw_size(aarch64_emit_context::get_context_reg_raw(actx,offsets.n_offset),int64); 
        case nzcv_z:            return ir_operand::copy_new_raw_size(aarch64_emit_context::get_context_reg_raw(actx,offsets.z_offset),int64); 
        case nzcv_c:            return ir_operand::copy_new_raw_size(aarch64_emit_context::get_context_reg_raw(actx,offsets.c_offset),int64); 
        case nzcv_v:            return ir_operand::copy_new_raw_size(aarch64_emit_context::get_context_reg_raw(actx,offsets.v_offset),int64); 
        case fpcr:              return aarch64_emit_context::get_context_reg_raw(actx,offsets.fpcr_offset); 
        case fpsr:              return aarch64_emit_context::get_context_reg_raw(actx,offsets.fpsr_offset); 
        case exclusive_address: return aarch64_emit_context::get_context_reg_raw(actx,offsets.exclusive_address_offset); 
        case exclusive_value:   return aarch64_emit_context::get_context_reg_raw(actx,offsets.exclusive_value_offset); 
        case thread_local_0:    return aarch64_emit_context::get_context_reg_raw(actx,offsets.thread_local_0); 
        case thread_local_1:    return aarch64_emit_context::get_context_reg_raw(actx,offsets.thread_local_1); 
        default: throw_error();
    }
}

void _sys_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;
    aarch64_context_offsets offsets = actx->process->guest_context_offset_data;
    
    switch (reg_id)
    {
        case nzcv_n:            aarch64_emit_context::set_context_reg_raw(actx,offsets.n_offset, value);                    break;
        case nzcv_z:            aarch64_emit_context::set_context_reg_raw(actx,offsets.z_offset, value);                    break;
        case nzcv_c:            aarch64_emit_context::set_context_reg_raw(actx,offsets.c_offset, value);                    break;
        case nzcv_v:            aarch64_emit_context::set_context_reg_raw(actx,offsets.v_offset, value);                    break;
        case fpcr:              aarch64_emit_context::set_context_reg_raw(actx,offsets.fpcr_offset, value);                 break;
        case fpsr:              aarch64_emit_context::set_context_reg_raw(actx,offsets.fpsr_offset, value);                 break;
        case exclusive_address: aarch64_emit_context::set_context_reg_raw(actx,offsets.exclusive_address_offset, value);    break;
        case exclusive_value:   aarch64_emit_context::set_context_reg_raw(actx,offsets.exclusive_value_offset, value);      break;
        case thread_local_0:    aarch64_emit_context::set_context_reg_raw(actx,offsets.thread_local_0, value);              break;
        case thread_local_1:    aarch64_emit_context::set_context_reg_raw(actx,offsets.thread_local_1, value);              break;

        default: throw_error();
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
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;
    
    if (actx->translate_functions)
    {
        actx->branch_state = branch_type::short_branch;
        actx->basic_block_translate_que.insert(location);

        ir_operation_block::jump(ctx->ir, aarch64_emit_context::get_or_create_basic_block_label(actx, location));
    }
    else
    {
        aarch64_emit_context::branch_long((aarch64_emit_context*)ctx->context_data, ir_operand::create_con(location));
    }
}

void _branch_conditional_jit(ssa_emit_context* ctx, uint64_t condition_pass, uint64_t condition_fail, ir_operand condition)
{
    ir_operand end = ir_operation_block::create_label(ctx->ir);
    ir_operand yes = ir_operation_block::create_label(ctx->ir);

    ir_operation_block::jump_if(ctx->ir,yes, condition);

    {
        _branch_short_jit(ctx, condition_fail);
    }
    
    ir_operation_block::jump(ctx->ir,end);
    ir_operation_block::mark_label(ctx->ir, yes);

    {
        _branch_short_jit(ctx, condition_pass);
    }

    ir_operation_block::mark_label(ctx->ir, end);
}

ir_operand get_vector_context_jit(ssa_emit_context* ctx)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;
    guest_process* process = actx->process;

    return ssa_emit_context::emit_ssa(ctx, ir_add, actx->context_pointer, ir_operand::create_con(process->guest_context_offset_data.q_offset));
}

void store_context_jit(ssa_emit_context* ctx)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;
    guest_process* process = actx->process;

    aarch64_emit_context::emit_store_context(actx);
}

void load_context_jit(ssa_emit_context* ctx)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;
    guest_process* process = actx->process;

    aarch64_emit_context::emit_load_context(actx);
}

void call_supervisor_jit(ssa_emit_context* ctx, uint64_t svc)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;
    guest_process* process = actx->process;

    aarch64_emit_context::emit_store_context(actx);

    ir_operation_block::emitds(ctx->ir, 
        ir_external_call,
        ssa_emit_context::create_local(ctx, int64),
        ir_operand::create_con((uint64_t)process->svc_function),
        actx->context_pointer,
        ir_operand::create_con(svc)
    );

    ir_operation_block::emits(ctx->ir,ir_close_and_return ,ir_operand::create_con(actx->current_instruction_address + 4));
    
    actx->branch_state = branch_type::long_branch;
}

ir_operand call_counter_jit(ssa_emit_context* ctx)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;
    guest_process* process = actx->process;

    return ssa_emit_context::emit_ssa(ctx, ir_external_call, ir_operand::create_con((uint64_t)process->counter_function));
}

void undefined_with_jit(ssa_emit_context* ctx, uint64_t value)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;

    uint32_t instruction = actx->current_raw_instruction;

    std::cout << "ERROR "<< std::hex << std::setfill('0') << std::setw(16) << value << std::endl;
    std::cout << "Undefined instruction " << std::hex << instruction << " " << std::setfill('0') << std::setw(8) << std::hex << reverse_bytes(instruction) << std::endl;

    throw_error();
}

//Misc

uint64_t _get_pc_jit(ssa_emit_context* ctx)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;

    return actx->current_instruction_address;
}

void undefined_jit(ssa_emit_context* ctx){throw_error();};

ir_operand call_jit(ssa_emit_context* ctx, ir_operand a0, ir_operand a1, ir_operand a2, ir_operand a3, ir_operand a4, ir_operand a5, uint64_t function)
{
    ir_operand result = ssa_emit_context::create_local(ctx,int64);

    ir_operation_block::emitds(ctx->ir, ir_external_call, result, ir_operand::create_con(function), a0, a1, a2, a3, a4, a5);

    return result;
}

//Optimization
uint64_t use_fast_float_jit(ssa_emit_context* ctx)
{
    return 1;
}

uint64_t use_x86_sse_jit(ssa_emit_context* ctx)
{
    return 1;
}

uint64_t use_x86_sse2_jit(ssa_emit_context* ctx)
{
    return 1;
}

uint64_t use_x86_sse41_jit(ssa_emit_context* ctx)
{
    return 1;
}

uint64_t use_x86_jit(ssa_emit_context* ctx)
{
    return 1;
}

uint64_t use_x86_lzcnt_jit(ssa_emit_context* ctx)
{
    return 1;
}

static ir_operand x86_add_subtract_set_flags_jit(ssa_emit_context* ctx,uint64_t O, ir_operand n, ir_operand m, bool is_add)
{
    aarch64_emit_context* actx = (aarch64_emit_context*)ctx->context_data;
    aarch64_context_offsets offsets = actx->process->guest_context_offset_data;

    ir_operand result = ssa_emit_context::create_local(ctx, O);

    ir_operand destinations[] = 
    {
        result,
        aarch64_emit_context::get_context_reg_raw(actx, offsets.n_offset),
        aarch64_emit_context::get_context_reg_raw(actx, offsets.z_offset),
        aarch64_emit_context::get_context_reg_raw(actx, offsets.c_offset),
        aarch64_emit_context::get_context_reg_raw(actx, offsets.v_offset),
    };

    ir_operand sources[] = 
    {
        n,
        m
    };

    ir_operation_block::emit_with(ctx->ir, is_add ? x86_add_flags : x86_sub_flags, destinations, 5, sources, 2);

    if (!is_add)
    {
        ir_operand c = aarch64_emit_context::get_context_reg_raw(actx, offsets.c_offset);

        ir_operation_block::emitds(ctx->ir, ir_bitwise_exclusive_or, c, c, ir_operand::create_con(1, c.meta_data));
    }

    return result;
}

ir_operand x86_add_set_flags_jit(ssa_emit_context* ctx,uint64_t O, ir_operand n, ir_operand m)
{
    return x86_add_subtract_set_flags_jit(ctx, O, n, m, true);
}

ir_operand x86_subtract_set_flags_jit(ssa_emit_context* ctx,uint64_t O, ir_operand n, ir_operand m)
{
    return x86_add_subtract_set_flags_jit(ctx, O, n, m, false);
}

ir_operand intrinsic_unary_jit(ssa_emit_context* ctx,uint64_t R, uint64_t instruction, ir_operand source)
{
    return ssa_emit_context::emit_ssa(ctx, (ir_instructions)instruction, source, R);
}

ir_operand intrinsic_binary_jit(ssa_emit_context* ctx,uint64_t R, uint64_t instruction, ir_operand source_0, ir_operand source_1)
{
    return ssa_emit_context::emit_ssa(ctx, (ir_instructions)instruction, source_0, source_1);
}

ir_operand intrinsic_binary_imm_jit(ssa_emit_context* ctx,uint64_t R, uint64_t instruction, ir_operand source_0, uint64_t source_1)
{
    return ssa_emit_context::emit_ssa(ctx, (ir_instructions)instruction, source_0, ir_operand::create_con(source_1), false);
}

ir_operand intrinsic_ternary_imm_jit(ssa_emit_context* ctx,uint64_t R, uint64_t instruction, ir_operand source_0, ir_operand source_1, uint64_t source_2)
{
    return ssa_emit_context::emit_ssa(ctx, (ir_instructions)instruction, source_0, source_1, ir_operand::create_con(source_2));
}

ir_operand intrinsic_ternary_jit(ssa_emit_context* ctx,uint64_t R, uint64_t instruction, ir_operand source_0, ir_operand source_1, ir_operand source_2)
{
    return ssa_emit_context::emit_ssa(ctx, (ir_instructions)instruction, source_0, source_1, source_2);
}