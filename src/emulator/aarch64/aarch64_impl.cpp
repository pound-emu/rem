#include "aarch64_impl.h"
#include "string.h"
#include "tools/big_number.h"

static void append_table(guest_process* process, std::string encoding, void* emit, void* interperate, std::string name)
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

		ir_operation_block::emitds(ctx->ir, ir_vector_insert, result, result, source, ir_operand::create_con(0), ir_operand::create_con(8 << ir_operand::get_raw_size(&source)));

		return result;
	}
	else
	{
		ir_operand result = ssa_emit_context::create_local(ctx, new_size);
		
		ir_operation_block::emitds(ctx->ir, ir_vector_extract, result, source, ir_operand::create_con(0), ir_operand::create_con(8 << new_size));

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

static void call_load_store_register_offset_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int size = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int opc = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int option = (instruction >> 13) & 7;
	int S = (instruction >> 12) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_offset_interpreter(ctx, size, VR, opc, Rm, option, S, Rn, Rt);
}

static void emit_load_store_register_offset_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int size = (instruction >> 30) & 3;
	int VR = (instruction >> 26) & 1;
	int opc = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int option = (instruction >> 13) & 7;
	int S = (instruction >> 12) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_register_offset_jit(ctx, size, VR, opc, Rm, option, S, Rn, Rt);
}

static void call_load_store_exclusive_ordered_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int size = (instruction >> 30) & 3;
	int ordered = (instruction >> 23) & 1;
	int L = (instruction >> 22) & 1;
	int Rs = (instruction >> 16) & 31;
	int o0 = (instruction >> 15) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_exclusive_ordered_interpreter(ctx, size, ordered, L, Rs, o0, Rn, Rt);
}

static void emit_load_store_exclusive_ordered_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int size = (instruction >> 30) & 3;
	int ordered = (instruction >> 23) & 1;
	int L = (instruction >> 22) & 1;
	int Rs = (instruction >> 16) & 31;
	int o0 = (instruction >> 15) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	load_store_exclusive_ordered_jit(ctx, size, ordered, L, Rs, o0, Rn, Rt);
}

static void call_branch_register_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int l = (instruction >> 21) & 1;
	int Rn = (instruction >> 5) & 31;
	branch_register_interpreter(ctx, l, Rn);
}

static void emit_branch_register_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int l = (instruction >> 21) & 1;
	int Rn = (instruction >> 5) & 31;
	branch_register_jit(ctx, l, Rn);
}

static void call_return_register_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Rn = (instruction >> 5) & 31;
	return_register_interpreter(ctx, Rn);
}

static void emit_return_register_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Rn = (instruction >> 5) & 31;
	return_register_jit(ctx, Rn);
}

static void call_test_bit_branch_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int b5 = (instruction >> 31) & 1;
	int op = (instruction >> 24) & 1;
	int b40 = (instruction >> 19) & 31;
	int imm14 = (instruction >> 5) & 16383;
	int Rt = (instruction >> 0) & 31;
	test_bit_branch_interpreter(ctx, b5, op, b40, imm14, Rt);
}

static void emit_test_bit_branch_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int b5 = (instruction >> 31) & 1;
	int op = (instruction >> 24) & 1;
	int b40 = (instruction >> 19) & 31;
	int imm14 = (instruction >> 5) & 16383;
	int Rt = (instruction >> 0) & 31;
	test_bit_branch_jit(ctx, b5, op, b40, imm14, Rt);
}

static void call_compare_and_branch_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 24) & 1;
	int imm19 = (instruction >> 5) & 524287;
	int Rt = (instruction >> 0) & 31;
	compare_and_branch_interpreter(ctx, sf, op, imm19, Rt);
}

static void emit_compare_and_branch_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int op = (instruction >> 24) & 1;
	int imm19 = (instruction >> 5) & 524287;
	int Rt = (instruction >> 0) & 31;
	compare_and_branch_jit(ctx, sf, op, imm19, Rt);
}

static void call_b_unconditional_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int op = (instruction >> 31) & 1;
	int imm26 = (instruction >> 0) & 67108863;
	b_unconditional_interpreter(ctx, op, imm26);
}

static void emit_b_unconditional_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int op = (instruction >> 31) & 1;
	int imm26 = (instruction >> 0) & 67108863;
	b_unconditional_jit(ctx, op, imm26);
}

static void call_b_conditional_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int imm19 = (instruction >> 5) & 524287;
	int cond = (instruction >> 0) & 15;
	b_conditional_interpreter(ctx, imm19, cond);
}

static void emit_b_conditional_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int imm19 = (instruction >> 5) & 524287;
	int cond = (instruction >> 0) & 15;
	b_conditional_jit(ctx, imm19, cond);
}

static void call_svc_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int imm16 = (instruction >> 5) & 65535;
	svc_interpreter(ctx, imm16);
}

static void emit_svc_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int imm16 = (instruction >> 5) & 65535;
	svc_jit(ctx, imm16);
}

static void call_dup_general_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int imm5 = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	dup_general_interpreter(ctx, Q, imm5, Rn, Rd);
}

static void emit_dup_general_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int imm5 = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	dup_general_jit(ctx, Q, imm5, Rn, Rd);
}

static void call_dup_element_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int imm5 = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	dup_element_scalar_interpreter(ctx, imm5, Rn, Rd);
}

static void emit_dup_element_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int imm5 = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	dup_element_scalar_jit(ctx, imm5, Rn, Rd);
}

static void call_dup_element_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int imm5 = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	dup_element_vector_interpreter(ctx, Q, imm5, Rn, Rd);
}

static void emit_dup_element_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int imm5 = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	dup_element_vector_jit(ctx, Q, imm5, Rn, Rd);
}

static void call_move_to_gp_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int imm5 = (instruction >> 16) & 31;
	int U = (instruction >> 12) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	move_to_gp_interpreter(ctx, Q, imm5, U, Rn, Rd);
}

static void emit_move_to_gp_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int imm5 = (instruction >> 16) & 31;
	int U = (instruction >> 12) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	move_to_gp_jit(ctx, Q, imm5, U, Rn, Rd);
}

static void call_ins_general_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int imm5 = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	ins_general_interpreter(ctx, imm5, Rn, Rd);
}

static void emit_ins_general_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int imm5 = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	ins_general_jit(ctx, imm5, Rn, Rd);
}

static void call_ins_element_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int imm5 = (instruction >> 16) & 31;
	int imm4 = (instruction >> 11) & 15;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	ins_element_interpreter(ctx, imm5, imm4, Rn, Rd);
}

static void emit_ins_element_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int imm5 = (instruction >> 16) & 31;
	int imm4 = (instruction >> 11) & 15;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	ins_element_jit(ctx, imm5, imm4, Rn, Rd);
}

static void call_movi_immediate_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int op = (instruction >> 29) & 1;
	int immhi = (instruction >> 16) & 7;
	int cmode = (instruction >> 12) & 15;
	int immlo = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	movi_immediate_interpreter(ctx, Q, op, immhi, cmode, immlo, Rd);
}

static void emit_movi_immediate_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int op = (instruction >> 29) & 1;
	int immhi = (instruction >> 16) & 7;
	int cmode = (instruction >> 12) & 15;
	int immlo = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	movi_immediate_jit(ctx, Q, op, immhi, cmode, immlo, Rd);
}

static void call_fmov_general_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int rmode = (instruction >> 19) & 1;
	int opcode = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmov_general_interpreter(ctx, sf, ftype, rmode, opcode, Rn, Rd);
}

static void emit_fmov_general_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int rmode = (instruction >> 19) & 1;
	int opcode = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmov_general_jit(ctx, sf, ftype, rmode, opcode, Rn, Rd);
}

static void call_convert_to_float_gp_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int U = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	convert_to_float_gp_interpreter(ctx, sf, ftype, U, Rn, Rd);
}

static void emit_convert_to_float_gp_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int U = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	convert_to_float_gp_jit(ctx, sf, ftype, U, Rn, Rd);
}

static void call_convert_to_float_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int U = (instruction >> 29) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	convert_to_float_vector_interpreter(ctx, U, sz, Rn, Rd);
}

static void emit_convert_to_float_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int U = (instruction >> 29) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	convert_to_float_vector_jit(ctx, U, sz, Rn, Rd);
}

static void call_floating_point_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int opcode = (instruction >> 12) & 15;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	floating_point_scalar_interpreter(ctx, ftype, Rm, opcode, Rn, Rd);
}

static void emit_floating_point_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int opcode = (instruction >> 12) & 15;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	floating_point_scalar_jit(ctx, ftype, Rm, opcode, Rn, Rd);
}

static void call_conversion_between_floating_point_and_fixed_point_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int S = (instruction >> 29) & 1;
	int ftype = (instruction >> 22) & 3;
	int rmode = (instruction >> 19) & 3;
	int opcode = (instruction >> 16) & 7;
	int scale = (instruction >> 10) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	conversion_between_floating_point_and_fixed_point_interpreter(ctx, sf, S, ftype, rmode, opcode, scale, Rn, Rd);
}

static void emit_conversion_between_floating_point_and_fixed_point_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int S = (instruction >> 29) & 1;
	int ftype = (instruction >> 22) & 3;
	int rmode = (instruction >> 19) & 3;
	int opcode = (instruction >> 16) & 7;
	int scale = (instruction >> 10) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	conversion_between_floating_point_and_fixed_point_jit(ctx, sf, S, ftype, rmode, opcode, scale, Rn, Rd);
}

static void call_advanced_simd_three_same_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int U = (instruction >> 29) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int opcode = (instruction >> 11) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	advanced_simd_three_same_interpreter(ctx, Q, U, size, Rm, opcode, Rn, Rd);
}

static void emit_advanced_simd_three_same_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int U = (instruction >> 29) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int opcode = (instruction >> 11) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	advanced_simd_three_same_jit(ctx, Q, U, size, Rm, opcode, Rn, Rd);
}

static void call_floating_point_conditional_select_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int cond = (instruction >> 12) & 15;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	floating_point_conditional_select_interpreter(ctx, ftype, Rm, cond, Rn, Rd);
}

static void emit_floating_point_conditional_select_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int cond = (instruction >> 12) & 15;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	floating_point_conditional_select_jit(ctx, ftype, Rm, cond, Rn, Rd);
}

static void call_fmov_scalar_immediate_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int imm8 = (instruction >> 13) & 255;
	int Rd = (instruction >> 0) & 31;
	fmov_scalar_immediate_interpreter(ctx, ftype, imm8, Rd);
}

static void emit_fmov_scalar_immediate_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int imm8 = (instruction >> 13) & 255;
	int Rd = (instruction >> 0) & 31;
	fmov_scalar_immediate_jit(ctx, ftype, imm8, Rd);
}

static void call_fcvt_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int opc = (instruction >> 15) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcvt_interpreter(ctx, ftype, opc, Rn, Rd);
}

static void emit_fcvt_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int opc = (instruction >> 15) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcvt_jit(ctx, ftype, opc, Rn, Rd);
}

static void call_floating_point_data_processing_one_source_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int M = (instruction >> 31) & 1;
	int S = (instruction >> 29) & 1;
	int ftype = (instruction >> 22) & 3;
	int opcode = (instruction >> 15) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	floating_point_data_processing_one_source_interpreter(ctx, M, S, ftype, opcode, Rn, Rd);
}

static void emit_floating_point_data_processing_one_source_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int M = (instruction >> 31) & 1;
	int S = (instruction >> 29) & 1;
	int ftype = (instruction >> 22) & 3;
	int opcode = (instruction >> 15) & 63;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	floating_point_data_processing_one_source_jit(ctx, M, S, ftype, opcode, Rn, Rd);
}

static void call_fcmp_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int opc = (instruction >> 3) & 1;
	fcmp_interpreter(ctx, ftype, Rm, Rn, opc);
}

static void emit_fcmp_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int opc = (instruction >> 3) & 1;
	fcmp_jit(ctx, ftype, Rm, Rn, opc);
}

static void call_fccmp_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int cond = (instruction >> 12) & 15;
	int Rn = (instruction >> 5) & 31;
	int nzcv = (instruction >> 0) & 15;
	fccmp_interpreter(ctx, ftype, Rm, cond, Rn, nzcv);
}

static void emit_fccmp_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int cond = (instruction >> 12) & 15;
	int Rn = (instruction >> 5) & 31;
	int nzcv = (instruction >> 0) & 15;
	fccmp_jit(ctx, ftype, Rm, cond, Rn, nzcv);
}

static void call_fcvtz_scalar_integer_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int U = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcvtz_scalar_integer_interpreter(ctx, sf, ftype, U, Rn, Rd);
}

static void emit_fcvtz_scalar_integer_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int U = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcvtz_scalar_integer_jit(ctx, sf, ftype, U, Rn, Rd);
}

static void call_fcvtn_scalar_integer_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int U = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcvtn_scalar_integer_interpreter(ctx, sf, ftype, U, Rn, Rd);
}

static void emit_fcvtn_scalar_integer_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int U = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcvtn_scalar_integer_jit(ctx, sf, ftype, U, Rn, Rd);
}

static void call_fcvta_scalar_integer_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int U = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcvta_scalar_integer_interpreter(ctx, sf, ftype, U, Rn, Rd);
}

static void emit_fcvta_scalar_integer_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int U = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcvta_scalar_integer_jit(ctx, sf, ftype, U, Rn, Rd);
}

static void call_fcvtm_scalar_integer_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int U = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcvtm_scalar_integer_interpreter(ctx, sf, ftype, U, Rn, Rd);
}

static void emit_fcvtm_scalar_integer_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int U = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcvtm_scalar_integer_jit(ctx, sf, ftype, U, Rn, Rd);
}

static void call_fcvtp_scalar_integer_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int U = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcvtp_scalar_integer_interpreter(ctx, sf, ftype, U, Rn, Rd);
}

static void emit_fcvtp_scalar_integer_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sf = (instruction >> 31) & 1;
	int ftype = (instruction >> 22) & 3;
	int U = (instruction >> 16) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcvtp_scalar_integer_jit(ctx, sf, ftype, U, Rn, Rd);
}

static void call_addlv_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int U = (instruction >> 29) & 1;
	int size = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	addlv_interpreter(ctx, Q, U, size, Rn, Rd);
}

static void emit_addlv_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int U = (instruction >> 29) & 1;
	int size = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	addlv_jit(ctx, Q, U, size, Rn, Rd);
}

static void call_cnt_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	cnt_interpreter(ctx, Q, size, Rn, Rd);
}

static void emit_cnt_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	cnt_jit(ctx, Q, size, Rn, Rd);
}

static void call_msr_register_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int imm15 = (instruction >> 5) & 32767;
	int Rt = (instruction >> 0) & 31;
	msr_register_interpreter(ctx, imm15, Rt);
}

static void emit_msr_register_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int imm15 = (instruction >> 5) & 32767;
	int Rt = (instruction >> 0) & 31;
	msr_register_jit(ctx, imm15, Rt);
}

static void call_mrs_register_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int imm15 = (instruction >> 5) & 32767;
	int Rt = (instruction >> 0) & 31;
	mrs_register_interpreter(ctx, imm15, Rt);
}

static void emit_mrs_register_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int imm15 = (instruction >> 5) & 32767;
	int Rt = (instruction >> 0) & 31;
	mrs_register_jit(ctx, imm15, Rt);
}

static void call_hints_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int imm7 = (instruction >> 5) & 127;
	hints_interpreter(ctx, imm7);
}

static void emit_hints_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int imm7 = (instruction >> 5) & 127;
	hints_jit(ctx, imm7);
}

static void call_sys_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int L = (instruction >> 21) & 1;
	int imm19 = (instruction >> 0) & 524287;
	sys_interpreter(ctx, L, imm19);
}

static void emit_sys_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int L = (instruction >> 21) & 1;
	int imm19 = (instruction >> 0) & 524287;
	sys_jit(ctx, L, imm19);
}

static void call_barriers_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int CRm = (instruction >> 8) & 15;
	int op2 = (instruction >> 5) & 7;
	int Rt = (instruction >> 0) & 31;
	barriers_interpreter(ctx, CRm, op2, Rt);
}

static void emit_barriers_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int CRm = (instruction >> 8) & 15;
	int op2 = (instruction >> 5) & 7;
	int Rt = (instruction >> 0) & 31;
	barriers_jit(ctx, CRm, op2, Rt);
}

