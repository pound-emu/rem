#include "guest_register_store.h"
#include "assert.h"
#include "ssa_emit_context.h"

void guest_register_store::create(guest_register_store* result, ssa_emit_context* ir, int guest_context_size)
{
    result->ssa_emit_context_context = ir;
    result->guest_context_size = guest_context_size;

    result->guest_registers = (guest_register*)arena_allocator::allocate_recursive(ir->ir->allocator, guest_context_size * sizeof(guest_register));

    for (int i = 0; i < guest_context_size; ++i)
    {
        result->guest_registers[i].free_guest = true;
        result->guest_registers[i].mode = guest_usage_none;
    }
}

void guest_register_store::create_register(guest_register_store* guest_register_store_context, int offset, uint64_t type)
{
    assert(offset <= guest_register_store_context->guest_context_size);

    guest_register* result = &guest_register_store_context->guest_registers[offset];

    int size = type & UINT32_MAX;
    int byte_count = 1 << size;

    for (int i = 0; i < size; ++i)
    {
        assert(result[i].free_guest);
    }

    result->free_guest = false;
    result->raw_register = ssa_emit_context::create_global(guest_register_store_context->ssa_emit_context_context, type);
}

ir_operand guest_register_store::request_guest_register(guest_register_store* guest_register_store_context, int index)
{
    assert(index <= guest_register_store_context->guest_context_size);
    
    guest_register* guest_register = &guest_register_store_context->guest_registers[index];

    if (guest_register->free_guest)
    {
        assert(false);

        throw 0;
    }

    guest_register->mode |= guest_usage::guest_usage_read;

    return guest_register->raw_register;
}

void guest_register_store::write_to_guest_register(guest_register_store* guest_register_store_context, int index, ir_operand new_value)
{
    assert(index <= guest_register_store_context->guest_context_size);
    
    guest_register* guest_register = &guest_register_store_context->guest_registers[index];
    ir_operation_block* ir = guest_register_store_context->ssa_emit_context_context->ir;

    if (guest_register->free_guest)
    {
        assert(false);

        throw 0;
    }

    ir_operand raw_register = guest_register->raw_register;

    raw_register = ir_operand::copy_new_raw_size(raw_register, new_value.meta_data);

    ir_operation_block::emitds(ir, ir_move, raw_register, new_value);

    guest_register->mode |= guest_usage::guest_usage_write;
}

void guest_register_store::emit_load_context(guest_register_store* guest_register_store_context)
{
    ir_operation_block* ir = guest_register_store_context->ssa_emit_context_context->ir;

    ir_operation_block::emits(ir, ir_guest_load_context);
}

void guest_register_store::emit_store_context(guest_register_store* guest_register_store_context)
{
    ir_operation_block* ir = guest_register_store_context->ssa_emit_context_context->ir;

    ir_operation_block::emits(ir, ir_guest_store_context);
}