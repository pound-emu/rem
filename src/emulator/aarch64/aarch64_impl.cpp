#include "aarch64_impl.h"
#include "string.h"
#include "tools/big_number.h"

static void append_table(aarch64_process* process, std::string encoding, void* emit, void* interperate, std::string name)
{
	uint32_t instruction = 0;
	uint32_t mask = 0;

	for (int i = 0; i < 32; ++i)
	{
		char working = encoding[i];

		int shift = 31 - i;

		if (working == '1')
		{
			instruction |= 1UL << shift;
		}

		if (working != '-')
		{
			mask |= 1UL << shift;
		}
	}

	fixed_length_decoder<uint32_t>::insert_entry(&process->decoder, instruction, mask, emit, interperate, name);
}

template <typename T>
int64_t sign_extend(T src) 
{
    switch (sizeof(T))
    {
        case 1: return (int8_t)src;
        case 2: return (int16_t)src;
        case 4: return (int32_t)src;
    }

    return src;
}

template <typename T>
T rotate_right(T src, int ammount)
{
	int INT_BITS = sizeof(T) * 8;

	return (src >> ammount)|(src << (INT_BITS - ammount));
}

static ir_operand copy_new_raw_size(ssa_emit_context* ctx, ir_operand source, uint64_t new_size)
{
	bool div = new_size >= int128;
	bool siv = ir_operand::is_vector(&source);

	if (div == siv)
	{
		if (new_size >= ir_operand::get_raw_size(&source))
		{
			return source;
		} 
		else
		{
			source = ir_operand::copy_new_raw_size(source, new_size);

			return ssa_emit_context::emit_ssa(ctx, ir_move, source);
		}
	}
	else if (div && !siv)
	{	
		ir_operand result = ssa_emit_context::emit_ssa(ctx, ir_vector_zero, int128);

		ir_operation_block::emitds(ctx->ir, ir_vector_insert, result, result, source, ir_operand::create_con(0), ir_operand::create_con(64));

		return result;
	}
	else
	{
		ir_operand result = ssa_emit_context::create_local(ctx, new_size);
		
		ir_operation_block::emitds(ctx->ir, ir_vector_extract, result, source, ir_operand::create_con(0), ir_operand::create_con(64));

		return result;
	}
}

static void call_add_subtract_imm12_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 30) & 1;
	int S = (instruction >> 29) & 1;
	int sh = (instruction >> 22) & 1;
	int imm12 = (instruction >> 10) & 4095;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	add_subtract_imm12_interpreter(ctx, sf, op, S, sh, imm12, Rn, Rd);
}

static void emit_add_subtract_imm12_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 30) & 1;
	int S = (instruction >> 29) & 1;
	int sh = (instruction >> 22) & 1;
	int imm12 = (instruction >> 10) & 4095;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	add_subtract_imm12_jit(ctx, sf, op, S, sh, imm12, Rn, Rd);
}

static void call_add_subtract_shifted_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 30) & 1;
	int S = (instruction >> 29) & 1;
	int shift = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int imm6 = (instruction >> 10) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	add_subtract_shifted_interpreter(ctx, sf, op, S, shift, Rm, imm6, Rn, Rd);
}

static void emit_add_subtract_shifted_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 30) & 1;
	int S = (instruction >> 29) & 1;
	int shift = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int imm6 = (instruction >> 10) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	add_subtract_shifted_jit(ctx, sf, op, S, shift, Rm, imm6, Rn, Rd);
}

static void call_add_subtract_extended_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 30) & 1;
	int S = (instruction >> 29) & 1;
	int Rm = (instruction >> 16) & 31;
	int option = (instruction >> 13) & 7;
	int imm3 = (instruction >> 10) & 7;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	add_subtract_extended_interpreter(ctx, sf, op, S, Rm, option, imm3, Rn, Rd);
}

static void emit_add_subtract_extended_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 30) & 1;
	int S = (instruction >> 29) & 1;
	int Rm = (instruction >> 16) & 31;
	int option = (instruction >> 13) & 7;
	int imm3 = (instruction >> 10) & 7;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	add_subtract_extended_jit(ctx, sf, op, S, Rm, option, imm3, Rn, Rd);
}

static void call_add_subtract_carry_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 30) & 1;
	int S = (instruction >> 29) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	add_subtract_carry_interpreter(ctx, sf, op, S, Rm, Rn, Rd);
}

static void emit_add_subtract_carry_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 30) & 1;
	int S = (instruction >> 29) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	add_subtract_carry_jit(ctx, sf, op, S, Rm, Rn, Rd);
}

static void call_shift_variable_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int Rm = (instruction >> 16) & 31;
	int op2 = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	shift_variable_interpreter(ctx, sf, Rm, op2, Rn, Rd);
}

static void emit_shift_variable_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int Rm = (instruction >> 16) & 31;
	int op2 = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	shift_variable_jit(ctx, sf, Rm, op2, Rn, Rd);
}

static void call_multiply_with_32_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int U = (instruction >> 23) & 1;
	int Rm = (instruction >> 16) & 31;
	int o0 = (instruction >> 15) & 1;
	int Ra = (instruction >> 10) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	multiply_with_32_interpreter(ctx, U, Rm, o0, Ra, Rn, Rd);
}

static void emit_multiply_with_32_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int U = (instruction >> 23) & 1;
	int Rm = (instruction >> 16) & 31;
	int o0 = (instruction >> 15) & 1;
	int Ra = (instruction >> 10) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	multiply_with_32_jit(ctx, U, Rm, o0, Ra, Rn, Rd);
}

static void call_multiply_hi_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int U = (instruction >> 23) & 1;
	int Rm = (instruction >> 16) & 31;
	int o0 = (instruction >> 15) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	multiply_hi_interpreter(ctx, U, Rm, o0, Rn, Rd);
}

static void emit_multiply_hi_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int U = (instruction >> 23) & 1;
	int Rm = (instruction >> 16) & 31;
	int o0 = (instruction >> 15) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	multiply_hi_jit(ctx, U, Rm, o0, Rn, Rd);
}

static void call_multiply_additive_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int Rm = (instruction >> 16) & 31;
	int o0 = (instruction >> 15) & 1;
	int Ra = (instruction >> 10) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	multiply_additive_interpreter(ctx, sf, Rm, o0, Ra, Rn, Rd);
}

static void emit_multiply_additive_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int Rm = (instruction >> 16) & 31;
	int o0 = (instruction >> 15) & 1;
	int Ra = (instruction >> 10) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	multiply_additive_jit(ctx, sf, Rm, o0, Ra, Rn, Rd);
}

static void call_divide_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int Rm = (instruction >> 16) & 31;
	int o1 = (instruction >> 10) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	divide_interpreter(ctx, sf, Rm, o1, Rn, Rd);
}

static void emit_divide_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int Rm = (instruction >> 16) & 31;
	int o1 = (instruction >> 10) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	divide_jit(ctx, sf, Rm, o1, Rn, Rd);
}

static void call_rbit_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	rbit_interpreter(ctx, sf, Rn, Rd);
}

static void emit_rbit_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	rbit_jit(ctx, sf, Rn, Rd);
}

static void call_rev16_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	rev16_interpreter(ctx, sf, Rn, Rd);
}

static void emit_rev16_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	rev16_jit(ctx, sf, Rn, Rd);
}

static void call_reverse_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int opc = (instruction >> 10) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	reverse_interpreter(ctx, sf, opc, Rn, Rd);
}

static void emit_reverse_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int opc = (instruction >> 10) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	reverse_jit(ctx, sf, opc, Rn, Rd);
}

static void call_count_leading_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int s = (instruction >> 10) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	count_leading_interpreter(ctx, sf, s, Rn, Rd);
}

static void emit_count_leading_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int s = (instruction >> 10) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	count_leading_jit(ctx, sf, s, Rn, Rd);
}

static void call_extr_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int N = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int imms = (instruction >> 10) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	extr_interpreter(ctx, sf, N, Rm, imms, Rn, Rd);
}

static void emit_extr_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int N = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int imms = (instruction >> 10) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	extr_jit(ctx, sf, N, Rm, imms, Rn, Rd);
}

static void call_bitfield_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int opc = (instruction >> 29) & 3;
	int N = (instruction >> 22) & 1;
	int immr = (instruction >> 16) & 63;
	int imms = (instruction >> 10) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	bitfield_interpreter(ctx, sf, opc, N, immr, imms, Rn, Rd);
}

static void emit_bitfield_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int opc = (instruction >> 29) & 3;
	int N = (instruction >> 22) & 1;
	int immr = (instruction >> 16) & 63;
	int imms = (instruction >> 10) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	bitfield_jit(ctx, sf, opc, N, immr, imms, Rn, Rd);
}

static void call_logical_immediate_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int opc = (instruction >> 29) & 3;
	int N = (instruction >> 22) & 1;
	int immr = (instruction >> 16) & 63;
	int imms = (instruction >> 10) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	logical_immediate_interpreter(ctx, sf, opc, N, immr, imms, Rn, Rd);
}

static void emit_logical_immediate_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int opc = (instruction >> 29) & 3;
	int N = (instruction >> 22) & 1;
	int immr = (instruction >> 16) & 63;
	int imms = (instruction >> 10) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	logical_immediate_jit(ctx, sf, opc, N, immr, imms, Rn, Rd);
}

static void call_logical_shifted_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int opc = (instruction >> 29) & 3;
	int shift = (instruction >> 22) & 3;
	int N = (instruction >> 21) & 1;
	int Rm = (instruction >> 16) & 31;
	int imm6 = (instruction >> 10) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	logical_shifted_interpreter(ctx, sf, opc, shift, N, Rm, imm6, Rn, Rd);
}

static void emit_logical_shifted_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int opc = (instruction >> 29) & 3;
	int shift = (instruction >> 22) & 3;
	int N = (instruction >> 21) & 1;
	int Rm = (instruction >> 16) & 31;
	int imm6 = (instruction >> 10) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	logical_shifted_jit(ctx, sf, opc, shift, N, Rm, imm6, Rn, Rd);
}

static void call_conditional_select_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 30) & 1;
	int S = (instruction >> 29) & 1;
	int Rm = (instruction >> 16) & 31;
	int cond = (instruction >> 12) & 15;
	int op2 = (instruction >> 10) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	conditional_select_interpreter(ctx, sf, op, S, Rm, cond, op2, Rn, Rd);
}

static void emit_conditional_select_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 30) & 1;
	int S = (instruction >> 29) & 1;
	int Rm = (instruction >> 16) & 31;
	int cond = (instruction >> 12) & 15;
	int op2 = (instruction >> 10) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	conditional_select_jit(ctx, sf, op, S, Rm, cond, op2, Rn, Rd);
}

static void call_conditional_compare_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int cond = (instruction >> 12) & 15;
	int mode = (instruction >> 11) & 1;
	int Rn = (instruction >> 5) & 31;
	int nzcv = (instruction >> 0) & 15;
	conditional_compare_interpreter(ctx, sf, op, Rm, cond, mode, Rn, nzcv);
}

static void emit_conditional_compare_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int cond = (instruction >> 12) & 15;
	int mode = (instruction >> 11) & 1;
	int Rn = (instruction >> 5) & 31;
	int nzcv = (instruction >> 0) & 15;
	conditional_compare_jit(ctx, sf, op, Rm, cond, mode, Rn, nzcv);
}

static void call_move_wide_immediate_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int opc = (instruction >> 29) & 3;
	int hw = (instruction >> 21) & 3;
	int imm16 = (instruction >> 5) & 65535;
	int Rd = (instruction >> 0) & 31;
	move_wide_immediate_interpreter(ctx, sf, opc, hw, imm16, Rd);
}

static void emit_move_wide_immediate_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int opc = (instruction >> 29) & 3;
	int hw = (instruction >> 21) & 3;
	int imm16 = (instruction >> 5) & 65535;
	int Rd = (instruction >> 0) & 31;
	move_wide_immediate_jit(ctx, sf, opc, hw, imm16, Rd);
}

static void call_pc_rel_addressing_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int op = (instruction >> 31) & 1;
	int immlo = (instruction >> 29) & 3;
	int immhi = (instruction >> 5) & 524287;
	int Rd = (instruction >> 0) & 31;
	pc_rel_addressing_interpreter(ctx, op, immlo, immhi, Rd);
}

static void emit_pc_rel_addressing_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int op = (instruction >> 31) & 1;
	int immlo = (instruction >> 29) & 3;
	int immhi = (instruction >> 5) & 524287;
	int Rd = (instruction >> 0) & 31;
	pc_rel_addressing_jit(ctx, op, immlo, immhi, Rd);
}

static void call_load_store_register_post_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int size = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int opc = (instruction >> 22) & 3;
	int imm9 = (instruction >> 12) & 511;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_post_interpreter(ctx, size, VR, opc, imm9, Rn, Rt);
}

static void emit_load_store_register_post_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int size = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int opc = (instruction >> 22) & 3;
	int imm9 = (instruction >> 12) & 511;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_post_jit(ctx, size, VR, opc, imm9, Rn, Rt);
}

static void call_load_store_register_pre_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int size = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int opc = (instruction >> 22) & 3;
	int imm9 = (instruction >> 12) & 511;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_pre_interpreter(ctx, size, VR, opc, imm9, Rn, Rt);
}

static void emit_load_store_register_pre_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int size = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int opc = (instruction >> 22) & 3;
	int imm9 = (instruction >> 12) & 511;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_pre_jit(ctx, size, VR, opc, imm9, Rn, Rt);
}

static void call_load_store_register_unscaled_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int size = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int opc = (instruction >> 22) & 3;
	int imm9 = (instruction >> 12) & 511;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_unscaled_interpreter(ctx, size, VR, opc, imm9, Rn, Rt);
}

static void emit_load_store_register_unscaled_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int size = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int opc = (instruction >> 22) & 3;
	int imm9 = (instruction >> 12) & 511;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_unscaled_jit(ctx, size, VR, opc, imm9, Rn, Rt);
}

static void call_load_store_register_pair_imm_offset_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int opc = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int L = (instruction >> 22) & 1;
	int imm7 = (instruction >> 15) & 127;
	int Rt2 = (instruction >> 10) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_pair_imm_offset_interpreter(ctx, opc, VR, L, imm7, Rt2, Rn, Rt);
}

static void emit_load_store_register_pair_imm_offset_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int opc = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int L = (instruction >> 22) & 1;
	int imm7 = (instruction >> 15) & 127;
	int Rt2 = (instruction >> 10) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_pair_imm_offset_jit(ctx, opc, VR, L, imm7, Rt2, Rn, Rt);
}

static void call_load_store_register_pair_imm_post_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int opc = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int L = (instruction >> 22) & 1;
	int imm7 = (instruction >> 15) & 127;
	int Rt2 = (instruction >> 10) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_pair_imm_post_interpreter(ctx, opc, VR, L, imm7, Rt2, Rn, Rt);
}

static void emit_load_store_register_pair_imm_post_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int opc = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int L = (instruction >> 22) & 1;
	int imm7 = (instruction >> 15) & 127;
	int Rt2 = (instruction >> 10) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_pair_imm_post_jit(ctx, opc, VR, L, imm7, Rt2, Rn, Rt);
}

static void call_load_store_register_pair_imm_pre_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int opc = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int L = (instruction >> 22) & 1;
	int imm7 = (instruction >> 15) & 127;
	int Rt2 = (instruction >> 10) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_pair_imm_pre_interpreter(ctx, opc, VR, L, imm7, Rt2, Rn, Rt);
}

static void emit_load_store_register_pair_imm_pre_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int opc = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int L = (instruction >> 22) & 1;
	int imm7 = (instruction >> 15) & 127;
	int Rt2 = (instruction >> 10) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_pair_imm_pre_jit(ctx, opc, VR, L, imm7, Rt2, Rn, Rt);
}

static void call_load_store_register_imm_unsigned_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int size = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int opc = (instruction >> 22) & 3;
	int imm12 = (instruction >> 10) & 4095;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_imm_unsigned_interpreter(ctx, size, VR, opc, imm12, Rn, Rt);
}

static void emit_load_store_register_imm_unsigned_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int size = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int opc = (instruction >> 22) & 3;
	int imm12 = (instruction >> 10) & 4095;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_imm_unsigned_jit(ctx, size, VR, opc, imm12, Rn, Rt);
}

static void call_br_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Rn = (instruction >> 5) & 31;
	br_interpreter(ctx, Rn);
}

static void emit_br_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Rn = (instruction >> 5) & 31;
	br_jit(ctx, Rn);
}