void init_aarch64_decoder(guest_process* process)
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
	append_table(process, "--111-00--1---------10----------", (void*)emit_load_store_register_offset_jit, (void*)call_load_store_register_offset_interpreter, "load_store_register_offset");
	append_table(process, "--001000--0------11111----------", (void*)emit_load_store_exclusive_ordered_jit, (void*)call_load_store_exclusive_ordered_interpreter, "load_store_exclusive_ordered");
	append_table(process, "1101011000-11111000000-----00000", (void*)emit_branch_register_jit, (void*)call_branch_register_interpreter, "branch_register");
	append_table(process, "1101011001011111000000-----00000", (void*)emit_return_register_jit, (void*)call_return_register_interpreter, "return_register");
	append_table(process, "-011011-------------------------", (void*)emit_test_bit_branch_jit, (void*)call_test_bit_branch_interpreter, "test_bit_branch");
	append_table(process, "-011010-------------------------", (void*)emit_compare_and_branch_jit, (void*)call_compare_and_branch_interpreter, "compare_and_branch");
	append_table(process, "-00101--------------------------", (void*)emit_b_unconditional_jit, (void*)call_b_unconditional_interpreter, "b_unconditional");
	append_table(process, "01010100-------------------0----", (void*)emit_b_conditional_jit, (void*)call_b_conditional_interpreter, "b_conditional");
	append_table(process, "11010100000----------------00001", (void*)emit_svc_jit, (void*)call_svc_interpreter, "svc");
	append_table(process, "0-001110000-----000011----------", (void*)emit_dup_general_jit, (void*)call_dup_general_interpreter, "dup_general");
	append_table(process, "01011110000-----000001----------", (void*)emit_dup_element_scalar_jit, (void*)call_dup_element_scalar_interpreter, "dup_element_scalar");
	append_table(process, "0-001110000-----000001----------", (void*)emit_dup_element_vector_jit, (void*)call_dup_element_vector_interpreter, "dup_element_vector");
	append_table(process, "0-001110000-----001-11----------", (void*)emit_move_to_gp_jit, (void*)call_move_to_gp_interpreter, "move_to_gp");
	append_table(process, "01001110000-----000111----------", (void*)emit_ins_general_jit, (void*)call_ins_general_interpreter, "ins_general");
	append_table(process, "01101110000-----0----1----------", (void*)emit_ins_element_jit, (void*)call_ins_element_interpreter, "ins_element");
	append_table(process, "0--0111100000-------01----------", (void*)emit_movi_immediate_jit, (void*)call_movi_immediate_interpreter, "movi_immediate");
	append_table(process, "-0011110--10-11-000000----------", (void*)emit_fmov_general_jit, (void*)call_fmov_general_interpreter, "fmov_general");
	append_table(process, "-0011110--10001-000000----------", (void*)emit_convert_to_float_gp_jit, (void*)call_convert_to_float_gp_interpreter, "convert_to_float_gp");
	append_table(process, "01-111100-100001110110----------", (void*)emit_convert_to_float_vector_jit, (void*)call_convert_to_float_vector_interpreter, "convert_to_float_vector");
	append_table(process, "00011110--1---------10----------", (void*)emit_floating_point_scalar_jit, (void*)call_floating_point_scalar_interpreter, "floating_point_scalar");
	append_table(process, "-0-11110--0---------------------", (void*)emit_conversion_between_floating_point_and_fixed_point_jit, (void*)call_conversion_between_floating_point_and_fixed_point_interpreter, "conversion_between_floating_point_and_fixed_point");
	append_table(process, "0--01110--1----------1----------", (void*)emit_advanced_simd_three_same_jit, (void*)call_advanced_simd_three_same_interpreter, "advanced_simd_three_same");
	append_table(process, "00011110--1---------11----------", (void*)emit_floating_point_conditional_select_jit, (void*)call_floating_point_conditional_select_interpreter, "floating_point_conditional_select");
	append_table(process, "00011110--1--------10000000-----", (void*)emit_fmov_scalar_immediate_jit, (void*)call_fmov_scalar_immediate_interpreter, "fmov_scalar_immediate");
	append_table(process, "00011110--10001--10000----------", (void*)emit_fcvt_jit, (void*)call_fcvt_interpreter, "fcvt");
	append_table(process, "-0-11110--1------10000----------", (void*)emit_floating_point_data_processing_one_source_jit, (void*)call_floating_point_data_processing_one_source_interpreter, "floating_point_data_processing_one_source");
	append_table(process, "00011110--1-----001000-----0-000", (void*)emit_fcmp_jit, (void*)call_fcmp_interpreter, "fcmp");
	append_table(process, "00011110--1---------01-----0----", (void*)emit_fccmp_jit, (void*)call_fccmp_interpreter, "fccmp");
	append_table(process, "-0011110--11100-000000----------", (void*)emit_fcvtz_scalar_integer_jit, (void*)call_fcvtz_scalar_integer_interpreter, "fcvtz_scalar_integer");
	append_table(process, "-0011110--10000-000000----------", (void*)emit_fcvtn_scalar_integer_jit, (void*)call_fcvtn_scalar_integer_interpreter, "fcvtn_scalar_integer");
	append_table(process, "-0011110--10010-000000----------", (void*)emit_fcvta_scalar_integer_jit, (void*)call_fcvta_scalar_integer_interpreter, "fcvta_scalar_integer");
	append_table(process, "-0011110--11000-000000----------", (void*)emit_fcvtm_scalar_integer_jit, (void*)call_fcvtm_scalar_integer_interpreter, "fcvtm_scalar_integer");
	append_table(process, "-0011110--10100-000000----------", (void*)emit_fcvtp_scalar_integer_jit, (void*)call_fcvtp_scalar_integer_interpreter, "fcvtp_scalar_integer");
	append_table(process, "0--01110--110000001110----------", (void*)emit_addlv_jit, (void*)call_addlv_interpreter, "addlv");
	append_table(process, "0-001110--100000010110----------", (void*)emit_cnt_jit, (void*)call_cnt_interpreter, "cnt");
	append_table(process, "110101010001--------------------", (void*)emit_msr_register_jit, (void*)call_msr_register_interpreter, "msr_register");
	append_table(process, "110101010011--------------------", (void*)emit_mrs_register_jit, (void*)call_mrs_register_interpreter, "mrs_register");
	append_table(process, "11010101000000110010-------11111", (void*)emit_hints_jit, (void*)call_hints_interpreter, "hints");
	append_table(process, "1101010100-01-------------------", (void*)emit_sys_jit, (void*)call_sys_interpreter, "sys");
	append_table(process, "11010101000000110011------------", (void*)emit_barriers_jit, (void*)call_barriers_interpreter, "barriers");
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
		uint32_t result = add_subtract_carry_impl_interpreter<uint32_t>(ctx,operand1,operand2,((uint64_t)S == (uint64_t)1ULL),((uint64_t)op == (uint64_t)0ULL),(uint32_t)_sys_interpreter(ctx,nzcv_c));
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = X_interpreter(ctx,Rm);
		uint64_t result = add_subtract_carry_impl_interpreter<uint64_t>(ctx,operand1,operand2,((uint64_t)S == (uint64_t)1ULL),((uint64_t)op == (uint64_t)0ULL),(uint64_t)_sys_interpreter(ctx,nzcv_c));
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
			_sys_interpreter(ctx,nzcv_n,(uint64_t)((uint32_t)(sign_extend((uint32_t)result) < sign_extend((uint32_t)0ULL))));
			_sys_interpreter(ctx,nzcv_z,(uint64_t)((uint32_t)result == (uint32_t)0ULL));
			_sys_interpreter(ctx,nzcv_c,(uint64_t)0ULL);
			_sys_interpreter(ctx,nzcv_v,(uint64_t)0ULL);
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
			_sys_interpreter(ctx,nzcv_n,(uint64_t)((uint64_t)(sign_extend((uint64_t)result) < sign_extend((uint64_t)0ULL))));
			_sys_interpreter(ctx,nzcv_z,(uint64_t)((uint64_t)result == (uint64_t)0ULL));
			_sys_interpreter(ctx,nzcv_c,(uint64_t)0ULL);
			_sys_interpreter(ctx,nzcv_v,(uint64_t)0ULL);
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
				_sys_interpreter(ctx,nzcv_n,(uint64_t)((uint32_t)(sign_extend((uint32_t)result) < sign_extend((uint32_t)0ULL))));
				_sys_interpreter(ctx,nzcv_z,(uint64_t)((uint32_t)result == (uint32_t)0ULL));
				_sys_interpreter(ctx,nzcv_c,(uint64_t)0ULL);
				_sys_interpreter(ctx,nzcv_v,(uint64_t)0ULL);
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
				_sys_interpreter(ctx,nzcv_n,(uint64_t)((uint64_t)(sign_extend((uint64_t)result) < sign_extend((uint64_t)0ULL))));
				_sys_interpreter(ctx,nzcv_z,(uint64_t)((uint64_t)result == (uint64_t)0ULL));
				_sys_interpreter(ctx,nzcv_c,(uint64_t)0ULL);
				_sys_interpreter(ctx,nzcv_v,(uint64_t)0ULL);
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
			_sys_interpreter(ctx,nzcv_n,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,nzcv_z,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,nzcv_c,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,nzcv_v,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL));
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
			_sys_interpreter(ctx,nzcv_n,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,nzcv_z,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,nzcv_c,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,nzcv_v,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL));
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
		instruction_pc = ((uint64_t)instruction_pc & (uint64_t)~4095ULL);
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
	{
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
		}
		
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
		{
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
			}
			
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
		{
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
				}
				
			}
			
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
		{
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
			}
			
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
		{
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
				}
				
			}
			
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

void load_store_register_offset_interpreter(interpreter_data* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t Rm, uint64_t option, uint64_t S, uint64_t Rn, uint64_t Rt)
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
		uint64_t shift = ((uint64_t)scale * (uint64_t)S);
		uint64_t offset = a_extend_reg_64_interpreter(ctx,Rm,option,shift);
		if ((((uint64_t)opc & (uint64_t)1ULL)))
		memop = 0ULL;
		else
		memop = 1ULL;
		uint64_t datasize = ((uint64_t)8ULL << (uint64_t)scale);
		uint64_t address = ((uint64_t)XSP_interpreter(ctx,Rn) + (uint64_t)offset);
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
		}
		
	}
	else
	{
		uint64_t memop;
		uint64_t regsize;
		uint64_t _signed;
		uint64_t scale = size;
		uint64_t shift = ((uint64_t)scale * (uint64_t)S);
		uint64_t offset = a_extend_reg_64_interpreter(ctx,Rm,option,shift);
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
		uint64_t address = ((uint64_t)XSP_interpreter(ctx,Rn) + (uint64_t)offset);
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
			}
			
		}
		
	}
}

void load_store_exclusive_ordered_interpreter(interpreter_data* ctx, uint64_t size, uint64_t ordered, uint64_t L, uint64_t Rs, uint64_t o0, uint64_t Rn, uint64_t Rt)
{
	uint64_t is_exclusive = ((uint64_t)ordered == (uint64_t)0ULL);
	if ((L))
	{
		load_exclusive_interpreter(ctx,is_exclusive,size,Rn,Rt);
	}
	else
	{
		store_exclusive_interpreter(ctx,is_exclusive,size,Rn,Rt,Rs);
	}
}

void load_exclusive_interpreter(interpreter_data* ctx, uint64_t is_exclusive, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	uint64_t datasize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t address = XSP_interpreter(ctx,Rn);
	if (datasize == 8ULL)
	{
		uint8_t value = mem_interpreter<uint8_t>(ctx,address);
		if ((is_exclusive))
		{
			_sys_interpreter(ctx,exclusive_address,((uint64_t)address & (uint64_t)~63ULL));
			_sys_interpreter(ctx,exclusive_value,(uint64_t)value);
		}
		X_interpreter(ctx,Rt,(uint64_t)value);
	}
	if (datasize == 16ULL)
	{
		uint16_t value = mem_interpreter<uint16_t>(ctx,address);
		if ((is_exclusive))
		{
			_sys_interpreter(ctx,exclusive_address,((uint64_t)address & (uint64_t)~63ULL));
			_sys_interpreter(ctx,exclusive_value,(uint64_t)value);
		}
		X_interpreter(ctx,Rt,(uint64_t)value);
	}
	if (datasize == 32ULL)
	{
		uint32_t value = mem_interpreter<uint32_t>(ctx,address);
		if ((is_exclusive))
		{
			_sys_interpreter(ctx,exclusive_address,((uint64_t)address & (uint64_t)~63ULL));
			_sys_interpreter(ctx,exclusive_value,(uint64_t)value);
		}
		X_interpreter(ctx,Rt,(uint64_t)value);
	}
	if (datasize == 64ULL)
	{
		uint64_t value = mem_interpreter<uint64_t>(ctx,address);
		if ((is_exclusive))
		{
			_sys_interpreter(ctx,exclusive_address,((uint64_t)address & (uint64_t)~63ULL));
			_sys_interpreter(ctx,exclusive_value,(uint64_t)value);
		}
		X_interpreter(ctx,Rt,(uint64_t)value);
	}
	
}

void store_exclusive_interpreter(interpreter_data* ctx, uint64_t is_exclusive, uint64_t size, uint64_t Rn, uint64_t Rt, uint64_t Rs)
{
	uint64_t datasize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t address = XSP_interpreter(ctx,Rn);
	if (datasize == 8ULL)
	{
		if ((is_exclusive))
		{
			uint64_t mask = ~63ULL;
			uint64_t _exclusive_address = _sys_interpreter(ctx,exclusive_address);
			if ((((uint64_t)_exclusive_address == (uint64_t)(((uint64_t)address & (uint64_t)mask)))))
			{
				uint8_t to_swap = X_interpreter(ctx,Rt);
				uint8_t expecting = _sys_interpreter(ctx,exclusive_value);
				uint8_t cas_success = compare_and_swap_interpreter(ctx,address,(uint64_t)expecting,(uint64_t)to_swap,datasize);
				X_interpreter(ctx,Rs,(uint64_t)((uint8_t)cas_success ^ (uint8_t)1ULL));
			}
			else
			{
				X_interpreter(ctx,Rs,(uint64_t)1ULL);
			}
		}
		else
		{
			mem_interpreter<uint8_t>(ctx,address,(uint8_t)X_interpreter(ctx,Rt));
		}
	}
	if (datasize == 16ULL)
	{
		if ((is_exclusive))
		{
			uint64_t mask = ~63ULL;
			uint64_t _exclusive_address = _sys_interpreter(ctx,exclusive_address);
			if ((((uint64_t)_exclusive_address == (uint64_t)(((uint64_t)address & (uint64_t)mask)))))
			{
				uint16_t to_swap = X_interpreter(ctx,Rt);
				uint16_t expecting = _sys_interpreter(ctx,exclusive_value);
				uint16_t cas_success = compare_and_swap_interpreter(ctx,address,(uint64_t)expecting,(uint64_t)to_swap,datasize);
				X_interpreter(ctx,Rs,(uint64_t)((uint16_t)cas_success ^ (uint16_t)1ULL));
			}
			else
			{
				X_interpreter(ctx,Rs,(uint64_t)1ULL);
			}
		}
		else
		{
			mem_interpreter<uint16_t>(ctx,address,(uint16_t)X_interpreter(ctx,Rt));
		}
	}
	if (datasize == 32ULL)
	{
		if ((is_exclusive))
		{
			uint64_t mask = ~63ULL;
			uint64_t _exclusive_address = _sys_interpreter(ctx,exclusive_address);
			if ((((uint64_t)_exclusive_address == (uint64_t)(((uint64_t)address & (uint64_t)mask)))))
			{
				uint32_t to_swap = X_interpreter(ctx,Rt);
				uint32_t expecting = _sys_interpreter(ctx,exclusive_value);
				uint32_t cas_success = compare_and_swap_interpreter(ctx,address,(uint64_t)expecting,(uint64_t)to_swap,datasize);
				X_interpreter(ctx,Rs,(uint64_t)((uint32_t)cas_success ^ (uint32_t)1ULL));
			}
			else
			{
				X_interpreter(ctx,Rs,(uint64_t)1ULL);
			}
		}
		else
		{
			mem_interpreter<uint32_t>(ctx,address,(uint32_t)X_interpreter(ctx,Rt));
		}
	}
	if (datasize == 64ULL)
	{
		if ((is_exclusive))
		{
			uint64_t mask = ~63ULL;
			uint64_t _exclusive_address = _sys_interpreter(ctx,exclusive_address);
			if ((((uint64_t)_exclusive_address == (uint64_t)(((uint64_t)address & (uint64_t)mask)))))
			{
				uint64_t to_swap = X_interpreter(ctx,Rt);
				uint64_t expecting = _sys_interpreter(ctx,exclusive_value);
				uint64_t cas_success = compare_and_swap_interpreter(ctx,address,(uint64_t)expecting,(uint64_t)to_swap,datasize);
				X_interpreter(ctx,Rs,(uint64_t)((uint64_t)cas_success ^ (uint64_t)1ULL));
			}
			else
			{
				X_interpreter(ctx,Rs,(uint64_t)1ULL);
			}
		}
		else
		{
			mem_interpreter<uint64_t>(ctx,address,(uint64_t)X_interpreter(ctx,Rt));
		}
	}
	
}

void branch_register_interpreter(interpreter_data* ctx, uint64_t l, uint64_t Rn)
{
	branch_long_universal_interpreter(ctx,Rn,l);
}

void return_register_interpreter(interpreter_data* ctx, uint64_t Rn)
{
	branch_long_universal_interpreter(ctx,Rn,0ULL);
}

void test_bit_branch_interpreter(interpreter_data* ctx, uint64_t b5, uint64_t op, uint64_t b40, uint64_t imm14, uint64_t Rt)
{
	uint64_t bit_pos = ((uint64_t)b40 + (uint64_t)(((uint64_t)b5 << (uint64_t)5ULL)));
	uint64_t new_location = ((uint64_t)_get_pc_interpreter(ctx) + (uint64_t)(((uint64_t)sign_extend_interpreter(ctx,imm14,14ULL) << (uint64_t)2ULL)));
	uint64_t next_location = ((uint64_t)_get_pc_interpreter(ctx) + (uint64_t)4ULL);
	uint64_t src = X_interpreter(ctx,Rt);
	uint8_t branch_pass = ((uint64_t)(((uint64_t)(((uint64_t)src >> (uint64_t)bit_pos)) & (uint64_t)1ULL)) == (uint64_t)op);
	_branch_conditional_interpreter(ctx,new_location,next_location,(uint64_t)branch_pass);
}

void compare_and_branch_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t imm19, uint64_t Rt)
{
	if (sf == 0ULL)
	{
		uint64_t new_location = ((uint64_t)_get_pc_interpreter(ctx) + (uint64_t)(((uint64_t)sign_extend_interpreter(ctx,imm19,19ULL) << (uint64_t)2ULL)));
		uint64_t next_location = ((uint64_t)_get_pc_interpreter(ctx) + (uint64_t)4ULL);
		uint32_t operand = X_interpreter(ctx,Rt);
		uint8_t branch_pass;
		if ((!op))
		{
			branch_pass = ((uint32_t)operand == (uint32_t)0ULL);
		}
		else
		{
			branch_pass = ((uint32_t)operand != (uint32_t)0ULL);
		}
		_branch_conditional_interpreter(ctx,new_location,next_location,(uint64_t)branch_pass);
	}
	if (sf == 1ULL)
	{
		uint64_t new_location = ((uint64_t)_get_pc_interpreter(ctx) + (uint64_t)(((uint64_t)sign_extend_interpreter(ctx,imm19,19ULL) << (uint64_t)2ULL)));
		uint64_t next_location = ((uint64_t)_get_pc_interpreter(ctx) + (uint64_t)4ULL);
		uint64_t operand = X_interpreter(ctx,Rt);
		uint8_t branch_pass;
		if ((!op))
		{
			branch_pass = ((uint64_t)operand == (uint64_t)0ULL);
		}
		else
		{
			branch_pass = ((uint64_t)operand != (uint64_t)0ULL);
		}
		_branch_conditional_interpreter(ctx,new_location,next_location,(uint64_t)branch_pass);
	}
	
}

void b_unconditional_interpreter(interpreter_data* ctx, uint64_t op, uint64_t imm26)
{
	uint64_t new_location = ((uint64_t)_get_pc_interpreter(ctx) + (uint64_t)(((uint64_t)sign_extend_interpreter(ctx,imm26,26ULL) << (uint64_t)2ULL)));
	if ((op))
	{
		uint64_t next_location = ((uint64_t)_get_pc_interpreter(ctx) + (uint64_t)4ULL);
		X_interpreter(ctx,30ULL,(uint64_t)next_location);
	}
	_branch_short_interpreter(ctx,new_location);
}

void b_conditional_interpreter(interpreter_data* ctx, uint64_t imm19, uint64_t cond)
{
	uint64_t new_location = ((uint64_t)_get_pc_interpreter(ctx) + (uint64_t)(((uint64_t)sign_extend_interpreter(ctx,imm19,19ULL) << (uint64_t)2ULL)));
	uint64_t next_location = ((uint64_t)_get_pc_interpreter(ctx) + (uint64_t)4ULL);
	_branch_conditional_interpreter(ctx,new_location,next_location,(uint64_t)condition_holds_interpreter(ctx,cond));
}

