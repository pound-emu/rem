#include "ssa_emit_context.h"
#include "ir/checks.h"

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

ir_operand ssa_emit_context::emit_ssa(ssa_emit_context* ctx, ir_instructions instruction, ir_operand x)
{
    ir_operand result = create_local(ctx, x.meta_data);

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