void init_aarch64_decoder(aarch64_process* process)
{
	append_table(process, "---100010-----------------------", (void*)emit_add_subtract_imm12_jit, (void*)call_add_subtract_imm12_interpreter, "add_subtract_imm12");
	append_table(process, "---01011--0---------------------", (void*)emit_add_subtract_shifted_jit, (void*)call_add_subtract_shifted_interpreter, "add_subtract_shifted");
	append_table(process, "---01011001---------------------", (void*)emit_add_subtract_extended_jit, (void*)call_add_subtract_extended_interpreter, "add_subtract_extended");
	append_table(process, "---11010000-----000000----------", (void*)emit_add_subtract_carry_jit, (void*)call_add_subtract_carry_interpreter, "add_subtract_carry");
	append_table(process, "-0011010110-----0010------------", (void*)emit_shift_variable_jit, (void*)call_shift_variable_interpreter, "shift_variable");
	append_table(process, "10011011-01---------------------", (void*)emit_multiply_with_32_jit, (void*)call_multiply_with_32_interpreter, "multiply_with_32");
	append_table(process, "10011011-10------11111----------", (void*)emit_multiply_hi_jit, (void*)call_multiply_hi_interpreter, "multiply_hi");
	append_table(process, "-0011011000---------------------", (void*)emit_multiply_additive_jit, (void*)call_multiply_additive_interpreter, "multiply_additive");
	append_table(process, "-0011010110-----00001-----------", (void*)emit_divide_jit, (void*)call_divide_interpreter, "divide");
	append_table(process, "-101101011000000000000----------", (void*)emit_rbit_jit, (void*)call_rbit_interpreter, "rbit");
	append_table(process, "-101101011000000000001----------", (void*)emit_rev16_jit, (void*)call_rev16_interpreter, "rev16");
	append_table(process, "-10110101100000000001-----------", (void*)emit_reverse_jit, (void*)call_reverse_interpreter, "reverse");
	append_table(process, "-10110101100000000010-----------", (void*)emit_count_leading_jit, (void*)call_count_leading_interpreter, "count_leading");
	append_table(process, "-00100111-0---------------------", (void*)emit_extr_jit, (void*)call_extr_interpreter, "extr");
	append_table(process, "---100110-----------------------", (void*)emit_bitfield_jit, (void*)call_bitfield_interpreter, "bitfield");
	append_table(process, "---100100-----------------------", (void*)emit_logical_immediate_jit, (void*)call_logical_immediate_interpreter, "logical_immediate");
	append_table(process, "---01010------------------------", (void*)emit_logical_shifted_jit, (void*)call_logical_shifted_interpreter, "logical_shifted");
	append_table(process, "---11010100---------0-----------", (void*)emit_conditional_select_jit, (void*)call_conditional_select_interpreter, "conditional_select");
	append_table(process, "--111010010----------0-----0----", (void*)emit_conditional_compare_jit, (void*)call_conditional_compare_interpreter, "conditional_compare");
	append_table(process, "---100101-----------------------", (void*)emit_move_wide_immediate_jit, (void*)call_move_wide_immediate_interpreter, "move_wide_immediate");
	append_table(process, "---10000------------------------", (void*)emit_pc_rel_addressing_jit, (void*)call_pc_rel_addressing_interpreter, "pc_rel_addressing");
	append_table(process, "--111-00--0---------01----------", (void*)emit_load_store_register_post_jit, (void*)call_load_store_register_post_interpreter, "load_store_register_post");
	append_table(process, "--111-00--0---------11----------", (void*)emit_load_store_register_pre_jit, (void*)call_load_store_register_pre_interpreter, "load_store_register_pre");
	append_table(process, "--111-00--0---------00----------", (void*)emit_load_store_register_unscaled_jit, (void*)call_load_store_register_unscaled_interpreter, "load_store_register_unscaled");
	append_table(process, "--101-010-----------------------", (void*)emit_load_store_register_pair_imm_offset_jit, (void*)call_load_store_register_pair_imm_offset_interpreter, "load_store_register_pair_imm_offset");
	append_table(process, "--101-001-----------------------", (void*)emit_load_store_register_pair_imm_post_jit, (void*)call_load_store_register_pair_imm_post_interpreter, "load_store_register_pair_imm_post");
	append_table(process, "--101-011-----------------------", (void*)emit_load_store_register_pair_imm_pre_jit, (void*)call_load_store_register_pair_imm_pre_interpreter, "load_store_register_pair_imm_pre");
	append_table(process, "--111-01------------------------", (void*)emit_load_store_register_imm_unsigned_jit, (void*)call_load_store_register_imm_unsigned_interpreter, "load_store_register_imm_unsigned");
	append_table(process, "1101011000011111000000-----00000", (void*)emit_br_jit, (void*)call_br_interpreter, "br");
}

void add_subtract_imm12_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t sh, uint64_t imm12, uint64_t Rn, uint64_t Rd)
{
	if (sf == 0ULL)
	{
		uint32_t operand1 = XSP_interpreter(ctx,Rn);
		uint32_t operand2 = decode_add_subtract_imm_12_interpreter(ctx,imm12,sh);
		uint32_t d = add_subtract_impl_interpreter<uint32_t>(ctx,operand1,operand2,S,((uint64_t)op == (uint64_t)0ULL));
		if ((S))
		{
			X_interpreter(ctx,Rd,(uint64_t)d);
		}
		else
		{
			XSP_interpreter(ctx,Rd,(uint64_t)d);
		}
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = XSP_interpreter(ctx,Rn);
		uint64_t operand2 = decode_add_subtract_imm_12_interpreter(ctx,imm12,sh);
		uint64_t d = add_subtract_impl_interpreter<uint64_t>(ctx,operand1,operand2,S,((uint64_t)op == (uint64_t)0ULL));
		if ((S))
		{
			X_interpreter(ctx,Rd,(uint64_t)d);
		}
		else
		{
			XSP_interpreter(ctx,Rd,(uint64_t)d);
		}
	}
	
}

void add_subtract_shifted_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t shift, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd)
{
	uint64_t shift_ammount = imm6;
	if (sf == 0ULL)
	{
		uint32_t operand1 = X_interpreter(ctx,Rn);
		uint32_t operand2 = a_shift_reg_interpreter<uint32_t>(ctx,Rm,shift,shift_ammount);
		uint32_t result = add_subtract_impl_interpreter<uint32_t>(ctx,operand1,operand2,S,((uint64_t)op == (uint64_t)0ULL));
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = a_shift_reg_interpreter<uint64_t>(ctx,Rm,shift,shift_ammount);
		uint64_t result = add_subtract_impl_interpreter<uint64_t>(ctx,operand1,operand2,S,((uint64_t)op == (uint64_t)0ULL));
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	
}

void add_subtract_extended_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t option, uint64_t imm3, uint64_t Rn, uint64_t Rd)
{
	if (sf == 0ULL)
	{
		uint64_t shift = imm3;
		uint64_t extend_type = option;
		uint32_t operand1 = XSP_interpreter(ctx,Rn);
		uint32_t operand2 = a_extend_reg_interpreter<uint32_t>(ctx,Rm,extend_type,shift);
		uint32_t result = add_subtract_impl_interpreter<uint32_t>(ctx,operand1,operand2,S,((uint64_t)op == (uint64_t)0ULL));
		if ((S))
		{
			X_interpreter(ctx,Rd,(uint64_t)result);
		}
		else
		{
			XSP_interpreter(ctx,Rd,(uint64_t)result);
		}
	}
	if (sf == 1ULL)
	{
		uint64_t shift = imm3;
		uint64_t extend_type = option;
		uint64_t operand1 = XSP_interpreter(ctx,Rn);
		uint64_t operand2 = a_extend_reg_interpreter<uint64_t>(ctx,Rm,extend_type,shift);
		uint64_t result = add_subtract_impl_interpreter<uint64_t>(ctx,operand1,operand2,S,((uint64_t)op == (uint64_t)0ULL));
		if ((S))
		{
			X_interpreter(ctx,Rd,(uint64_t)result);
		}
		else
		{
			XSP_interpreter(ctx,Rd,(uint64_t)result);
		}
	}
	
}

void add_subtract_carry_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if (sf == 0ULL)
	{
		uint32_t operand1 = X_interpreter(ctx,Rn);
		uint32_t operand2 = X_interpreter(ctx,Rm);
		uint32_t result = add_subtract_carry_impl_interpreter<uint32_t>(ctx,operand1,operand2,((uint64_t)S == (uint64_t)1ULL),((uint64_t)op == (uint64_t)0ULL),(uint32_t)_sys_interpreter(ctx,2ULL));
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = X_interpreter(ctx,Rm);
		uint64_t result = add_subtract_carry_impl_interpreter<uint64_t>(ctx,operand1,operand2,((uint64_t)S == (uint64_t)1ULL),((uint64_t)op == (uint64_t)0ULL),(uint64_t)_sys_interpreter(ctx,2ULL));
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	
}