void svc_interpreter(interpreter_data* ctx, uint64_t imm16)
{
	call_supervisor_interpreter(ctx,imm16);
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

uint64_t a_extend_reg_64_interpreter(interpreter_data* ctx, uint64_t m, uint64_t extend_type, uint64_t shift)
{
	uint64_t val = X_interpreter(ctx,m);
	if ((((uint64_t)extend_type == (uint64_t)0ULL)))
	{
		val = ((uint64_t)val & (uint64_t)255ULL);
	}
	else if ((((uint64_t)extend_type == (uint64_t)1ULL)))
	{
		val = ((uint64_t)val & (uint64_t)65535ULL);
	}
	else if ((((uint64_t)extend_type == (uint64_t)2ULL)))
	{
		val = ((uint64_t)val & (uint64_t)4294967295ULL);
	}
	else if ((((uint64_t)extend_type == (uint64_t)4ULL)))
	{
		val = (uint64_t)sign_extend((uint8_t)val);
	}
	else if ((((uint64_t)extend_type == (uint64_t)5ULL)))
	{
		val = (uint64_t)sign_extend((uint16_t)val);
	}
	else if ((((uint64_t)extend_type == (uint64_t)6ULL)))
	{
		val = (uint64_t)sign_extend((uint32_t)val);
	}
	return ((uint64_t)val << (uint64_t)shift);
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
O vector_shift_interpreter(interpreter_data* ctx, O element, O shift, uint64_t bit_count, uint64_t is_unsigned)
{
	O result = 0ULL;
	if ((!is_unsigned))
	{
		element = (uint64_t)sign_extend(element);
	}
	if ((((O)(sign_extend((O)shift) >= sign_extend((O)0ULL)))))
	{
		if ((((O)(sign_extend((O)shift) >= sign_extend((O)bit_count)))))
		{
			result = 0ULL;
		}
		else
		{
			result = ((O)element << (O)shift);
		}
	}
	else
	{
		shift = -shift;
		if ((((O)(sign_extend((O)shift) >= sign_extend((O)bit_count)))))
		{
			if ((((O)((O)(((O)(((O)element >> (O)(((uint64_t)bit_count - (uint64_t)1ULL)))) & (O)1ULL)) == (O)1ULL) && (O)!is_unsigned)))
			{
				result = -1ULL;
			}
			else
			{
				result = 0ULL;
			}
		}
		else
		{
			if ((!is_unsigned))
			{
				result = ((O)(sign_extend((O)element) >> sign_extend((O)shift)));
			}
			else
			{
				result = ((O)element >> (O)shift);
			}
		}
	}
	return result;
}

void convert_to_int_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t Rd, uint64_t Rn, uint64_t round, uint64_t is_unsigned, uint64_t to_vector)
{
	uint64_t operand = V_interpreter(ctx,Rn);
	uint64_t intsize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t fltsize = get_flt_size_interpreter(ctx,ftype);
	uint64_t result = FPToFixed_interpreter(ctx,operand,0ULL,is_unsigned,round,intsize,fltsize);
	if ((to_vector))
	{
		V_interpreter(ctx,Rd,(uint128_t)result);
	}
	else
	{
		X_interpreter(ctx,Rd,result);
	}
}

uint64_t lowest_bit_set_c_interpreter(interpreter_data* ctx, uint64_t source)
{
	uint64_t size = 32ULL;
	for (uint64_t i = 0; i < (size); i++)
	{
		uint64_t working_bit = ((uint64_t)(((uint64_t)source >> (uint64_t)i)) & (uint64_t)1ULL);
		if ((working_bit))
		{
			return i;
		}
	}
	return size;
}

void dup_element_interpreter(interpreter_data* ctx, uint64_t index, uint64_t esize, uint64_t elements, uint64_t n, uint64_t d)
{
	uint128_t operand = V_interpreter(ctx,n);
	uint128_t result = 0;
	uint64_t element = uint128_t::extract(operand, index, esize);
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint128_t::insert(&result, e, esize, element);
	}
	V_interpreter(ctx,d,result);
}

uint64_t get_flt_size_interpreter(interpreter_data* ctx, uint64_t ftype)
{
	if ((((uint64_t)ftype == (uint64_t)2ULL)))
	{
		return 64ULL;
	}
	else
	{
		return ((uint64_t)8ULL << (uint64_t)(((uint64_t)ftype ^ (uint64_t)2ULL)));
	}
}

uint64_t vfp_expand_imm_interpreter(interpreter_data* ctx, uint64_t imm8, uint64_t N)
{
	uint64_t E;
	if ((((uint64_t)N == (uint64_t)16ULL)))
	{
		E = 5ULL;
	}
	else if ((((uint64_t)N == (uint64_t)32ULL)))
	{
		E = 8ULL;
	}
	else
	{
		E = 11ULL;
	}
	uint64_t F = ((uint64_t)(((uint64_t)N - (uint64_t)E)) - (uint64_t)1ULL);
	uint64_t sign = ((uint64_t)(((uint64_t)imm8 >> (uint64_t)7ULL)) & (uint64_t)1ULL);
	uint64_t exp = ((uint64_t)~(bit_c_interpreter(ctx,imm8,6ULL)) & (uint64_t)1ULL);
	exp = ((uint64_t)(((uint64_t)exp << (uint64_t)(((uint64_t)E - (uint64_t)3ULL)))) | (uint64_t)replicate_c_interpreter(ctx,bit_c_interpreter(ctx,imm8,6ULL),1ULL,((uint64_t)E - (uint64_t)3ULL)));
	exp = ((uint64_t)(((uint64_t)exp << (uint64_t)2ULL)) | (uint64_t)bits_c_interpreter(ctx,imm8,5ULL,4ULL));
	uint64_t frac = ((uint64_t)bits_c_interpreter(ctx,imm8,3ULL,0ULL) << (uint64_t)(((uint64_t)F - (uint64_t)4ULL)));
	uint64_t result = sign;
	result = ((uint64_t)(((uint64_t)result << (uint64_t)(((uint64_t)((uint64_t)1ULL + (uint64_t)(((uint64_t)E - (uint64_t)3ULL))) + (uint64_t)2ULL)))) | (uint64_t)exp);
	result = ((uint64_t)(((uint64_t)result << (uint64_t)(((uint64_t)4ULL + (uint64_t)(((uint64_t)F - (uint64_t)4ULL)))))) | (uint64_t)frac);
	return result;
}

uint64_t expand_imm_interpreter(interpreter_data* ctx, uint64_t op, uint64_t cmode, uint64_t imm8)
{
	uint64_t imm64 = 0ULL;
	uint64_t cmode_test = bits_c_interpreter(ctx,cmode,3ULL,1ULL);
	if ((((uint64_t)cmode_test == (uint64_t)0ULL)))
	{
		imm64 = replicate_c_interpreter(ctx,imm8,32ULL,2ULL);
	}
	else if ((((uint64_t)cmode_test == (uint64_t)1ULL)))
	{
		imm64 = replicate_c_interpreter(ctx,((uint64_t)imm8 << (uint64_t)8ULL),32ULL,2ULL);
	}
	else if ((((uint64_t)cmode_test == (uint64_t)2ULL)))
	{
		imm64 = replicate_c_interpreter(ctx,((uint64_t)imm8 << (uint64_t)16ULL),32ULL,2ULL);
	}
	else if ((((uint64_t)cmode_test == (uint64_t)3ULL)))
	{
		imm64 = replicate_c_interpreter(ctx,((uint64_t)imm8 << (uint64_t)24ULL),32ULL,2ULL);
	}
	else if ((((uint64_t)cmode_test == (uint64_t)4ULL)))
	{
		imm64 = replicate_c_interpreter(ctx,imm8,16ULL,4ULL);
	}
	else if ((((uint64_t)cmode_test == (uint64_t)5ULL)))
	{
		imm64 = replicate_c_interpreter(ctx,((uint64_t)imm8 << (uint64_t)8ULL),16ULL,4ULL);
	}
	else if ((((uint64_t)cmode_test == (uint64_t)6ULL)))
	{
		if ((((uint64_t)(((uint64_t)cmode & (uint64_t)1ULL)) == (uint64_t)0ULL)))
		{
			imm64 = replicate_c_interpreter(ctx,((uint64_t)(((uint64_t)imm8 << (uint64_t)8ULL)) | (uint64_t)ones_interpreter(ctx,8ULL)),32ULL,2ULL);
		}
		else
		{
			imm64 = replicate_c_interpreter(ctx,((uint64_t)(((uint64_t)imm8 << (uint64_t)16ULL)) | (uint64_t)ones_interpreter(ctx,16ULL)),32ULL,2ULL);
		}
	}
	else if ((((uint64_t)cmode_test == (uint64_t)7ULL)))
	{
		if ((((uint64_t)((uint64_t)bit_c_interpreter(ctx,cmode,0ULL) == (uint64_t)0ULL) && (uint64_t)((uint64_t)op == (uint64_t)0ULL))))
		{
			imm64 = replicate_c_interpreter(ctx,imm8,8ULL,8ULL);
		}
		else if ((((uint64_t)((uint64_t)bit_c_interpreter(ctx,cmode,0ULL) == (uint64_t)0ULL) && (uint64_t)((uint64_t)op == (uint64_t)1ULL))))
		{
			for (uint64_t i = 0; i < (8ULL); i++)
			{
				uint64_t part = ((uint64_t)(((uint64_t)0ULL - (uint64_t)(((uint64_t)(((uint64_t)imm8 >> (uint64_t)i)) & (uint64_t)1ULL)))) & (uint64_t)255ULL);
				imm64 = ((uint64_t)imm64 | (uint64_t)(((uint64_t)part << (uint64_t)(((uint64_t)i * (uint64_t)8ULL)))));
			}
		}
		else if ((((uint64_t)((uint64_t)bit_c_interpreter(ctx,cmode,0ULL) == (uint64_t)1ULL) && (uint64_t)((uint64_t)op == (uint64_t)0ULL))))
		{
			uint64_t p0 = bit_c_interpreter(ctx,imm8,7ULL);
			uint64_t p1 = ((uint64_t)(~(bit_c_interpreter(ctx,imm8,6ULL))) & (uint64_t)1ULL);
			uint64_t p2 = replicate_c_interpreter(ctx,bit_c_interpreter(ctx,imm8,6ULL),1ULL,5ULL);
			uint64_t p3 = bits_c_interpreter(ctx,imm8,5ULL,0ULL);
			uint64_t p4 = 0ULL;
			uint64_t working = ((uint64_t)((uint64_t)((uint64_t)((uint64_t)p4 | (uint64_t)(((uint64_t)p3 << (uint64_t)19ULL))) | (uint64_t)(((uint64_t)p2 << (uint64_t)(((uint64_t)19ULL + (uint64_t)6ULL))))) | (uint64_t)(((uint64_t)p1 << (uint64_t)(((uint64_t)((uint64_t)19ULL + (uint64_t)6ULL) + (uint64_t)5ULL))))) | (uint64_t)(((uint64_t)p0 << (uint64_t)(((uint64_t)((uint64_t)((uint64_t)19ULL + (uint64_t)6ULL) + (uint64_t)5ULL) + (uint64_t)1ULL)))));
			imm64 = replicate_c_interpreter(ctx,working,32ULL,2ULL);
		}
		else if ((((uint64_t)((uint64_t)bit_c_interpreter(ctx,cmode,0ULL) == (uint64_t)1ULL) && (uint64_t)((uint64_t)op == (uint64_t)1ULL))))
		{
			uint64_t p0 = bit_c_interpreter(ctx,imm8,7ULL);
			uint64_t p1 = ((uint64_t)(~(bit_c_interpreter(ctx,imm8,6ULL))) & (uint64_t)1ULL);
			uint64_t p2 = replicate_c_interpreter(ctx,bit_c_interpreter(ctx,imm8,6ULL),1ULL,8ULL);
			uint64_t p3 = bits_c_interpreter(ctx,imm8,5ULL,0ULL);
			uint64_t p4 = 0ULL;
			imm64 = ((uint64_t)((uint64_t)((uint64_t)((uint64_t)p4 | (uint64_t)(((uint64_t)p3 << (uint64_t)48ULL))) | (uint64_t)(((uint64_t)p2 << (uint64_t)(((uint64_t)48ULL + (uint64_t)6ULL))))) | (uint64_t)(((uint64_t)p1 << (uint64_t)(((uint64_t)((uint64_t)48ULL + (uint64_t)6ULL) + (uint64_t)8ULL))))) | (uint64_t)(((uint64_t)p0 << (uint64_t)(((uint64_t)((uint64_t)((uint64_t)48ULL + (uint64_t)6ULL) + (uint64_t)8ULL) + (uint64_t)1ULL)))));
		}
		else
		{
			undefined_interpreter(ctx);
		}
	}
	else
	{
		undefined_interpreter(ctx);
	}
	return imm64;
}

void VPart_interpreter(interpreter_data* ctx, uint64_t n, uint64_t part, uint64_t width, uint64_t value)
{
	if ((((uint64_t)part == (uint64_t)0ULL)))
	{
		V_interpreter(ctx,n,(uint128_t)value);
	}
	else
	{
		uint128_t src = V_interpreter(ctx,n);
		uint128_t::insert(&src, 1ULL, 64ULL, value);
		V_interpreter(ctx,n,src);
	}
}

void dup_general_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_interpreter(ctx,bits_c_interpreter(ctx,imm5,3ULL,0ULL));
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t element = X_interpreter(ctx,Rn);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t result = 0;
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint128_t::insert(&result, e, esize, element);
	}
	V_interpreter(ctx,Rd,result);
}

void dup_element_scalar_interpreter(interpreter_data* ctx, uint64_t imm5, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_interpreter(ctx,bits_c_interpreter(ctx,imm5,3ULL,0ULL));
	uint64_t index = bits_c_interpreter(ctx,imm5,4ULL,((uint64_t)size + (uint64_t)1ULL));
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)bit_c_interpreter(ctx,imm5,4ULL));
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = esize;
	uint64_t elements = 1ULL;
	dup_element_interpreter(ctx,index,esize,elements,Rn,Rd);
}

void dup_element_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_interpreter(ctx,bits_c_interpreter(ctx,imm5,3ULL,0ULL));
	uint64_t index = bits_c_interpreter(ctx,imm5,4ULL,((uint64_t)size + (uint64_t)1ULL));
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)bit_c_interpreter(ctx,imm5,4ULL));
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	dup_element_interpreter(ctx,index,esize,elements,Rn,Rd);
}

void move_to_gp_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t imm5, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_interpreter(ctx,bits_c_interpreter(ctx,imm5,3ULL,0ULL));
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)Q);
	uint64_t index = bits_c_interpreter(ctx,imm5,4ULL,((uint64_t)size + (uint64_t)1ULL));
	uint128_t operand = V_interpreter(ctx,Rn);
	if (esize == 8ULL)
	{
		if (datasize == 32ULL)
		{
			uint32_t working_operand = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				working_operand = (uint32_t)sign_extend((uint8_t)working_operand);
			}
			X_interpreter(ctx,Rd,(uint64_t)working_operand);
		}
		if (datasize == 64ULL)
		{
			uint64_t working_operand = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				working_operand = (uint64_t)sign_extend((uint8_t)working_operand);
			}
			X_interpreter(ctx,Rd,(uint64_t)working_operand);
		}
		
	}
	if (esize == 16ULL)
	{
		if (datasize == 32ULL)
		{
			uint32_t working_operand = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				working_operand = (uint32_t)sign_extend((uint16_t)working_operand);
			}
			X_interpreter(ctx,Rd,(uint64_t)working_operand);
		}
		if (datasize == 64ULL)
		{
			uint64_t working_operand = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				working_operand = (uint64_t)sign_extend((uint16_t)working_operand);
			}
			X_interpreter(ctx,Rd,(uint64_t)working_operand);
		}
		
	}
	if (esize == 32ULL)
	{
		if (datasize == 32ULL)
		{
			uint32_t working_operand = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				working_operand = (uint32_t)sign_extend((uint32_t)working_operand);
			}
			X_interpreter(ctx,Rd,(uint64_t)working_operand);
		}
		if (datasize == 64ULL)
		{
			uint64_t working_operand = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				working_operand = (uint64_t)sign_extend((uint32_t)working_operand);
			}
			X_interpreter(ctx,Rd,(uint64_t)working_operand);
		}
		
	}
	if (esize == 64ULL)
	{
		if (datasize == 32ULL)
		{
			uint32_t working_operand = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				working_operand = (uint32_t)sign_extend((uint64_t)working_operand);
			}
			X_interpreter(ctx,Rd,(uint64_t)working_operand);
		}
		if (datasize == 64ULL)
		{
			uint64_t working_operand = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				working_operand = (uint64_t)sign_extend((uint64_t)working_operand);
			}
			X_interpreter(ctx,Rd,(uint64_t)working_operand);
		}
		
	}
	
}

void ins_general_interpreter(interpreter_data* ctx, uint64_t imm5, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_interpreter(ctx,bits_c_interpreter(ctx,imm5,3ULL,0ULL));
	uint64_t index = bits_c_interpreter(ctx,imm5,4ULL,((uint64_t)size + (uint64_t)1ULL));
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t element = X_interpreter(ctx,Rn);
	uint128_t result = V_interpreter(ctx,Rd);
	uint128_t::insert(&result, index, esize, element);
	V_interpreter(ctx,Rd,result);
}

void ins_element_interpreter(interpreter_data* ctx, uint64_t imm5, uint64_t imm4, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_interpreter(ctx,bits_c_interpreter(ctx,imm5,3ULL,0ULL));
	uint64_t dst_index = bits_c_interpreter(ctx,imm5,4ULL,((uint64_t)size + (uint64_t)1ULL));
	uint64_t src_index = bits_c_interpreter(ctx,imm4,3ULL,size);
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint128_t operand = V_interpreter(ctx,Rn);
	uint128_t result = V_interpreter(ctx,Rd);
	uint128_t::insert(&result, dst_index, esize, (uint128_t::extract(operand, src_index, esize)));
	V_interpreter(ctx,Rd,result);
}

void movi_immediate_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t op, uint64_t immhi, uint64_t cmode, uint64_t immlo, uint64_t Rd)
{
	uint64_t d = Rd;
	uint64_t imm8 = ((uint64_t)(((uint64_t)immhi << (uint64_t)5ULL)) | (uint64_t)immlo);
	uint64_t cmode_helper = ((uint64_t)(((uint64_t)cmode << (uint64_t)1ULL)) | (uint64_t)op);
	uint64_t mode = 0ULL;
	if ((((uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)19ULL)) == (uint64_t)2ULL)) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)27ULL)) == (uint64_t)18ULL)))))
	{
		mode = 0ULL;
	}
	else if ((((uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)19ULL)) == (uint64_t)3ULL)) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)27ULL)) == (uint64_t)19ULL)))))
	{
		mode = 1ULL;
	}
	else if ((((uint64_t)((uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)19ULL)) == (uint64_t)1ULL)) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)27ULL)) == (uint64_t)17ULL))) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)29ULL)) == (uint64_t)25ULL)))))
	{
		mode = 2ULL;
	}
	else if ((((uint64_t)((uint64_t)((uint64_t)((uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)19ULL)) == (uint64_t)0ULL)) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)27ULL)) == (uint64_t)16ULL))) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)29ULL)) == (uint64_t)24ULL))) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)30ULL)) == (uint64_t)28ULL))) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)31ULL)) == (uint64_t)30ULL)))))
	{
		mode = 3ULL;
	}
	else if ((((uint64_t)cmode_helper == (uint64_t)31ULL)))
	{
		if ((((uint64_t)Q == (uint64_t)0ULL)))
		{
			undefined_interpreter(ctx);
		}
		mode = 3ULL;
	}
	else
	{
		undefined_interpreter(ctx);
	}
	uint64_t imm64 = expand_imm_interpreter(ctx,op,cmode,imm8);
	uint128_t imm = imm64;
	if ((Q))
	{
		uint128_t::insert(&imm, 1ULL, 64ULL, imm64);
	}
	uint128_t operand = 0;
	uint128_t result = 0;
	if ((((uint64_t)mode == (uint64_t)3ULL)))
	{
		result = imm;
	}
	else if ((((uint64_t)mode == (uint64_t)2ULL)))
	{
		for (uint64_t e = 0; e < (((uint64_t)Q + (uint64_t)1ULL)); e++)
		{
			uint128_t::insert(&result, e, 64ULL, ~(uint128_t::extract(imm, e, 64ULL)));
		}
	}
	else if ((((uint64_t)mode == (uint64_t)0ULL)))
	{
		operand = V_interpreter(ctx,Rd);
		for (uint64_t e = 0; e < (((uint64_t)Q + (uint64_t)1ULL)); e++)
		{
			uint128_t::insert(&result, e, 64ULL, (((uint64_t)(uint128_t::extract(operand, e, 64ULL)) | (uint64_t)(uint128_t::extract(imm, e, 64ULL)))));
		}
	}
	else if ((((uint64_t)mode == (uint64_t)1ULL)))
	{
		operand = V_interpreter(ctx,Rd);
		for (uint64_t e = 0; e < (((uint64_t)Q + (uint64_t)1ULL)); e++)
		{
			uint128_t::insert(&result, e, 64ULL, (((uint64_t)(uint128_t::extract(operand, e, 64ULL)) & (uint64_t)~(uint128_t::extract(imm, e, 64ULL)))));
		}
	}
	else
	{
		undefined_interpreter(ctx);
	}
	V_interpreter(ctx,Rd,result);
}

