#ifndef ARM_EMIT_CONTEXT_H
#define ARM_EMIT_CONTEXT_H

#include "emulator/guest_register_store.h"

struct ssa_emit_context
{
    ir_operation_block*     ir;
    uint64_t                local_top;
    uint64_t                global_bottom;

    static void             create(ssa_emit_context* ctx, ir_operation_block* ir);
    static void             reset_local(ssa_emit_context* ctx);

    static ir_operand       create_local(ssa_emit_context* ctx, uint64_t new_size);
    static ir_operand       create_global(ssa_emit_context* ctx, uint64_t new_size);
    static ir_operand       emit_ssa(ssa_emit_context* ctx, ir_instructions instruction, ir_operand x);
    static ir_operand       emit_ssa(ssa_emit_context* ctx, ir_instructions instruction, ir_operand x, ir_operand y);
    static ir_operand       emit_ssa(ssa_emit_context* ctx, ir_instructions instruction, ir_operand x, ir_operand y, ir_operand z);
    static ir_operand       to_con(ssa_emit_context* ctx, uint64_t source);
};

#endif