void shift_variable_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t Rm, uint64_t op2, uint64_t Rn, uint64_t Rd)
{
	if (sf == 0ULL)
	{
		uint32_t operand1 = X_interpreter(ctx,Rn);
		uint32_t operand2 = X_interpreter(ctx,Rm);
		uint64_t mask = ((uint64_t)(((uint64_t)32ULL << (uint64_t)sf)) - (uint64_t)1ULL);
		uint32_t result;
		operand2 = ((uint32_t)operand2 & (uint32_t)mask);
		if ((((uint64_t)op2 == (uint64_t)0ULL)))
		{
			result = ((uint32_t)operand1 << (uint32_t)operand2);
		}
		else if ((((uint64_t)op2 == (uint64_t)1ULL)))
		{
			result = ((uint32_t)operand1 >> (uint32_t)operand2);
		}
		else if ((((uint64_t)op2 == (uint64_t)2ULL)))
		{
			result = ((uint32_t)(sign_extend((uint32_t)operand1) >> sign_extend((uint32_t)operand2)));
		}
		else
		{
			result = (rotate_right((uint32_t)operand1,(uint32_t)operand2));
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = X_interpreter(ctx,Rm);
		uint64_t mask = ((uint64_t)(((uint64_t)32ULL << (uint64_t)sf)) - (uint64_t)1ULL);
		uint64_t result;
		operand2 = ((uint64_t)operand2 & (uint64_t)mask);
		if ((((uint64_t)op2 == (uint64_t)0ULL)))
		{
			result = ((uint64_t)operand1 << (uint64_t)operand2);
		}
		else if ((((uint64_t)op2 == (uint64_t)1ULL)))
		{
			result = ((uint64_t)operand1 >> (uint64_t)operand2);
		}
		else if ((((uint64_t)op2 == (uint64_t)2ULL)))
		{
			result = ((uint64_t)(sign_extend((uint64_t)operand1) >> sign_extend((uint64_t)operand2)));
		}
		else
		{
			result = (rotate_right((uint64_t)operand1,(uint64_t)operand2));
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	
}

void multiply_with_32_interpreter(interpreter_data* ctx, uint64_t U, uint64_t Rm, uint64_t o0, uint64_t Ra, uint64_t Rn, uint64_t Rd)
{
	uint64_t operand1 = X_interpreter(ctx,Rn);
	uint64_t operand2 = X_interpreter(ctx,Rm);
	uint64_t operand3 = X_interpreter(ctx,Ra);
	uint64_t is_add = ((uint64_t)o0 == (uint64_t)0ULL);
	uint64_t is_signed = ((uint64_t)U == (uint64_t)0ULL);
	if ((is_signed))
	{
		operand1 = (uint64_t)sign_extend((uint32_t)operand1);
		operand2 = (uint64_t)sign_extend((uint32_t)operand2);
	}
	else
	{
		operand1 = ((uint64_t)operand1 & (uint64_t)4294967295ULL);
		operand2 = ((uint64_t)operand2 & (uint64_t)4294967295ULL);
	}
	uint64_t result;
	if ((is_add))
	{
		result = ((uint64_t)operand3 + (uint64_t)(((uint64_t)operand1 * (uint64_t)operand2)));
	}
	else
	{
		result = ((uint64_t)operand3 - (uint64_t)(((uint64_t)operand1 * (uint64_t)operand2)));
	}
	X_interpreter(ctx,Rd,result);
}

void multiply_hi_interpreter(interpreter_data* ctx, uint64_t U, uint64_t Rm, uint64_t o0, uint64_t Rn, uint64_t Rd)
{
	uint64_t operand1 = X_interpreter(ctx,Rn);
	uint64_t operand2 = X_interpreter(ctx,Rm);
	uint64_t is_signed = ((uint64_t)U == (uint64_t)0ULL);
	uint64_t result;
	if ((is_signed))
	{
		result = (multiply_hi((uint64_t)operand1,(uint64_t)operand2, true));
	}
	else
	{
		result = (multiply_hi((uint64_t)operand1,(uint64_t)operand2, false));
	}
	X_interpreter(ctx,Rd,result);
}

void multiply_additive_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t Rm, uint64_t o0, uint64_t Ra, uint64_t Rn, uint64_t Rd)
{
	if (sf == 0ULL)
	{
		uint32_t operand1 = X_interpreter(ctx,Rn);
		uint32_t operand2 = X_interpreter(ctx,Rm);
		uint32_t operand3 = X_interpreter(ctx,Ra);
		uint64_t is_add = ((uint64_t)o0 == (uint64_t)0ULL);
		uint32_t result;
		if ((is_add))
		{
			result = ((uint32_t)operand3 + (uint32_t)(((uint32_t)operand1 * (uint32_t)operand2)));
		}
		else
		{
			result = ((uint32_t)operand3 - (uint32_t)(((uint32_t)operand1 * (uint32_t)operand2)));
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = X_interpreter(ctx,Rm);
		uint64_t operand3 = X_interpreter(ctx,Ra);
		uint64_t is_add = ((uint64_t)o0 == (uint64_t)0ULL);
		uint64_t result;
		if ((is_add))
		{
			result = ((uint64_t)operand3 + (uint64_t)(((uint64_t)operand1 * (uint64_t)operand2)));
		}
		else
		{
			result = ((uint64_t)operand3 - (uint64_t)(((uint64_t)operand1 * (uint64_t)operand2)));
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	
}

void divide_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t Rm, uint64_t o1, uint64_t Rn, uint64_t Rd)
{
	uint64_t is_signed = ((uint64_t)o1 == (uint64_t)1ULL);
	if (sf == 0ULL)
	{
		uint32_t operand1 = X_interpreter(ctx,Rn);
		uint32_t operand2 = X_interpreter(ctx,Rm);
		uint32_t result;
		if ((((uint32_t)operand2 == (uint32_t)0ULL)))
		{
			X_interpreter(ctx,Rd,(uint64_t)0ULL);
		}
		else
		{
			if ((is_signed))
			{
				uint64_t min = 9223372036854775808ULL;
				if ((!sf))
				{
					min = ((uint64_t)min >> (uint64_t)32ULL);
				}
				if ((((uint32_t)((uint32_t)operand1 == (uint32_t)min) && (uint32_t)((uint32_t)operand2 == (uint32_t)-1ULL))))
				{
					X_interpreter(ctx,Rd,(uint64_t)min);
				}
				else
				{
					X_interpreter(ctx,Rd,(uint64_t)((uint32_t)(sign_extend((uint32_t)operand1) / sign_extend((uint32_t)operand2))));
				}
			}
			else
			{
				X_interpreter(ctx,Rd,(uint64_t)((uint32_t)operand1 / (uint32_t)operand2));
			}
		}
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = X_interpreter(ctx,Rm);
		uint64_t result;
		if ((((uint64_t)operand2 == (uint64_t)0ULL)))
		{
			X_interpreter(ctx,Rd,(uint64_t)0ULL);
		}
		else
		{
			if ((is_signed))
			{
				uint64_t min = 9223372036854775808ULL;
				if ((!sf))
				{
					min = ((uint64_t)min >> (uint64_t)32ULL);
				}
				if ((((uint64_t)((uint64_t)operand1 == (uint64_t)min) && (uint64_t)((uint64_t)operand2 == (uint64_t)-1ULL))))
				{
					X_interpreter(ctx,Rd,(uint64_t)min);
				}
				else
				{
					X_interpreter(ctx,Rd,(uint64_t)((uint64_t)(sign_extend((uint64_t)operand1) / sign_extend((uint64_t)operand2))));
				}
			}
			else
			{
				X_interpreter(ctx,Rd,(uint64_t)((uint64_t)operand1 / (uint64_t)operand2));
			}
		}
	}
	
}

void rbit_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	if (sf == 0ULL)
	{
		uint32_t operand = X_interpreter(ctx,Rn);
		uint32_t result = 0ULL;
		for (uint64_t i = 0; i < (datasize); i++)
		{
			uint32_t working = ((uint32_t)(((uint32_t)operand >> (uint32_t)i)) & (uint32_t)1ULL);
			result = ((uint32_t)result | (uint32_t)(((uint32_t)working << (uint32_t)(((uint64_t)((uint64_t)datasize - (uint64_t)i) - (uint64_t)1ULL)))));
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand = X_interpreter(ctx,Rn);
		uint64_t result = 0ULL;
		for (uint64_t i = 0; i < (datasize); i++)
		{
			uint64_t working = ((uint64_t)(((uint64_t)operand >> (uint64_t)i)) & (uint64_t)1ULL);
			result = ((uint64_t)result | (uint64_t)(((uint64_t)working << (uint64_t)(((uint64_t)((uint64_t)datasize - (uint64_t)i) - (uint64_t)1ULL)))));
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	
}

void rev16_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t Rn, uint64_t Rd)
{
	if (sf == 0ULL)
	{
		uint64_t count = ((uint64_t)2ULL << (uint64_t)sf);
		uint32_t working = X_interpreter(ctx,Rn);
		uint32_t result = 0ULL;
		for (uint64_t i = 0; i < (count); i++)
		{
			uint32_t part = ((uint32_t)(((uint32_t)working >> (uint32_t)(((uint64_t)i * (uint64_t)16ULL)))) & (uint32_t)65535ULL);
			part = reverse_bytes_interpreter<uint32_t>(ctx,part,2ULL);
			result = ((uint32_t)result | (uint32_t)(((uint32_t)part << (uint32_t)(((uint64_t)i * (uint64_t)16ULL)))));
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t count = ((uint64_t)2ULL << (uint64_t)sf);
		uint64_t working = X_interpreter(ctx,Rn);
		uint64_t result = 0ULL;
		for (uint64_t i = 0; i < (count); i++)
		{
			uint64_t part = ((uint64_t)(((uint64_t)working >> (uint64_t)(((uint64_t)i * (uint64_t)16ULL)))) & (uint64_t)65535ULL);
			part = reverse_bytes_interpreter<uint64_t>(ctx,part,2ULL);
			result = ((uint64_t)result | (uint64_t)(((uint64_t)part << (uint64_t)(((uint64_t)i * (uint64_t)16ULL)))));
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	
}

void reverse_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	if (sf == 0ULL)
	{
		uint32_t working = X_interpreter(ctx,Rn);
		uint32_t result;
		if ((((uint64_t)sf == (uint64_t)opc)))
		{
			result = reverse_bytes_interpreter<uint32_t>(ctx,working,((uint64_t)4ULL << (uint64_t)sf));
		}
		else
		{
			result = ((uint32_t)reverse_bytes_interpreter<uint32_t>(ctx,working,4ULL) | (uint32_t)(((uint32_t)reverse_bytes_interpreter<uint32_t>(ctx,((uint32_t)working >> (uint32_t)32ULL),4ULL) << (uint32_t)32ULL)));
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t working = X_interpreter(ctx,Rn);
		uint64_t result;
		if ((((uint64_t)sf == (uint64_t)opc)))
		{
			result = reverse_bytes_interpreter<uint64_t>(ctx,working,((uint64_t)4ULL << (uint64_t)sf));
		}
		else
		{
			result = ((uint64_t)reverse_bytes_interpreter<uint64_t>(ctx,working,4ULL) | (uint64_t)(((uint64_t)reverse_bytes_interpreter<uint64_t>(ctx,((uint64_t)working >> (uint64_t)32ULL),4ULL) << (uint64_t)32ULL)));
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	
}

void count_leading_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t s, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	if (sf == 0ULL)
	{
		uint32_t operand = X_interpreter(ctx,Rn);
		uint32_t result = 0ULL;
		uint32_t done = 0ULL;
		uint32_t sig_bit;
		if ((s))
		{
			sig_bit = ((uint32_t)(((uint32_t)operand >> (uint32_t)(((uint64_t)datasize - (uint64_t)1ULL)))) & (uint32_t)1ULL);
			datasize = ((uint64_t)datasize - (uint64_t)1ULL);
		}
		for (uint64_t i = 0; i < (datasize); i++)
		{
			uint32_t working = ((uint32_t)(((uint32_t)operand >> (uint32_t)(((uint64_t)((uint64_t)datasize - (uint64_t)i) - (uint64_t)1ULL)))) & (uint32_t)1ULL);
			if ((s))
			{
				if ((((uint32_t)working != (uint32_t)sig_bit)))
				{
					done = 1ULL;
				}
			}
			else if ((working))
			{
				done = 1ULL;
			}
			if ((!done))
			{
				result = ((uint32_t)result + (uint32_t)1ULL);
			}
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand = X_interpreter(ctx,Rn);
		uint64_t result = 0ULL;
		uint64_t done = 0ULL;
		uint64_t sig_bit;
		if ((s))
		{
			sig_bit = ((uint64_t)(((uint64_t)operand >> (uint64_t)(((uint64_t)datasize - (uint64_t)1ULL)))) & (uint64_t)1ULL);
			datasize = ((uint64_t)datasize - (uint64_t)1ULL);
		}
		for (uint64_t i = 0; i < (datasize); i++)
		{
			uint64_t working = ((uint64_t)(((uint64_t)operand >> (uint64_t)(((uint64_t)((uint64_t)datasize - (uint64_t)i) - (uint64_t)1ULL)))) & (uint64_t)1ULL);
			if ((s))
			{
				if ((((uint64_t)working != (uint64_t)sig_bit)))
				{
					done = 1ULL;
				}
			}
			else if ((working))
			{
				done = 1ULL;
			}
			if ((!done))
			{
				result = ((uint64_t)result + (uint64_t)1ULL);
			}
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	
}

void extr_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t N, uint64_t Rm, uint64_t imms, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	if (sf == 0ULL)
	{
		uint32_t operand1 = X_interpreter(ctx,Rn);
		uint32_t operand2 = X_interpreter(ctx,Rm);
		uint64_t lsb = imms;
		uint32_t result;
		if ((((uint64_t)lsb == (uint64_t)0ULL)))
		{
			result = operand2;
		}
		else if ((((uint64_t)Rn == (uint64_t)Rm)))
		{
			result = (rotate_right((uint32_t)operand1,(uint32_t)lsb));
		}
		else
		{
			result = ((uint32_t)(((uint32_t)operand2 >> (uint32_t)lsb)) | (uint32_t)(((uint32_t)operand1 << (uint32_t)(((uint64_t)datasize - (uint64_t)lsb)))));
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = X_interpreter(ctx,Rm);
		uint64_t lsb = imms;
		uint64_t result;
		if ((((uint64_t)lsb == (uint64_t)0ULL)))
		{
			result = operand2;
		}
		else if ((((uint64_t)Rn == (uint64_t)Rm)))
		{
			result = (rotate_right((uint64_t)operand1,(uint64_t)lsb));
		}
		else
		{
			result = ((uint64_t)(((uint64_t)operand2 >> (uint64_t)lsb)) | (uint64_t)(((uint64_t)operand1 << (uint64_t)(((uint64_t)datasize - (uint64_t)lsb)))));
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	
}

void bitfield_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t N, uint64_t immr, uint64_t imms, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t tmask = decode_bitmask_tmask_interpreter(ctx,N,imms,immr,0ULL,datasize,1ULL);
	uint64_t wmask = decode_bitmask_tmask_interpreter(ctx,N,imms,immr,0ULL,datasize,0ULL);
	uint64_t inzero;
	uint64_t _extend;
	if ((((uint64_t)opc == (uint64_t)0ULL)))
	{
		inzero = 1ULL;
		_extend = 1ULL;
	}
	else if ((((uint64_t)opc == (uint64_t)1ULL)))
	{
		inzero = 0ULL;
		_extend = 0ULL;
	}
	else if ((((uint64_t)opc == (uint64_t)2ULL)))
	{
		inzero = 1ULL;
		_extend = 0ULL;
	}
	else
	{
		undefined_interpreter(ctx);
	}
	if (sf == 0ULL)
	{
		uint32_t dst;
		uint32_t src = X_interpreter(ctx,Rn);
		if ((inzero))
		{
			dst = 0ULL;
		}
		else
		{
			dst = X_interpreter(ctx,Rd);
		}
		uint32_t bot = ((uint32_t)(((uint32_t)dst & (uint32_t)~wmask)) | (uint32_t)(((uint32_t)((rotate_right((uint32_t)src,(uint32_t)immr))) & (uint32_t)wmask)));
		uint32_t top;
		if ((_extend))
		{
			top = ((uint32_t)0ULL - (uint32_t)(((uint32_t)(((uint32_t)src >> (uint32_t)imms)) & (uint32_t)1ULL)));
		}
		else
		{
			top = dst;
		}
		X_interpreter(ctx,Rd,(uint64_t)((uint32_t)(((uint32_t)top & (uint32_t)~tmask)) | (uint32_t)(((uint32_t)bot & (uint32_t)tmask))));
	}
	if (sf == 1ULL)
	{
		uint64_t dst;
		uint64_t src = X_interpreter(ctx,Rn);
		if ((inzero))
		{
			dst = 0ULL;
		}
		else
		{
			dst = X_interpreter(ctx,Rd);
		}
		uint64_t bot = ((uint64_t)(((uint64_t)dst & (uint64_t)~wmask)) | (uint64_t)(((uint64_t)((rotate_right((uint64_t)src,(uint64_t)immr))) & (uint64_t)wmask)));
		uint64_t top;
		if ((_extend))
		{
			top = ((uint64_t)0ULL - (uint64_t)(((uint64_t)(((uint64_t)src >> (uint64_t)imms)) & (uint64_t)1ULL)));
		}
		else
		{
			top = dst;
		}
		X_interpreter(ctx,Rd,(uint64_t)((uint64_t)(((uint64_t)top & (uint64_t)~tmask)) | (uint64_t)(((uint64_t)bot & (uint64_t)tmask))));
	}
	
}

void logical_immediate_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t N, uint64_t immr, uint64_t imms, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	if (sf == 0ULL)
	{
		uint32_t operand1 = X_interpreter(ctx,Rn);
		uint32_t operand2 = decode_bitmask_tmask_interpreter(ctx,N,imms,immr,1ULL,datasize,0ULL);
		uint32_t result;
		if ((((uint64_t)opc == (uint64_t)0ULL)))
		{
			result = ((uint32_t)operand1 & (uint32_t)operand2);
		}
		else if ((((uint64_t)opc == (uint64_t)1ULL)))
		{
			result = ((uint32_t)operand1 | (uint32_t)operand2);
		}
		else if ((((uint64_t)opc == (uint64_t)2ULL)))
		{
			result = ((uint32_t)operand1 ^ (uint32_t)operand2);
		}
		else if ((((uint64_t)opc == (uint64_t)3ULL)))
		{
			result = ((uint32_t)operand1 & (uint32_t)operand2);
			X_interpreter(ctx,Rd,(uint64_t)result);
			_sys_interpreter(ctx,0ULL,(uint64_t)((uint32_t)(sign_extend((uint32_t)result) < sign_extend((uint32_t)0ULL))));
			_sys_interpreter(ctx,1ULL,(uint64_t)((uint32_t)result == (uint32_t)0ULL));
			_sys_interpreter(ctx,2ULL,(uint64_t)0ULL);
			_sys_interpreter(ctx,3ULL,(uint64_t)0ULL);
			return;
		}
		XSP_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = decode_bitmask_tmask_interpreter(ctx,N,imms,immr,1ULL,datasize,0ULL);
		uint64_t result;
		if ((((uint64_t)opc == (uint64_t)0ULL)))
		{
			result = ((uint64_t)operand1 & (uint64_t)operand2);
		}
		else if ((((uint64_t)opc == (uint64_t)1ULL)))
		{
			result = ((uint64_t)operand1 | (uint64_t)operand2);
		}
		else if ((((uint64_t)opc == (uint64_t)2ULL)))
		{
			result = ((uint64_t)operand1 ^ (uint64_t)operand2);
		}
		else if ((((uint64_t)opc == (uint64_t)3ULL)))
		{
			result = ((uint64_t)operand1 & (uint64_t)operand2);
			X_interpreter(ctx,Rd,(uint64_t)result);
			_sys_interpreter(ctx,0ULL,(uint64_t)((uint64_t)(sign_extend((uint64_t)result) < sign_extend((uint64_t)0ULL))));
			_sys_interpreter(ctx,1ULL,(uint64_t)((uint64_t)result == (uint64_t)0ULL));
			_sys_interpreter(ctx,2ULL,(uint64_t)0ULL);
			_sys_interpreter(ctx,3ULL,(uint64_t)0ULL);
			return;
		}
		XSP_interpreter(ctx,Rd,(uint64_t)result);
	}
	
}

void logical_shifted_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t shift, uint64_t N, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd)
{
	uint64_t shift_type = shift;
	uint64_t shift_ammount = imm6;
	if (sf == 0ULL)
	{
		uint32_t operand1 = X_interpreter(ctx,Rn);
		uint32_t operand2 = a_shift_reg_interpreter<uint32_t>(ctx,Rm,shift_type,shift_ammount);
		if ((N))
		{
			operand2 = ~operand2;
		}
		uint32_t result;
		if ((((uint64_t)((uint64_t)opc == (uint64_t)0ULL) || (uint64_t)((uint64_t)opc == (uint64_t)3ULL))))
		{
			result = ((uint32_t)operand1 & (uint32_t)operand2);
			if ((((uint64_t)opc == (uint64_t)3ULL)))
			{
				_sys_interpreter(ctx,0ULL,(uint64_t)((uint32_t)(sign_extend((uint32_t)result) < sign_extend((uint32_t)0ULL))));
				_sys_interpreter(ctx,1ULL,(uint64_t)((uint32_t)result == (uint32_t)0ULL));
				_sys_interpreter(ctx,2ULL,(uint64_t)0ULL);
				_sys_interpreter(ctx,3ULL,(uint64_t)0ULL);
			}
		}
		else if ((((uint64_t)opc == (uint64_t)1ULL)))
		{
			result = ((uint32_t)operand1 | (uint32_t)operand2);
		}
		else if ((((uint64_t)opc == (uint64_t)2ULL)))
		{
			result = ((uint32_t)operand1 ^ (uint32_t)operand2);
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = a_shift_reg_interpreter<uint64_t>(ctx,Rm,shift_type,shift_ammount);
		if ((N))
		{
			operand2 = ~operand2;
		}
		uint64_t result;
		if ((((uint64_t)((uint64_t)opc == (uint64_t)0ULL) || (uint64_t)((uint64_t)opc == (uint64_t)3ULL))))
		{
			result = ((uint64_t)operand1 & (uint64_t)operand2);
			if ((((uint64_t)opc == (uint64_t)3ULL)))
			{
				_sys_interpreter(ctx,0ULL,(uint64_t)((uint64_t)(sign_extend((uint64_t)result) < sign_extend((uint64_t)0ULL))));
				_sys_interpreter(ctx,1ULL,(uint64_t)((uint64_t)result == (uint64_t)0ULL));
				_sys_interpreter(ctx,2ULL,(uint64_t)0ULL);
				_sys_interpreter(ctx,3ULL,(uint64_t)0ULL);
			}
		}
		else if ((((uint64_t)opc == (uint64_t)1ULL)))
		{
			result = ((uint64_t)operand1 | (uint64_t)operand2);
		}
		else if ((((uint64_t)opc == (uint64_t)2ULL)))
		{
			result = ((uint64_t)operand1 ^ (uint64_t)operand2);
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	
}

void conditional_select_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t cond, uint64_t op2, uint64_t Rn, uint64_t Rd)
{
	uint64_t incrament = op2;
	uint64_t invert = op;
	if (sf == 0ULL)
	{
		uint32_t operand1 = X_interpreter(ctx,Rn);
		uint32_t operand2 = X_interpreter(ctx,Rm);
		uint32_t condition_pass = condition_holds_interpreter(ctx,cond);
		if ((condition_pass))
		{
			X_interpreter(ctx,Rd,(uint64_t)operand1);
		}
		else
		{
			if ((invert))
			{
				operand2 = ~operand2;
			}
			if ((incrament))
			{
				operand2 = ((uint32_t)operand2 + (uint32_t)1ULL);
			}
			X_interpreter(ctx,Rd,(uint64_t)operand2);
		}
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = X_interpreter(ctx,Rm);
		uint64_t condition_pass = condition_holds_interpreter(ctx,cond);
		if ((condition_pass))
		{
			X_interpreter(ctx,Rd,(uint64_t)operand1);
		}
		else
		{
			if ((invert))
			{
				operand2 = ~operand2;
			}
			if ((incrament))
			{
				operand2 = ((uint64_t)operand2 + (uint64_t)1ULL);
			}
			X_interpreter(ctx,Rd,(uint64_t)operand2);
		}
	}
	
}

void conditional_compare_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t Rm, uint64_t cond, uint64_t mode, uint64_t Rn, uint64_t nzcv)
{
	if (sf == 0ULL)
	{
		if ((condition_holds_interpreter(ctx,cond)))
		{
			uint32_t operand1 = X_interpreter(ctx,Rn);
			uint32_t operand2;
			if ((mode))
			{
				operand2 = Rm;
			}
			else
			{
				operand2 = X_interpreter(ctx,Rm);
			}
			add_subtract_impl_interpreter<uint32_t>(ctx,operand1,operand2,1ULL,((uint64_t)op == (uint64_t)0ULL));
		}
		else
		{
			_sys_interpreter(ctx,0ULL,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,1ULL,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,2ULL,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,3ULL,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL));
		}
	}
	if (sf == 1ULL)
	{
		if ((condition_holds_interpreter(ctx,cond)))
		{
			uint64_t operand1 = X_interpreter(ctx,Rn);
			uint64_t operand2;
			if ((mode))
			{
				operand2 = Rm;
			}
			else
			{
				operand2 = X_interpreter(ctx,Rm);
			}
			add_subtract_impl_interpreter<uint64_t>(ctx,operand1,operand2,1ULL,((uint64_t)op == (uint64_t)0ULL));
		}
		else
		{
			_sys_interpreter(ctx,0ULL,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,1ULL,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,2ULL,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,3ULL,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL));
		}
	}
	
}

void move_wide_immediate_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t hw, uint64_t imm16, uint64_t Rd)
{
	uint64_t shift = ((uint64_t)hw * (uint64_t)16ULL);
	uint64_t immediate = ((uint64_t)imm16 << (uint64_t)shift);
	if (sf == 0ULL)
	{
		uint32_t result;
		if ((((uint64_t)opc == (uint64_t)0ULL)))
		{
			result = ~immediate;
		}
		else if ((((uint64_t)opc == (uint64_t)3ULL)))
		{
			result = X_interpreter(ctx,Rd);
			result = ((uint32_t)result & (uint32_t)~(((uint64_t)65535ULL << (uint64_t)shift)));
			result = ((uint32_t)result | (uint32_t)immediate);
		}
		else
		{
			result = immediate;
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t result;
		if ((((uint64_t)opc == (uint64_t)0ULL)))
		{
			result = ~immediate;
		}
		else if ((((uint64_t)opc == (uint64_t)3ULL)))
		{
			result = X_interpreter(ctx,Rd);
			result = ((uint64_t)result & (uint64_t)~(((uint64_t)65535ULL << (uint64_t)shift)));
			result = ((uint64_t)result | (uint64_t)immediate);
		}
		else
		{
			result = immediate;
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	
}

void pc_rel_addressing_interpreter(interpreter_data* ctx, uint64_t op, uint64_t immlo, uint64_t immhi, uint64_t Rd)
{
	uint64_t offset = sign_extend_interpreter(ctx,((uint64_t)(((uint64_t)immhi << (uint64_t)2ULL)) | (uint64_t)immlo),21ULL);
	uint64_t instruction_pc = _get_pc_interpreter(ctx);
	if ((op))
	{
		offset = ((uint64_t)offset << (uint64_t)12ULL);
		instruction_pc = ((uint64_t)instruction_pc & (uint64_t)4095ULL);
	}
	X_interpreter(ctx,Rd,(uint64_t)((uint64_t)instruction_pc + (uint64_t)offset));
}

void load_store_register_post_interpreter(interpreter_data* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t Rn, uint64_t Rt)
{
	load_store_register_imm_unscaled_interpreter(ctx,size,VR,opc,imm9,1ULL,Rn,Rt);
}

void load_store_register_pre_interpreter(interpreter_data* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t Rn, uint64_t Rt)
{
	load_store_register_imm_unscaled_interpreter(ctx,size,VR,opc,imm9,3ULL,Rn,Rt);
}

void load_store_register_unscaled_interpreter(interpreter_data* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t Rn, uint64_t Rt)
{
	load_store_register_imm_unscaled_interpreter(ctx,size,VR,opc,imm9,0ULL,Rn,Rt);
}

void load_store_register_pair_imm_offset_interpreter(interpreter_data* ctx, uint64_t opc, uint64_t VR, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt)
{
	load_store_register_pair_imm_interpreter(ctx,opc,VR,2ULL,L,imm7,Rt2,Rn,Rt);
}

void load_store_register_pair_imm_post_interpreter(interpreter_data* ctx, uint64_t opc, uint64_t VR, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt)
{
	load_store_register_pair_imm_interpreter(ctx,opc,VR,1ULL,L,imm7,Rt2,Rn,Rt);
}

void load_store_register_pair_imm_pre_interpreter(interpreter_data* ctx, uint64_t opc, uint64_t VR, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt)
{
	load_store_register_pair_imm_interpreter(ctx,opc,VR,3ULL,L,imm7,Rt2,Rn,Rt);
}

void load_store_register_pair_imm_interpreter(interpreter_data* ctx, uint64_t opc, uint64_t VR, uint64_t wb, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt)
{
	uint64_t wback = ((uint64_t)wb != (uint64_t)2ULL);
	uint64_t postindex = ((uint64_t)wb == (uint64_t)1ULL);
	uint64_t memop = !L;
	uint64_t is_signed = (((uint64_t)opc & (uint64_t)1ULL));
	uint64_t scale;
	if ((VR))
	{
		scale = ((uint64_t)2ULL + (uint64_t)opc);
	}
	else
	{
		scale = ((uint64_t)2ULL + (uint64_t)(((uint64_t)(((uint64_t)opc >> (uint64_t)1ULL)) & (uint64_t)1ULL)));
	}
	uint64_t datasize = ((uint64_t)8ULL << (uint64_t)scale);
	uint64_t offset = ((uint64_t)sign_extend_interpreter(ctx,imm7,7ULL) << (uint64_t)scale);
	uint64_t dbytes = ((uint64_t)datasize / (uint64_t)8ULL);
	uint64_t address = XSP_interpreter(ctx,Rn);
	if ((!postindex))
	{
		address = ((uint64_t)address + (uint64_t)offset);
	}
	if (datasize == 8ULL)
	{
		if ((VR))
		{
			if ((((uint64_t)memop == (uint64_t)0ULL)))
			{
				uint8_t d0 = mem_interpreter<uint8_t>(ctx,((uint64_t)address + (uint64_t)0ULL));
				uint8_t d1 = mem_interpreter<uint8_t>(ctx,((uint64_t)address + (uint64_t)dbytes));
				V_interpreter(ctx,Rt,(uint128_t)d0);
				V_interpreter(ctx,Rt2,(uint128_t)d1);
			}
			else
			{
				mem_interpreter<uint8_t>(ctx,address,(uint8_t)V_interpreter(ctx,Rt));
				mem_interpreter<uint8_t>(ctx,((uint64_t)address + (uint64_t)dbytes),(uint8_t)V_interpreter(ctx,Rt2));
			}
		}
		else
		{
			if ((((uint64_t)memop == (uint64_t)0ULL)))
			{
				uint64_t d0 = mem_interpreter<uint8_t>(ctx,((uint64_t)address + (uint64_t)0ULL));
				uint64_t d1 = mem_interpreter<uint8_t>(ctx,((uint64_t)address + (uint64_t)dbytes));
				if ((is_signed))
				{
					d0 = (uint64_t)sign_extend((uint32_t)d0);
					d1 = (uint64_t)sign_extend((uint32_t)d1);
				}
				X_interpreter(ctx,Rt,d0);
				X_interpreter(ctx,Rt2,d1);
			}
			else
			{
				mem_interpreter<uint8_t>(ctx,address,(uint8_t)X_interpreter(ctx,Rt));
				mem_interpreter<uint8_t>(ctx,((uint64_t)address + (uint64_t)dbytes),(uint8_t)X_interpreter(ctx,Rt2));
			}
		}
		if ((wback))
		{
			if ((((uint64_t)((uint64_t)((uint64_t)((uint64_t)Rn != (uint64_t)31ULL) && (uint64_t)(((uint64_t)((uint64_t)((uint64_t)Rt == (uint64_t)Rn) || (uint64_t)((uint64_t)Rt2 == (uint64_t)Rn)) || (uint64_t)((uint64_t)Rt == (uint64_t)Rt2)))) && (uint64_t)!VR) && (uint64_t)((uint64_t)memop == (uint64_t)0ULL))))
			{
				undefined_interpreter(ctx);
			}
			if ((postindex))
			{
				address = ((uint64_t)address + (uint64_t)offset);
			}
			XSP_interpreter(ctx,Rn,address);
		}
	}
	if (datasize == 16ULL)
	{
		if ((VR))
		{
			if ((((uint64_t)memop == (uint64_t)0ULL)))
			{
				uint16_t d0 = mem_interpreter<uint16_t>(ctx,((uint64_t)address + (uint64_t)0ULL));
				uint16_t d1 = mem_interpreter<uint16_t>(ctx,((uint64_t)address + (uint64_t)dbytes));
				V_interpreter(ctx,Rt,(uint128_t)d0);
				V_interpreter(ctx,Rt2,(uint128_t)d1);
			}
			else
			{
				mem_interpreter<uint16_t>(ctx,address,(uint16_t)V_interpreter(ctx,Rt));
				mem_interpreter<uint16_t>(ctx,((uint64_t)address + (uint64_t)dbytes),(uint16_t)V_interpreter(ctx,Rt2));
			}
		}
		else
		{
			if ((((uint64_t)memop == (uint64_t)0ULL)))
			{
				uint64_t d0 = mem_interpreter<uint16_t>(ctx,((uint64_t)address + (uint64_t)0ULL));
				uint64_t d1 = mem_interpreter<uint16_t>(ctx,((uint64_t)address + (uint64_t)dbytes));
				if ((is_signed))
				{
					d0 = (uint64_t)sign_extend((uint32_t)d0);
					d1 = (uint64_t)sign_extend((uint32_t)d1);
				}
				X_interpreter(ctx,Rt,d0);
				X_interpreter(ctx,Rt2,d1);
			}
			else
			{
				mem_interpreter<uint16_t>(ctx,address,(uint16_t)X_interpreter(ctx,Rt));
				mem_interpreter<uint16_t>(ctx,((uint64_t)address + (uint64_t)dbytes),(uint16_t)X_interpreter(ctx,Rt2));
			}
		}
		if ((wback))
		{
			if ((((uint64_t)((uint64_t)((uint64_t)((uint64_t)Rn != (uint64_t)31ULL) && (uint64_t)(((uint64_t)((uint64_t)((uint64_t)Rt == (uint64_t)Rn) || (uint64_t)((uint64_t)Rt2 == (uint64_t)Rn)) || (uint64_t)((uint64_t)Rt == (uint64_t)Rt2)))) && (uint64_t)!VR) && (uint64_t)((uint64_t)memop == (uint64_t)0ULL))))
			{
				undefined_interpreter(ctx);
			}
			if ((postindex))
			{
				address = ((uint64_t)address + (uint64_t)offset);
			}
			XSP_interpreter(ctx,Rn,address);
		}
	}
	if (datasize == 32ULL)
	{
		if ((VR))
		{
			if ((((uint64_t)memop == (uint64_t)0ULL)))
			{
				uint32_t d0 = mem_interpreter<uint32_t>(ctx,((uint64_t)address + (uint64_t)0ULL));
				uint32_t d1 = mem_interpreter<uint32_t>(ctx,((uint64_t)address + (uint64_t)dbytes));
				V_interpreter(ctx,Rt,(uint128_t)d0);
				V_interpreter(ctx,Rt2,(uint128_t)d1);
			}
			else
			{
				mem_interpreter<uint32_t>(ctx,address,(uint32_t)V_interpreter(ctx,Rt));
				mem_interpreter<uint32_t>(ctx,((uint64_t)address + (uint64_t)dbytes),(uint32_t)V_interpreter(ctx,Rt2));
			}
		}
		else
		{
			if ((((uint64_t)memop == (uint64_t)0ULL)))
			{
				uint64_t d0 = mem_interpreter<uint32_t>(ctx,((uint64_t)address + (uint64_t)0ULL));
				uint64_t d1 = mem_interpreter<uint32_t>(ctx,((uint64_t)address + (uint64_t)dbytes));
				if ((is_signed))
				{
					d0 = (uint64_t)sign_extend((uint32_t)d0);
					d1 = (uint64_t)sign_extend((uint32_t)d1);
				}
				X_interpreter(ctx,Rt,d0);
				X_interpreter(ctx,Rt2,d1);
			}
			else
			{
				mem_interpreter<uint32_t>(ctx,address,(uint32_t)X_interpreter(ctx,Rt));
				mem_interpreter<uint32_t>(ctx,((uint64_t)address + (uint64_t)dbytes),(uint32_t)X_interpreter(ctx,Rt2));
			}
		}
		if ((wback))
		{
			if ((((uint64_t)((uint64_t)((uint64_t)((uint64_t)Rn != (uint64_t)31ULL) && (uint64_t)(((uint64_t)((uint64_t)((uint64_t)Rt == (uint64_t)Rn) || (uint64_t)((uint64_t)Rt2 == (uint64_t)Rn)) || (uint64_t)((uint64_t)Rt == (uint64_t)Rt2)))) && (uint64_t)!VR) && (uint64_t)((uint64_t)memop == (uint64_t)0ULL))))
			{
				undefined_interpreter(ctx);
			}
			if ((postindex))
			{
				address = ((uint64_t)address + (uint64_t)offset);
			}
			XSP_interpreter(ctx,Rn,address);
		}
	}
	if (datasize == 64ULL)
	{
		if ((VR))
		{
			if ((((uint64_t)memop == (uint64_t)0ULL)))
			{
				uint64_t d0 = mem_interpreter<uint64_t>(ctx,((uint64_t)address + (uint64_t)0ULL));
				uint64_t d1 = mem_interpreter<uint64_t>(ctx,((uint64_t)address + (uint64_t)dbytes));
				V_interpreter(ctx,Rt,(uint128_t)d0);
				V_interpreter(ctx,Rt2,(uint128_t)d1);
			}
			else
			{
				mem_interpreter<uint64_t>(ctx,address,(uint64_t)V_interpreter(ctx,Rt));
				mem_interpreter<uint64_t>(ctx,((uint64_t)address + (uint64_t)dbytes),(uint64_t)V_interpreter(ctx,Rt2));
			}
		}
		else
		{
			if ((((uint64_t)memop == (uint64_t)0ULL)))
			{
				uint64_t d0 = mem_interpreter<uint64_t>(ctx,((uint64_t)address + (uint64_t)0ULL));
				uint64_t d1 = mem_interpreter<uint64_t>(ctx,((uint64_t)address + (uint64_t)dbytes));
				if ((is_signed))
				{
					d0 = (uint64_t)sign_extend((uint32_t)d0);
					d1 = (uint64_t)sign_extend((uint32_t)d1);
				}
				X_interpreter(ctx,Rt,d0);
				X_interpreter(ctx,Rt2,d1);
			}
			else
			{
				mem_interpreter<uint64_t>(ctx,address,(uint64_t)X_interpreter(ctx,Rt));
				mem_interpreter<uint64_t>(ctx,((uint64_t)address + (uint64_t)dbytes),(uint64_t)X_interpreter(ctx,Rt2));
			}
		}
		if ((wback))
		{
			if ((((uint64_t)((uint64_t)((uint64_t)((uint64_t)Rn != (uint64_t)31ULL) && (uint64_t)(((uint64_t)((uint64_t)((uint64_t)Rt == (uint64_t)Rn) || (uint64_t)((uint64_t)Rt2 == (uint64_t)Rn)) || (uint64_t)((uint64_t)Rt == (uint64_t)Rt2)))) && (uint64_t)!VR) && (uint64_t)((uint64_t)memop == (uint64_t)0ULL))))
			{
				undefined_interpreter(ctx);
			}
			if ((postindex))
			{
				address = ((uint64_t)address + (uint64_t)offset);
			}
			XSP_interpreter(ctx,Rn,address);
		}
	}
	if (datasize == 128ULL)
	{
		if ((VR))
		{
			if ((((uint64_t)memop == (uint64_t)0ULL)))
			{
				uint128_t d0 = mem_interpreter<uint128_t>(ctx,((uint64_t)address + (uint64_t)0ULL));
				uint128_t d1 = mem_interpreter<uint128_t>(ctx,((uint64_t)address + (uint64_t)dbytes));
				V_interpreter(ctx,Rt,(uint128_t)d0);
				V_interpreter(ctx,Rt2,(uint128_t)d1);
			}
			else
			{
				mem_interpreter<uint128_t>(ctx,address,(uint128_t)V_interpreter(ctx,Rt));
				mem_interpreter<uint128_t>(ctx,((uint64_t)address + (uint64_t)dbytes),(uint128_t)V_interpreter(ctx,Rt2));
			}
		}
		else
		{
			if ((((uint64_t)memop == (uint64_t)0ULL)))
			{
				uint64_t d0 = mem_interpreter<uint128_t>(ctx,((uint64_t)address + (uint64_t)0ULL));
				uint64_t d1 = mem_interpreter<uint128_t>(ctx,((uint64_t)address + (uint64_t)dbytes));
				if ((is_signed))
				{
					d0 = (uint64_t)sign_extend((uint32_t)d0);
					d1 = (uint64_t)sign_extend((uint32_t)d1);
				}
				X_interpreter(ctx,Rt,d0);
				X_interpreter(ctx,Rt2,d1);
			}
			else
			{
				mem_interpreter<uint128_t>(ctx,address,(uint128_t)X_interpreter(ctx,Rt));
				mem_interpreter<uint128_t>(ctx,((uint64_t)address + (uint64_t)dbytes),(uint128_t)X_interpreter(ctx,Rt2));
			}
		}
		if ((wback))
		{
			if ((((uint64_t)((uint64_t)((uint64_t)((uint64_t)Rn != (uint64_t)31ULL) && (uint64_t)(((uint64_t)((uint64_t)((uint64_t)Rt == (uint64_t)Rn) || (uint64_t)((uint64_t)Rt2 == (uint64_t)Rn)) || (uint64_t)((uint64_t)Rt == (uint64_t)Rt2)))) && (uint64_t)!VR) && (uint64_t)((uint64_t)memop == (uint64_t)0ULL))))
			{
				undefined_interpreter(ctx);
			}
			if ((postindex))
			{
				address = ((uint64_t)address + (uint64_t)offset);
			}
			XSP_interpreter(ctx,Rn,address);
		}
	}
	
}

void load_store_register_imm_unsigned_interpreter(interpreter_data* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm12, uint64_t Rn, uint64_t Rt)
{
	if ((((uint64_t)((uint64_t)((uint64_t)size == (uint64_t)3ULL) && (uint64_t)((uint64_t)VR == (uint64_t)0ULL)) && (uint64_t)((uint64_t)opc == (uint64_t)2ULL))))
	{
		return;
	}
	uint64_t is_vector = ((uint64_t)VR == (uint64_t)1ULL);
	uint64_t wback = 0ULL;
	uint64_t postindex = 0ULL;
	if ((is_vector))
	{
		uint64_t memop;
		uint64_t regsize;
		uint64_t _signed;
		uint64_t scale = ((uint64_t)(((uint64_t)bit_c_interpreter(ctx,opc,1ULL) << (uint64_t)2ULL)) | (uint64_t)size);
		uint64_t offset = ((uint64_t)imm12 << (uint64_t)scale);
		if ((((uint64_t)opc & (uint64_t)1ULL)))
		memop = 0ULL;
		else
		memop = 1ULL;
		uint64_t datasize = ((uint64_t)8ULL << (uint64_t)scale);
		uint64_t address = XSP_interpreter(ctx,Rn);
		if ((!postindex))
		{
			address = ((uint64_t)address + (uint64_t)offset);
		}
		if (datasize == 8ULL)
		{
			if ((((uint64_t)memop == (uint64_t)1ULL)))
			{
				uint8_t data = V_interpreter(ctx,Rt);
				mem_interpreter<uint8_t>(ctx,address,data);
			}
			else
			{
				uint8_t data = mem_interpreter<uint8_t>(ctx,address);
				V_interpreter(ctx,Rt,(uint128_t)data);
			}
			if ((wback))
			{
				if ((postindex))
				{
					address = ((uint64_t)address + (uint64_t)offset);
				}
				XSP_interpreter(ctx,Rn,address);
			}
		}
		if (datasize == 16ULL)
		{
			if ((((uint64_t)memop == (uint64_t)1ULL)))
			{
				uint16_t data = V_interpreter(ctx,Rt);
				mem_interpreter<uint16_t>(ctx,address,data);
			}
			else
			{
				uint16_t data = mem_interpreter<uint16_t>(ctx,address);
				V_interpreter(ctx,Rt,(uint128_t)data);
			}
			if ((wback))
			{
				if ((postindex))
				{
					address = ((uint64_t)address + (uint64_t)offset);
				}
				XSP_interpreter(ctx,Rn,address);
			}
		}
		if (datasize == 32ULL)
		{
			if ((((uint64_t)memop == (uint64_t)1ULL)))
			{
				uint32_t data = V_interpreter(ctx,Rt);
				mem_interpreter<uint32_t>(ctx,address,data);
			}
			else
			{
				uint32_t data = mem_interpreter<uint32_t>(ctx,address);
				V_interpreter(ctx,Rt,(uint128_t)data);
			}
			if ((wback))
			{
				if ((postindex))
				{
					address = ((uint64_t)address + (uint64_t)offset);
				}
				XSP_interpreter(ctx,Rn,address);
			}
		}
		if (datasize == 64ULL)
		{
			if ((((uint64_t)memop == (uint64_t)1ULL)))
			{
				uint64_t data = V_interpreter(ctx,Rt);
				mem_interpreter<uint64_t>(ctx,address,data);
			}
			else
			{
				uint64_t data = mem_interpreter<uint64_t>(ctx,address);
				V_interpreter(ctx,Rt,(uint128_t)data);
			}
			if ((wback))
			{
				if ((postindex))
				{
					address = ((uint64_t)address + (uint64_t)offset);
				}
				XSP_interpreter(ctx,Rn,address);
			}
		}
		if (datasize == 128ULL)
		{
			if ((((uint64_t)memop == (uint64_t)1ULL)))
			{
				uint128_t data = V_interpreter(ctx,Rt);
				mem_interpreter<uint128_t>(ctx,address,data);
			}
			else
			{
				uint128_t data = mem_interpreter<uint128_t>(ctx,address);
				V_interpreter(ctx,Rt,(uint128_t)data);
			}
			if ((wback))
			{
				if ((postindex))
				{
					address = ((uint64_t)address + (uint64_t)offset);
				}
				XSP_interpreter(ctx,Rn,address);
			}
		}
		
	}
	else
	{
		uint64_t memop;
		uint64_t regsize;
		uint64_t _signed;
		uint64_t scale = size;
		uint64_t offset = ((uint64_t)imm12 << (uint64_t)scale);
		if ((((uint64_t)bit_c_interpreter(ctx,opc,1ULL) == (uint64_t)0ULL)))
		{
			if ((((uint64_t)opc & (uint64_t)1ULL)))
			memop = 0ULL;
			else
			memop = 1ULL;
			if ((((uint64_t)size == (uint64_t)3ULL)))
			regsize = 64ULL;
			else
			regsize = 32ULL;
			_signed = 0ULL;
		}
		else
		{
			if ((((uint64_t)size == (uint64_t)3ULL)))
			{
				undefined_interpreter(ctx);
			}
			else
			{
				memop = 0ULL;
				if ((((uint64_t)opc & (uint64_t)1ULL)))
				regsize = 32ULL;
				else
				regsize = 64ULL;
				_signed = 1ULL;
			}
		}
		uint64_t datasize = ((uint64_t)8ULL << (uint64_t)scale);
		uint64_t address = XSP_interpreter(ctx,Rn);
		if ((!postindex))
		{
			address = ((uint64_t)address + (uint64_t)offset);
		}
		if (datasize == 8ULL)
		{
			if (regsize == 32ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint8_t>(ctx,address,(uint8_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint8_t>(ctx,address);
					if ((_signed))
					{
						n = (uint32_t)sign_extend((uint8_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			if (regsize == 64ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint8_t>(ctx,address,(uint8_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint8_t>(ctx,address);
					if ((_signed))
					{
						n = (uint64_t)sign_extend((uint8_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			
		}
		if (datasize == 16ULL)
		{
			if (regsize == 32ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint16_t>(ctx,address,(uint16_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint16_t>(ctx,address);
					if ((_signed))
					{
						n = (uint32_t)sign_extend((uint16_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			if (regsize == 64ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint16_t>(ctx,address,(uint16_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint16_t>(ctx,address);
					if ((_signed))
					{
						n = (uint64_t)sign_extend((uint16_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			
		}
		if (datasize == 32ULL)
		{
			if (regsize == 32ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint32_t>(ctx,address,(uint32_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint32_t>(ctx,address);
					if ((_signed))
					{
						n = (uint32_t)sign_extend((uint32_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			if (regsize == 64ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint32_t>(ctx,address,(uint32_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint32_t>(ctx,address);
					if ((_signed))
					{
						n = (uint64_t)sign_extend((uint32_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			
		}
		if (datasize == 64ULL)
		{
			if (regsize == 32ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint64_t>(ctx,address,(uint64_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint64_t>(ctx,address);
					if ((_signed))
					{
						n = (uint32_t)sign_extend((uint64_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			if (regsize == 64ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint64_t>(ctx,address,(uint64_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint64_t>(ctx,address);
					if ((_signed))
					{
						n = (uint64_t)sign_extend((uint64_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			
		}
		
	}
}

void load_store_register_imm_unscaled_interpreter(interpreter_data* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t wb, uint64_t Rn, uint64_t Rt)
{
	if ((((uint64_t)((uint64_t)((uint64_t)size == (uint64_t)3ULL) && (uint64_t)((uint64_t)VR == (uint64_t)0ULL)) && (uint64_t)((uint64_t)opc == (uint64_t)2ULL))))
	{
		return;
	}
	uint64_t is_vector = ((uint64_t)VR == (uint64_t)1ULL);
	uint64_t wback = ((uint64_t)wb != (uint64_t)0ULL);
	uint64_t postindex = ((uint64_t)wb == (uint64_t)1ULL);
	uint64_t offset = sign_extend_interpreter(ctx,imm9,9ULL);
	if ((is_vector))
	{
		uint64_t memop;
		uint64_t regsize;
		uint64_t _signed;
		uint64_t scale = ((uint64_t)(((uint64_t)bit_c_interpreter(ctx,opc,1ULL) << (uint64_t)2ULL)) | (uint64_t)size);
		if ((((uint64_t)opc & (uint64_t)1ULL)))
		memop = 0ULL;
		else
		memop = 1ULL;
		uint64_t datasize = ((uint64_t)8ULL << (uint64_t)scale);
		uint64_t address = XSP_interpreter(ctx,Rn);
		if ((!postindex))
		{
			address = ((uint64_t)address + (uint64_t)offset);
		}
		if (datasize == 8ULL)
		{
			if ((((uint64_t)memop == (uint64_t)1ULL)))
			{
				uint8_t data = V_interpreter(ctx,Rt);
				mem_interpreter<uint8_t>(ctx,address,data);
			}
			else
			{
				uint8_t data = mem_interpreter<uint8_t>(ctx,address);
				V_interpreter(ctx,Rt,(uint128_t)data);
			}
			if ((wback))
			{
				if ((postindex))
				{
					address = ((uint64_t)address + (uint64_t)offset);
				}
				XSP_interpreter(ctx,Rn,address);
			}
		}
		if (datasize == 16ULL)
		{
			if ((((uint64_t)memop == (uint64_t)1ULL)))
			{
				uint16_t data = V_interpreter(ctx,Rt);
				mem_interpreter<uint16_t>(ctx,address,data);
			}
			else
			{
				uint16_t data = mem_interpreter<uint16_t>(ctx,address);
				V_interpreter(ctx,Rt,(uint128_t)data);
			}
			if ((wback))
			{
				if ((postindex))
				{
					address = ((uint64_t)address + (uint64_t)offset);
				}
				XSP_interpreter(ctx,Rn,address);
			}
		}
		if (datasize == 32ULL)
		{
			if ((((uint64_t)memop == (uint64_t)1ULL)))
			{
				uint32_t data = V_interpreter(ctx,Rt);
				mem_interpreter<uint32_t>(ctx,address,data);
			}
			else
			{
				uint32_t data = mem_interpreter<uint32_t>(ctx,address);
				V_interpreter(ctx,Rt,(uint128_t)data);
			}
			if ((wback))
			{
				if ((postindex))
				{
					address = ((uint64_t)address + (uint64_t)offset);
				}
				XSP_interpreter(ctx,Rn,address);
			}
		}
		if (datasize == 64ULL)
		{
			if ((((uint64_t)memop == (uint64_t)1ULL)))
			{
				uint64_t data = V_interpreter(ctx,Rt);
				mem_interpreter<uint64_t>(ctx,address,data);
			}
			else
			{
				uint64_t data = mem_interpreter<uint64_t>(ctx,address);
				V_interpreter(ctx,Rt,(uint128_t)data);
			}
			if ((wback))
			{
				if ((postindex))
				{
					address = ((uint64_t)address + (uint64_t)offset);
				}
				XSP_interpreter(ctx,Rn,address);
			}
		}
		if (datasize == 128ULL)
		{
			if ((((uint64_t)memop == (uint64_t)1ULL)))
			{
				uint128_t data = V_interpreter(ctx,Rt);
				mem_interpreter<uint128_t>(ctx,address,data);
			}
			else
			{
				uint128_t data = mem_interpreter<uint128_t>(ctx,address);
				V_interpreter(ctx,Rt,(uint128_t)data);
			}
			if ((wback))
			{
				if ((postindex))
				{
					address = ((uint64_t)address + (uint64_t)offset);
				}
				XSP_interpreter(ctx,Rn,address);
			}
		}
		
	}
	else
	{
		uint64_t memop;
		uint64_t regsize;
		uint64_t _signed;
		uint64_t scale = size;
		if ((((uint64_t)bit_c_interpreter(ctx,opc,1ULL) == (uint64_t)0ULL)))
		{
			if ((((uint64_t)opc & (uint64_t)1ULL)))
			memop = 0ULL;
			else
			memop = 1ULL;
			if ((((uint64_t)size == (uint64_t)3ULL)))
			regsize = 64ULL;
			else
			regsize = 32ULL;
			_signed = 0ULL;
		}
		else
		{
			if ((((uint64_t)size == (uint64_t)3ULL)))
			{
				undefined_interpreter(ctx);
			}
			else
			{
				memop = 0ULL;
				if ((((uint64_t)opc & (uint64_t)1ULL)))
				regsize = 32ULL;
				else
				regsize = 64ULL;
				_signed = 1ULL;
			}
		}
		uint64_t datasize = ((uint64_t)8ULL << (uint64_t)scale);
		uint64_t address = XSP_interpreter(ctx,Rn);
		if ((!postindex))
		{
			address = ((uint64_t)address + (uint64_t)offset);
		}
		if (datasize == 8ULL)
		{
			if (regsize == 32ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint8_t>(ctx,address,(uint8_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint8_t>(ctx,address);
					if ((_signed))
					{
						n = (uint32_t)sign_extend((uint8_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			if (regsize == 64ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint8_t>(ctx,address,(uint8_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint8_t>(ctx,address);
					if ((_signed))
					{
						n = (uint64_t)sign_extend((uint8_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			
		}
		if (datasize == 16ULL)
		{
			if (regsize == 32ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint16_t>(ctx,address,(uint16_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint16_t>(ctx,address);
					if ((_signed))
					{
						n = (uint32_t)sign_extend((uint16_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			if (regsize == 64ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint16_t>(ctx,address,(uint16_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint16_t>(ctx,address);
					if ((_signed))
					{
						n = (uint64_t)sign_extend((uint16_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			
		}
		if (datasize == 32ULL)
		{
			if (regsize == 32ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint32_t>(ctx,address,(uint32_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint32_t>(ctx,address);
					if ((_signed))
					{
						n = (uint32_t)sign_extend((uint32_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			if (regsize == 64ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint32_t>(ctx,address,(uint32_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint32_t>(ctx,address);
					if ((_signed))
					{
						n = (uint64_t)sign_extend((uint32_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			
		}
		if (datasize == 64ULL)
		{
			if (regsize == 32ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint64_t>(ctx,address,(uint64_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint64_t>(ctx,address);
					if ((_signed))
					{
						n = (uint32_t)sign_extend((uint64_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			if (regsize == 64ULL)
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					uint64_t data = X_interpreter(ctx,Rt);
					mem_interpreter<uint64_t>(ctx,address,(uint64_t)data);
				}
				else
				{
					uint64_t n = mem_interpreter<uint64_t>(ctx,address);
					if ((_signed))
					{
						n = (uint64_t)sign_extend((uint64_t)n);
					}
					X_interpreter(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ((uint64_t)address + (uint64_t)offset);
					}
					XSP_interpreter(ctx,Rn,address);
				}
			}
			
		}
		
	}
}

void br_interpreter(interpreter_data* ctx, uint64_t Rn)
{
	branch_long_universal_interpreter(ctx,Rn,0ULL);
}

uint64_t sign_extend_interpreter(interpreter_data* ctx, uint64_t source, uint64_t count)
{
	uint64_t max = 64ULL;
	uint64_t shift = ((uint64_t)max - (uint64_t)count);
	return ((uint64_t)(sign_extend((uint64_t)(((uint64_t)source << (uint64_t)shift))) >> sign_extend((uint64_t)shift)));
}

template <typename O>
O a_shift_reg_interpreter(interpreter_data* ctx, uint64_t m, uint64_t shift_type, uint64_t ammount)
{
	O result = X_interpreter(ctx,m);
	if ((((uint64_t)shift_type == (uint64_t)0ULL)))
	{
		return ((O)result << (O)ammount);
	}
	else if ((((uint64_t)shift_type == (uint64_t)1ULL)))
	{
		return ((O)result >> (O)ammount);
	}
	else if ((((uint64_t)shift_type == (uint64_t)2ULL)))
	{
		return ((O)(sign_extend((O)result) >> sign_extend((O)ammount)));
	}
	else
	{
		return (rotate_right((O)result,(O)ammount));
	}
}

template <typename O>
O a_extend_reg_interpreter(interpreter_data* ctx, uint64_t m, uint64_t extend_type, uint64_t shift)
{
	O val = X_interpreter(ctx,m);
	if ((((uint64_t)extend_type == (uint64_t)0ULL)))
	{
		val = ((O)val & (O)255ULL);
	}
	else if ((((uint64_t)extend_type == (uint64_t)1ULL)))
	{
		val = ((O)val & (O)65535ULL);
	}
	else if ((((uint64_t)extend_type == (uint64_t)2ULL)))
	{
		val = ((O)val & (O)4294967295ULL);
	}
	else if ((((uint64_t)extend_type == (uint64_t)4ULL)))
	{
		val = (O)sign_extend((uint8_t)val);
	}
	else if ((((uint64_t)extend_type == (uint64_t)5ULL)))
	{
		val = (O)sign_extend((uint16_t)val);
	}
	else if ((((uint64_t)extend_type == (uint64_t)6ULL)))
	{
		val = (O)sign_extend((uint32_t)val);
	}
	return ((O)val << (O)shift);
}

template <typename O>
O reverse_bytes_interpreter(interpreter_data* ctx, O source, uint64_t byte_count)
{
	O result = 0ULL;
	for (uint64_t i = 0; i < (byte_count); i++)
	{
		O working = ((O)(((O)source >> (O)(((uint64_t)i * (uint64_t)8ULL)))) & (O)255ULL);
		result = ((O)result | (O)(((O)working << (O)(((uint64_t)(((uint64_t)((uint64_t)byte_count - (uint64_t)i) - (uint64_t)1ULL)) * (uint64_t)8ULL)))));
	}
	return result;
}

uint64_t highest_bit_set_c_interpreter(interpreter_data* ctx, uint64_t src, uint64_t size)
{
	for (uint64_t i = 0; i < (size); i++)
	{
		uint64_t bit_check = ((uint64_t)((uint64_t)size - (uint64_t)i) - (uint64_t)1ULL);
		uint64_t bit = ((uint64_t)(((uint64_t)src >> (uint64_t)bit_check)) & (uint64_t)1ULL);
		if ((bit))
		{
			return bit_check;
		}
	}
	return -1ULL;
}

uint64_t ones_interpreter(interpreter_data* ctx, uint64_t size)
{
	if ((((uint64_t)size == (uint64_t)64ULL)))
	{
		return 18446744073709551615ULL;
	}
	return ((uint64_t)(((uint64_t)1ULL << (uint64_t)size)) - (uint64_t)1ULL);
}

uint64_t replicate_c_interpreter(interpreter_data* ctx, uint64_t source, uint64_t source_size, uint64_t count)
{
	uint64_t result = 0ULL;
	for (uint64_t i = 0; i < (count); i++)
	{
		result = ((uint64_t)result | (uint64_t)(((uint64_t)(((uint64_t)source & (uint64_t)ones_interpreter(ctx,source_size))) << (uint64_t)(((uint64_t)i * (uint64_t)source_size)))));
	}
	return result;
}

uint64_t bits_c_interpreter(interpreter_data* ctx, uint64_t source, uint64_t top, uint64_t bottom)
{
	top = ((uint64_t)top + (uint64_t)1ULL);
	uint64_t size = ((uint64_t)top - (uint64_t)bottom);
	uint64_t mask = ones_interpreter(ctx,size);
	return ((uint64_t)(((uint64_t)source >> (uint64_t)bottom)) & (uint64_t)mask);
}

uint64_t bit_c_interpreter(interpreter_data* ctx, uint64_t source, uint64_t bit)
{
	return ((uint64_t)(((uint64_t)source >> (uint64_t)bit)) & (uint64_t)1ULL);
}

uint64_t rotate_right_bits_interpreter(interpreter_data* ctx, uint64_t source, uint64_t ammount, uint64_t bit_count)
{
	source = ((uint64_t)source & (uint64_t)ones_interpreter(ctx,bit_count));
	return (((uint64_t)(((uint64_t)source >> (uint64_t)ammount)) | (uint64_t)(((uint64_t)source << (uint64_t)(((uint64_t)bit_count - (uint64_t)ammount))))));
}

uint64_t decode_bitmask_tmask_interpreter(interpreter_data* ctx, uint64_t immN, uint64_t imms, uint64_t immr, uint64_t immediate, uint64_t M, uint64_t return_tmask)
{
	uint64_t levels;
	uint64_t len = highest_bit_set_c_interpreter(ctx,((uint64_t)(((uint64_t)immN << (uint64_t)6ULL)) | (uint64_t)(((uint64_t)~imms & (uint64_t)ones_interpreter(ctx,6ULL)))),7ULL);
	if ((((uint64_t)len < (uint64_t)1ULL)))
	{
		undefined_interpreter(ctx);
	}
	levels = ones_interpreter(ctx,len);
	if ((((uint64_t)immediate && (uint64_t)((uint64_t)(((uint64_t)imms & (uint64_t)levels)) == (uint64_t)levels))))
	{
		undefined_interpreter(ctx);
	}
	uint64_t s = ((uint64_t)imms & (uint64_t)levels);
	uint64_t r = ((uint64_t)immr & (uint64_t)levels);
	uint64_t diff = ((uint64_t)s - (uint64_t)r);
	uint64_t esize = ((uint64_t)1ULL << (uint64_t)len);
	uint64_t d = bits_c_interpreter(ctx,diff,((uint64_t)len - (uint64_t)1ULL),0ULL);
	uint64_t welem = ones_interpreter(ctx,((uint64_t)s + (uint64_t)1ULL));
	uint64_t telem = ones_interpreter(ctx,((uint64_t)d + (uint64_t)1ULL));
	if ((return_tmask))
	{
		return replicate_c_interpreter(ctx,telem,esize,((uint64_t)M / (uint64_t)esize));
	}
	else
	{
		return replicate_c_interpreter(ctx,rotate_right_bits_interpreter(ctx,welem,r,esize),esize,((uint64_t)M / (uint64_t)esize));
	}
}

uint64_t decode_add_subtract_imm_12_interpreter(interpreter_data* ctx, uint64_t source, uint64_t shift)
{
	return ((uint64_t)source << (uint64_t)(((uint64_t)shift * (uint64_t)12ULL)));
}

template <typename O>
O add_subtract_impl_interpreter(interpreter_data* ctx, O n, O m, uint64_t set_flags, uint64_t is_add)
{
	O d;
	if ((is_add))
	{
		d = ((O)n + (O)m);
	}
	else
	{
		d = ((O)n - (O)m);
	}
	if ((set_flags))
	{
		_sys_interpreter(ctx,0ULL,(uint64_t)((O)(sign_extend((O)d) < sign_extend((O)0ULL))));
		_sys_interpreter(ctx,1ULL,(uint64_t)((O)d == (O)0ULL));
		if ((is_add))
		{
			_sys_interpreter(ctx,2ULL,(uint64_t)((O)d < (O)n));
			_sys_interpreter(ctx,3ULL,(uint64_t)((O)(sign_extend((O)(((O)(((O)d ^ (O)n)) & (O)~(((O)n ^ (O)m))))) < sign_extend((O)0ULL))));
		}
		else
		{
			_sys_interpreter(ctx,2ULL,(uint64_t)((O)n >= (O)m));
			_sys_interpreter(ctx,3ULL,(uint64_t)((O)(sign_extend((O)(((O)(((O)d ^ (O)n)) & (O)(((O)n ^ (O)m))))) < sign_extend((O)0ULL))));
		}
	}
	return d;
}

template <typename O>
O add_subtract_carry_impl_interpreter(interpreter_data* ctx, O n, O m, uint64_t set_flags, uint64_t is_add, O carry)
{
	O d;
	if ((is_add))
	{
		d = ((O)((O)n + (O)m) + (O)carry);
	}
	else
	{
		d = ((O)((O)n - (O)m) - (O)(((O)carry ^ (O)1ULL)));
	}
	if ((set_flags))
	{
		_sys_interpreter(ctx,0ULL,(uint64_t)((O)(sign_extend((O)d) < sign_extend((O)0ULL))));
		_sys_interpreter(ctx,1ULL,(uint64_t)((O)d == (O)0ULL));
		if ((is_add))
		{
			_sys_interpreter(ctx,2ULL,(uint64_t)((O)(((O)((O)d == (O)n) && (O)carry)) | (O)((O)d < (O)n)));
			_sys_interpreter(ctx,3ULL,(uint64_t)((O)(sign_extend((O)(((O)(((O)d ^ (O)n)) & (O)~(((O)n ^ (O)m))))) < sign_extend((O)0ULL))));
		}
		else
		{
			_sys_interpreter(ctx,2ULL,(uint64_t)((O)(((O)((O)n == (O)m) && (O)carry)) | (O)((O)n > (O)m)));
			_sys_interpreter(ctx,3ULL,(uint64_t)((O)(sign_extend((O)(((O)(((O)d ^ (O)n)) & (O)(((O)n ^ (O)m))))) < sign_extend((O)0ULL))));
		}
	}
	return d;
}

uint8_t condition_holds_interpreter(interpreter_data* ctx, uint64_t cond)
{
	uint8_t n = _sys_interpreter(ctx,0ULL);
	uint8_t z = _sys_interpreter(ctx,1ULL);
	uint8_t c = _sys_interpreter(ctx,2ULL);
	uint8_t v = _sys_interpreter(ctx,3ULL);
	uint64_t raw_condition = ((uint64_t)cond >> (uint64_t)1ULL);
	uint8_t result;
	if ((((uint64_t)raw_condition == (uint64_t)0ULL)))
	{
		result = ((uint8_t)z == (uint8_t)1ULL);
	}
	else if ((((uint64_t)raw_condition == (uint64_t)1ULL)))
	{
		result = ((uint8_t)c == (uint8_t)1ULL);
	}
	else if ((((uint64_t)raw_condition == (uint64_t)2ULL)))
	{
		result = ((uint8_t)n == (uint8_t)1ULL);
	}
	else if ((((uint64_t)raw_condition == (uint64_t)3ULL)))
	{
		result = ((uint8_t)v == (uint8_t)1ULL);
	}
	else if ((((uint64_t)raw_condition == (uint64_t)4ULL)))
	{
		result = ((uint8_t)((uint8_t)c == (uint8_t)1ULL) && (uint8_t)((uint8_t)z == (uint8_t)0ULL));
	}
	else if ((((uint64_t)raw_condition == (uint64_t)5ULL)))
	{
		result = ((uint8_t)n == (uint8_t)v);
	}
	else if ((((uint64_t)raw_condition == (uint64_t)6ULL)))
	{
		result = ((uint8_t)(((uint8_t)n == (uint8_t)v)) && (uint8_t)((uint8_t)z == (uint8_t)0ULL));
	}
	else
	{
		result = 1ULL;
	}
	if ((((uint64_t)(((uint64_t)cond & (uint64_t)1ULL)) && (uint64_t)((uint64_t)cond != (uint64_t)15ULL))))
	{
		result = ((uint8_t)result ^ (uint8_t)1ULL);
	}
	return result;
}

void branch_long_universal_interpreter(interpreter_data* ctx, uint64_t Rn, uint64_t link)
{
	uint64_t branch_location = X_interpreter(ctx,Rn);
	if ((link))
	{
		uint64_t link_address = ((uint64_t)_get_pc_interpreter(ctx) + (uint64_t)4ULL);
		X_interpreter(ctx,30ULL,link_address);
	}
	_branch_long_interpreter(ctx,branch_location);
}

template <typename O>
void mem_interpreter(interpreter_data* ctx, uint64_t address, O value)
{
	address = translate_address_interpreter(ctx,address);
	*(O*)address = value;
}

template <typename O>
O mem_interpreter(interpreter_data* ctx, uint64_t address)
{
	address = translate_address_interpreter(ctx,address);
	return *(O*)address;
}

uint64_t XSP_interpreter(interpreter_data* ctx, uint64_t reg_id)
{
	return _x_interpreter(ctx,reg_id);
}

void XSP_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value)
{
	_x_interpreter(ctx,reg_id,value);
}

uint64_t X_interpreter(interpreter_data* ctx, uint64_t reg_id)
{
	if ((((uint64_t)reg_id == (uint64_t)31ULL)))
	{
		return 0ULL;
	}
	else
	{
		return _x_interpreter(ctx,reg_id);
	}
}

void X_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value)
{
	if ((((uint64_t)reg_id == (uint64_t)31ULL)))
	{
		return;
	}
	else
	{
		_x_interpreter(ctx,reg_id,value);
	}
}

void add_subtract_imm12_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t sh, uint64_t imm12, uint64_t Rn, uint64_t Rd)
{
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = copy_new_raw_size(ctx, XSP_jit(ctx,Rn), O);
		ir_operand operand2 = ir_operand::create_con(decode_add_subtract_imm_12_jit(ctx,imm12,sh), O);
		ir_operand d = copy_new_raw_size(ctx, add_subtract_impl_jit(ctx,O,operand1,operand2,S,((uint64_t)op == (uint64_t)0ULL)), O);
		if ((S))
		{
			X_jit(ctx,Rd,copy_new_raw_size(ctx, d, int64));
		}
		else
		{
			XSP_jit(ctx,Rd,copy_new_raw_size(ctx, d, int64));
		}
	}
}

void add_subtract_shifted_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t shift, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd)
{
	uint64_t shift_ammount = imm6;
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand operand2 = copy_new_raw_size(ctx, a_shift_reg_jit(ctx,O,Rm,shift,shift_ammount), O);
		ir_operand result = copy_new_raw_size(ctx, add_subtract_impl_jit(ctx,O,operand1,operand2,S,((uint64_t)op == (uint64_t)0ULL)), O);
		X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void add_subtract_extended_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t option, uint64_t imm3, uint64_t Rn, uint64_t Rd)
{
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		uint64_t shift = imm3;
		uint64_t extend_type = option;
		ir_operand operand1 = copy_new_raw_size(ctx, XSP_jit(ctx,Rn), O);
		ir_operand operand2 = copy_new_raw_size(ctx, a_extend_reg_jit(ctx,O,Rm,extend_type,shift), O);
		ir_operand result = copy_new_raw_size(ctx, add_subtract_impl_jit(ctx,O,operand1,operand2,S,((uint64_t)op == (uint64_t)0ULL)), O);
		if ((S))
		{
			X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
		}
		else
		{
			XSP_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
		}
	}
}

void add_subtract_carry_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand operand2 = copy_new_raw_size(ctx, X_jit(ctx,Rm), O);
		ir_operand result = copy_new_raw_size(ctx, add_subtract_carry_impl_jit(ctx,O,operand1,operand2,((uint64_t)S == (uint64_t)1ULL),((uint64_t)op == (uint64_t)0ULL),copy_new_raw_size(ctx, _sys_jit(ctx,2ULL), O)), O);
		X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void shift_variable_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rm, uint64_t op2, uint64_t Rn, uint64_t Rd)
{
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand operand2 = copy_new_raw_size(ctx, X_jit(ctx,Rm), O);
		uint64_t mask = ((uint64_t)(((uint64_t)32ULL << (uint64_t)sf)) - (uint64_t)1ULL);
		ir_operand result;
		operand2 = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, operand2, ir_operand::create_con(mask, O));
		if ((((uint64_t)op2 == (uint64_t)0ULL)))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_shift_left, operand1, operand2);
		}
		else if ((((uint64_t)op2 == (uint64_t)1ULL)))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, operand1, operand2);
		}
		else if ((((uint64_t)op2 == (uint64_t)2ULL)))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_shift_right_signed, operand1, operand2);
		}
		else
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_rotate_right, operand1, operand2);
		}
		X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void multiply_with_32_jit(ssa_emit_context* ctx, uint64_t U, uint64_t Rm, uint64_t o0, uint64_t Ra, uint64_t Rn, uint64_t Rd)
{
	ir_operand operand1 = X_jit(ctx,Rn);
	ir_operand operand2 = X_jit(ctx,Rm);
	ir_operand operand3 = X_jit(ctx,Ra);
	uint64_t is_add = ((uint64_t)o0 == (uint64_t)0ULL);
	uint64_t is_signed = ((uint64_t)U == (uint64_t)0ULL);
	if ((is_signed))
	{
		operand1 = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, operand1, int32), int64);
		operand2 = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, operand2, int32), int64);
	}
	else
	{
		operand1 = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, operand1, ir_operand::create_con(4294967295ULL, int64));
		operand2 = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, operand2, ir_operand::create_con(4294967295ULL, int64));
	}
	ir_operand result;
	if ((is_add))
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_add, operand3, ssa_emit_context::emit_ssa(ctx, ir_multiply, operand1, operand2));
	}
	else
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_subtract, operand3, ssa_emit_context::emit_ssa(ctx, ir_multiply, operand1, operand2));
	}
	X_jit(ctx,Rd,result);
}

void multiply_hi_jit(ssa_emit_context* ctx, uint64_t U, uint64_t Rm, uint64_t o0, uint64_t Rn, uint64_t Rd)
{
	ir_operand operand1 = X_jit(ctx,Rn);
	ir_operand operand2 = X_jit(ctx,Rm);
	uint64_t is_signed = ((uint64_t)U == (uint64_t)0ULL);
	ir_operand result;
	if ((is_signed))
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_multiply_hi_signed, operand1, operand2);
	}
	else
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_multiply_hi_unsigned, operand1, operand2);
	}
	X_jit(ctx,Rd,result);
}

void multiply_additive_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rm, uint64_t o0, uint64_t Ra, uint64_t Rn, uint64_t Rd)
{
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand operand2 = copy_new_raw_size(ctx, X_jit(ctx,Rm), O);
		ir_operand operand3 = copy_new_raw_size(ctx, X_jit(ctx,Ra), O);
		uint64_t is_add = ((uint64_t)o0 == (uint64_t)0ULL);
		ir_operand result;
		if ((is_add))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_add, operand3, ssa_emit_context::emit_ssa(ctx, ir_multiply, operand1, operand2));
		}
		else
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_subtract, operand3, ssa_emit_context::emit_ssa(ctx, ir_multiply, operand1, operand2));
		}
		X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void divide_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rm, uint64_t o1, uint64_t Rn, uint64_t Rd)
{
	uint64_t is_signed = ((uint64_t)o1 == (uint64_t)1ULL);
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand operand2 = copy_new_raw_size(ctx, X_jit(ctx,Rm), O);
		ir_operand result;
		{
		    ir_operand end = ir_operation_block::create_label(ctx->ir);
		    ir_operand yes = ir_operation_block::create_label(ctx->ir);
		
		    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_equal, operand2, ir_operand::create_con(0ULL, O));
		
		    ir_operation_block::jump_if(ctx->ir,yes, condition);
			{
				if ((is_signed))
				{
					uint64_t min = 9223372036854775808ULL;
					if ((!sf))
					{
						min = ((uint64_t)min >> (uint64_t)32ULL);
					}
					{
					    ir_operand end = ir_operation_block::create_label(ctx->ir);
					    ir_operand yes = ir_operation_block::create_label(ctx->ir);
					
					    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, operand1, ir_operand::create_con(min, O)), ssa_emit_context::emit_ssa(ctx, ir_compare_equal, operand2, ir_operand::create_con(-1ULL, O)));
					
					    ir_operation_block::jump_if(ctx->ir,yes, condition);
						{
							X_jit(ctx,Rd,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_divide_signed, operand1, operand2), int64));
						}
					    
					    ir_operation_block::jump(ctx->ir,end);
					    ir_operation_block::mark_label(ctx->ir, yes);
					
						{
							X_jit(ctx,Rd,ir_operand::create_con(min, int64));
						}
					
					    ir_operation_block::mark_label(ctx->ir, end);
					}
				}
				else
				{
					X_jit(ctx,Rd,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_divide_unsigned, operand1, operand2), int64));
				}
			}
		    
		    ir_operation_block::jump(ctx->ir,end);
		    ir_operation_block::mark_label(ctx->ir, yes);
		
			{
				X_jit(ctx,Rd,ir_operand::create_con(0ULL, int64));
			}
		
		    ir_operation_block::mark_label(ctx->ir, end);
		}
	}
}

void rbit_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand result = ir_operand::create_con(0ULL, O);
		for (uint64_t i = 0; i < (datasize); i++)
		{
			ir_operand working = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, operand, ir_operand::create_con(i, O)), ir_operand::create_con(1ULL, O));
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, result, ssa_emit_context::emit_ssa(ctx, ir_shift_left, working, ir_operand::create_con((((uint64_t)((uint64_t)datasize - (uint64_t)i) - (uint64_t)1ULL)), O)));
		}
		X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void rev16_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rn, uint64_t Rd)
{
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		uint64_t count = ((uint64_t)2ULL << (uint64_t)sf);
		ir_operand working = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand result = ir_operand::create_con(0ULL, O);
		for (uint64_t i = 0; i < (count); i++)
		{
			ir_operand part = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, working, ir_operand::create_con((((uint64_t)i * (uint64_t)16ULL)), O)), ir_operand::create_con(65535ULL, O));
			part = copy_new_raw_size(ctx, reverse_bytes_jit(ctx,O,part,2ULL), O);
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, result, ssa_emit_context::emit_ssa(ctx, ir_shift_left, part, ir_operand::create_con((((uint64_t)i * (uint64_t)16ULL)), O)));
		}
		X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void reverse_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand working = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand result;
		if ((((uint64_t)sf == (uint64_t)opc)))
		{
			result = copy_new_raw_size(ctx, reverse_bytes_jit(ctx,O,working,((uint64_t)4ULL << (uint64_t)sf)), O);
		}
		else
		{
			result = copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, reverse_bytes_jit(ctx,O,working,4ULL), ssa_emit_context::emit_ssa(ctx, ir_shift_left, reverse_bytes_jit(ctx,O,ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, working, ir_operand::create_con(32ULL, O)),4ULL), ir_operand::create_con(32ULL, O))), O);
		}
		X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void count_leading_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t s, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand result = ssa_emit_context::emit_ssa(ctx, ir_move, ir_operand::create_con(0ULL, O));
		ir_operand done = ssa_emit_context::emit_ssa(ctx, ir_move, ir_operand::create_con(0ULL, O));
		ir_operand sig_bit;
		if ((s))
		{
			sig_bit = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, operand, ir_operand::create_con((((uint64_t)datasize - (uint64_t)1ULL)), O)), ir_operand::create_con(1ULL, O));
			datasize = ((uint64_t)datasize - (uint64_t)1ULL);
		}
		for (uint64_t i = 0; i < (datasize); i++)
		{
			ir_operand working = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, operand, ir_operand::create_con((((uint64_t)((uint64_t)datasize - (uint64_t)i) - (uint64_t)1ULL)), O)), ir_operand::create_con(1ULL, O));
			if ((s))
			{
				{
				    ir_operand end = ir_operation_block::create_label(ctx->ir);
				    ir_operand yes = ir_operation_block::create_label(ctx->ir);
				
				    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_not_equal, working, sig_bit);
				
				    ir_operation_block::jump_if(ctx->ir,yes, condition);
					/* TODO if statements without a no should not have this*/
				    
				    ir_operation_block::jump(ctx->ir,end);
				    ir_operation_block::mark_label(ctx->ir, yes);
				
					{
						ssa_emit_context::move(ctx,done,ir_operand::create_con(1ULL, O));
					}
				
				    ir_operation_block::mark_label(ctx->ir, end);
				}
			}
			else {
			    ir_operand end = ir_operation_block::create_label(ctx->ir);
			    ir_operand yes = ir_operation_block::create_label(ctx->ir);
			
			    ir_operand condition = working;
			
			    ir_operation_block::jump_if(ctx->ir,yes, condition);
				/* TODO if statements without a no should not have this*/
			    
			    ir_operation_block::jump(ctx->ir,end);
			    ir_operation_block::mark_label(ctx->ir, yes);
			
				{
					ssa_emit_context::move(ctx,done,ir_operand::create_con(1ULL, O));
				}
			
			    ir_operation_block::mark_label(ctx->ir, end);
			}
			{
			    ir_operand end = ir_operation_block::create_label(ctx->ir);
			    ir_operand yes = ir_operation_block::create_label(ctx->ir);
			
			    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_logical_not, done);
			
			    ir_operation_block::jump_if(ctx->ir,yes, condition);
				/* TODO if statements without a no should not have this*/
			    
			    ir_operation_block::jump(ctx->ir,end);
			    ir_operation_block::mark_label(ctx->ir, yes);
			
				{
					ssa_emit_context::move(ctx,result,ssa_emit_context::emit_ssa(ctx, ir_add, result, ir_operand::create_con(1ULL, O)));
				}
			
			    ir_operation_block::mark_label(ctx->ir, end);
			}
		}
		X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void extr_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t N, uint64_t Rm, uint64_t imms, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand operand2 = copy_new_raw_size(ctx, X_jit(ctx,Rm), O);
		uint64_t lsb = imms;
		ir_operand result;
		if ((((uint64_t)lsb == (uint64_t)0ULL)))
		{
			result = operand2;
		}
		else if ((((uint64_t)Rn == (uint64_t)Rm)))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_rotate_right, operand1, ir_operand::create_con(lsb, O));
		}
		else
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, operand2, ir_operand::create_con(lsb, O)), ssa_emit_context::emit_ssa(ctx, ir_shift_left, operand1, ir_operand::create_con((((uint64_t)datasize - (uint64_t)lsb)), O)));
		}
		X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void bitfield_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t N, uint64_t immr, uint64_t imms, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t tmask = decode_bitmask_tmask_jit(ctx,N,imms,immr,0ULL,datasize,1ULL);
	uint64_t wmask = decode_bitmask_tmask_jit(ctx,N,imms,immr,0ULL,datasize,0ULL);
	uint64_t inzero;
	uint64_t _extend;
	if ((((uint64_t)opc == (uint64_t)0ULL)))
	{
		inzero = 1ULL;
		_extend = 1ULL;
	}
	else if ((((uint64_t)opc == (uint64_t)1ULL)))
	{
		inzero = 0ULL;
		_extend = 0ULL;
	}
	else if ((((uint64_t)opc == (uint64_t)2ULL)))
	{
		inzero = 1ULL;
		_extend = 0ULL;
	}
	else
	{
		undefined_jit(ctx);
	}
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand dst;
		ir_operand src = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		if ((inzero))
		{
			dst = ir_operand::create_con(0ULL, O);
		}
		else
		{
			dst = copy_new_raw_size(ctx, X_jit(ctx,Rd), O);
		}
		ir_operand bot = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, dst, ir_operand::create_con(~wmask, O)), ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_rotate_right, src, ir_operand::create_con(immr, O)), ir_operand::create_con(wmask, O)));
		ir_operand top;
		if ((_extend))
		{
			top = ssa_emit_context::emit_ssa(ctx, ir_subtract, ir_operand::create_con(0ULL, O), ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, src, ir_operand::create_con(imms, O)), ir_operand::create_con(1ULL, O)));
		}
		else
		{
			top = dst;
		}
		X_jit(ctx,Rd,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, top, ir_operand::create_con(~tmask, O)), ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, bot, ir_operand::create_con(tmask, O))), int64));
	}
}

void logical_immediate_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t N, uint64_t immr, uint64_t imms, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand operand2 = ir_operand::create_con(decode_bitmask_tmask_jit(ctx,N,imms,immr,1ULL,datasize,0ULL), O);
		ir_operand result;
		if ((((uint64_t)opc == (uint64_t)0ULL)))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, operand1, operand2);
		}
		else if ((((uint64_t)opc == (uint64_t)1ULL)))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, operand1, operand2);
		}
		else if ((((uint64_t)opc == (uint64_t)2ULL)))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, operand1, operand2);
		}
		else if ((((uint64_t)opc == (uint64_t)3ULL)))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, operand1, operand2);
			X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
			_sys_jit(ctx,0ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, result, ir_operand::create_con(0ULL, O)), int64));
			_sys_jit(ctx,1ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, result, ir_operand::create_con(0ULL, O)), int64));
			_sys_jit(ctx,2ULL,ir_operand::create_con(0ULL, int64));
			_sys_jit(ctx,3ULL,ir_operand::create_con(0ULL, int64));
			return;
		}
		XSP_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void logical_shifted_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t shift, uint64_t N, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd)
{
	uint64_t shift_type = shift;
	uint64_t shift_ammount = imm6;
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand operand2 = copy_new_raw_size(ctx, a_shift_reg_jit(ctx,O,Rm,shift_type,shift_ammount), O);
		if ((N))
		{
			operand2 = ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, operand2);
		}
		ir_operand result;
		if ((((uint64_t)((uint64_t)opc == (uint64_t)0ULL) || (uint64_t)((uint64_t)opc == (uint64_t)3ULL))))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, operand1, operand2);
			if ((((uint64_t)opc == (uint64_t)3ULL)))
			{
				_sys_jit(ctx,0ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, result, ir_operand::create_con(0ULL, O)), int64));
				_sys_jit(ctx,1ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, result, ir_operand::create_con(0ULL, O)), int64));
				_sys_jit(ctx,2ULL,ir_operand::create_con(0ULL, int64));
				_sys_jit(ctx,3ULL,ir_operand::create_con(0ULL, int64));
			}
		}
		else if ((((uint64_t)opc == (uint64_t)1ULL)))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, operand1, operand2);
		}
		else if ((((uint64_t)opc == (uint64_t)2ULL)))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, operand1, operand2);
		}
		X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void conditional_select_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t cond, uint64_t op2, uint64_t Rn, uint64_t Rd)
{
	uint64_t incrament = op2;
	uint64_t invert = op;
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand operand2 = copy_new_raw_size(ctx, X_jit(ctx,Rm), O);
		ir_operand condition_pass = copy_new_raw_size(ctx, condition_holds_jit(ctx,cond), O);
		{
		    ir_operand end = ir_operation_block::create_label(ctx->ir);
		    ir_operand yes = ir_operation_block::create_label(ctx->ir);
		
		    ir_operand condition = condition_pass;
		
		    ir_operation_block::jump_if(ctx->ir,yes, condition);
			{
				if ((invert))
				{
					operand2 = ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, operand2);
				}
				if ((incrament))
				{
					operand2 = ssa_emit_context::emit_ssa(ctx, ir_add, operand2, ir_operand::create_con(1ULL, O));
				}
				X_jit(ctx,Rd,copy_new_raw_size(ctx, operand2, int64));
			}
		    
		    ir_operation_block::jump(ctx->ir,end);
		    ir_operation_block::mark_label(ctx->ir, yes);
		
			{
				X_jit(ctx,Rd,copy_new_raw_size(ctx, operand1, int64));
			}
		
		    ir_operation_block::mark_label(ctx->ir, end);
		}
	}
}