void fmov_general_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t rmode, uint64_t opcode, uint64_t Rn, uint64_t Rd)
{
	uint64_t intsize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t fltsize = get_flt_size_interpreter(ctx,ftype);
	if ((((uint64_t)intsize != (uint64_t)fltsize)))
	{
		undefined_interpreter(ctx);
	}
	uint64_t size = intsize;
	uint64_t part = rmode;
	uint64_t int_to_float = opcode;
	if ((int_to_float))
	{
		uint64_t src = X_interpreter(ctx,Rn);
		if (size == 32ULL)
		{
			VPart_interpreter(ctx,Rd,part,size,(uint64_t)(uint32_t)src);
		}
		if (size == 64ULL)
		{
			VPart_interpreter(ctx,Rd,part,size,(uint64_t)(uint64_t)src);
		}
		
	}
	else
	{
		uint128_t v = V_interpreter(ctx,Rn);
		uint64_t src = uint128_t::extract(v, part, size);
		X_interpreter(ctx,Rd,src);
	}
}

void convert_to_float_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd, uint64_t from_vector)
{
	uint64_t intsize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t fltsize = get_flt_size_interpreter(ctx,ftype);
	if (intsize == 32ULL)
	{
		if (fltsize == 16ULL)
		{
			uint16_t result;
			uint32_t operand;
			if ((from_vector))
			{
				operand = V_interpreter(ctx,Rn);
			}
			else
			{
				operand = X_interpreter(ctx,Rn);
			}
			if ((U))
			{
				result = convert_to_float<uint16_t, uint32_t>(operand, 0);
			}
			else
			{
				result = convert_to_float<uint16_t, uint32_t>(operand, 1);
			}
			V_interpreter(ctx,Rd,(uint128_t)result);
		}
		if (fltsize == 32ULL)
		{
			uint32_t result;
			uint32_t operand;
			if ((from_vector))
			{
				operand = V_interpreter(ctx,Rn);
			}
			else
			{
				operand = X_interpreter(ctx,Rn);
			}
			if ((U))
			{
				result = convert_to_float<uint32_t, uint32_t>(operand, 0);
			}
			else
			{
				result = convert_to_float<uint32_t, uint32_t>(operand, 1);
			}
			V_interpreter(ctx,Rd,(uint128_t)result);
		}
		if (fltsize == 64ULL)
		{
			uint64_t result;
			uint32_t operand;
			if ((from_vector))
			{
				operand = V_interpreter(ctx,Rn);
			}
			else
			{
				operand = X_interpreter(ctx,Rn);
			}
			if ((U))
			{
				result = convert_to_float<uint64_t, uint32_t>(operand, 0);
			}
			else
			{
				result = convert_to_float<uint64_t, uint32_t>(operand, 1);
			}
			V_interpreter(ctx,Rd,(uint128_t)result);
		}
		
	}
	if (intsize == 64ULL)
	{
		if (fltsize == 16ULL)
		{
			uint16_t result;
			uint64_t operand;
			if ((from_vector))
			{
				operand = V_interpreter(ctx,Rn);
			}
			else
			{
				operand = X_interpreter(ctx,Rn);
			}
			if ((U))
			{
				result = convert_to_float<uint16_t, uint64_t>(operand, 0);
			}
			else
			{
				result = convert_to_float<uint16_t, uint64_t>(operand, 1);
			}
			V_interpreter(ctx,Rd,(uint128_t)result);
		}
		if (fltsize == 32ULL)
		{
			uint32_t result;
			uint64_t operand;
			if ((from_vector))
			{
				operand = V_interpreter(ctx,Rn);
			}
			else
			{
				operand = X_interpreter(ctx,Rn);
			}
			if ((U))
			{
				result = convert_to_float<uint32_t, uint64_t>(operand, 0);
			}
			else
			{
				result = convert_to_float<uint32_t, uint64_t>(operand, 1);
			}
			V_interpreter(ctx,Rd,(uint128_t)result);
		}
		if (fltsize == 64ULL)
		{
			uint64_t result;
			uint64_t operand;
			if ((from_vector))
			{
				operand = V_interpreter(ctx,Rn);
			}
			else
			{
				operand = X_interpreter(ctx,Rn);
			}
			if ((U))
			{
				result = convert_to_float<uint64_t, uint64_t>(operand, 0);
			}
			else
			{
				result = convert_to_float<uint64_t, uint64_t>(operand, 1);
			}
			V_interpreter(ctx,Rd,(uint128_t)result);
		}
		
	}
	
}

void convert_to_float_gp_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_float_interpreter(ctx,sf,ftype,U,Rn,Rd,0ULL);
}

void convert_to_float_vector_interpreter(interpreter_data* ctx, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	convert_to_float_interpreter(ctx,sz,sz,U,Rn,Rd,1ULL);
}

void floating_point_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t opcode, uint64_t Rn, uint64_t Rd)
{
	uint64_t fltsize = get_flt_size_interpreter(ctx,ftype);
	uint64_t operand1 = V_interpreter(ctx,Rn);
	uint64_t operand2 = V_interpreter(ctx,Rm);
	uint64_t result;
	uint64_t fpcr_state = _sys_interpreter(ctx,fpcr);
	if ((((uint64_t)opcode == (uint64_t)0ULL)))
	{
		result = FPMul_interpreter(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)1ULL)))
	{
		result = FPDiv_interpreter(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)2ULL)))
	{
		result = FPAdd_interpreter(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)3ULL)))
	{
		result = FPSub_interpreter(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)4ULL)))
	{
		result = FPMax_interpreter(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)5ULL)))
	{
		result = FPMin_interpreter(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)6ULL)))
	{
		result = FPMaxNum_interpreter(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)7ULL)))
	{
		result = FPMinNum_interpreter(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)8ULL)))
	{
		result = FPMul_interpreter(ctx,operand1,operand2,fpcr_state,fltsize);
		result = FPNeg_interpreter(ctx,result,fpcr_state,fltsize);
	}
	else
	{
		undefined_with_interpreter(ctx,opcode);
	}
	uint128_t vector = 0;
	uint128_t::insert(&vector, 0ULL, fltsize, result);
	V_interpreter(ctx,Rd,vector);
}

void conversion_between_floating_point_and_fixed_point_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t S, uint64_t ftype, uint64_t rmode, uint64_t opcode, uint64_t scale, uint64_t Rn, uint64_t Rd)
{
	uint64_t intsize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t fltsize = get_flt_size_interpreter(ctx,ftype);
	uint64_t fracbits = ((uint64_t)64ULL - (uint64_t)scale);
	uint64_t result;
	if ((((uint64_t)rmode == (uint64_t)0ULL)))
	{
		uint64_t source = X_interpreter(ctx,Rn);
		result = FixedToFP_interpreter(ctx,source,fracbits,((uint64_t)opcode == (uint64_t)3ULL),fltsize,intsize);
		V_interpreter(ctx,Rd,(uint128_t)result);
	}
	else if ((((uint64_t)rmode == (uint64_t)3ULL)))
	{
		uint64_t source = V_interpreter(ctx,Rn);
		result = FPToFixed_interpreter(ctx,source,fracbits,((uint64_t)opcode == (uint64_t)1ULL),FPRounding_ZERO,intsize,fltsize);
		X_interpreter(ctx,Rd,result);
	}
	else
	{
		undefined_with_interpreter(ctx,0ULL);
	}
}

void advanced_simd_three_same_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rm, uint64_t opcode, uint64_t Rn, uint64_t Rd)
{
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t result = 0;
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	if (esize == 8ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element2 = uint128_t::extract(operand2, e, esize);
			if ((((uint64_t)opcode == (uint64_t)0ULL)))
			{
				if ((!U))
				{
					element1 = (uint64_t)sign_extend((uint8_t)element1);
					element2 = (uint64_t)sign_extend((uint8_t)element2);
				}
				uint128_t::insert(&result, e, esize, ((uint64_t)(((uint64_t)element1 + (uint64_t)element2)) >> (uint64_t)1ULL));
			}
			else if ((((uint64_t)opcode == (uint64_t)2ULL)))
			{
				if ((!U))
				{
					element1 = (uint64_t)sign_extend((uint8_t)element1);
					element2 = (uint64_t)sign_extend((uint8_t)element2);
				}
				uint128_t::insert(&result, e, esize, ((uint64_t)(((uint64_t)((uint64_t)element1 + (uint64_t)element2) + (uint64_t)1ULL)) >> (uint64_t)1ULL));
			}
			else if ((((uint64_t)opcode == (uint64_t)4ULL)))
			{
				if ((!U))
				{
					element1 = (uint64_t)sign_extend((uint8_t)element1);
					element2 = (uint64_t)sign_extend((uint8_t)element2);
				}
				uint128_t::insert(&result, e, esize, ((uint64_t)(((uint64_t)element1 - (uint64_t)element2)) >> (uint64_t)1ULL));
			}
			else if ((((uint64_t)opcode == (uint64_t)6ULL)))
			{
				uint8_t valid;
				if ((U))
				{
					valid = ((uint8_t)((uint8_t)element1) > (uint8_t)((uint8_t)element2));
				}
				else
				{
					valid = ((uint8_t)(sign_extend((uint8_t)((uint8_t)element1)) > sign_extend((uint8_t)((uint8_t)element2))));
				}
				uint128_t::insert(&result, e, esize, (((uint8_t)0ULL - (uint8_t)valid)));
			}
			else if ((((uint64_t)opcode == (uint64_t)7ULL)))
			{
				uint8_t valid;
				if ((U))
				{
					valid = ((uint8_t)((uint8_t)element1) >= (uint8_t)((uint8_t)element2));
				}
				else
				{
					valid = ((uint8_t)(sign_extend((uint8_t)((uint8_t)element1)) >= sign_extend((uint8_t)((uint8_t)element2))));
				}
				uint128_t::insert(&result, e, esize, (((uint8_t)0ULL - (uint8_t)valid)));
			}
			else if ((((uint64_t)((uint64_t)opcode == (uint64_t)8ULL) && (uint64_t)((uint64_t)size == (uint64_t)2ULL))))
			{
				uint8_t element = element1;
				uint8_t shift = (uint8_t)sign_extend((uint8_t)element2);
				element = vector_shift_interpreter<uint8_t>(ctx,element,shift,esize,U);
				uint128_t::insert(&result, e, esize, element);
			}
			else if ((((uint64_t)opcode == (uint64_t)12ULL)))
			{
				uint8_t working = 0ULL;
				if ((U))
				{
					if ((((uint64_t)element1 > (uint64_t)element2)))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				else
				{
					if ((((uint8_t)(sign_extend((uint8_t)((uint8_t)element1)) > sign_extend((uint8_t)((uint8_t)element2))))))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				uint128_t::insert(&result, e, esize, working);
			}
			else if ((((uint64_t)opcode == (uint64_t)13ULL)))
			{
				uint8_t working = 0ULL;
				if ((U))
				{
					if ((((uint64_t)element1 < (uint64_t)element2)))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				else
				{
					if ((((uint8_t)(sign_extend((uint8_t)((uint8_t)element1)) < sign_extend((uint8_t)((uint8_t)element2))))))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				uint128_t::insert(&result, e, esize, working);
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)(((uint64_t)(((uint64_t)size >> (uint64_t)1ULL)) == (uint64_t)1ULL))) && (uint64_t)((uint64_t)U == (uint64_t)0ULL))))
			{
				if ((((uint64_t)size & (uint64_t)1ULL)))
				{
					element2 = ~element2;
				}
				uint128_t::insert(&result, e, esize, (((uint64_t)element1 | (uint64_t)element2)));
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)(((uint64_t)(((uint64_t)size >> (uint64_t)1ULL)) == (uint64_t)0ULL))) && (uint64_t)((uint64_t)U == (uint64_t)0ULL))))
			{
				if ((((uint64_t)size & (uint64_t)1ULL)))
				{
					element2 = ~element2;
				}
				uint128_t::insert(&result, e, esize, (((uint64_t)element1 & (uint64_t)element2)));
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)((uint64_t)size == (uint64_t)0ULL)) && (uint64_t)((uint64_t)U == (uint64_t)1ULL))))
			{
				uint128_t::insert(&result, e, esize, (((uint64_t)element1 ^ (uint64_t)element2)));
			}
			else
			{
				undefined_with_interpreter(ctx,0ULL);
			}
		}
		V_interpreter(ctx,Rd,result);
	}
	if (esize == 16ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element2 = uint128_t::extract(operand2, e, esize);
			if ((((uint64_t)opcode == (uint64_t)0ULL)))
			{
				if ((!U))
				{
					element1 = (uint64_t)sign_extend((uint16_t)element1);
					element2 = (uint64_t)sign_extend((uint16_t)element2);
				}
				uint128_t::insert(&result, e, esize, ((uint64_t)(((uint64_t)element1 + (uint64_t)element2)) >> (uint64_t)1ULL));
			}
			else if ((((uint64_t)opcode == (uint64_t)2ULL)))
			{
				if ((!U))
				{
					element1 = (uint64_t)sign_extend((uint16_t)element1);
					element2 = (uint64_t)sign_extend((uint16_t)element2);
				}
				uint128_t::insert(&result, e, esize, ((uint64_t)(((uint64_t)((uint64_t)element1 + (uint64_t)element2) + (uint64_t)1ULL)) >> (uint64_t)1ULL));
			}
			else if ((((uint64_t)opcode == (uint64_t)4ULL)))
			{
				if ((!U))
				{
					element1 = (uint64_t)sign_extend((uint16_t)element1);
					element2 = (uint64_t)sign_extend((uint16_t)element2);
				}
				uint128_t::insert(&result, e, esize, ((uint64_t)(((uint64_t)element1 - (uint64_t)element2)) >> (uint64_t)1ULL));
			}
			else if ((((uint64_t)opcode == (uint64_t)6ULL)))
			{
				uint16_t valid;
				if ((U))
				{
					valid = ((uint16_t)((uint16_t)element1) > (uint16_t)((uint16_t)element2));
				}
				else
				{
					valid = ((uint16_t)(sign_extend((uint16_t)((uint16_t)element1)) > sign_extend((uint16_t)((uint16_t)element2))));
				}
				uint128_t::insert(&result, e, esize, (((uint16_t)0ULL - (uint16_t)valid)));
			}
			else if ((((uint64_t)opcode == (uint64_t)7ULL)))
			{
				uint16_t valid;
				if ((U))
				{
					valid = ((uint16_t)((uint16_t)element1) >= (uint16_t)((uint16_t)element2));
				}
				else
				{
					valid = ((uint16_t)(sign_extend((uint16_t)((uint16_t)element1)) >= sign_extend((uint16_t)((uint16_t)element2))));
				}
				uint128_t::insert(&result, e, esize, (((uint16_t)0ULL - (uint16_t)valid)));
			}
			else if ((((uint64_t)((uint64_t)opcode == (uint64_t)8ULL) && (uint64_t)((uint64_t)size == (uint64_t)2ULL))))
			{
				uint16_t element = element1;
				uint16_t shift = (uint16_t)sign_extend((uint8_t)element2);
				element = vector_shift_interpreter<uint16_t>(ctx,element,shift,esize,U);
				uint128_t::insert(&result, e, esize, element);
			}
			else if ((((uint64_t)opcode == (uint64_t)12ULL)))
			{
				uint16_t working = 0ULL;
				if ((U))
				{
					if ((((uint64_t)element1 > (uint64_t)element2)))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				else
				{
					if ((((uint16_t)(sign_extend((uint16_t)((uint16_t)element1)) > sign_extend((uint16_t)((uint16_t)element2))))))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				uint128_t::insert(&result, e, esize, working);
			}
			else if ((((uint64_t)opcode == (uint64_t)13ULL)))
			{
				uint16_t working = 0ULL;
				if ((U))
				{
					if ((((uint64_t)element1 < (uint64_t)element2)))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				else
				{
					if ((((uint16_t)(sign_extend((uint16_t)((uint16_t)element1)) < sign_extend((uint16_t)((uint16_t)element2))))))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				uint128_t::insert(&result, e, esize, working);
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)(((uint64_t)(((uint64_t)size >> (uint64_t)1ULL)) == (uint64_t)1ULL))) && (uint64_t)((uint64_t)U == (uint64_t)0ULL))))
			{
				if ((((uint64_t)size & (uint64_t)1ULL)))
				{
					element2 = ~element2;
				}
				uint128_t::insert(&result, e, esize, (((uint64_t)element1 | (uint64_t)element2)));
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)(((uint64_t)(((uint64_t)size >> (uint64_t)1ULL)) == (uint64_t)0ULL))) && (uint64_t)((uint64_t)U == (uint64_t)0ULL))))
			{
				if ((((uint64_t)size & (uint64_t)1ULL)))
				{
					element2 = ~element2;
				}
				uint128_t::insert(&result, e, esize, (((uint64_t)element1 & (uint64_t)element2)));
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)((uint64_t)size == (uint64_t)0ULL)) && (uint64_t)((uint64_t)U == (uint64_t)1ULL))))
			{
				uint128_t::insert(&result, e, esize, (((uint64_t)element1 ^ (uint64_t)element2)));
			}
			else
			{
				undefined_with_interpreter(ctx,0ULL);
			}
		}
		V_interpreter(ctx,Rd,result);
	}
	if (esize == 32ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element2 = uint128_t::extract(operand2, e, esize);
			if ((((uint64_t)opcode == (uint64_t)0ULL)))
			{
				if ((!U))
				{
					element1 = (uint64_t)sign_extend((uint32_t)element1);
					element2 = (uint64_t)sign_extend((uint32_t)element2);
				}
				uint128_t::insert(&result, e, esize, ((uint64_t)(((uint64_t)element1 + (uint64_t)element2)) >> (uint64_t)1ULL));
			}
			else if ((((uint64_t)opcode == (uint64_t)2ULL)))
			{
				if ((!U))
				{
					element1 = (uint64_t)sign_extend((uint32_t)element1);
					element2 = (uint64_t)sign_extend((uint32_t)element2);
				}
				uint128_t::insert(&result, e, esize, ((uint64_t)(((uint64_t)((uint64_t)element1 + (uint64_t)element2) + (uint64_t)1ULL)) >> (uint64_t)1ULL));
			}
			else if ((((uint64_t)opcode == (uint64_t)4ULL)))
			{
				if ((!U))
				{
					element1 = (uint64_t)sign_extend((uint32_t)element1);
					element2 = (uint64_t)sign_extend((uint32_t)element2);
				}
				uint128_t::insert(&result, e, esize, ((uint64_t)(((uint64_t)element1 - (uint64_t)element2)) >> (uint64_t)1ULL));
			}
			else if ((((uint64_t)opcode == (uint64_t)6ULL)))
			{
				uint32_t valid;
				if ((U))
				{
					valid = ((uint32_t)((uint32_t)element1) > (uint32_t)((uint32_t)element2));
				}
				else
				{
					valid = ((uint32_t)(sign_extend((uint32_t)((uint32_t)element1)) > sign_extend((uint32_t)((uint32_t)element2))));
				}
				uint128_t::insert(&result, e, esize, (((uint32_t)0ULL - (uint32_t)valid)));
			}
			else if ((((uint64_t)opcode == (uint64_t)7ULL)))
			{
				uint32_t valid;
				if ((U))
				{
					valid = ((uint32_t)((uint32_t)element1) >= (uint32_t)((uint32_t)element2));
				}
				else
				{
					valid = ((uint32_t)(sign_extend((uint32_t)((uint32_t)element1)) >= sign_extend((uint32_t)((uint32_t)element2))));
				}
				uint128_t::insert(&result, e, esize, (((uint32_t)0ULL - (uint32_t)valid)));
			}
			else if ((((uint64_t)((uint64_t)opcode == (uint64_t)8ULL) && (uint64_t)((uint64_t)size == (uint64_t)2ULL))))
			{
				uint32_t element = element1;
				uint32_t shift = (uint32_t)sign_extend((uint8_t)element2);
				element = vector_shift_interpreter<uint32_t>(ctx,element,shift,esize,U);
				uint128_t::insert(&result, e, esize, element);
			}
			else if ((((uint64_t)opcode == (uint64_t)12ULL)))
			{
				uint32_t working = 0ULL;
				if ((U))
				{
					if ((((uint64_t)element1 > (uint64_t)element2)))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				else
				{
					if ((((uint32_t)(sign_extend((uint32_t)((uint32_t)element1)) > sign_extend((uint32_t)((uint32_t)element2))))))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				uint128_t::insert(&result, e, esize, working);
			}
			else if ((((uint64_t)opcode == (uint64_t)13ULL)))
			{
				uint32_t working = 0ULL;
				if ((U))
				{
					if ((((uint64_t)element1 < (uint64_t)element2)))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				else
				{
					if ((((uint32_t)(sign_extend((uint32_t)((uint32_t)element1)) < sign_extend((uint32_t)((uint32_t)element2))))))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				uint128_t::insert(&result, e, esize, working);
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)(((uint64_t)(((uint64_t)size >> (uint64_t)1ULL)) == (uint64_t)1ULL))) && (uint64_t)((uint64_t)U == (uint64_t)0ULL))))
			{
				if ((((uint64_t)size & (uint64_t)1ULL)))
				{
					element2 = ~element2;
				}
				uint128_t::insert(&result, e, esize, (((uint64_t)element1 | (uint64_t)element2)));
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)(((uint64_t)(((uint64_t)size >> (uint64_t)1ULL)) == (uint64_t)0ULL))) && (uint64_t)((uint64_t)U == (uint64_t)0ULL))))
			{
				if ((((uint64_t)size & (uint64_t)1ULL)))
				{
					element2 = ~element2;
				}
				uint128_t::insert(&result, e, esize, (((uint64_t)element1 & (uint64_t)element2)));
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)((uint64_t)size == (uint64_t)0ULL)) && (uint64_t)((uint64_t)U == (uint64_t)1ULL))))
			{
				uint128_t::insert(&result, e, esize, (((uint64_t)element1 ^ (uint64_t)element2)));
			}
			else
			{
				undefined_with_interpreter(ctx,0ULL);
			}
		}
		V_interpreter(ctx,Rd,result);
	}
	if (esize == 64ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element2 = uint128_t::extract(operand2, e, esize);
			if ((((uint64_t)opcode == (uint64_t)0ULL)))
			{
				if ((!U))
				{
					element1 = (uint64_t)sign_extend((uint64_t)element1);
					element2 = (uint64_t)sign_extend((uint64_t)element2);
				}
				uint128_t::insert(&result, e, esize, ((uint64_t)(((uint64_t)element1 + (uint64_t)element2)) >> (uint64_t)1ULL));
			}
			else if ((((uint64_t)opcode == (uint64_t)2ULL)))
			{
				if ((!U))
				{
					element1 = (uint64_t)sign_extend((uint64_t)element1);
					element2 = (uint64_t)sign_extend((uint64_t)element2);
				}
				uint128_t::insert(&result, e, esize, ((uint64_t)(((uint64_t)((uint64_t)element1 + (uint64_t)element2) + (uint64_t)1ULL)) >> (uint64_t)1ULL));
			}
			else if ((((uint64_t)opcode == (uint64_t)4ULL)))
			{
				if ((!U))
				{
					element1 = (uint64_t)sign_extend((uint64_t)element1);
					element2 = (uint64_t)sign_extend((uint64_t)element2);
				}
				uint128_t::insert(&result, e, esize, ((uint64_t)(((uint64_t)element1 - (uint64_t)element2)) >> (uint64_t)1ULL));
			}
			else if ((((uint64_t)opcode == (uint64_t)6ULL)))
			{
				uint64_t valid;
				if ((U))
				{
					valid = ((uint64_t)((uint64_t)element1) > (uint64_t)((uint64_t)element2));
				}
				else
				{
					valid = ((uint64_t)(sign_extend((uint64_t)((uint64_t)element1)) > sign_extend((uint64_t)((uint64_t)element2))));
				}
				uint128_t::insert(&result, e, esize, (((uint64_t)0ULL - (uint64_t)valid)));
			}
			else if ((((uint64_t)opcode == (uint64_t)7ULL)))
			{
				uint64_t valid;
				if ((U))
				{
					valid = ((uint64_t)((uint64_t)element1) >= (uint64_t)((uint64_t)element2));
				}
				else
				{
					valid = ((uint64_t)(sign_extend((uint64_t)((uint64_t)element1)) >= sign_extend((uint64_t)((uint64_t)element2))));
				}
				uint128_t::insert(&result, e, esize, (((uint64_t)0ULL - (uint64_t)valid)));
			}
			else if ((((uint64_t)((uint64_t)opcode == (uint64_t)8ULL) && (uint64_t)((uint64_t)size == (uint64_t)2ULL))))
			{
				uint64_t element = element1;
				uint64_t shift = (uint64_t)sign_extend((uint8_t)element2);
				element = vector_shift_interpreter<uint64_t>(ctx,element,shift,esize,U);
				uint128_t::insert(&result, e, esize, element);
			}
			else if ((((uint64_t)opcode == (uint64_t)12ULL)))
			{
				uint64_t working = 0ULL;
				if ((U))
				{
					if ((((uint64_t)element1 > (uint64_t)element2)))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				else
				{
					if ((((uint64_t)(sign_extend((uint64_t)((uint64_t)element1)) > sign_extend((uint64_t)((uint64_t)element2))))))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				uint128_t::insert(&result, e, esize, working);
			}
			else if ((((uint64_t)opcode == (uint64_t)13ULL)))
			{
				uint64_t working = 0ULL;
				if ((U))
				{
					if ((((uint64_t)element1 < (uint64_t)element2)))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				else
				{
					if ((((uint64_t)(sign_extend((uint64_t)((uint64_t)element1)) < sign_extend((uint64_t)((uint64_t)element2))))))
					{
						working = element1;
					}
					else
					{
						working = element2;
					}
				}
				uint128_t::insert(&result, e, esize, working);
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)(((uint64_t)(((uint64_t)size >> (uint64_t)1ULL)) == (uint64_t)1ULL))) && (uint64_t)((uint64_t)U == (uint64_t)0ULL))))
			{
				if ((((uint64_t)size & (uint64_t)1ULL)))
				{
					element2 = ~element2;
				}
				uint128_t::insert(&result, e, esize, (((uint64_t)element1 | (uint64_t)element2)));
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)(((uint64_t)(((uint64_t)size >> (uint64_t)1ULL)) == (uint64_t)0ULL))) && (uint64_t)((uint64_t)U == (uint64_t)0ULL))))
			{
				if ((((uint64_t)size & (uint64_t)1ULL)))
				{
					element2 = ~element2;
				}
				uint128_t::insert(&result, e, esize, (((uint64_t)element1 & (uint64_t)element2)));
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)((uint64_t)size == (uint64_t)0ULL)) && (uint64_t)((uint64_t)U == (uint64_t)1ULL))))
			{
				uint128_t::insert(&result, e, esize, (((uint64_t)element1 ^ (uint64_t)element2)));
			}
			else
			{
				undefined_with_interpreter(ctx,0ULL);
			}
		}
		V_interpreter(ctx,Rd,result);
	}
	
}

