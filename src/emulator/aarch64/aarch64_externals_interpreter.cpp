#include "aarch64_impl.h"
#include "aarch64_emit_context.h"
#include <iostream>
#include <iomanip>
#include <mutex>
#include "debugging.h"

std::mutex global_lock;

//Memory


uint64_t translate_address_interpreter(interpreter_data* ctx, uint64_t address)
{
    guest_process* process = ctx->process_context;

    return (uint64_t)process->guest_memory_context.translate_address(process->guest_memory_context.base, address);
}

template<typename T>
T compare_and_swap_impl(uint64_t address_src, T expecting, T to_swap)
{
    T* address = (T*)address_src;

    if (*address == expecting)
    {
        *address = to_swap;

        return true;
    }

    return false;
}

uint64_t _compare_and_swap_interpreter(interpreter_data* ctx, uint64_t physical_address, uint64_t expecting, uint64_t to_swap, uint64_t size)
{
    global_lock.lock();

    bool result;

    switch (size)
    {
        case 8:     result = compare_and_swap_impl<uint8_t>(physical_address, expecting, to_swap); break;
        case 16:    result = compare_and_swap_impl<uint16_t>(physical_address, expecting, to_swap); break;
        case 32:    result = compare_and_swap_impl<uint32_t>(physical_address, expecting, to_swap); break;
        case 64:    result = compare_and_swap_impl<uint64_t>(physical_address, expecting, to_swap); break;
        case 128:   result = compare_and_swap_impl<uint128_t>(physical_address, expecting, to_swap); break;
        default:    throw_error();
    }

    global_lock.unlock();

    return result;
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

void undefined_interpreter(interpreter_data* ctx){throw_error();};

static uint32_t reverse_bytes(uint32_t source)
{
    uint32_t result = 0;

    for (int i = 0; i < 4; ++i)
    {
        int s_bit = (3 - i) * 8;

        result |= ((source >> s_bit) & 255) << (i * 8);
    }

    return result;
}

void undefined_with_interpreter(interpreter_data* ctx, uint64_t value)
{
    uint32_t instruction = ctx->current_instruction;

    std::cout << "ERROR " << value << std::endl;
    std::cout << "Undefined instruction " << std::hex << instruction << " " << std::setfill('0') << std::setw(8) << std::hex << reverse_bytes(instruction) << std::endl;

    throw_error();
}

void call_supervisor_interpreter(interpreter_data* ctx, uint64_t svc)
{
    guest_process* process = (guest_process*)ctx->process_context;

    ((void(*)(void*, int))process->svc_function)(ctx->process_context, svc);
}