void conditional_compare_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t Rm, uint64_t cond, uint64_t mode, uint64_t Rn, uint64_t nzcv)
{
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		{
		    ir_operand end = ir_operation_block::create_label(ctx->ir);
		    ir_operand yes = ir_operation_block::create_label(ctx->ir);
		
		    ir_operand condition = condition_holds_jit(ctx,cond);
		
		    ir_operation_block::jump_if(ctx->ir,yes, condition);
			{
				_sys_jit(ctx,0ULL,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL), int64));
				_sys_jit(ctx,1ULL,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL), int64));
				_sys_jit(ctx,2ULL,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL), int64));
				_sys_jit(ctx,3ULL,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL), int64));
			}
		    
		    ir_operation_block::jump(ctx->ir,end);
		    ir_operation_block::mark_label(ctx->ir, yes);
		
			{
				ir_operand operand1 = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
				ir_operand operand2;
				if ((mode))
				{
					operand2 = ir_operand::create_con(Rm, O);
				}
				else
				{
					operand2 = copy_new_raw_size(ctx, X_jit(ctx,Rm), O);
				}
				add_subtract_impl_jit(ctx,O,operand1,operand2,1ULL,((uint64_t)op == (uint64_t)0ULL));
			}
		
		    ir_operation_block::mark_label(ctx->ir, end);
		}
	}
}