void floating_point_conditional_select_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t Rd)
{
	uint64_t fltsize = get_flt_size_interpreter(ctx,ftype);
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	if (fltsize == 32ULL)
	{
		uint32_t result = 0ULL;
		if ((condition_holds_interpreter(ctx,cond)))
		{
			result = operand1;
		}
		else
		{
			result = operand2;
		}
		V_interpreter(ctx,Rd,(uint128_t)result);
	}
	if (fltsize == 64ULL)
	{
		uint64_t result = 0ULL;
		if ((condition_holds_interpreter(ctx,cond)))
		{
			result = operand1;
		}
		else
		{
			result = operand2;
		}
		V_interpreter(ctx,Rd,(uint128_t)result);
	}
	
}

void fmov_scalar_immediate_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t imm8, uint64_t Rd)
{
	uint64_t fltsize = get_flt_size_interpreter(ctx,ftype);
	uint64_t imm = vfp_expand_imm_interpreter(ctx,imm8,fltsize);
	V_interpreter(ctx,Rd,(uint128_t)imm);
}

void fcvt_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t opc, uint64_t Rn, uint64_t Rd)
{
	uint64_t srcsize = get_flt_size_interpreter(ctx,ftype);
	uint64_t dstsize = get_flt_size_interpreter(ctx,opc);
	uint64_t operand = V_interpreter(ctx,Rn);
	V_interpreter(ctx,Rd,(uint128_t)FPConvert_interpreter(ctx,operand,dstsize,srcsize));
}

void floating_point_data_processing_one_source_interpreter(interpreter_data* ctx, uint64_t M, uint64_t S, uint64_t ftype, uint64_t opcode, uint64_t Rn, uint64_t Rd)
{
	uint64_t fltsize = get_flt_size_interpreter(ctx,ftype);
	uint64_t operand = V_interpreter(ctx,Rn);
	uint64_t result = 0ULL;
	uint64_t fpcr_state = _sys_interpreter(ctx,fpcr);
	if ((((uint64_t)opcode == (uint64_t)1ULL)))
	{
		result = FPAbs_interpreter(ctx,operand,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)2ULL)))
	{
		result = FPNeg_interpreter(ctx,operand,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)3ULL)))
	{
		result = FPSqrt_interpreter(ctx,operand,fpcr_state,fltsize);
	}
	else
	{
		undefined_with_interpreter(ctx,0ULL);
	}
	uint128_t vector = 0;
	uint128_t::insert(&vector, 0ULL, fltsize, result);
	V_interpreter(ctx,Rd,vector);
}

void fcmp_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t opc)
{
	uint64_t datasize = get_flt_size_interpreter(ctx,ftype);
	uint64_t cmp_with_zero = ((uint64_t)opc == (uint64_t)1ULL);
	uint64_t operand1 = V_interpreter(ctx,Rn);
	uint64_t operand2;
	uint64_t fpcr_state = _sys_interpreter(ctx,fpcr);
	if ((cmp_with_zero))
	{
		operand2 = 0ULL;
	}
	else
	{
		operand2 = V_interpreter(ctx,Rm);
	}
	uint64_t nzcv = FPCompare_interpreter(ctx,operand1,operand2,fpcr_state,datasize);
	_sys_interpreter(ctx,nzcv_n,((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL));
	_sys_interpreter(ctx,nzcv_z,((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL));
	_sys_interpreter(ctx,nzcv_c,((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL));
	_sys_interpreter(ctx,nzcv_v,((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL));
}

void fccmp_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t nzcv)
{
	uint64_t datasize = get_flt_size_interpreter(ctx,ftype);
	uint64_t operand1 = V_interpreter(ctx,Rn);
	uint64_t operand2 = V_interpreter(ctx,Rm);
	uint64_t fpcr_state = _sys_interpreter(ctx,fpcr);
	if ((condition_holds_interpreter(ctx,cond)))
	{
		uint64_t success_nzcv = FPCompare_interpreter(ctx,operand1,operand2,fpcr_state,datasize);
		_sys_interpreter(ctx,nzcv_n,((uint64_t)(((uint64_t)success_nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL));
		_sys_interpreter(ctx,nzcv_z,((uint64_t)(((uint64_t)success_nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL));
		_sys_interpreter(ctx,nzcv_c,((uint64_t)(((uint64_t)success_nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL));
		_sys_interpreter(ctx,nzcv_v,((uint64_t)(((uint64_t)success_nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL));
	}
	else
	{
		_sys_interpreter(ctx,nzcv_n,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL));
		_sys_interpreter(ctx,nzcv_z,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL));
		_sys_interpreter(ctx,nzcv_c,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL));
		_sys_interpreter(ctx,nzcv_v,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL));
	}
}

void fcvtz_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_interpreter(ctx,sf,ftype,Rd,Rn,FPRounding_ZERO,U,0ULL);
}

void fcvtn_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_interpreter(ctx,sf,ftype,Rd,Rn,FPRounding_TIEEVEN,U,0ULL);
}

void fcvta_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_interpreter(ctx,sf,ftype,Rd,Rn,FPRounding_TIEAWAY,U,0ULL);
}

void fcvtm_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_interpreter(ctx,sf,ftype,Rd,Rn,FPRounding_NEGINF,U,0ULL);
}

void fcvtp_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_interpreter(ctx,sf,ftype,Rd,Rn,FPRounding_POSINF,U,0ULL);
}

void addlv_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t is_unsigned = U;
	uint128_t source = V_interpreter(ctx,Rn);
	uint128_t operand = V_interpreter(ctx,Rn);
	uint64_t sum = 0ULL;
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint64_t working = (uint128_t::extract(operand, e, esize));
		if ((!is_unsigned))
		{
			if (esize == 8ULL)
			{
				working = (uint64_t)sign_extend((uint8_t)working);
			}
			if (esize == 16ULL)
			{
				working = (uint64_t)sign_extend((uint16_t)working);
			}
			if (esize == 32ULL)
			{
				working = (uint64_t)sign_extend((uint32_t)working);
			}
			if (esize == 64ULL)
			{
				working = (uint64_t)sign_extend((uint64_t)working);
			}
			
		}
		sum = ((uint64_t)sum + (uint64_t)working);
	}
	uint128_t final = 0;
	uint128_t::insert(&final, 0ULL, (((uint64_t)2ULL * (uint64_t)esize)), sum);
	V_interpreter(ctx,Rd,final);
}

void cnt_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = 8ULL;
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)8ULL);
	uint128_t source = V_interpreter(ctx,Rn);
	uint128_t result = 0;
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint8_t working = uint128_t::extract(source, e, esize);
		uint8_t count = 0ULL;
		for (uint64_t b = 0; b < (esize); b++)
		{
			uint8_t bit = ((uint8_t)(((uint8_t)working >> (uint8_t)b)) & (uint8_t)1ULL);
			if ((bit))
			{
				count = (((uint8_t)count + (uint8_t)1ULL));
			}
		}
		uint128_t::insert(&result, e, esize, count);
	}
	V_interpreter(ctx,Rd,result);
}

uint64_t compare_and_swap_interpreter(interpreter_data* ctx, uint64_t address, uint64_t expecting, uint64_t to_swap, uint64_t size)
{
	address = translate_address_interpreter(ctx,address);
	return _compare_and_swap_interpreter(ctx,address,expecting,to_swap,size);
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

void msr_register_interpreter(interpreter_data* ctx, uint64_t imm15, uint64_t Rt)
{
	uint64_t operand = X_interpreter(ctx,Rt);
	if ((((uint64_t)imm15 == (uint64_t)23072ULL)))
	{
		_sys_interpreter(ctx,fpcr,operand);
	}
	else if ((((uint64_t)imm15 == (uint64_t)23073ULL)))
	{
		_sys_interpreter(ctx,fpsr,operand);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24194ULL)))
	{
		_sys_interpreter(ctx,thread_local_1,operand);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24195ULL)))
	{
		_sys_interpreter(ctx,thread_local_0,operand);
	}
	else
	{
		undefined_with_interpreter(ctx,imm15);
	}
}

void mrs_register_interpreter(interpreter_data* ctx, uint64_t imm15, uint64_t Rt)
{
	uint64_t operand;
	if ((((uint64_t)imm15 == (uint64_t)23072ULL)))
	{
		operand = _sys_interpreter(ctx,fpcr);
	}
	else if ((((uint64_t)imm15 == (uint64_t)23073ULL)))
	{
		operand = _sys_interpreter(ctx,fpsr);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24194ULL)))
	{
		operand = _sys_interpreter(ctx,thread_local_1);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24195ULL)))
	{
		operand = _sys_interpreter(ctx,thread_local_0);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24321ULL)))
	{
		operand = call_counter_interpreter(ctx);
	}
	else if ((((uint64_t)imm15 == (uint64_t)22529ULL)))
	{
		operand = 2219098116ULL;
	}
	else
	{
		undefined_with_interpreter(ctx,imm15);
	}
	X_interpreter(ctx,Rt,operand);
}

void hints_interpreter(interpreter_data* ctx, uint64_t imm7)
{
}

void sys_interpreter(interpreter_data* ctx, uint64_t L, uint64_t imm19)
{
}

void barriers_interpreter(interpreter_data* ctx, uint64_t CRm, uint64_t op2, uint64_t Rt)
{
	if ((((uint64_t)((uint64_t)op2 == (uint64_t)2ULL) && (uint64_t)((uint64_t)Rt == (uint64_t)31ULL))))
	{
		_sys_interpreter(ctx,exclusive_address,(uint64_t)-1ULL);
		_sys_interpreter(ctx,exclusive_value,(uint64_t)-1ULL);
	}
}

