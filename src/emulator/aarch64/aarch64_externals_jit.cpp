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

ir_operand _compare_and_swap_jit(ssa_emit_context* ctx, ir_operand physical_address, ir_operand expecting, ir_operand to_swap, uint64_t size)
{
    switch (size)
    {
        case 8:     size = int8;    break;
        case 16:    size = int16;   break;
        case 32:    size = int32;   break;
        case 64:    size = int64;   break;
        default:    throw_error();
    }

    ir_operand result = ssa_emit_context::create_local(ctx, size);

    ir_operation_block::emitds(ctx->ir,ir_instructions::ir_compare_and_swap, result, physical_address, expecting, to_swap);

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

    aarch64_emit_context::emit_load_context(actx);
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

uint64_t x86_enable_sse_jit(ssa_emit_context* ctx)
{
    return true;
}

uint64_t x86_enable_avx_jit(ssa_emit_context* ctx)
{
    return true;
}

ir_operand FPAdd_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
    ir_operand function = ir_operand::create_con((uint64_t)&FPAdd_I);
    ir_operand result = ssa_emit_context::create_local(ctx, int64);
    
    ir_operation_block::emitds(ctx->ir, ir_external_call, result, function,  operand1, operand2, FPCR, ir_operand::create_con(N));

    return result;
}

ir_operand FPSub_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
    ir_operand function = ir_operand::create_con((uint64_t)&FPSub_I);
    ir_operand result = ssa_emit_context::create_local(ctx, int64);
    
    ir_operation_block::emitds(ctx->ir, ir_external_call, result, function, operand1, operand2, FPCR, ir_operand::create_con(N));

    return result;
}

ir_operand FPMul_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
    ir_operand function = ir_operand::create_con((uint64_t)&FPMul_I);
    ir_operand result = ssa_emit_context::create_local(ctx, int64);
    
    ir_operation_block::emitds(ctx->ir, ir_external_call, result, function, operand1, operand2, FPCR, ir_operand::create_con(N));

    return result;
}

ir_operand FPDiv_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
    ir_operand function = ir_operand::create_con((uint64_t)&FPDiv_I);
    ir_operand result = ssa_emit_context::create_local(ctx, int64);
    
    ir_operation_block::emitds(ctx->ir, ir_external_call, result, function, operand1, operand2, FPCR, ir_operand::create_con(N));

    return result;
}

ir_operand FPMax_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
    ir_operand function = ir_operand::create_con((uint64_t)&FPMax_I);
    ir_operand result = ssa_emit_context::create_local(ctx, int64);
    
    ir_operation_block::emitds(ctx->ir, ir_external_call, result, function, operand1, operand2, FPCR, ir_operand::create_con(N));

    return result;
}

ir_operand FPMin_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
    ir_operand function = ir_operand::create_con((uint64_t)&FPMin_I);
    ir_operand result = ssa_emit_context::create_local(ctx, int64);
    
    ir_operation_block::emitds(ctx->ir, ir_external_call, result, function, operand1, operand2, FPCR, ir_operand::create_con(N));

    return result;
}

ir_operand FPMaxNum_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
    ir_operand function = ir_operand::create_con((uint64_t)&FPMaxNum_I);
    ir_operand result = ssa_emit_context::create_local(ctx, int64);
    
    ir_operation_block::emitds(ctx->ir, ir_external_call, result, function, operand1, operand2, FPCR, ir_operand::create_con(N));

    return result;
}

ir_operand FPMinNum_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
    ir_operand function = ir_operand::create_con((uint64_t)&FPMinNum_I);
    ir_operand result = ssa_emit_context::create_local(ctx, int64);
    
    ir_operation_block::emitds(ctx->ir, ir_external_call, result, function, operand1, operand2, FPCR, ir_operand::create_con(N));

    return result;
}

ir_operand FPSqrt_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N)
{
    ir_operand function = ir_operand::create_con((uint64_t)&FPSqrt_I);
    ir_operand result = ssa_emit_context::create_local(ctx, int64);
    
    ir_operation_block::emitds(ctx->ir, ir_external_call, result, function, operand, FPCR, ir_operand::create_con(N));

    return result;
}

ir_operand FPNeg_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N)
{
    ir_operand function = ir_operand::create_con((uint64_t)&FPNeg_I);
    ir_operand result = ssa_emit_context::create_local(ctx, int64);
    
    ir_operation_block::emitds(ctx->ir, ir_external_call, result, function, operand, FPCR, ir_operand::create_con(N));

    return result;
}

ir_operand FPAbs_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N)
{
    ir_operand function = ir_operand::create_con((uint64_t)&FPAbs_I);
    ir_operand result = ssa_emit_context::create_local(ctx, int64);
    
    ir_operation_block::emitds(ctx->ir, ir_external_call, result, function, operand, FPCR, ir_operand::create_con(N));

    return result;
}