void move_wide_immediate_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t hw, uint64_t imm16, uint64_t Rd)
{
	uint64_t shift = ((uint64_t)hw * (uint64_t)16ULL);
	uint64_t immediate = ((uint64_t)imm16 << (uint64_t)shift);
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand result;
		if ((((uint64_t)opc == (uint64_t)0ULL)))
		{
			result = ir_operand::create_con(~immediate, O);
		}
		else if ((((uint64_t)opc == (uint64_t)3ULL)))
		{
			result = copy_new_raw_size(ctx, X_jit(ctx,Rd), O);
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, result, ir_operand::create_con(~(((uint64_t)65535ULL << (uint64_t)shift)), O));
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, result, ir_operand::create_con(immediate, O));
		}
		else
		{
			result = ir_operand::create_con(immediate, O);
		}
		X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void pc_rel_addressing_jit(ssa_emit_context* ctx, uint64_t op, uint64_t immlo, uint64_t immhi, uint64_t Rd)
{
	uint64_t offset = sign_extend_jit(ctx,((uint64_t)(((uint64_t)immhi << (uint64_t)2ULL)) | (uint64_t)immlo),21ULL);
	uint64_t instruction_pc = _get_pc_jit(ctx);
	if ((op))
	{
		offset = ((uint64_t)offset << (uint64_t)12ULL);
		instruction_pc = ((uint64_t)instruction_pc & (uint64_t)4095ULL);
	}
	X_jit(ctx,Rd,ir_operand::create_con(((uint64_t)instruction_pc + (uint64_t)offset), int64));
}

