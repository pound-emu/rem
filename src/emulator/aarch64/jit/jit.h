#include "arm_emit_context.h"
ir_operand X(arm_emit_context* ctx, uint64_t reg_id);
void X(arm_emit_context* ctx, uint64_t reg_id, ir_operand value);
ir_operand SP(arm_emit_context* ctx);
void SP(arm_emit_context* ctx, ir_operand value);
