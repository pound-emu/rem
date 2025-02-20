#include "aarch64_impl.h"
#include "aarch64_emit_context.h"
#include <iostream>
#include <iomanip>
#include <mutex>
#include "debugging.h"
#include "tools/numbers.h"

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
        case nzcv_n:            return *((char*)ctx->register_data + offsets->n_offset) & 1;
        case nzcv_z:            return *((char*)ctx->register_data + offsets->z_offset) & 1;
        case nzcv_c:            return *((char*)ctx->register_data + offsets->c_offset) & 1;
        case nzcv_v:            return *((char*)ctx->register_data + offsets->v_offset) & 1;
        case fpcr:              return *(uint64_t*)((uint64_t)ctx->register_data + offsets->fpcr_offset);
        case fpsr:              return *(uint64_t*)((uint64_t)ctx->register_data + offsets->fpsr_offset);
        case exclusive_value:   return *(uint64_t*)((uint64_t)ctx->register_data + offsets->exclusive_value_offset);
        case exclusive_address: return *(uint64_t*)((uint64_t)ctx->register_data + offsets->exclusive_address_offset);
        case thread_local_0:    return *(uint64_t*)((uint64_t)ctx->register_data + offsets->thread_local_0);
        case thread_local_1:    return *(uint64_t*)((uint64_t)ctx->register_data + offsets->thread_local_1);
        default: throw_error();
    }
}

void _sys_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value)
{
    aarch64_context_offsets* offsets = &ctx->process_context->guest_context_offset_data;

    switch (reg_id)
    {
        case nzcv_n:            *((char*)ctx->register_data + offsets->n_offset) = value & 1;                                   break;
        case nzcv_z:            *((char*)ctx->register_data + offsets->z_offset) = value & 1;                                   break;
        case nzcv_c:            *((char*)ctx->register_data + offsets->c_offset) = value & 1;                                   break;
        case nzcv_v:            *((char*)ctx->register_data + offsets->v_offset) = value & 1;                                   break;
        case fpcr:              *(uint64_t*)((uint64_t)ctx->register_data + offsets->fpcr_offset) = value;                      break;
        case fpsr:              *(uint64_t*)((uint64_t)ctx->register_data + offsets->fpsr_offset) = value;                      break;
        case exclusive_value:   *(uint64_t*)((uint64_t)ctx->register_data + offsets->exclusive_value_offset) = value;           break;
        case exclusive_address: *(uint64_t*)((uint64_t)ctx->register_data + offsets->exclusive_address_offset) = value;         break;
        case thread_local_0:    *(uint64_t*)((uint64_t)ctx->register_data + offsets->thread_local_0) = value;                   break;
        case thread_local_1:    *(uint64_t*)((uint64_t)ctx->register_data + offsets->thread_local_1) = value;                   break;
        default: throw_error();
    }
}

uint128_t V_interpreter(interpreter_data* ctx, uint64_t reg_id)
{
    aarch64_context_offsets* offsets = &ctx->process_context->guest_context_offset_data;

    assert(reg_id >= 0 && reg_id <= 32);

	void* data_pointer = (char*)ctx->register_data + offsets->q_offset + (reg_id * 16);

	return *(uint128_t*)data_pointer;
}

void V_interpreter(interpreter_data* ctx, uint64_t reg_id, uint128_t value)
{
    aarch64_context_offsets* offsets = &ctx->process_context->guest_context_offset_data;

    assert(reg_id >= 0 && reg_id <= 32);

    void* data_pointer = (char*)ctx->register_data + offsets->q_offset + (reg_id * 16);

	*(uint128_t*)data_pointer = value;
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

uint64_t get_vector_context_interpreter(interpreter_data* ctx)
{
    aarch64_context_offsets* offsets = &ctx->process_context->guest_context_offset_data;

	void* data_pointer = (char*)ctx->register_data + offsets->q_offset;

	return (uint64_t)data_pointer;
}

void store_context_interpreter(interpreter_data* ctx){};
void load_context_interpreter(interpreter_data* ctx){};

//Misc

uint64_t _get_pc_interpreter(interpreter_data* ctx)
{
    return ctx->current_pc;
}

void undefined_interpreter(interpreter_data* ctx){throw_error();};

uint64_t x86_enable_sse_interpreter(interpreter_data* ctx)
{
    return false;
}

uint64_t x86_enable_avx_interpreter(interpreter_data* ctx)
{
    return false;
}

void undefined_with_interpreter(interpreter_data* ctx, uint64_t value)
{
    uint32_t instruction = ctx->current_instruction;

    std::cout << "ERROR "<< std::hex << std::setfill('0') << std::setw(16) << value << std::endl;
    std::cout << "Undefined instruction " << std::hex << instruction << " " << std::setfill('0') << std::setw(8) << std::hex << reverse_bytes(instruction) << std::endl;

    throw_error();
}

void call_supervisor_interpreter(interpreter_data* ctx, uint64_t svc)
{
    guest_process* process = (guest_process*)ctx->process_context;

    ((void(*)(void*, int))process->svc_function)(ctx->process_context, svc);

    ctx->branch_type = long_branch;
    ctx->current_pc = ctx->current_pc + 4;
}

uint64_t call_counter_interpreter(interpreter_data* ctx)
{
    guest_process* process = (guest_process*)ctx->process_context;

    return ((uint64_t(*)())process->counter_function)();
}

uint64_t call_interpreter(interpreter_data* ctx, uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t function)
{
	return ((uint64_t(*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t))function)(a0, a1, a2, a3, a4, a5);
}

//Optimize
uint64_t use_x86_sse_interpreter(interpreter_data* ctx)
{
    return false;
}

uint64_t use_x86_sse2_interpreter(interpreter_data* ctx)
{
    return false;
}


uint64_t use_fast_float_interpreter(interpreter_data* ctx)
{
    return false;
}

uint64_t use_x86_sse41_interpreter(interpreter_data* ctx)
{
    return true;
}

uint64_t use_x86_interpreter(interpreter_data* ctx)
{
    return false;
}

uint64_t use_x86_lzcnt_interpreter(interpreter_data* ctx)
{
    return false;
}