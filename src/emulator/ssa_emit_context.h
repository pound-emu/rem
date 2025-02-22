#ifndef ARM_EMIT_CONTEXT_H
#define ARM_EMIT_CONTEXT_H

#include "emulator/guest_register_store.h"

struct ssa_emit_context
{
    ir_operation_block*     ir;
    uint64_t                local_top;
    uint64_t                global_bottom;

    ir_operand              memory_base;

    void*                   context_data;

    static void             create(ssa_emit_context* ctx, ir_operation_block* ir);
    static void             reset_local(ssa_emit_context* ctx);

    static ir_operand       create_local(ssa_emit_context* ctx, uint64_t new_size);
    static ir_operand       create_global(ssa_emit_context* ctx, uint64_t new_size);
    static ir_operand       emit_ssa(ssa_emit_context* ctx, ir_instructions instruction, uint64_t new_type = UINT64_MAX);
    static ir_operand       emit_ssa(ssa_emit_context* ctx, ir_instructions instruction, ir_operand x, uint64_t new_type = UINT64_MAX);
    static ir_operand       emit_ssa(ssa_emit_context* ctx, ir_instructions instruction, ir_operand x, ir_operand y, bool check_size = true);
    static ir_operand       emit_ssa(ssa_emit_context* ctx, ir_instructions instruction, ir_operand x, ir_operand y, ir_operand z, bool check_size = false);
    static ir_operand       to_con(ssa_emit_context* ctx, uint64_t source);
    static ir_operand       vector_extract(ssa_emit_context* ctx, ir_operand source, int index, int size);
    static ir_operand       vector_zero(ssa_emit_context* ctx);
    static ir_operand       vector_one(ssa_emit_context* ctx);
    static ir_operand       convert_to_float(ssa_emit_context* ctx, ir_operand source, uint64_t result_size,uint64_t source_size, bool is_signed);
    static ir_operand       convert_to_integer(ssa_emit_context* ctx, ir_operand source, uint64_t result_size,uint64_t source_size, bool is_signed);
    static void             move(ssa_emit_context* ctx, ir_operand result, ir_operand source);
    static void             store(ssa_emit_context* ctx, ir_operand physical_address, ir_operand value);
    static void             vector_insert(ssa_emit_context* ctx, ir_operand result, int index, int size, ir_operand value);
};

#endif