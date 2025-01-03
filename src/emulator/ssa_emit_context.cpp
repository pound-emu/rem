#include "ssa_emit_context.h"
#include "ir/checks.h"
#include "debugging.h"

void ssa_emit_context::create(ssa_emit_context* ctx, ir_operation_block* ir)
{
    ctx->ir = ir;
    ctx->local_top = 0;
    ctx->global_bottom = UINT64_MAX;
}

ir_operand ssa_emit_context::create_global(ssa_emit_context* ctx, uint64_t new_size)
{
    uint64_t global_index = ctx->global_bottom--;

    return ir_operand::create_reg(global_index, new_size);
}

ir_operand ssa_emit_context::create_local(ssa_emit_context* ctx, uint64_t new_size)
{
    uint64_t local_index = ctx->local_top++;

    return ir_operand::create_reg(local_index, new_size);
}

ir_operand ssa_emit_context::emit_ssa(ssa_emit_context* ctx, ir_instructions instruction, uint64_t new_type)
{
    ir_operand result = create_local(ctx, new_type);

    ir_operation_block::emitds(ctx->ir, instruction, result);

    return result;
}

ir_operand ssa_emit_context::emit_ssa(ssa_emit_context* ctx, ir_instructions instruction, ir_operand x, uint64_t new_type)
{
    if (new_type == UINT64_MAX)
    {
        new_type = x.meta_data;
    }

    ir_operand result = create_local(ctx, new_type);

    ir_operation_block::emitds(ctx->ir, instruction, result, x);

    return result;
}

ir_operand ssa_emit_context::emit_ssa(ssa_emit_context* ctx, ir_instructions instruction, ir_operand x, ir_operand y)
{
    ir_operand result = create_local(ctx, x.meta_data);

    assert_same_size({x, y});

    ir_operation_block::emitds(ctx->ir, instruction, result, x, y);

    return result;
}

ir_operand ssa_emit_context::emit_ssa(ssa_emit_context* ctx, ir_instructions instruction, ir_operand x, ir_operand y, ir_operand z)
{
    ir_operand result = create_local(ctx, x.meta_data);

    assert_same_size({x, y, z});

    ir_operation_block::emitds(ctx->ir, instruction, result, x, y, z);

    return result;
}

void ssa_emit_context::reset_local(ssa_emit_context* ctx)
{
    ctx->local_top = 0;
}

void ssa_emit_context::move(ssa_emit_context* ctx, ir_operand result, ir_operand source)
{
    assert_same_size({result, source});

    if (ir_operand::is_constant(&result))
    {
        throw_error();
    }

    ir_operation_block::emitds(ctx->ir, ir_move, result, source);
}

void ssa_emit_context::store(ssa_emit_context* ctx, ir_operand physical_address, ir_operand value)
{
    ir_operation_block::emits(ctx->ir, ir_store, physical_address, value);
}

void ssa_emit_context::vector_insert(ssa_emit_context* ctx, ir_operand result, int index, int size, ir_operand value)
{
    ir_operation_block::emitds(ctx->ir, ir_vector_insert, result, result, value, ir_operand::create_con(index), ir_operand::create_con(size));
}

ir_operand ssa_emit_context::vector_extract(ssa_emit_context* ctx, ir_operand source, int index, int size)
{
    ir_operand result = ssa_emit_context::create_local(ctx, int64);
		
    ir_operation_block::emitds(ctx->ir, ir_vector_extract, result, source, ir_operand::create_con(index), ir_operand::create_con(size));

    return result;
}

ir_operand ssa_emit_context::vector_zero(ssa_emit_context* ctx)
{
    return ssa_emit_context::emit_ssa(ctx, ir_vector_zero, int128);
}

ir_operand ssa_emit_context::convert_to_float(ssa_emit_context* ctx, ir_operand source, uint64_t result_size, uint64_t source_size, bool is_signed)
{
    uint64_t instruction = is_signed ? ir_convert_to_float_signed : ir_convert_to_float_unsigned;

    source = ir_operand::copy_new_raw_size(source, source_size);
    ir_operand result = ssa_emit_context::create_local(ctx, result_size);

    ir_operation_block::emitds(ctx->ir, instruction, result, source);

    return result;
}