void load_store_register_post_jit(ssa_emit_context* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t Rn, uint64_t Rt)
{
	load_store_register_imm_unscaled_jit(ctx,size,VR,opc,imm9,1ULL,Rn,Rt);
}

void load_store_register_pre_jit(ssa_emit_context* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t Rn, uint64_t Rt)
{
	load_store_register_imm_unscaled_jit(ctx,size,VR,opc,imm9,3ULL,Rn,Rt);
}

void load_store_register_unscaled_jit(ssa_emit_context* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t Rn, uint64_t Rt)
{
	load_store_register_imm_unscaled_jit(ctx,size,VR,opc,imm9,0ULL,Rn,Rt);
}

void load_store_register_pair_imm_offset_jit(ssa_emit_context* ctx, uint64_t opc, uint64_t VR, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt)
{
	load_store_register_pair_imm_jit(ctx,opc,VR,2ULL,L,imm7,Rt2,Rn,Rt);
}

void load_store_register_pair_imm_post_jit(ssa_emit_context* ctx, uint64_t opc, uint64_t VR, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt)
{
	load_store_register_pair_imm_jit(ctx,opc,VR,1ULL,L,imm7,Rt2,Rn,Rt);
}

void load_store_register_pair_imm_pre_jit(ssa_emit_context* ctx, uint64_t opc, uint64_t VR, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt)
{
	load_store_register_pair_imm_jit(ctx,opc,VR,3ULL,L,imm7,Rt2,Rn,Rt);
}