void add_subtract_imm12_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t sh, uint64_t imm12, uint64_t Rn, uint64_t Rd)
{
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
	{
		ir_operand operand1 = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand operand2 = copy_new_raw_size(ctx, a_shift_reg_jit(ctx,O,Rm,shift,shift_ammount), O);
		ir_operand result = copy_new_raw_size(ctx, add_subtract_impl_jit(ctx,O,operand1,operand2,S,((uint64_t)op == (uint64_t)0ULL)), O);
		X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void add_subtract_extended_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t option, uint64_t imm3, uint64_t Rn, uint64_t Rd)
{
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
	{
		ir_operand operand1 = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand operand2 = copy_new_raw_size(ctx, X_jit(ctx,Rm), O);
		ir_operand result = copy_new_raw_size(ctx, add_subtract_carry_impl_jit(ctx,O,operand1,operand2,((uint64_t)S == (uint64_t)1ULL),((uint64_t)op == (uint64_t)0ULL),copy_new_raw_size(ctx, _sys_jit(ctx,nzcv_c), O)), O);
		X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void shift_variable_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rm, uint64_t op2, uint64_t Rn, uint64_t Rd)
{
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
			_sys_jit(ctx,nzcv_n,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, result, ir_operand::create_con(0ULL, O)), int64));
			_sys_jit(ctx,nzcv_z,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, result, ir_operand::create_con(0ULL, O)), int64));
			_sys_jit(ctx,nzcv_c,ir_operand::create_con(0ULL, int64));
			_sys_jit(ctx,nzcv_v,ir_operand::create_con(0ULL, int64));
			return;
		}
		XSP_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
	}
}

void logical_shifted_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t shift, uint64_t N, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd)
{
	uint64_t shift_type = shift;
	uint64_t shift_ammount = imm6;
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
				_sys_jit(ctx,nzcv_n,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, result, ir_operand::create_con(0ULL, O)), int64));
				_sys_jit(ctx,nzcv_z,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, result, ir_operand::create_con(0ULL, O)), int64));
				_sys_jit(ctx,nzcv_c,ir_operand::create_con(0ULL, int64));
				_sys_jit(ctx,nzcv_v,ir_operand::create_con(0ULL, int64));
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
	{
		{
		    ir_operand end = ir_operation_block::create_label(ctx->ir);
		    ir_operand yes = ir_operation_block::create_label(ctx->ir);
		
		    ir_operand condition = condition_holds_jit(ctx,cond);
		
		    ir_operation_block::jump_if(ctx->ir,yes, condition);
			{
				_sys_jit(ctx,nzcv_n,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL), int64));
				_sys_jit(ctx,nzcv_z,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL), int64));
				_sys_jit(ctx,nzcv_c,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL), int64));
				_sys_jit(ctx,nzcv_v,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL), int64));
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
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
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
		instruction_pc = ((uint64_t)instruction_pc & (uint64_t)~4095ULL);
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
	{
		uint64_t S = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : datasize == 128ULL ? int128 : 0;
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
		}
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
		{
			uint64_t O = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : datasize == 128ULL ? int128 : 0;
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
			}
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
		{
			uint64_t S = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : 0;
			{
				uint64_t R = regsize == 32ULL ? int32 : regsize == 64ULL ? int64 : 0;
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
				}
			}
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
		{
			uint64_t O = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : datasize == 128ULL ? int128 : 0;
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
			}
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
		{
			uint64_t S = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : 0;
			{
				uint64_t R = regsize == 32ULL ? int32 : regsize == 64ULL ? int64 : 0;
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
				}
			}
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

void load_store_register_offset_jit(ssa_emit_context* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t Rm, uint64_t option, uint64_t S, uint64_t Rn, uint64_t Rt)
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
		uint64_t shift = ((uint64_t)scale * (uint64_t)S);
		ir_operand offset = a_extend_reg_64_jit(ctx,Rm,option,shift);
		if ((((uint64_t)opc & (uint64_t)1ULL)))
		memop = 0ULL;
		else
		memop = 1ULL;
		uint64_t datasize = ((uint64_t)8ULL << (uint64_t)scale);
		ir_operand address = ssa_emit_context::emit_ssa(ctx, ir_add, XSP_jit(ctx,Rn), offset);
		uint64_t O = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : datasize == 128ULL ? int128 : 0;
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
		}
	}
	else
	{
		uint64_t memop;
		uint64_t regsize;
		uint64_t _signed;
		uint64_t scale = size;
		uint64_t shift = ((uint64_t)scale * (uint64_t)S);
		ir_operand offset = a_extend_reg_64_jit(ctx,Rm,option,shift);
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
		ir_operand address = ssa_emit_context::emit_ssa(ctx, ir_add, XSP_jit(ctx,Rn), offset);
		uint64_t S = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : 0;
		{
			uint64_t R = regsize == 32ULL ? int32 : regsize == 64ULL ? int64 : 0;
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
			}
		}
	}
}

void load_store_exclusive_ordered_jit(ssa_emit_context* ctx, uint64_t size, uint64_t ordered, uint64_t L, uint64_t Rs, uint64_t o0, uint64_t Rn, uint64_t Rt)
{
	uint64_t is_exclusive = ((uint64_t)ordered == (uint64_t)0ULL);
	if ((L))
	{
		load_exclusive_jit(ctx,is_exclusive,size,Rn,Rt);
	}
	else
	{
		store_exclusive_jit(ctx,is_exclusive,size,Rn,Rt,Rs);
	}
}

void load_exclusive_jit(ssa_emit_context* ctx, uint64_t is_exclusive, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	uint64_t datasize = ((uint64_t)8ULL << (uint64_t)size);
	ir_operand address = XSP_jit(ctx,Rn);
	uint64_t S = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : 0;
	{
		ir_operand value = copy_new_raw_size(ctx, mem_jit(ctx,S,address), S);
		if ((is_exclusive))
		{
			_sys_jit(ctx,exclusive_address,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, address, ir_operand::create_con(~63ULL, int64)));
			_sys_jit(ctx,exclusive_value,copy_new_raw_size(ctx, value, int64));
		}
		X_jit(ctx,Rt,copy_new_raw_size(ctx, value, int64));
	}
}

void store_exclusive_jit(ssa_emit_context* ctx, uint64_t is_exclusive, uint64_t size, uint64_t Rn, uint64_t Rt, uint64_t Rs)
{
	uint64_t datasize = ((uint64_t)8ULL << (uint64_t)size);
	ir_operand address = XSP_jit(ctx,Rn);
	uint64_t S = datasize == 8ULL ? int8 : datasize == 16ULL ? int16 : datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : 0;
	{
		if ((is_exclusive))
		{
			ir_operand mask = ir_operand::create_con(~63ULL, int64);
			ir_operand _exclusive_address = _sys_jit(ctx,exclusive_address);
			{
			    ir_operand end = ir_operation_block::create_label(ctx->ir);
			    ir_operand yes = ir_operation_block::create_label(ctx->ir);
			
			    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_equal, _exclusive_address, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, address, mask));
			
			    ir_operation_block::jump_if(ctx->ir,yes, condition);
				{
					X_jit(ctx,Rs,ir_operand::create_con(1ULL, int64));
				}
			    
			    ir_operation_block::jump(ctx->ir,end);
			    ir_operation_block::mark_label(ctx->ir, yes);
			
				{
					ir_operand to_swap = copy_new_raw_size(ctx, X_jit(ctx,Rt), S);
					ir_operand expecting = copy_new_raw_size(ctx, _sys_jit(ctx,exclusive_value), S);
					ir_operand cas_success = copy_new_raw_size(ctx, compare_and_swap_jit(ctx,address,copy_new_raw_size(ctx, expecting, int64),copy_new_raw_size(ctx, to_swap, int64),datasize), S);
					X_jit(ctx,Rs,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, cas_success, ir_operand::create_con(1ULL, S)), int64));
				}
			
			    ir_operation_block::mark_label(ctx->ir, end);
			}
		}
		else
		{
			mem_jit(ctx,S,address,copy_new_raw_size(ctx, X_jit(ctx,Rt), S));
		}
	}
}

void branch_register_jit(ssa_emit_context* ctx, uint64_t l, uint64_t Rn)
{
	branch_long_universal_jit(ctx,Rn,l);
}

void return_register_jit(ssa_emit_context* ctx, uint64_t Rn)
{
	branch_long_universal_jit(ctx,Rn,0ULL);
}

void test_bit_branch_jit(ssa_emit_context* ctx, uint64_t b5, uint64_t op, uint64_t b40, uint64_t imm14, uint64_t Rt)
{
	uint64_t bit_pos = ((uint64_t)b40 + (uint64_t)(((uint64_t)b5 << (uint64_t)5ULL)));
	uint64_t new_location = ((uint64_t)_get_pc_jit(ctx) + (uint64_t)(((uint64_t)sign_extend_jit(ctx,imm14,14ULL) << (uint64_t)2ULL)));
	uint64_t next_location = ((uint64_t)_get_pc_jit(ctx) + (uint64_t)4ULL);
	ir_operand src = X_jit(ctx,Rt);
	ir_operand branch_pass = copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, src, ir_operand::create_con(bit_pos, int64)), ir_operand::create_con(1ULL, int64)), ir_operand::create_con(op, int64)), int8);
	_branch_conditional_jit(ctx,new_location,next_location,copy_new_raw_size(ctx, branch_pass, int64));
}

void compare_and_branch_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t imm19, uint64_t Rt)
{
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
	{
		uint64_t new_location = ((uint64_t)_get_pc_jit(ctx) + (uint64_t)(((uint64_t)sign_extend_jit(ctx,imm19,19ULL) << (uint64_t)2ULL)));
		uint64_t next_location = ((uint64_t)_get_pc_jit(ctx) + (uint64_t)4ULL);
		ir_operand operand = copy_new_raw_size(ctx, X_jit(ctx,Rt), O);
		ir_operand branch_pass;
		if ((!op))
		{
			branch_pass = copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, operand, ir_operand::create_con(0ULL, O)), int8);
		}
		else
		{
			branch_pass = copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_not_equal, operand, ir_operand::create_con(0ULL, O)), int8);
		}
		_branch_conditional_jit(ctx,new_location,next_location,copy_new_raw_size(ctx, branch_pass, int64));
	}
}

void b_unconditional_jit(ssa_emit_context* ctx, uint64_t op, uint64_t imm26)
{
	uint64_t new_location = ((uint64_t)_get_pc_jit(ctx) + (uint64_t)(((uint64_t)sign_extend_jit(ctx,imm26,26ULL) << (uint64_t)2ULL)));
	if ((op))
	{
		uint64_t next_location = ((uint64_t)_get_pc_jit(ctx) + (uint64_t)4ULL);
		X_jit(ctx,30ULL,ir_operand::create_con(next_location, int64));
	}
	_branch_short_jit(ctx,new_location);
}

void b_conditional_jit(ssa_emit_context* ctx, uint64_t imm19, uint64_t cond)
{
	uint64_t new_location = ((uint64_t)_get_pc_jit(ctx) + (uint64_t)(((uint64_t)sign_extend_jit(ctx,imm19,19ULL) << (uint64_t)2ULL)));
	uint64_t next_location = ((uint64_t)_get_pc_jit(ctx) + (uint64_t)4ULL);
	_branch_conditional_jit(ctx,new_location,next_location,copy_new_raw_size(ctx, condition_holds_jit(ctx,cond), int64));
}

void svc_jit(ssa_emit_context* ctx, uint64_t imm16)
{
	call_supervisor_jit(ctx,imm16);
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

ir_operand a_extend_reg_64_jit(ssa_emit_context* ctx, uint64_t m, uint64_t extend_type, uint64_t shift)
{
	ir_operand val = X_jit(ctx,m);
	if ((((uint64_t)extend_type == (uint64_t)0ULL)))
	{
		val = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, val, ir_operand::create_con(255ULL, int64));
	}
	else if ((((uint64_t)extend_type == (uint64_t)1ULL)))
	{
		val = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, val, ir_operand::create_con(65535ULL, int64));
	}
	else if ((((uint64_t)extend_type == (uint64_t)2ULL)))
	{
		val = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, val, ir_operand::create_con(4294967295ULL, int64));
	}
	else if ((((uint64_t)extend_type == (uint64_t)4ULL)))
	{
		val = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, val, int8), int64);
	}
	else if ((((uint64_t)extend_type == (uint64_t)5ULL)))
	{
		val = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, val, int16), int64);
	}
	else if ((((uint64_t)extend_type == (uint64_t)6ULL)))
	{
		val = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, val, int32), int64);
	}
	return ssa_emit_context::emit_ssa(ctx, ir_shift_left, val, ir_operand::create_con(shift, int64));
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

ir_operand vector_shift_jit(ssa_emit_context* ctx,uint64_t O, ir_operand element, ir_operand shift, uint64_t bit_count, uint64_t is_unsigned)
{
	ir_operand result = ssa_emit_context::emit_ssa(ctx, ir_move, ir_operand::create_con(0ULL, O));
	if ((!is_unsigned))
	{
		element = copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx,ir_sign_extend,element, int64), O);
	}
	{
	    ir_operand end = ir_operation_block::create_label(ctx->ir);
	    ir_operand yes = ir_operation_block::create_label(ctx->ir);
	
	    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_equal_signed, shift, ir_operand::create_con(0ULL, O));
	
	    ir_operation_block::jump_if(ctx->ir,yes, condition);
		{
			ssa_emit_context::move(ctx,shift,ssa_emit_context::emit_ssa(ctx, ir_negate, shift));
			{
			    ir_operand end = ir_operation_block::create_label(ctx->ir);
			    ir_operand yes = ir_operation_block::create_label(ctx->ir);
			
			    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_equal_signed, shift, ir_operand::create_con(bit_count, O));
			
			    ir_operation_block::jump_if(ctx->ir,yes, condition);
				{
					if ((!is_unsigned))
					{
						ssa_emit_context::move(ctx,result,ssa_emit_context::emit_ssa(ctx, ir_shift_right_signed, element, shift));
					}
					else
					{
						ssa_emit_context::move(ctx,result,ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, element, shift));
					}
				}
			    
			    ir_operation_block::jump(ctx->ir,end);
			    ir_operation_block::mark_label(ctx->ir, yes);
			
				{
					{
					    ir_operand end = ir_operation_block::create_label(ctx->ir);
					    ir_operand yes = ir_operation_block::create_label(ctx->ir);
					
					    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, element, ir_operand::create_con((((uint64_t)bit_count - (uint64_t)1ULL)), O)), ir_operand::create_con(1ULL, O)), ir_operand::create_con(1ULL, O)), ir_operand::create_con(!is_unsigned, O));
					
					    ir_operation_block::jump_if(ctx->ir,yes, condition);
						{
							ssa_emit_context::move(ctx,result,ir_operand::create_con(0ULL, O));
						}
					    
					    ir_operation_block::jump(ctx->ir,end);
					    ir_operation_block::mark_label(ctx->ir, yes);
					
						{
							ssa_emit_context::move(ctx,result,ir_operand::create_con(-1ULL, O));
						}
					
					    ir_operation_block::mark_label(ctx->ir, end);
					}
				}
			
			    ir_operation_block::mark_label(ctx->ir, end);
			}
		}
	    
	    ir_operation_block::jump(ctx->ir,end);
	    ir_operation_block::mark_label(ctx->ir, yes);
	
		{
			{
			    ir_operand end = ir_operation_block::create_label(ctx->ir);
			    ir_operand yes = ir_operation_block::create_label(ctx->ir);
			
			    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_equal_signed, shift, ir_operand::create_con(bit_count, O));
			
			    ir_operation_block::jump_if(ctx->ir,yes, condition);
				{
					ssa_emit_context::move(ctx,result,ssa_emit_context::emit_ssa(ctx, ir_shift_left, element, shift));
				}
			    
			    ir_operation_block::jump(ctx->ir,end);
			    ir_operation_block::mark_label(ctx->ir, yes);
			
				{
					ssa_emit_context::move(ctx,result,ir_operand::create_con(0ULL, O));
				}
			
			    ir_operation_block::mark_label(ctx->ir, end);
			}
		}
	
	    ir_operation_block::mark_label(ctx->ir, end);
	}
	return result;
}

void convert_to_int_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t Rd, uint64_t Rn, uint64_t round, uint64_t is_unsigned, uint64_t to_vector)
{
	ir_operand operand = copy_new_raw_size(ctx, V_jit(ctx,Rn), int64);
	uint64_t intsize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t fltsize = get_flt_size_jit(ctx,ftype);
	ir_operand result = FPToFixed_jit(ctx,operand,0ULL,is_unsigned,round,intsize,fltsize);
	if ((to_vector))
	{
		V_jit(ctx,Rd,copy_new_raw_size(ctx, result, int128));
	}
	else
	{
		X_jit(ctx,Rd,result);
	}
}

uint64_t lowest_bit_set_c_jit(ssa_emit_context* ctx, uint64_t source)
{
	uint64_t size = 32ULL;
	for (uint64_t i = 0; i < (size); i++)
	{
		uint64_t working_bit = ((uint64_t)(((uint64_t)source >> (uint64_t)i)) & (uint64_t)1ULL);
		if ((working_bit))
		{
			return i;
		}
	}
	return size;
}

void dup_element_jit(ssa_emit_context* ctx, uint64_t index, uint64_t esize, uint64_t elements, uint64_t n, uint64_t d)
{
	ir_operand operand = V_jit(ctx,n);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	ir_operand element = ssa_emit_context::vector_extract(ctx,operand, index, esize);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ssa_emit_context::vector_insert(ctx,result, e, esize, element);
	}
	V_jit(ctx,d,result);
}

uint64_t get_flt_size_jit(ssa_emit_context* ctx, uint64_t ftype)
{
	if ((((uint64_t)ftype == (uint64_t)2ULL)))
	{
		return 64ULL;
	}
	else
	{
		return ((uint64_t)8ULL << (uint64_t)(((uint64_t)ftype ^ (uint64_t)2ULL)));
	}
}

uint64_t vfp_expand_imm_jit(ssa_emit_context* ctx, uint64_t imm8, uint64_t N)
{
	uint64_t E;
	if ((((uint64_t)N == (uint64_t)16ULL)))
	{
		E = 5ULL;
	}
	else if ((((uint64_t)N == (uint64_t)32ULL)))
	{
		E = 8ULL;
	}
	else
	{
		E = 11ULL;
	}
	uint64_t F = ((uint64_t)(((uint64_t)N - (uint64_t)E)) - (uint64_t)1ULL);
	uint64_t sign = ((uint64_t)(((uint64_t)imm8 >> (uint64_t)7ULL)) & (uint64_t)1ULL);
	uint64_t exp = ((uint64_t)~(bit_c_jit(ctx,imm8,6ULL)) & (uint64_t)1ULL);
	exp = ((uint64_t)(((uint64_t)exp << (uint64_t)(((uint64_t)E - (uint64_t)3ULL)))) | (uint64_t)replicate_c_jit(ctx,bit_c_jit(ctx,imm8,6ULL),1ULL,((uint64_t)E - (uint64_t)3ULL)));
	exp = ((uint64_t)(((uint64_t)exp << (uint64_t)2ULL)) | (uint64_t)bits_c_jit(ctx,imm8,5ULL,4ULL));
	uint64_t frac = ((uint64_t)bits_c_jit(ctx,imm8,3ULL,0ULL) << (uint64_t)(((uint64_t)F - (uint64_t)4ULL)));
	uint64_t result = sign;
	result = ((uint64_t)(((uint64_t)result << (uint64_t)(((uint64_t)((uint64_t)1ULL + (uint64_t)(((uint64_t)E - (uint64_t)3ULL))) + (uint64_t)2ULL)))) | (uint64_t)exp);
	result = ((uint64_t)(((uint64_t)result << (uint64_t)(((uint64_t)4ULL + (uint64_t)(((uint64_t)F - (uint64_t)4ULL)))))) | (uint64_t)frac);
	return result;
}

