#include <inttypes.h>
#include "emulator/ssa_emit_context.h"
#include "aarch64_context_offsets.h"
#include "aarch64_process.h"
#include "emulator/branch_type.h"

struct interpreter_data
{
    aarch64_process*    process_context;
    void*               register_data;
    uint64_t            current_pc;
    int                 branch_type;
};

void init_aarch64_decoder(aarch64_process* process);

//INTERPRETER
uint64_t decode_add_subtract_imm_12_interpreter(interpreter_data* ctx, uint64_t source, uint64_t shift);
template <typename O>
O add_subtract_impl_interpreter(interpreter_data* ctx, O n, O m, uint64_t set_flags, uint64_t is_add);
void add_subtract_imm12_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t sh, uint64_t imm12, uint64_t Rn, uint64_t Rd);
void add_subtract_shifted_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t shift, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd);
void add_subtract_extended_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t option, uint64_t imm3, uint64_t Rn, uint64_t Rd);
uint64_t sign_extend_interpreter(interpreter_data* ctx, uint64_t source, uint64_t count);
template <typename O>
O a_shift_reg_interpreter(interpreter_data* ctx, uint64_t m, uint64_t shift_type, uint64_t ammount);
template <typename O>
O a_extend_reg_interpreter(interpreter_data* ctx, uint64_t m, uint64_t extend_type, uint64_t shift);
void logical_shifted_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t shift, uint64_t N, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd);
void move_wide_immediate_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t hw, uint64_t imm16, uint64_t Rd);
void branch_long_universal_interpreter(interpreter_data* ctx, uint64_t Rn, uint64_t link);
void br_interpreter(interpreter_data* ctx, uint64_t Rn);
void pc_rel_addressing_interpreter(interpreter_data* ctx, uint64_t op, uint64_t immlo, uint64_t immhi, uint64_t Rd);
void divide_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t Rm, uint64_t o1, uint64_t Rn, uint64_t Rd);
uint64_t XSP_interpreter(interpreter_data* ctx, uint64_t reg_id);
void XSP_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value);
uint64_t X_interpreter(interpreter_data* ctx, uint64_t reg_id);
void X_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value);
uint64_t _x_interpreter(interpreter_data* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void _x_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value);//THIS FUNCTION IS USER DEFINED
uint64_t _sys_interpreter(interpreter_data* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void _sys_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value);//THIS FUNCTION IS USER DEFINED
void _branch_long_interpreter(interpreter_data* ctx, uint64_t location);//THIS FUNCTION IS USER DEFINED
uint64_t _get_pc_interpreter(interpreter_data* ctx);//THIS FUNCTION IS USER DEFINED

//JIT
uint64_t decode_add_subtract_imm_12_jit(ssa_emit_context* ctx, uint64_t source, uint64_t shift);
ir_operand add_subtract_impl_jit(ssa_emit_context* ctx,uint64_t O, ir_operand n, ir_operand m, uint64_t set_flags, uint64_t is_add);
void add_subtract_imm12_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t sh, uint64_t imm12, uint64_t Rn, uint64_t Rd);
void add_subtract_shifted_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t shift, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd);
void add_subtract_extended_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t option, uint64_t imm3, uint64_t Rn, uint64_t Rd);
uint64_t sign_extend_jit(ssa_emit_context* ctx, uint64_t source, uint64_t count);
ir_operand a_shift_reg_jit(ssa_emit_context* ctx,uint64_t O, uint64_t m, uint64_t shift_type, uint64_t ammount);
ir_operand a_extend_reg_jit(ssa_emit_context* ctx,uint64_t O, uint64_t m, uint64_t extend_type, uint64_t shift);
void logical_shifted_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t shift, uint64_t N, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd);
void move_wide_immediate_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t hw, uint64_t imm16, uint64_t Rd);
void branch_long_universal_jit(ssa_emit_context* ctx, uint64_t Rn, uint64_t link);
void br_jit(ssa_emit_context* ctx, uint64_t Rn);
void pc_rel_addressing_jit(ssa_emit_context* ctx, uint64_t op, uint64_t immlo, uint64_t immhi, uint64_t Rd);
void divide_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rm, uint64_t o1, uint64_t Rn, uint64_t Rd);
ir_operand XSP_jit(ssa_emit_context* ctx, uint64_t reg_id);
void XSP_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);
ir_operand X_jit(ssa_emit_context* ctx, uint64_t reg_id);
void X_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);
ir_operand _x_jit(ssa_emit_context* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void _x_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);//THIS FUNCTION IS USER DEFINED
ir_operand _sys_jit(ssa_emit_context* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void _sys_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);//THIS FUNCTION IS USER DEFINED
void _branch_long_jit(ssa_emit_context* ctx, ir_operand location);//THIS FUNCTION IS USER DEFINED
uint64_t _get_pc_jit(ssa_emit_context* ctx);//THIS FUNCTION IS USER DEFINED