void load_store_register_pair_imm_jit(ssa_emit_context* ctx, uint64_t opc, uint64_t VR, uint64_t wb, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt)
{
	uint64_t wback = ((uint64_t)wb != (uint64_t)2ULL);
	uint64_t postindex = ((uint64_t)wb == (uint64_t)1ULL);
	uint64_t memop = !L;
	uint64_t is_signed = (((uint64_t)opc & (uint64_t)1ULL));
	uint64_t scale;
	if ((VR))
	{
		scale = ((uint64_t)2ULL + (uint64_t)opc);
	}
	else
	{
		scale = ((uint64_t)2ULL + (uint64_t)(((uint64_t)(((uint64_t)opc >> (uint64_t)1ULL)) & (uint64_t)1ULL)));
	}
	uint64_t datasize = ((uint64_t)8ULL << (uint64_t)scale);
	uint64_t offset = ((uint64_t)sign_extend_jit(ctx,imm7,7ULL) << (uint64_t)scale);
	uint64_t dbytes = ((uint64_t)datasize / (uint64_t)8ULL);
	ir_operand address = XSP_jit(ctx,Rn);
	if ((!postindex))
	{
		address = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offset, int64));
	}
	uint64_t S = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : datasize == 128ULL ? int128 : throw 0;
	{
		if ((VR))
		{
			if ((((uint64_t)memop == (uint64_t)0ULL)))
			{
				ir_operand d0 = copy_new_raw_size(ctx, mem_jit(ctx,S,ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(0ULL, int64))), S);
				ir_operand d1 = copy_new_raw_size(ctx, mem_jit(ctx,S,ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(dbytes, int64))), S);
				V_jit(ctx,Rt,copy_new_raw_size(ctx, d0, int128));
				V_jit(ctx,Rt2,copy_new_raw_size(ctx, d1, int128));
			}
			else
			{
				mem_jit(ctx,S,address,copy_new_raw_size(ctx, V_jit(ctx,Rt), S));
				mem_jit(ctx,S,ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(dbytes, int64)),copy_new_raw_size(ctx, V_jit(ctx,Rt2), S));
			}
		}
		else
		{
			if ((((uint64_t)memop == (uint64_t)0ULL)))
			{
				ir_operand d0 = copy_new_raw_size(ctx, mem_jit(ctx,S,ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(0ULL, int64))), int64);
				ir_operand d1 = copy_new_raw_size(ctx, mem_jit(ctx,S,ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(dbytes, int64))), int64);
				if ((is_signed))
				{
					d0 = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, d0, int32), int64);
					d1 = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, d1, int32), int64);
				}
				X_jit(ctx,Rt,d0);
				X_jit(ctx,Rt2,d1);
			}
			else
			{
				mem_jit(ctx,S,address,copy_new_raw_size(ctx, X_jit(ctx,Rt), S));
				mem_jit(ctx,S,ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(dbytes, int64)),copy_new_raw_size(ctx, X_jit(ctx,Rt2), S));
			}
		}
		if ((wback))
		{
			if ((((uint64_t)((uint64_t)((uint64_t)((uint64_t)Rn != (uint64_t)31ULL) && (uint64_t)(((uint64_t)((uint64_t)((uint64_t)Rt == (uint64_t)Rn) || (uint64_t)((uint64_t)Rt2 == (uint64_t)Rn)) || (uint64_t)((uint64_t)Rt == (uint64_t)Rt2)))) && (uint64_t)!VR) && (uint64_t)((uint64_t)memop == (uint64_t)0ULL))))
			{
				undefined_jit(ctx);
			}
			if ((postindex))
			{
				address = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offset, int64));
			}
			XSP_jit(ctx,Rn,address);
		}
	}
}

void load_store_register_imm_unsigned_jit(ssa_emit_context* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm12, uint64_t Rn, uint64_t Rt)
{
	if ((((uint64_t)((uint64_t)((uint64_t)size == (uint64_t)3ULL) && (uint64_t)((uint64_t)VR == (uint64_t)0ULL)) && (uint64_t)((uint64_t)opc == (uint64_t)2ULL))))
	{
		return;
	}
	uint64_t is_vector = ((uint64_t)VR == (uint64_t)1ULL);
	uint64_t wback = 0ULL;
	uint64_t postindex = 0ULL;
	if ((is_vector))
	{
		uint64_t memop;
		uint64_t regsize;
		uint64_t _signed;
		uint64_t scale = ((uint64_t)(((uint64_t)bit_c_jit(ctx,opc,1ULL) << (uint64_t)2ULL)) | (uint64_t)size);
		uint64_t offset = ((uint64_t)imm12 << (uint64_t)scale);
		if ((((uint64_t)opc & (uint64_t)1ULL)))
		memop = 0ULL;
		else
		memop = 1ULL;
		uint64_t datasize = ((uint64_t)8ULL << (uint64_t)scale);
		ir_operand address = XSP_jit(ctx,Rn);
		if ((!postindex))
		{
			address = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offset, int64));
		}
		uint64_t O = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : datasize == 128ULL ? int128 : throw 0;
		{
			if ((((uint64_t)memop == (uint64_t)1ULL)))
			{
				ir_operand data = copy_new_raw_size(ctx, V_jit(ctx,Rt), O);
				mem_jit(ctx,O,address,data);
			}
			else
			{
				ir_operand data = copy_new_raw_size(ctx, mem_jit(ctx,O,address), O);
				V_jit(ctx,Rt,copy_new_raw_size(ctx, data, int128));
			}
			if ((wback))
			{
				if ((postindex))
				{
					address = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offset, int64));
				}
				XSP_jit(ctx,Rn,address);
			}
		}
	}
	else
	{
		uint64_t memop;
		uint64_t regsize;
		uint64_t _signed;
		uint64_t scale = size;
		uint64_t offset = ((uint64_t)imm12 << (uint64_t)scale);
		if ((((uint64_t)bit_c_jit(ctx,opc,1ULL) == (uint64_t)0ULL)))
		{
			if ((((uint64_t)opc & (uint64_t)1ULL)))
			memop = 0ULL;
			else
			memop = 1ULL;
			if ((((uint64_t)size == (uint64_t)3ULL)))
			regsize = 64ULL;
			else
			regsize = 32ULL;
			_signed = 0ULL;
		}
		else
		{
			if ((((uint64_t)size == (uint64_t)3ULL)))
			{
				undefined_jit(ctx);
			}
			else
			{
				memop = 0ULL;
				if ((((uint64_t)opc & (uint64_t)1ULL)))
				regsize = 32ULL;
				else
				regsize = 64ULL;
				_signed = 1ULL;
			}
		}
		uint64_t datasize = ((uint64_t)8ULL << (uint64_t)scale);
		ir_operand address = XSP_jit(ctx,Rn);
		if ((!postindex))
		{
			address = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offset, int64));
		}
		uint64_t S = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : throw 0;
		{
			uint64_t R = regsize == 32ULL ? int32 : regsize == 64ULL ? int64 : throw 0;
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					ir_operand data = X_jit(ctx,Rt);
					mem_jit(ctx,S,address,copy_new_raw_size(ctx, data, S));
				}
				else
				{
					ir_operand n = copy_new_raw_size(ctx, mem_jit(ctx,S,address), int64);
					if ((_signed))
					{
						n = copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, n, S), R), int64);
					}
					X_jit(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offset, int64));
					}
					XSP_jit(ctx,Rn,address);
				}
			}
		}
	}
}