uint64_t expand_imm_jit(ssa_emit_context* ctx, uint64_t op, uint64_t cmode, uint64_t imm8)
{
	uint64_t imm64 = 0ULL;
	uint64_t cmode_test = bits_c_jit(ctx,cmode,3ULL,1ULL);
	if ((((uint64_t)cmode_test == (uint64_t)0ULL)))
	{
		imm64 = replicate_c_jit(ctx,imm8,32ULL,2ULL);
	}
	else if ((((uint64_t)cmode_test == (uint64_t)1ULL)))
	{
		imm64 = replicate_c_jit(ctx,((uint64_t)imm8 << (uint64_t)8ULL),32ULL,2ULL);
	}
	else if ((((uint64_t)cmode_test == (uint64_t)2ULL)))
	{
		imm64 = replicate_c_jit(ctx,((uint64_t)imm8 << (uint64_t)16ULL),32ULL,2ULL);
	}
	else if ((((uint64_t)cmode_test == (uint64_t)3ULL)))
	{
		imm64 = replicate_c_jit(ctx,((uint64_t)imm8 << (uint64_t)24ULL),32ULL,2ULL);
	}
	else if ((((uint64_t)cmode_test == (uint64_t)4ULL)))
	{
		imm64 = replicate_c_jit(ctx,imm8,16ULL,4ULL);
	}
	else if ((((uint64_t)cmode_test == (uint64_t)5ULL)))
	{
		imm64 = replicate_c_jit(ctx,((uint64_t)imm8 << (uint64_t)8ULL),16ULL,4ULL);
	}
	else if ((((uint64_t)cmode_test == (uint64_t)6ULL)))
	{
		if ((((uint64_t)(((uint64_t)cmode & (uint64_t)1ULL)) == (uint64_t)0ULL)))
		{
			imm64 = replicate_c_jit(ctx,((uint64_t)(((uint64_t)imm8 << (uint64_t)8ULL)) | (uint64_t)ones_jit(ctx,8ULL)),32ULL,2ULL);
		}
		else
		{
			imm64 = replicate_c_jit(ctx,((uint64_t)(((uint64_t)imm8 << (uint64_t)16ULL)) | (uint64_t)ones_jit(ctx,16ULL)),32ULL,2ULL);
		}
	}
	else if ((((uint64_t)cmode_test == (uint64_t)7ULL)))
	{
		if ((((uint64_t)((uint64_t)bit_c_jit(ctx,cmode,0ULL) == (uint64_t)0ULL) && (uint64_t)((uint64_t)op == (uint64_t)0ULL))))
		{
			imm64 = replicate_c_jit(ctx,imm8,8ULL,8ULL);
		}
		else if ((((uint64_t)((uint64_t)bit_c_jit(ctx,cmode,0ULL) == (uint64_t)0ULL) && (uint64_t)((uint64_t)op == (uint64_t)1ULL))))
		{
			for (uint64_t i = 0; i < (8ULL); i++)
			{
				uint64_t part = ((uint64_t)(((uint64_t)0ULL - (uint64_t)(((uint64_t)(((uint64_t)imm8 >> (uint64_t)i)) & (uint64_t)1ULL)))) & (uint64_t)255ULL);
				imm64 = ((uint64_t)imm64 | (uint64_t)(((uint64_t)part << (uint64_t)(((uint64_t)i * (uint64_t)8ULL)))));
			}
		}
		else if ((((uint64_t)((uint64_t)bit_c_jit(ctx,cmode,0ULL) == (uint64_t)1ULL) && (uint64_t)((uint64_t)op == (uint64_t)0ULL))))
		{
			uint64_t p0 = bit_c_jit(ctx,imm8,7ULL);
			uint64_t p1 = ((uint64_t)(~(bit_c_jit(ctx,imm8,6ULL))) & (uint64_t)1ULL);
			uint64_t p2 = replicate_c_jit(ctx,bit_c_jit(ctx,imm8,6ULL),1ULL,5ULL);
			uint64_t p3 = bits_c_jit(ctx,imm8,5ULL,0ULL);
			uint64_t p4 = 0ULL;
			uint64_t working = ((uint64_t)((uint64_t)((uint64_t)((uint64_t)p4 | (uint64_t)(((uint64_t)p3 << (uint64_t)19ULL))) | (uint64_t)(((uint64_t)p2 << (uint64_t)(((uint64_t)19ULL + (uint64_t)6ULL))))) | (uint64_t)(((uint64_t)p1 << (uint64_t)(((uint64_t)((uint64_t)19ULL + (uint64_t)6ULL) + (uint64_t)5ULL))))) | (uint64_t)(((uint64_t)p0 << (uint64_t)(((uint64_t)((uint64_t)((uint64_t)19ULL + (uint64_t)6ULL) + (uint64_t)5ULL) + (uint64_t)1ULL)))));
			imm64 = replicate_c_jit(ctx,working,32ULL,2ULL);
		}
		else if ((((uint64_t)((uint64_t)bit_c_jit(ctx,cmode,0ULL) == (uint64_t)1ULL) && (uint64_t)((uint64_t)op == (uint64_t)1ULL))))
		{
			uint64_t p0 = bit_c_jit(ctx,imm8,7ULL);
			uint64_t p1 = ((uint64_t)(~(bit_c_jit(ctx,imm8,6ULL))) & (uint64_t)1ULL);
			uint64_t p2 = replicate_c_jit(ctx,bit_c_jit(ctx,imm8,6ULL),1ULL,8ULL);
			uint64_t p3 = bits_c_jit(ctx,imm8,5ULL,0ULL);
			uint64_t p4 = 0ULL;
			imm64 = ((uint64_t)((uint64_t)((uint64_t)((uint64_t)p4 | (uint64_t)(((uint64_t)p3 << (uint64_t)48ULL))) | (uint64_t)(((uint64_t)p2 << (uint64_t)(((uint64_t)48ULL + (uint64_t)6ULL))))) | (uint64_t)(((uint64_t)p1 << (uint64_t)(((uint64_t)((uint64_t)48ULL + (uint64_t)6ULL) + (uint64_t)8ULL))))) | (uint64_t)(((uint64_t)p0 << (uint64_t)(((uint64_t)((uint64_t)((uint64_t)48ULL + (uint64_t)6ULL) + (uint64_t)8ULL) + (uint64_t)1ULL)))));
		}
		else
		{
			undefined_jit(ctx);
		}
	}
	else
	{
		undefined_jit(ctx);
	}
	return imm64;
}

void VPart_jit(ssa_emit_context* ctx, uint64_t n, uint64_t part, uint64_t width, ir_operand value)
{
	if ((((uint64_t)part == (uint64_t)0ULL)))
	{
		V_jit(ctx,n,copy_new_raw_size(ctx, value, int128));
	}
	else
	{
		ir_operand src = V_jit(ctx,n);
		ssa_emit_context::vector_insert(ctx,src, 1ULL, 64ULL, value);
		V_jit(ctx,n,src);
	}
}

void dup_general_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_jit(ctx,bits_c_jit(ctx,imm5,3ULL,0ULL));
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	ir_operand element = X_jit(ctx,Rn);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ssa_emit_context::vector_insert(ctx,result, e, esize, element);
	}
	V_jit(ctx,Rd,result);
}

void dup_element_scalar_jit(ssa_emit_context* ctx, uint64_t imm5, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_jit(ctx,bits_c_jit(ctx,imm5,3ULL,0ULL));
	uint64_t index = bits_c_jit(ctx,imm5,4ULL,((uint64_t)size + (uint64_t)1ULL));
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)bit_c_jit(ctx,imm5,4ULL));
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = esize;
	uint64_t elements = 1ULL;
	dup_element_jit(ctx,index,esize,elements,Rn,Rd);
}

void dup_element_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_jit(ctx,bits_c_jit(ctx,imm5,3ULL,0ULL));
	uint64_t index = bits_c_jit(ctx,imm5,4ULL,((uint64_t)size + (uint64_t)1ULL));
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)bit_c_jit(ctx,imm5,4ULL));
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	dup_element_jit(ctx,index,esize,elements,Rn,Rd);
}

void move_to_gp_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t imm5, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_jit(ctx,bits_c_jit(ctx,imm5,3ULL,0ULL));
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)Q);
	uint64_t index = bits_c_jit(ctx,imm5,4ULL,((uint64_t)size + (uint64_t)1ULL));
	ir_operand operand = V_jit(ctx,Rn);
	uint64_t S = esize == 8ULL ? int8 : esize == 16ULL ? int16 : esize == 32ULL ? int32 : esize == 64ULL ? int64 : 0;
	{
		uint64_t R = datasize == 32ULL ? int32 : datasize == 64ULL ? int64 : 0;
		{
			ir_operand working_operand = copy_new_raw_size(ctx, ssa_emit_context::vector_extract(ctx,operand, index, esize), R);
			if ((!U))
			{
				working_operand = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, working_operand, S), R);
			}
			X_jit(ctx,Rd,copy_new_raw_size(ctx, working_operand, int64));
		}
	}
}

void ins_general_jit(ssa_emit_context* ctx, uint64_t imm5, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_jit(ctx,bits_c_jit(ctx,imm5,3ULL,0ULL));
	uint64_t index = bits_c_jit(ctx,imm5,4ULL,((uint64_t)size + (uint64_t)1ULL));
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	ir_operand element = X_jit(ctx,Rn);
	ir_operand result = V_jit(ctx,Rd);
	ssa_emit_context::vector_insert(ctx,result, index, esize, element);
	V_jit(ctx,Rd,result);
}

void ins_element_jit(ssa_emit_context* ctx, uint64_t imm5, uint64_t imm4, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_jit(ctx,bits_c_jit(ctx,imm5,3ULL,0ULL));
	uint64_t dst_index = bits_c_jit(ctx,imm5,4ULL,((uint64_t)size + (uint64_t)1ULL));
	uint64_t src_index = bits_c_jit(ctx,imm4,3ULL,size);
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	ir_operand operand = V_jit(ctx,Rn);
	ir_operand result = V_jit(ctx,Rd);
	ssa_emit_context::vector_insert(ctx,result, dst_index, esize, ssa_emit_context::vector_extract(ctx,operand, src_index, esize));
	V_jit(ctx,Rd,result);
}

void movi_immediate_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t op, uint64_t immhi, uint64_t cmode, uint64_t immlo, uint64_t Rd)
{
	uint64_t d = Rd;
	uint64_t imm8 = ((uint64_t)(((uint64_t)immhi << (uint64_t)5ULL)) | (uint64_t)immlo);
	uint64_t cmode_helper = ((uint64_t)(((uint64_t)cmode << (uint64_t)1ULL)) | (uint64_t)op);
	uint64_t mode = 0ULL;
	if ((((uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)19ULL)) == (uint64_t)2ULL)) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)27ULL)) == (uint64_t)18ULL)))))
	{
		mode = 0ULL;
	}
	else if ((((uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)19ULL)) == (uint64_t)3ULL)) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)27ULL)) == (uint64_t)19ULL)))))
	{
		mode = 1ULL;
	}
	else if ((((uint64_t)((uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)19ULL)) == (uint64_t)1ULL)) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)27ULL)) == (uint64_t)17ULL))) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)29ULL)) == (uint64_t)25ULL)))))
	{
		mode = 2ULL;
	}
	else if ((((uint64_t)((uint64_t)((uint64_t)((uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)19ULL)) == (uint64_t)0ULL)) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)27ULL)) == (uint64_t)16ULL))) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)29ULL)) == (uint64_t)24ULL))) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)30ULL)) == (uint64_t)28ULL))) || (uint64_t)(((uint64_t)(((uint64_t)cmode_helper & (uint64_t)31ULL)) == (uint64_t)30ULL)))))
	{
		mode = 3ULL;
	}
	else if ((((uint64_t)cmode_helper == (uint64_t)31ULL)))
	{
		if ((((uint64_t)Q == (uint64_t)0ULL)))
		{
			undefined_jit(ctx);
		}
		mode = 3ULL;
	}
	else
	{
		undefined_jit(ctx);
	}
	ir_operand imm64 = ir_operand::create_con(expand_imm_jit(ctx,op,cmode,imm8), int64);
	ir_operand imm = copy_new_raw_size(ctx, imm64, int128);
	if ((Q))
	{
		ssa_emit_context::vector_insert(ctx,imm, 1ULL, 64ULL, imm64);
	}
	ir_operand operand = ssa_emit_context::vector_zero(ctx);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	if ((((uint64_t)mode == (uint64_t)3ULL)))
	{
		result = imm;
	}
	else if ((((uint64_t)mode == (uint64_t)2ULL)))
	{
		for (uint64_t e = 0; e < (((uint64_t)Q + (uint64_t)1ULL)); e++)
		{
			ssa_emit_context::vector_insert(ctx,result, e, 64ULL, ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, ssa_emit_context::vector_extract(ctx,imm, e, 64ULL)));
		}
	}
	else if ((((uint64_t)mode == (uint64_t)0ULL)))
	{
		operand = V_jit(ctx,Rd);
		for (uint64_t e = 0; e < (((uint64_t)Q + (uint64_t)1ULL)); e++)
		{
			ssa_emit_context::vector_insert(ctx,result, e, 64ULL, ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, ssa_emit_context::vector_extract(ctx,operand, e, 64ULL), ssa_emit_context::vector_extract(ctx,imm, e, 64ULL)));
		}
	}
	else if ((((uint64_t)mode == (uint64_t)1ULL)))
	{
		operand = V_jit(ctx,Rd);
		for (uint64_t e = 0; e < (((uint64_t)Q + (uint64_t)1ULL)); e++)
		{
			ssa_emit_context::vector_insert(ctx,result, e, 64ULL, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::vector_extract(ctx,operand, e, 64ULL), ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, ssa_emit_context::vector_extract(ctx,imm, e, 64ULL))));
		}
	}
	else
	{
		undefined_jit(ctx);
	}
	V_jit(ctx,Rd,result);
}

void fmov_general_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t rmode, uint64_t opcode, uint64_t Rn, uint64_t Rd)
{
	uint64_t intsize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t fltsize = get_flt_size_jit(ctx,ftype);
	if ((((uint64_t)intsize != (uint64_t)fltsize)))
	{
		undefined_jit(ctx);
	}
	uint64_t size = intsize;
	uint64_t part = rmode;
	uint64_t int_to_float = opcode;
	if ((int_to_float))
	{
		ir_operand src = X_jit(ctx,Rn);
		uint64_t O = size == 32ULL ? int32 : size == 64ULL ? int64 : 0;
		{
			VPart_jit(ctx,Rd,part,size,copy_new_raw_size(ctx, copy_new_raw_size(ctx, src, O), int64));
		}
	}
	else
	{
		ir_operand v = V_jit(ctx,Rn);
		ir_operand src = ssa_emit_context::vector_extract(ctx,v, part, size);
		X_jit(ctx,Rd,src);
	}
}

void convert_to_float_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd, uint64_t from_vector)
{
	uint64_t intsize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t fltsize = get_flt_size_jit(ctx,ftype);
	uint64_t I = intsize == 32ULL ? int32 : intsize == 64ULL ? int64 : 0;
	{
		uint64_t F = fltsize == 16ULL ? int16 : fltsize == 32ULL ? int32 : fltsize == 64ULL ? int64 : 0;
		{
			ir_operand result;
			ir_operand operand;
			if ((from_vector))
			{
				operand = copy_new_raw_size(ctx, V_jit(ctx,Rn), I);
			}
			else
			{
				operand = copy_new_raw_size(ctx, X_jit(ctx,Rn), I);
			}
			if ((U))
			{
				result = ssa_emit_context::convert_to_float(ctx,operand,F,I, 0);
			}
			else
			{
				result = ssa_emit_context::convert_to_float(ctx,operand,F,I, 1);
			}
			V_jit(ctx,Rd,copy_new_raw_size(ctx, result, int128));
		}
	}
}

void convert_to_float_gp_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_float_jit(ctx,sf,ftype,U,Rn,Rd,0ULL);
}

void convert_to_float_vector_jit(ssa_emit_context* ctx, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	convert_to_float_jit(ctx,sz,sz,U,Rn,Rd,1ULL);
}

void floating_point_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t opcode, uint64_t Rn, uint64_t Rd)
{
	uint64_t fltsize = get_flt_size_jit(ctx,ftype);
	ir_operand operand1 = copy_new_raw_size(ctx, V_jit(ctx,Rn), int64);
	ir_operand operand2 = copy_new_raw_size(ctx, V_jit(ctx,Rm), int64);
	ir_operand result;
	ir_operand fpcr_state = _sys_jit(ctx,fpcr);
	if ((((uint64_t)opcode == (uint64_t)0ULL)))
	{
		result = FPMul_jit(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)1ULL)))
	{
		result = FPDiv_jit(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)2ULL)))
	{
		result = FPAdd_jit(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)3ULL)))
	{
		result = FPSub_jit(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)4ULL)))
	{
		result = FPMax_jit(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)5ULL)))
	{
		result = FPMin_jit(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)6ULL)))
	{
		result = FPMaxNum_jit(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)7ULL)))
	{
		result = FPMinNum_jit(ctx,operand1,operand2,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)8ULL)))
	{
		result = FPMul_jit(ctx,operand1,operand2,fpcr_state,fltsize);
		result = FPNeg_jit(ctx,result,fpcr_state,fltsize);
	}
	else
	{
		undefined_with_jit(ctx,opcode);
	}
	ir_operand vector = ssa_emit_context::vector_zero(ctx);
	ssa_emit_context::vector_insert(ctx,vector, 0ULL, fltsize, result);
	V_jit(ctx,Rd,vector);
}

void conversion_between_floating_point_and_fixed_point_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t S, uint64_t ftype, uint64_t rmode, uint64_t opcode, uint64_t scale, uint64_t Rn, uint64_t Rd)
{
	uint64_t intsize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t fltsize = get_flt_size_jit(ctx,ftype);
	uint64_t fracbits = ((uint64_t)64ULL - (uint64_t)scale);
	ir_operand result;
	if ((((uint64_t)rmode == (uint64_t)0ULL)))
	{
		ir_operand source = X_jit(ctx,Rn);
		result = FixedToFP_jit(ctx,source,fracbits,((uint64_t)opcode == (uint64_t)3ULL),fltsize,intsize);
		V_jit(ctx,Rd,copy_new_raw_size(ctx, result, int128));
	}
	else if ((((uint64_t)rmode == (uint64_t)3ULL)))
	{
		ir_operand source = copy_new_raw_size(ctx, V_jit(ctx,Rn), int64);
		result = FPToFixed_jit(ctx,source,fracbits,((uint64_t)opcode == (uint64_t)1ULL),FPRounding_ZERO,intsize,fltsize);
		X_jit(ctx,Rd,result);
	}
	else
	{
		undefined_with_jit(ctx,0ULL);
	}
}

