#include "jit.h"
ir_operand add_subtract_impl(arm_emit_context* ctx, ir_operand n, ir_operand m, uint64_t set_flags, uint64_t is_add);
void add_subtract_imm12(arm_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t sh, uint64_t imm12, uint64_t Rn, uint64_t Rd);
ir_operand XSP(arm_emit_context* ctx, uint64_t reg_index);
void XSP(arm_emit_context* ctx, uint64_t reg_index, ir_operand value);