void load_store_register_imm_unscaled_jit(ssa_emit_context* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t wb, uint64_t Rn, uint64_t Rt)
{
	if ((((uint64_t)((uint64_t)((uint64_t)size == (uint64_t)3ULL) && (uint64_t)((uint64_t)VR == (uint64_t)0ULL)) && (uint64_t)((uint64_t)opc == (uint64_t)2ULL))))
	{
		return;
	}
	uint64_t is_vector = ((uint64_t)VR == (uint64_t)1ULL);
	uint64_t wback = ((uint64_t)wb != (uint64_t)0ULL);
	uint64_t postindex = ((uint64_t)wb == (uint64_t)1ULL);
	uint64_t offset = sign_extend_jit(ctx,imm9,9ULL);
	if ((is_vector))
	{
		uint64_t memop;
		uint64_t regsize;
		uint64_t _signed;
		uint64_t scale = ((uint64_t)(((uint64_t)bit_c_jit(ctx,opc,1ULL) << (uint64_t)2ULL)) | (uint64_t)size);
		if ((((uint64_t)opc & (uint64_t)1ULL)))
		memop = 0ULL;
		else
		memop = 1ULL;
		uint64_t datasize = ((uint64_t)8ULL << (uint64_t)scale);
		ir_operand address = XSP_jit(ctx,Rn);
		if ((!postindex))
		{
			address = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offset, int64));
		}
		uint64_t O = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : datasize == 128ULL ? int128 : throw 0;
		{
			if ((((uint64_t)memop == (uint64_t)1ULL)))
			{
				ir_operand data = copy_new_raw_size(ctx, V_jit(ctx,Rt), O);
				mem_jit(ctx,O,address,data);
			}
			else
			{
				ir_operand data = copy_new_raw_size(ctx, mem_jit(ctx,O,address), O);
				V_jit(ctx,Rt,copy_new_raw_size(ctx, data, int128));
			}
			if ((wback))
			{
				if ((postindex))
				{
					address = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offset, int64));
				}
				XSP_jit(ctx,Rn,address);
			}
		}
	}
	else
	{
		uint64_t memop;
		uint64_t regsize;
		uint64_t _signed;
		uint64_t scale = size;
		if ((((uint64_t)bit_c_jit(ctx,opc,1ULL) == (uint64_t)0ULL)))
		{
			if ((((uint64_t)opc & (uint64_t)1ULL)))
			memop = 0ULL;
			else
			memop = 1ULL;
			if ((((uint64_t)size == (uint64_t)3ULL)))
			regsize = 64ULL;
			else
			regsize = 32ULL;
			_signed = 0ULL;
		}
		else
		{
			if ((((uint64_t)size == (uint64_t)3ULL)))
			{
				undefined_jit(ctx);
			}
			else
			{
				memop = 0ULL;
				if ((((uint64_t)opc & (uint64_t)1ULL)))
				regsize = 32ULL;
				else
				regsize = 64ULL;
				_signed = 1ULL;
			}
		}
		uint64_t datasize = ((uint64_t)8ULL << (uint64_t)scale);
		ir_operand address = XSP_jit(ctx,Rn);
		if ((!postindex))
		{
			address = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offset, int64));
		}
		uint64_t S = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : throw 0;
		{
			uint64_t R = regsize == 32ULL ? int32 : regsize == 64ULL ? int64 : throw 0;
			{
				if ((((uint64_t)memop == (uint64_t)1ULL)))
				{
					ir_operand data = X_jit(ctx,Rt);
					mem_jit(ctx,S,address,copy_new_raw_size(ctx, data, S));
				}
				else
				{
					ir_operand n = copy_new_raw_size(ctx, mem_jit(ctx,S,address), int64);
					if ((_signed))
					{
						n = copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, n, S), R), int64);
					}
					X_jit(ctx,Rt,n);
				}
				if ((wback))
				{
					if ((postindex))
					{
						address = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offset, int64));
					}
					XSP_jit(ctx,Rn,address);
				}
			}
		}
	}
}

void br_jit(ssa_emit_context* ctx, uint64_t Rn)
{
	branch_long_universal_jit(ctx,Rn,0ULL);
}

uint64_t sign_extend_jit(ssa_emit_context* ctx, uint64_t source, uint64_t count)
{
	uint64_t max = 64ULL;
	uint64_t shift = ((uint64_t)max - (uint64_t)count);
	return ((uint64_t)(sign_extend((uint64_t)(((uint64_t)source << (uint64_t)shift))) >> sign_extend((uint64_t)shift)));
}

ir_operand a_shift_reg_jit(ssa_emit_context* ctx,uint64_t O, uint64_t m, uint64_t shift_type, uint64_t ammount)
{
	ir_operand result = copy_new_raw_size(ctx, X_jit(ctx,m), O);
	if ((((uint64_t)shift_type == (uint64_t)0ULL)))
	{
		return ssa_emit_context::emit_ssa(ctx, ir_shift_left, result, ir_operand::create_con(ammount, O));
	}
	else if ((((uint64_t)shift_type == (uint64_t)1ULL)))
	{
		return ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, result, ir_operand::create_con(ammount, O));
	}
	else if ((((uint64_t)shift_type == (uint64_t)2ULL)))
	{
		return ssa_emit_context::emit_ssa(ctx, ir_shift_right_signed, result, ir_operand::create_con(ammount, O));
	}
	else
	{
		return ssa_emit_context::emit_ssa(ctx, ir_rotate_right, result, ir_operand::create_con(ammount, O));
	}
}

ir_operand a_extend_reg_jit(ssa_emit_context* ctx,uint64_t O, uint64_t m, uint64_t extend_type, uint64_t shift)
{
	ir_operand val = copy_new_raw_size(ctx, X_jit(ctx,m), O);
	if ((((uint64_t)extend_type == (uint64_t)0ULL)))
	{
		val = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, val, ir_operand::create_con(255ULL, O));
	}
	else if ((((uint64_t)extend_type == (uint64_t)1ULL)))
	{
		val = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, val, ir_operand::create_con(65535ULL, O));
	}
	else if ((((uint64_t)extend_type == (uint64_t)2ULL)))
	{
		val = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, val, ir_operand::create_con(4294967295ULL, O));
	}
	else if ((((uint64_t)extend_type == (uint64_t)4ULL)))
	{
		val = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, val, int8), O);
	}
	else if ((((uint64_t)extend_type == (uint64_t)5ULL)))
	{
		val = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, val, int16), O);
	}
	else if ((((uint64_t)extend_type == (uint64_t)6ULL)))
	{
		val = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, val, int32), O);
	}
	return ssa_emit_context::emit_ssa(ctx, ir_shift_left, val, ir_operand::create_con(shift, O));
}

ir_operand reverse_bytes_jit(ssa_emit_context* ctx,uint64_t O, ir_operand source, uint64_t byte_count)
{
	ir_operand result = ir_operand::create_con(0ULL, O);
	for (uint64_t i = 0; i < (byte_count); i++)
	{
		ir_operand working = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, source, ir_operand::create_con((((uint64_t)i * (uint64_t)8ULL)), O)), ir_operand::create_con(255ULL, O));
		result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, result, ssa_emit_context::emit_ssa(ctx, ir_shift_left, working, ir_operand::create_con((((uint64_t)(((uint64_t)((uint64_t)byte_count - (uint64_t)i) - (uint64_t)1ULL)) * (uint64_t)8ULL)), O)));
	}
	return result;
}

uint64_t highest_bit_set_c_jit(ssa_emit_context* ctx, uint64_t src, uint64_t size)
{
	for (uint64_t i = 0; i < (size); i++)
	{
		uint64_t bit_check = ((uint64_t)((uint64_t)size - (uint64_t)i) - (uint64_t)1ULL);
		uint64_t bit = ((uint64_t)(((uint64_t)src >> (uint64_t)bit_check)) & (uint64_t)1ULL);
		if ((bit))
		{
			return bit_check;
		}
	}
	return -1ULL;
}

uint64_t ones_jit(ssa_emit_context* ctx, uint64_t size)
{
	if ((((uint64_t)size == (uint64_t)64ULL)))
	{
		return 18446744073709551615ULL;
	}
	return ((uint64_t)(((uint64_t)1ULL << (uint64_t)size)) - (uint64_t)1ULL);
}

uint64_t replicate_c_jit(ssa_emit_context* ctx, uint64_t source, uint64_t source_size, uint64_t count)
{
	uint64_t result = 0ULL;
	for (uint64_t i = 0; i < (count); i++)
	{
		result = ((uint64_t)result | (uint64_t)(((uint64_t)(((uint64_t)source & (uint64_t)ones_jit(ctx,source_size))) << (uint64_t)(((uint64_t)i * (uint64_t)source_size)))));
	}
	return result;
}

uint64_t bits_c_jit(ssa_emit_context* ctx, uint64_t source, uint64_t top, uint64_t bottom)
{
	top = ((uint64_t)top + (uint64_t)1ULL);
	uint64_t size = ((uint64_t)top - (uint64_t)bottom);
	uint64_t mask = ones_jit(ctx,size);
	return ((uint64_t)(((uint64_t)source >> (uint64_t)bottom)) & (uint64_t)mask);
}

uint64_t bit_c_jit(ssa_emit_context* ctx, uint64_t source, uint64_t bit)
{
	return ((uint64_t)(((uint64_t)source >> (uint64_t)bit)) & (uint64_t)1ULL);
}

uint64_t rotate_right_bits_jit(ssa_emit_context* ctx, uint64_t source, uint64_t ammount, uint64_t bit_count)
{
	source = ((uint64_t)source & (uint64_t)ones_jit(ctx,bit_count));
	return (((uint64_t)(((uint64_t)source >> (uint64_t)ammount)) | (uint64_t)(((uint64_t)source << (uint64_t)(((uint64_t)bit_count - (uint64_t)ammount))))));
}

uint64_t decode_bitmask_tmask_jit(ssa_emit_context* ctx, uint64_t immN, uint64_t imms, uint64_t immr, uint64_t immediate, uint64_t M, uint64_t return_tmask)
{
	uint64_t levels;
	uint64_t len = highest_bit_set_c_jit(ctx,((uint64_t)(((uint64_t)immN << (uint64_t)6ULL)) | (uint64_t)(((uint64_t)~imms & (uint64_t)ones_jit(ctx,6ULL)))),7ULL);
	if ((((uint64_t)len < (uint64_t)1ULL)))
	{
		undefined_jit(ctx);
	}
	levels = ones_jit(ctx,len);
	if ((((uint64_t)immediate && (uint64_t)((uint64_t)(((uint64_t)imms & (uint64_t)levels)) == (uint64_t)levels))))
	{
		undefined_jit(ctx);
	}
	uint64_t s = ((uint64_t)imms & (uint64_t)levels);
	uint64_t r = ((uint64_t)immr & (uint64_t)levels);
	uint64_t diff = ((uint64_t)s - (uint64_t)r);
	uint64_t esize = ((uint64_t)1ULL << (uint64_t)len);
	uint64_t d = bits_c_jit(ctx,diff,((uint64_t)len - (uint64_t)1ULL),0ULL);
	uint64_t welem = ones_jit(ctx,((uint64_t)s + (uint64_t)1ULL));
	uint64_t telem = ones_jit(ctx,((uint64_t)d + (uint64_t)1ULL));
	if ((return_tmask))
	{
		return replicate_c_jit(ctx,telem,esize,((uint64_t)M / (uint64_t)esize));
	}
	else
	{
		return replicate_c_jit(ctx,rotate_right_bits_jit(ctx,welem,r,esize),esize,((uint64_t)M / (uint64_t)esize));
	}
}

uint64_t decode_add_subtract_imm_12_jit(ssa_emit_context* ctx, uint64_t source, uint64_t shift)
{
	return ((uint64_t)source << (uint64_t)(((uint64_t)shift * (uint64_t)12ULL)));
}

ir_operand add_subtract_impl_jit(ssa_emit_context* ctx,uint64_t O, ir_operand n, ir_operand m, uint64_t set_flags, uint64_t is_add)
{
	ir_operand d;
	if ((is_add))
	{
		d = ssa_emit_context::emit_ssa(ctx, ir_add, n, m);
	}
	else
	{
		d = ssa_emit_context::emit_ssa(ctx, ir_subtract, n, m);
	}
	if ((set_flags))
	{
		_sys_jit(ctx,0ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, d, ir_operand::create_con(0ULL, O)), int64));
		_sys_jit(ctx,1ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, d, ir_operand::create_con(0ULL, O)), int64));
		if ((is_add))
		{
			_sys_jit(ctx,2ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_less_unsigned, d, n), int64));
			_sys_jit(ctx,3ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, d, n), ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, n, m))), ir_operand::create_con(0ULL, O)), int64));
		}
		else
		{
			_sys_jit(ctx,2ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_greater_equal_unsigned, n, m), int64));
			_sys_jit(ctx,3ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, d, n), ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, n, m)), ir_operand::create_con(0ULL, O)), int64));
		}
	}
	return d;
}

ir_operand add_subtract_carry_impl_jit(ssa_emit_context* ctx,uint64_t O, ir_operand n, ir_operand m, uint64_t set_flags, uint64_t is_add, ir_operand carry)
{
	ir_operand d;
	if ((is_add))
	{
		d = ssa_emit_context::emit_ssa(ctx, ir_add, ssa_emit_context::emit_ssa(ctx, ir_add, n, m), carry);
	}
	else
	{
		d = ssa_emit_context::emit_ssa(ctx, ir_subtract, ssa_emit_context::emit_ssa(ctx, ir_subtract, n, m), ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, carry, ir_operand::create_con(1ULL, O)));
	}
	if ((set_flags))
	{
		_sys_jit(ctx,0ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, d, ir_operand::create_con(0ULL, O)), int64));
		_sys_jit(ctx,1ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, d, ir_operand::create_con(0ULL, O)), int64));
		if ((is_add))
		{
			_sys_jit(ctx,2ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, d, n), carry), ssa_emit_context::emit_ssa(ctx, ir_compare_less_unsigned, d, n)), int64));
			_sys_jit(ctx,3ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, d, n), ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, n, m))), ir_operand::create_con(0ULL, O)), int64));
		}
		else
		{
			_sys_jit(ctx,2ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, n, m), carry), ssa_emit_context::emit_ssa(ctx, ir_compare_greater_unsigned, n, m)), int64));
			_sys_jit(ctx,3ULL,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, d, n), ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, n, m)), ir_operand::create_con(0ULL, O)), int64));
		}
	}
	return d;
}

ir_operand condition_holds_jit(ssa_emit_context* ctx, uint64_t cond)
{
	ir_operand n = copy_new_raw_size(ctx, _sys_jit(ctx,0ULL), int8);
	ir_operand z = copy_new_raw_size(ctx, _sys_jit(ctx,1ULL), int8);
	ir_operand c = copy_new_raw_size(ctx, _sys_jit(ctx,2ULL), int8);
	ir_operand v = copy_new_raw_size(ctx, _sys_jit(ctx,3ULL), int8);
	uint64_t raw_condition = ((uint64_t)cond >> (uint64_t)1ULL);
	ir_operand result;
	if ((((uint64_t)raw_condition == (uint64_t)0ULL)))
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_compare_equal, z, ir_operand::create_con(1ULL, int8));
	}
	else if ((((uint64_t)raw_condition == (uint64_t)1ULL)))
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_compare_equal, c, ir_operand::create_con(1ULL, int8));
	}
	else if ((((uint64_t)raw_condition == (uint64_t)2ULL)))
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_compare_equal, n, ir_operand::create_con(1ULL, int8));
	}
	else if ((((uint64_t)raw_condition == (uint64_t)3ULL)))
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_compare_equal, v, ir_operand::create_con(1ULL, int8));
	}
	else if ((((uint64_t)raw_condition == (uint64_t)4ULL)))
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, c, ir_operand::create_con(1ULL, int8)), ssa_emit_context::emit_ssa(ctx, ir_compare_equal, z, ir_operand::create_con(0ULL, int8)));
	}
	else if ((((uint64_t)raw_condition == (uint64_t)5ULL)))
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_compare_equal, n, v);
	}
	else if ((((uint64_t)raw_condition == (uint64_t)6ULL)))
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, n, v), ssa_emit_context::emit_ssa(ctx, ir_compare_equal, z, ir_operand::create_con(0ULL, int8)));
	}
	else
	{
		result = ir_operand::create_con(1ULL, int8);
	}
	if ((((uint64_t)(((uint64_t)cond & (uint64_t)1ULL)) && (uint64_t)((uint64_t)cond != (uint64_t)15ULL))))
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, result, ir_operand::create_con(1ULL, int8));
	}
	return result;
}

void branch_long_universal_jit(ssa_emit_context* ctx, uint64_t Rn, uint64_t link)
{
	ir_operand branch_location = X_jit(ctx,Rn);
	if ((link))
	{
		ir_operand link_address = ir_operand::create_con(((uint64_t)_get_pc_jit(ctx) + (uint64_t)4ULL), int64);
		X_jit(ctx,30ULL,link_address);
	}
	_branch_long_jit(ctx,branch_location);
}

void mem_jit(ssa_emit_context* ctx,uint64_t O, ir_operand address, ir_operand value)
{
	address = translate_address_jit(ctx,address);
	ssa_emit_context::store(ctx, address, value);
}

ir_operand mem_jit(ssa_emit_context* ctx,uint64_t O, ir_operand address)
{
	address = translate_address_jit(ctx,address);
	return ssa_emit_context::emit_ssa(ctx, ir_load, address, O);
}

ir_operand XSP_jit(ssa_emit_context* ctx, uint64_t reg_id)
{
	return _x_jit(ctx,reg_id);
}

void XSP_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value)
{
	_x_jit(ctx,reg_id,value);
}

ir_operand X_jit(ssa_emit_context* ctx, uint64_t reg_id)
{
	if ((((uint64_t)reg_id == (uint64_t)31ULL)))
	{
		return ir_operand::create_con(0ULL, int64);
	}
	else
	{
		return _x_jit(ctx,reg_id);
	}
}

void X_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value)
{
	if ((((uint64_t)reg_id == (uint64_t)31ULL)))
	{
		return;
	}
	else
	{
		_x_jit(ctx,reg_id,value);
	}
}