void advanced_simd_three_same_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rm, uint64_t opcode, uint64_t Rn, uint64_t Rd)
{
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t O = esize == 8ULL ? int8 : esize == 16ULL ? int16 : esize == 32ULL ? int32 : esize == 64ULL ? int64 : 0;
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
			ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, e, esize);
			if ((((uint64_t)opcode == (uint64_t)0ULL)))
			{
				if ((!U))
				{
					element1 = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, element1, O), int64);
					element2 = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, element2, O), int64);
				}
				ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, ssa_emit_context::emit_ssa(ctx, ir_add, element1, element2), ir_operand::create_con(1ULL, int64)));
			}
			else if ((((uint64_t)opcode == (uint64_t)2ULL)))
			{
				if ((!U))
				{
					element1 = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, element1, O), int64);
					element2 = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, element2, O), int64);
				}
				ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, ssa_emit_context::emit_ssa(ctx, ir_add, ssa_emit_context::emit_ssa(ctx, ir_add, element1, element2), ir_operand::create_con(1ULL, int64)), ir_operand::create_con(1ULL, int64)));
			}
			else if ((((uint64_t)opcode == (uint64_t)4ULL)))
			{
				if ((!U))
				{
					element1 = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, element1, O), int64);
					element2 = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, element2, O), int64);
				}
				ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, ssa_emit_context::emit_ssa(ctx, ir_subtract, element1, element2), ir_operand::create_con(1ULL, int64)));
			}
			else if ((((uint64_t)opcode == (uint64_t)6ULL)))
			{
				ir_operand valid;
				if ((U))
				{
					valid = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_unsigned, copy_new_raw_size(ctx, element1, O), copy_new_raw_size(ctx, element2, O));
				}
				else
				{
					valid = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_signed, copy_new_raw_size(ctx, element1, O), copy_new_raw_size(ctx, element2, O));
				}
				ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_subtract, ir_operand::create_con(0ULL, O), valid));
			}
			else if ((((uint64_t)opcode == (uint64_t)7ULL)))
			{
				ir_operand valid;
				if ((U))
				{
					valid = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_equal_unsigned, copy_new_raw_size(ctx, element1, O), copy_new_raw_size(ctx, element2, O));
				}
				else
				{
					valid = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_equal_signed, copy_new_raw_size(ctx, element1, O), copy_new_raw_size(ctx, element2, O));
				}
				ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_subtract, ir_operand::create_con(0ULL, O), valid));
			}
			else if ((((uint64_t)((uint64_t)opcode == (uint64_t)8ULL) && (uint64_t)((uint64_t)size == (uint64_t)2ULL))))
			{
				ir_operand element = copy_new_raw_size(ctx, element1, O);
				ir_operand shift = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, element2, int8), O);
				element = copy_new_raw_size(ctx, vector_shift_jit(ctx,O,element,shift,esize,U), O);
				ssa_emit_context::vector_insert(ctx,result, e, esize, element);
			}
			else if ((((uint64_t)opcode == (uint64_t)12ULL)))
			{
				ir_operand working = ssa_emit_context::emit_ssa(ctx, ir_move, ir_operand::create_con(0ULL, O));
				if ((U))
				{
					{
					    ir_operand end = ir_operation_block::create_label(ctx->ir);
					    ir_operand yes = ir_operation_block::create_label(ctx->ir);
					
					    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_unsigned, element1, element2);
					
					    ir_operation_block::jump_if(ctx->ir,yes, condition);
						{
							ssa_emit_context::move(ctx,working,copy_new_raw_size(ctx, element2, O));
						}
					    
					    ir_operation_block::jump(ctx->ir,end);
					    ir_operation_block::mark_label(ctx->ir, yes);
					
						{
							ssa_emit_context::move(ctx,working,copy_new_raw_size(ctx, element1, O));
						}
					
					    ir_operation_block::mark_label(ctx->ir, end);
					}
				}
				else
				{
					{
					    ir_operand end = ir_operation_block::create_label(ctx->ir);
					    ir_operand yes = ir_operation_block::create_label(ctx->ir);
					
					    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_signed, copy_new_raw_size(ctx, element1, O), copy_new_raw_size(ctx, element2, O));
					
					    ir_operation_block::jump_if(ctx->ir,yes, condition);
						{
							ssa_emit_context::move(ctx,working,copy_new_raw_size(ctx, element2, O));
						}
					    
					    ir_operation_block::jump(ctx->ir,end);
					    ir_operation_block::mark_label(ctx->ir, yes);
					
						{
							ssa_emit_context::move(ctx,working,copy_new_raw_size(ctx, element1, O));
						}
					
					    ir_operation_block::mark_label(ctx->ir, end);
					}
				}
				ssa_emit_context::vector_insert(ctx,result, e, esize, working);
			}
			else if ((((uint64_t)opcode == (uint64_t)13ULL)))
			{
				ir_operand working = ssa_emit_context::emit_ssa(ctx, ir_move, ir_operand::create_con(0ULL, O));
				if ((U))
				{
					{
					    ir_operand end = ir_operation_block::create_label(ctx->ir);
					    ir_operand yes = ir_operation_block::create_label(ctx->ir);
					
					    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_less_unsigned, element1, element2);
					
					    ir_operation_block::jump_if(ctx->ir,yes, condition);
						{
							ssa_emit_context::move(ctx,working,copy_new_raw_size(ctx, element2, O));
						}
					    
					    ir_operation_block::jump(ctx->ir,end);
					    ir_operation_block::mark_label(ctx->ir, yes);
					
						{
							ssa_emit_context::move(ctx,working,copy_new_raw_size(ctx, element1, O));
						}
					
					    ir_operation_block::mark_label(ctx->ir, end);
					}
				}
				else
				{
					{
					    ir_operand end = ir_operation_block::create_label(ctx->ir);
					    ir_operand yes = ir_operation_block::create_label(ctx->ir);
					
					    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, copy_new_raw_size(ctx, element1, O), copy_new_raw_size(ctx, element2, O));
					
					    ir_operation_block::jump_if(ctx->ir,yes, condition);
						{
							ssa_emit_context::move(ctx,working,copy_new_raw_size(ctx, element2, O));
						}
					    
					    ir_operation_block::jump(ctx->ir,end);
					    ir_operation_block::mark_label(ctx->ir, yes);
					
						{
							ssa_emit_context::move(ctx,working,copy_new_raw_size(ctx, element1, O));
						}
					
					    ir_operation_block::mark_label(ctx->ir, end);
					}
				}
				ssa_emit_context::vector_insert(ctx,result, e, esize, working);
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)(((uint64_t)(((uint64_t)size >> (uint64_t)1ULL)) == (uint64_t)1ULL))) && (uint64_t)((uint64_t)U == (uint64_t)0ULL))))
			{
				if ((((uint64_t)size & (uint64_t)1ULL)))
				{
					element2 = ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, element2);
				}
				ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, element1, element2));
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)(((uint64_t)(((uint64_t)size >> (uint64_t)1ULL)) == (uint64_t)0ULL))) && (uint64_t)((uint64_t)U == (uint64_t)0ULL))))
			{
				if ((((uint64_t)size & (uint64_t)1ULL)))
				{
					element2 = ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, element2);
				}
				ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, element1, element2));
			}
			else if ((((uint64_t)((uint64_t)((uint64_t)opcode == (uint64_t)3ULL) && (uint64_t)((uint64_t)size == (uint64_t)0ULL)) && (uint64_t)((uint64_t)U == (uint64_t)1ULL))))
			{
				ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, element1, element2));
			}
			else
			{
				undefined_with_jit(ctx,0ULL);
			}
		}
		V_jit(ctx,Rd,result);
	}
}

void floating_point_conditional_select_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t Rd)
{
	uint64_t fltsize = get_flt_size_jit(ctx,ftype);
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	uint64_t O = fltsize == 32ULL ? int32 : fltsize == 64ULL ? int64 : 0;
	{
		ir_operand result = ssa_emit_context::emit_ssa(ctx, ir_move, ir_operand::create_con(0ULL, O));
		{
		    ir_operand end = ir_operation_block::create_label(ctx->ir);
		    ir_operand yes = ir_operation_block::create_label(ctx->ir);
		
		    ir_operand condition = condition_holds_jit(ctx,cond);
		
		    ir_operation_block::jump_if(ctx->ir,yes, condition);
			{
				ssa_emit_context::move(ctx,result,copy_new_raw_size(ctx, operand2, O));
			}
		    
		    ir_operation_block::jump(ctx->ir,end);
		    ir_operation_block::mark_label(ctx->ir, yes);
		
			{
				ssa_emit_context::move(ctx,result,copy_new_raw_size(ctx, operand1, O));
			}
		
		    ir_operation_block::mark_label(ctx->ir, end);
		}
		V_jit(ctx,Rd,copy_new_raw_size(ctx, result, int128));
	}
}

void fmov_scalar_immediate_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t imm8, uint64_t Rd)
{
	uint64_t fltsize = get_flt_size_jit(ctx,ftype);
	ir_operand imm = ir_operand::create_con(vfp_expand_imm_jit(ctx,imm8,fltsize), int64);
	V_jit(ctx,Rd,copy_new_raw_size(ctx, imm, int128));
}

void fcvt_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t opc, uint64_t Rn, uint64_t Rd)
{
	uint64_t srcsize = get_flt_size_jit(ctx,ftype);
	uint64_t dstsize = get_flt_size_jit(ctx,opc);
	ir_operand operand = copy_new_raw_size(ctx, V_jit(ctx,Rn), int64);
	V_jit(ctx,Rd,copy_new_raw_size(ctx, FPConvert_jit(ctx,operand,dstsize,srcsize), int128));
}

void floating_point_data_processing_one_source_jit(ssa_emit_context* ctx, uint64_t M, uint64_t S, uint64_t ftype, uint64_t opcode, uint64_t Rn, uint64_t Rd)
{
	uint64_t fltsize = get_flt_size_jit(ctx,ftype);
	ir_operand operand = copy_new_raw_size(ctx, V_jit(ctx,Rn), int64);
	ir_operand result = ir_operand::create_con(0ULL, int64);
	ir_operand fpcr_state = _sys_jit(ctx,fpcr);
	if ((((uint64_t)opcode == (uint64_t)1ULL)))
	{
		result = FPAbs_jit(ctx,operand,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)2ULL)))
	{
		result = FPNeg_jit(ctx,operand,fpcr_state,fltsize);
	}
	else if ((((uint64_t)opcode == (uint64_t)3ULL)))
	{
		result = FPSqrt_jit(ctx,operand,fpcr_state,fltsize);
	}
	else
	{
		undefined_with_jit(ctx,0ULL);
	}
	ir_operand vector = ssa_emit_context::vector_zero(ctx);
	ssa_emit_context::vector_insert(ctx,vector, 0ULL, fltsize, result);
	V_jit(ctx,Rd,vector);
}

void fcmp_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t opc)
{
	uint64_t datasize = get_flt_size_jit(ctx,ftype);
	uint64_t cmp_with_zero = ((uint64_t)opc == (uint64_t)1ULL);
	ir_operand operand1 = copy_new_raw_size(ctx, V_jit(ctx,Rn), int64);
	ir_operand operand2;
	ir_operand fpcr_state = _sys_jit(ctx,fpcr);
	if ((cmp_with_zero))
	{
		operand2 = ir_operand::create_con(0ULL, int64);
	}
	else
	{
		operand2 = copy_new_raw_size(ctx, V_jit(ctx,Rm), int64);
	}
	ir_operand nzcv = FPCompare_jit(ctx,operand1,operand2,fpcr_state,datasize);
	_sys_jit(ctx,nzcv_n,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, nzcv, ir_operand::create_con(3ULL, int64)), ir_operand::create_con(1ULL, int64)));
	_sys_jit(ctx,nzcv_z,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, nzcv, ir_operand::create_con(2ULL, int64)), ir_operand::create_con(1ULL, int64)));
	_sys_jit(ctx,nzcv_c,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, nzcv, ir_operand::create_con(1ULL, int64)), ir_operand::create_con(1ULL, int64)));
	_sys_jit(ctx,nzcv_v,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, nzcv, ir_operand::create_con(0ULL, int64)), ir_operand::create_con(1ULL, int64)));
}

void fccmp_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t nzcv)
{
	uint64_t datasize = get_flt_size_jit(ctx,ftype);
	ir_operand operand1 = copy_new_raw_size(ctx, V_jit(ctx,Rn), int64);
	ir_operand operand2 = copy_new_raw_size(ctx, V_jit(ctx,Rm), int64);
	ir_operand fpcr_state = _sys_jit(ctx,fpcr);
	{
	    ir_operand end = ir_operation_block::create_label(ctx->ir);
	    ir_operand yes = ir_operation_block::create_label(ctx->ir);
	
	    ir_operand condition = condition_holds_jit(ctx,cond);
	
	    ir_operation_block::jump_if(ctx->ir,yes, condition);
		{
			_sys_jit(ctx,nzcv_n,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL), int64));
			_sys_jit(ctx,nzcv_z,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL), int64));
			_sys_jit(ctx,nzcv_c,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL), int64));
			_sys_jit(ctx,nzcv_v,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL), int64));
		}
	    
	    ir_operation_block::jump(ctx->ir,end);
	    ir_operation_block::mark_label(ctx->ir, yes);
	
		{
			ir_operand success_nzcv = FPCompare_jit(ctx,operand1,operand2,fpcr_state,datasize);
			_sys_jit(ctx,nzcv_n,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, success_nzcv, ir_operand::create_con(3ULL, int64)), ir_operand::create_con(1ULL, int64)));
			_sys_jit(ctx,nzcv_z,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, success_nzcv, ir_operand::create_con(2ULL, int64)), ir_operand::create_con(1ULL, int64)));
			_sys_jit(ctx,nzcv_c,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, success_nzcv, ir_operand::create_con(1ULL, int64)), ir_operand::create_con(1ULL, int64)));
			_sys_jit(ctx,nzcv_v,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, success_nzcv, ir_operand::create_con(0ULL, int64)), ir_operand::create_con(1ULL, int64)));
		}
	
	    ir_operation_block::mark_label(ctx->ir, end);
	}
}

void fcvtz_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_jit(ctx,sf,ftype,Rd,Rn,FPRounding_ZERO,U,0ULL);
}

void fcvtn_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_jit(ctx,sf,ftype,Rd,Rn,FPRounding_TIEEVEN,U,0ULL);
}

void fcvta_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_jit(ctx,sf,ftype,Rd,Rn,FPRounding_TIEAWAY,U,0ULL);
}

void fcvtm_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_jit(ctx,sf,ftype,Rd,Rn,FPRounding_NEGINF,U,0ULL);
}

void fcvtp_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_jit(ctx,sf,ftype,Rd,Rn,FPRounding_POSINF,U,0ULL);
}

void addlv_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t is_unsigned = U;
	ir_operand source = V_jit(ctx,Rn);
	ir_operand operand = V_jit(ctx,Rn);
	ir_operand sum = ir_operand::create_con(0ULL, int64);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ir_operand working = ssa_emit_context::vector_extract(ctx,operand, e, esize);
		if ((!is_unsigned))
		{
			uint64_t O = esize == 8ULL ? int8 : esize == 16ULL ? int16 : esize == 32ULL ? int32 : esize == 64ULL ? int64 : 0;
			{
				working = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, working, O), int64);
			}
		}
		sum = ssa_emit_context::emit_ssa(ctx, ir_add, sum, working);
	}
	ir_operand final = ssa_emit_context::vector_zero(ctx);
	ssa_emit_context::vector_insert(ctx,final, 0ULL, (((uint64_t)2ULL * (uint64_t)esize)), sum);
	V_jit(ctx,Rd,final);
}

void cnt_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = 8ULL;
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)8ULL);
	ir_operand source = V_jit(ctx,Rn);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ir_operand working = copy_new_raw_size(ctx, ssa_emit_context::vector_extract(ctx,source, e, esize), int8);
		ir_operand count = ssa_emit_context::emit_ssa(ctx, ir_move, ir_operand::create_con(0ULL, int8));
		for (uint64_t b = 0; b < (esize); b++)
		{
			ir_operand bit = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, working, ir_operand::create_con(b, int8)), ir_operand::create_con(1ULL, int8));
			{
			    ir_operand end = ir_operation_block::create_label(ctx->ir);
			    ir_operand yes = ir_operation_block::create_label(ctx->ir);
			
			    ir_operand condition = bit;
			
			    ir_operation_block::jump_if(ctx->ir,yes, condition);
				/* TODO if statements without a no should not have this*/
			    
			    ir_operation_block::jump(ctx->ir,end);
			    ir_operation_block::mark_label(ctx->ir, yes);
			
				{
					ssa_emit_context::move(ctx,count,ssa_emit_context::emit_ssa(ctx, ir_add, count, ir_operand::create_con(1ULL, int8)));
				}
			
			    ir_operation_block::mark_label(ctx->ir, end);
			}
		}
		ssa_emit_context::vector_insert(ctx,result, e, esize, count);
	}
	V_jit(ctx,Rd,result);
}

ir_operand compare_and_swap_jit(ssa_emit_context* ctx, ir_operand address, ir_operand expecting, ir_operand to_swap, uint64_t size)
{
	address = translate_address_jit(ctx,address);
	return _compare_and_swap_jit(ctx,address,expecting,to_swap,size);
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

void msr_register_jit(ssa_emit_context* ctx, uint64_t imm15, uint64_t Rt)
{
	ir_operand operand = X_jit(ctx,Rt);
	if ((((uint64_t)imm15 == (uint64_t)23072ULL)))
	{
		_sys_jit(ctx,fpcr,operand);
	}
	else if ((((uint64_t)imm15 == (uint64_t)23073ULL)))
	{
		_sys_jit(ctx,fpsr,operand);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24194ULL)))
	{
		_sys_jit(ctx,thread_local_1,operand);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24195ULL)))
	{
		_sys_jit(ctx,thread_local_0,operand);
	}
	else
	{
		undefined_with_jit(ctx,imm15);
	}
}

void mrs_register_jit(ssa_emit_context* ctx, uint64_t imm15, uint64_t Rt)
{
	ir_operand operand;
	if ((((uint64_t)imm15 == (uint64_t)23072ULL)))
	{
		operand = _sys_jit(ctx,fpcr);
	}
	else if ((((uint64_t)imm15 == (uint64_t)23073ULL)))
	{
		operand = _sys_jit(ctx,fpsr);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24194ULL)))
	{
		operand = _sys_jit(ctx,thread_local_1);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24195ULL)))
	{
		operand = _sys_jit(ctx,thread_local_0);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24321ULL)))
	{
		operand = call_counter_jit(ctx);
	}
	else if ((((uint64_t)imm15 == (uint64_t)22529ULL)))
	{
		operand = ir_operand::create_con(2219098116ULL, int64);
	}
	else
	{
		undefined_with_jit(ctx,imm15);
	}
	X_jit(ctx,Rt,operand);
}

void hints_jit(ssa_emit_context* ctx, uint64_t imm7)
{
}

void sys_jit(ssa_emit_context* ctx, uint64_t L, uint64_t imm19)
{
}

void barriers_jit(ssa_emit_context* ctx, uint64_t CRm, uint64_t op2, uint64_t Rt)
{
	if ((((uint64_t)((uint64_t)op2 == (uint64_t)2ULL) && (uint64_t)((uint64_t)Rt == (uint64_t)31ULL))))
	{
		_sys_jit(ctx,exclusive_address,ir_operand::create_con(-1ULL, int64));
		_sys_jit(ctx,exclusive_value,ir_operand::create_con(-1ULL, int64));
	}
}

