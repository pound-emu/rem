#include "aarch64_impl.h"
#include "string.h"
#include "tools/big_number.h"

static void append_table(guest_process* process, std::string encoding, void* emit, void* interperate, void* decoder_helper,std::string name)
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

	fixed_length_decoder<uint32_t>::insert_entry(&process->decoder, instruction, mask, emit, interperate, decoder_helper, name);
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
			source = ir_operand::copy_new_raw_size(source, new_size);

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

template <typename R>
R intrinsic_unary_interpreter(interpreter_data* ctx, uint64_t instruction, R source)
{

}

template <typename R>
R intrinsic_binary_interpreter(interpreter_data* ctx, uint64_t instruction, R source_0, R source_1)
{

}

template <typename R>
R intrinsic_binary_imm_interpreter(interpreter_data* ctx, uint64_t instruction, R source_0, uint64_t source_1)
{

}

template <typename R>
R intrinsic_ternary_interpreter(interpreter_data* ctx, uint64_t instruction, R source_0, R source_1, R source_2)
{

}

template <typename R>
R intrinsic_ternary_imm_interpreter(interpreter_data* ctx, uint64_t instruction, R source_0, R source_1, uint64_t source_2)
{

}

template <typename O>
O x86_add_set_flags_interpreter(interpreter_data* ctx, O n, O m)
{

}

template <typename O>
O x86_subtract_set_flags_interpreter(interpreter_data* ctx, O n, O m)
{

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

static void call_frintp_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frintp_scalar_interpreter(ctx, ftype, Rn, Rd);
}

static void emit_frintp_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frintp_scalar_jit(ctx, ftype, Rn, Rd);
}

static void call_frintm_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frintm_scalar_interpreter(ctx, ftype, Rn, Rd);
}

static void emit_frintm_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frintm_scalar_jit(ctx, ftype, Rn, Rd);
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

static void call_fadd_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fadd_vector_interpreter(ctx, Q, sz, Rm, Rn, Rd);
}

static void emit_fadd_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fadd_vector_jit(ctx, Q, sz, Rm, Rn, Rd);
}

static void call_fmul_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_vector_interpreter(ctx, Q, sz, Rm, Rn, Rd);
}

static void emit_fmul_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_vector_jit(ctx, Q, sz, Rm, Rn, Rd);
}

static void call_fsub_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fsub_vector_interpreter(ctx, Q, sz, Rm, Rn, Rd);
}

static void emit_fsub_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fsub_vector_jit(ctx, Q, sz, Rm, Rn, Rd);
}

static void call_fdiv_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fdiv_vector_interpreter(ctx, Q, sz, Rm, Rn, Rd);
}

static void emit_fdiv_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fdiv_vector_jit(ctx, Q, sz, Rm, Rn, Rd);
}

static void call_fmul_accumulate_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int neg = (instruction >> 23) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_accumulate_vector_interpreter(ctx, Q, neg, sz, Rm, Rn, Rd);
}

static void emit_fmul_accumulate_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int neg = (instruction >> 23) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_accumulate_vector_jit(ctx, Q, neg, sz, Rm, Rn, Rd);
}

static void call_faddp_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	faddp_vector_interpreter(ctx, Q, sz, Rm, Rn, Rd);
}

static void emit_faddp_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	faddp_vector_jit(ctx, Q, sz, Rm, Rn, Rd);
}

static void call_frsqrte_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frsqrte_vector_interpreter(ctx, Q, sz, Rn, Rd);
}

static void emit_frsqrte_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frsqrte_vector_jit(ctx, Q, sz, Rn, Rd);
}

static void call_frsqrts_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frsqrts_vector_interpreter(ctx, Q, sz, Rm, Rn, Rd);
}

static void emit_frsqrts_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frsqrts_vector_jit(ctx, Q, sz, Rm, Rn, Rd);
}

static void call_frecps_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frecps_vector_interpreter(ctx, Q, sz, Rm, Rn, Rd);
}

static void emit_frecps_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frecps_vector_jit(ctx, Q, sz, Rm, Rn, Rd);
}

static void call_fmul_scalar_by_element_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sz = (instruction >> 22) & 1;
	int L = (instruction >> 21) & 1;
	int M = (instruction >> 20) & 1;
	int Rm = (instruction >> 16) & 15;
	int H = (instruction >> 11) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_scalar_by_element_interpreter(ctx, sz, L, M, Rm, H, Rn, Rd);
}

static void emit_fmul_scalar_by_element_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sz = (instruction >> 22) & 1;
	int L = (instruction >> 21) & 1;
	int M = (instruction >> 20) & 1;
	int Rm = (instruction >> 16) & 15;
	int H = (instruction >> 11) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_scalar_by_element_jit(ctx, sz, L, M, Rm, H, Rn, Rd);
}

static void call_fmul_vector_by_element_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int L = (instruction >> 21) & 1;
	int M = (instruction >> 20) & 1;
	int Rm = (instruction >> 16) & 15;
	int H = (instruction >> 11) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_vector_by_element_interpreter(ctx, Q, sz, L, M, Rm, H, Rn, Rd);
}

static void emit_fmul_vector_by_element_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int L = (instruction >> 21) & 1;
	int M = (instruction >> 20) & 1;
	int Rm = (instruction >> 16) & 15;
	int H = (instruction >> 11) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_vector_by_element_jit(ctx, Q, sz, L, M, Rm, H, Rn, Rd);
}

static void call_fmul_accumulate_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sz = (instruction >> 22) & 1;
	int L = (instruction >> 21) & 1;
	int M = (instruction >> 20) & 1;
	int Rm = (instruction >> 16) & 15;
	int neg = (instruction >> 14) & 1;
	int H = (instruction >> 11) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_accumulate_scalar_interpreter(ctx, sz, L, M, Rm, neg, H, Rn, Rd);
}

static void emit_fmul_accumulate_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sz = (instruction >> 22) & 1;
	int L = (instruction >> 21) & 1;
	int M = (instruction >> 20) & 1;
	int Rm = (instruction >> 16) & 15;
	int neg = (instruction >> 14) & 1;
	int H = (instruction >> 11) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_accumulate_scalar_jit(ctx, sz, L, M, Rm, neg, H, Rn, Rd);
}

static void call_fmul_accumulate_element_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int L = (instruction >> 21) & 1;
	int M = (instruction >> 20) & 1;
	int Rm = (instruction >> 16) & 15;
	int neg = (instruction >> 14) & 1;
	int H = (instruction >> 11) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_accumulate_element_interpreter(ctx, Q, sz, L, M, Rm, neg, H, Rn, Rd);
}

static void emit_fmul_accumulate_element_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int L = (instruction >> 21) & 1;
	int M = (instruction >> 20) & 1;
	int Rm = (instruction >> 16) & 15;
	int neg = (instruction >> 14) & 1;
	int H = (instruction >> 11) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_accumulate_element_jit(ctx, Q, sz, L, M, Rm, neg, H, Rn, Rd);
}

static void call_faddp_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	faddp_scalar_interpreter(ctx, sz, Rn, Rd);
}

static void emit_faddp_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	faddp_scalar_jit(ctx, sz, Rn, Rd);
}

static void call_fcmeq_vector_zero_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcmeq_vector_zero_interpreter(ctx, Q, sz, Rn, Rd);
}

static void emit_fcmeq_vector_zero_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcmeq_vector_zero_jit(ctx, Q, sz, Rn, Rd);
}

static void call_fcmgt_vector_zero_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcmgt_vector_zero_interpreter(ctx, Q, sz, Rn, Rd);
}

static void emit_fcmgt_vector_zero_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcmgt_vector_zero_jit(ctx, Q, sz, Rn, Rd);
}

static void call_fcmge_vector_zero_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcmge_vector_zero_interpreter(ctx, Q, sz, Rn, Rd);
}

static void emit_fcmge_vector_zero_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcmge_vector_zero_jit(ctx, Q, sz, Rn, Rd);
}

static void call_fcmeq_vector_register_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcmeq_vector_register_interpreter(ctx, Q, sz, Rm, Rn, Rd);
}

static void emit_fcmeq_vector_register_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcmeq_vector_register_jit(ctx, Q, sz, Rm, Rn, Rd);
}

static void call_fcmgt_vector_register_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcmgt_vector_register_interpreter(ctx, Q, sz, Rm, Rn, Rd);
}

static void emit_fcmgt_vector_register_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcmgt_vector_register_jit(ctx, Q, sz, Rm, Rn, Rd);
}

static void call_fcmge_vector_register_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcmge_vector_register_interpreter(ctx, Q, sz, Rm, Rn, Rd);
}

static void emit_fcmge_vector_register_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fcmge_vector_register_jit(ctx, Q, sz, Rm, Rn, Rd);
}

static void call_fadd_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fadd_scalar_interpreter(ctx, ftype, Rm, Rn, Rd);
}

static void emit_fadd_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fadd_scalar_jit(ctx, ftype, Rm, Rn, Rd);
}

static void call_fsub_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fsub_scalar_interpreter(ctx, ftype, Rm, Rn, Rd);
}

static void emit_fsub_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fsub_scalar_jit(ctx, ftype, Rm, Rn, Rd);
}

static void call_fmul_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_scalar_interpreter(ctx, ftype, Rm, Rn, Rd);
}

static void emit_fmul_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmul_scalar_jit(ctx, ftype, Rm, Rn, Rd);
}

static void call_fdiv_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fdiv_scalar_interpreter(ctx, ftype, Rm, Rn, Rd);
}

static void emit_fdiv_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fdiv_scalar_jit(ctx, ftype, Rm, Rn, Rd);
}

static void call_fmax_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmax_scalar_interpreter(ctx, ftype, Rm, Rn, Rd);
}

static void emit_fmax_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmax_scalar_jit(ctx, ftype, Rm, Rn, Rd);
}

static void call_fmin_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmin_scalar_interpreter(ctx, ftype, Rm, Rn, Rd);
}

static void emit_fmin_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmin_scalar_jit(ctx, ftype, Rm, Rn, Rd);
}

static void call_fmaxnm_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmaxnm_scalar_interpreter(ctx, ftype, Rm, Rn, Rd);
}

static void emit_fmaxnm_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fmaxnm_scalar_jit(ctx, ftype, Rm, Rn, Rd);
}

static void call_fminnm_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fminnm_scalar_interpreter(ctx, ftype, Rm, Rn, Rd);
}

static void emit_fminnm_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fminnm_scalar_jit(ctx, ftype, Rm, Rn, Rd);
}

static void call_fnmul_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fnmul_scalar_interpreter(ctx, ftype, Rm, Rn, Rd);
}

static void emit_fnmul_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fnmul_scalar_jit(ctx, ftype, Rm, Rn, Rd);
}

static void call_fabs_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fabs_scalar_interpreter(ctx, ftype, Rn, Rd);
}

static void emit_fabs_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fabs_scalar_jit(ctx, ftype, Rn, Rd);
}

static void call_fneg_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fneg_scalar_interpreter(ctx, ftype, Rn, Rd);
}

static void emit_fneg_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fneg_scalar_jit(ctx, ftype, Rn, Rd);
}

static void call_fneg_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fneg_vector_interpreter(ctx, Q, sz, Rn, Rd);
}

static void emit_fneg_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fneg_vector_jit(ctx, Q, sz, Rn, Rd);
}

static void call_fsqrt_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fsqrt_scalar_interpreter(ctx, ftype, Rn, Rd);
}

static void emit_fsqrt_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int ftype = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fsqrt_scalar_jit(ctx, ftype, Rn, Rd);
}

static void call_fsqrt_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fsqrt_vector_interpreter(ctx, Q, sz, Rn, Rd);
}

static void emit_fsqrt_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	fsqrt_vector_jit(ctx, Q, sz, Rn, Rd);
}

static void call_frecpe_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frecpe_vector_interpreter(ctx, Q, sz, Rn, Rd);
}

static void emit_frecpe_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frecpe_vector_jit(ctx, Q, sz, Rn, Rd);
}

static void call_frsqrte_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frsqrte_scalar_interpreter(ctx, sz, Rn, Rd);
}

static void emit_frsqrte_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	frsqrte_scalar_jit(ctx, sz, Rn, Rd);
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

static void call_convert_to_float_vector_scalar_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int U = (instruction >> 29) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	convert_to_float_vector_scalar_interpreter(ctx, U, sz, Rn, Rd);
}

static void emit_convert_to_float_vector_scalar_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int U = (instruction >> 29) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	convert_to_float_vector_scalar_jit(ctx, U, sz, Rn, Rd);
}

static void call_convert_to_float_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int U = (instruction >> 29) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	convert_to_float_vector_interpreter(ctx, Q, U, sz, Rn, Rd);
}

static void emit_convert_to_float_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int U = (instruction >> 29) & 1;
	int sz = (instruction >> 22) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	convert_to_float_vector_jit(ctx, Q, U, sz, Rn, Rd);
}

static void call_shl_immedaite_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int immh = (instruction >> 19) & 15;
	int immb = (instruction >> 16) & 7;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	shl_immedaite_interpreter(ctx, Q, immh, immb, Rn, Rd);
}

static bool help_decode_shl_immedaite(uint32_t instruction)
{
	if (((instruction >> 19) & 15) == 0) return false;
	return true;
}

static void emit_shl_immedaite_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int immh = (instruction >> 19) & 15;
	int immb = (instruction >> 16) & 7;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	shl_immedaite_jit(ctx, Q, immh, immb, Rn, Rd);
}

static void call_sshr_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int immh = (instruction >> 19) & 15;
	int immb = (instruction >> 16) & 7;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	sshr_vector_interpreter(ctx, Q, immh, immb, Rn, Rd);
}

static bool help_decode_sshr_vector(uint32_t instruction)
{
	if (((instruction >> 19) & 15) == 0) return false;
	return true;
}

static void emit_sshr_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int immh = (instruction >> 19) & 15;
	int immb = (instruction >> 16) & 7;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	sshr_vector_jit(ctx, Q, immh, immb, Rn, Rd);
}

static void call_shll_shll2_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int U = (instruction >> 29) & 1;
	int immh = (instruction >> 19) & 15;
	int immb = (instruction >> 16) & 7;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	shll_shll2_interpreter(ctx, Q, U, immh, immb, Rn, Rd);
}

static bool help_decode_shll_shll2(uint32_t instruction)
{
	if (((instruction >> 19) & 15) == 0) return false;
	return true;
}

static void emit_shll_shll2_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int U = (instruction >> 29) & 1;
	int immh = (instruction >> 19) & 15;
	int immb = (instruction >> 16) & 7;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	shll_shll2_jit(ctx, Q, U, immh, immb, Rn, Rd);
}

static void call_shrn_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int immh = (instruction >> 19) & 15;
	int immb = (instruction >> 16) & 7;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	shrn_interpreter(ctx, Q, immh, immb, Rn, Rd);
}

static bool help_decode_shrn(uint32_t instruction)
{
	if (((instruction >> 19) & 15) == 0) return false;
	return true;
}

static void emit_shrn_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int immh = (instruction >> 19) & 15;
	int immb = (instruction >> 16) & 7;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	shrn_jit(ctx, Q, immh, immb, Rn, Rd);
}

static void call_rev64_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	rev64_vector_interpreter(ctx, Q, size, Rn, Rd);
}

static void emit_rev64_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	rev64_vector_jit(ctx, Q, size, Rn, Rd);
}

static void call_neg_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	neg_vector_interpreter(ctx, Q, size, Rn, Rd);
}

static void emit_neg_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	neg_vector_jit(ctx, Q, size, Rn, Rd);
}

static void call_not_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	not_vector_interpreter(ctx, Q, Rn, Rd);
}

static void emit_not_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	not_vector_jit(ctx, Q, Rn, Rd);
}

static void call_abs_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	abs_vector_interpreter(ctx, Q, size, Rn, Rd);
}

static void emit_abs_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	abs_vector_jit(ctx, Q, size, Rn, Rd);
}

static void call_mul_vector_index_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int L = (instruction >> 21) & 1;
	int M = (instruction >> 20) & 1;
	int Rm = (instruction >> 16) & 15;
	int H = (instruction >> 11) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	mul_vector_index_interpreter(ctx, Q, size, L, M, Rm, H, Rn, Rd);
}

static void emit_mul_vector_index_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int L = (instruction >> 21) & 1;
	int M = (instruction >> 20) & 1;
	int Rm = (instruction >> 16) & 15;
	int H = (instruction >> 11) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	mul_vector_index_jit(ctx, Q, size, L, M, Rm, H, Rn, Rd);
}

static void call_mul_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	mul_vector_interpreter(ctx, Q, size, Rm, Rn, Rd);
}

static void emit_mul_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	mul_vector_jit(ctx, Q, size, Rm, Rn, Rd);
}

static void call_ext_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int imm4 = (instruction >> 11) & 15;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	ext_interpreter(ctx, Q, Rm, imm4, Rn, Rd);
}

static void emit_ext_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int imm4 = (instruction >> 11) & 15;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	ext_jit(ctx, Q, Rm, imm4, Rn, Rd);
}

static void call_compare_above_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int U = (instruction >> 29) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	compare_above_interpreter(ctx, Q, U, size, Rm, Rn, Rd);
}

static void emit_compare_above_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int U = (instruction >> 29) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	compare_above_jit(ctx, Q, U, size, Rm, Rn, Rd);
}

static void call_shl_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int U = (instruction >> 29) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	shl_vector_interpreter(ctx, Q, U, size, Rm, Rn, Rd);
}

static void emit_shl_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int U = (instruction >> 29) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	shl_vector_jit(ctx, Q, U, size, Rm, Rn, Rd);
}

static void call_add_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	add_vector_interpreter(ctx, Q, size, Rm, Rn, Rd);
}

static void emit_add_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	add_vector_jit(ctx, Q, size, Rm, Rn, Rd);
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

static void call_orr_orn_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int invert = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	orr_orn_vector_interpreter(ctx, Q, invert, Rm, Rn, Rd);
}

static void emit_orr_orn_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int invert = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	orr_orn_vector_jit(ctx, Q, invert, Rm, Rn, Rd);
}

static void call_bsl_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	bsl_vector_interpreter(ctx, Q, Rm, Rn, Rd);
}

static void emit_bsl_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	bsl_vector_jit(ctx, Q, Rm, Rn, Rd);
}

static void call_and_bic_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int invert = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	and_bic_vector_interpreter(ctx, Q, invert, Rm, Rn, Rd);
}

static void emit_and_bic_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int invert = (instruction >> 22) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	and_bic_vector_jit(ctx, Q, invert, Rm, Rn, Rd);
}

static void call_eor_vector_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	eor_vector_interpreter(ctx, Q, Rm, Rn, Rd);
}

static void emit_eor_vector_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	eor_vector_jit(ctx, Q, Rm, Rn, Rd);
}

static void call_xnt_xnt2_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	xnt_xnt2_interpreter(ctx, Q, size, Rn, Rd);
}

static void emit_xnt_xnt2_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	xnt_xnt2_jit(ctx, Q, size, Rn, Rd);
}

static void call_zip_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int op = (instruction >> 14) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	zip_interpreter(ctx, Q, size, Rm, op, Rn, Rd);
}

static void emit_zip_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int op = (instruction >> 14) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	zip_jit(ctx, Q, size, Rm, op, Rn, Rd);
}

static void call_trn_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int op = (instruction >> 14) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	trn_interpreter(ctx, Q, size, Rm, op, Rn, Rd);
}

static void emit_trn_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 22) & 3;
	int Rm = (instruction >> 16) & 31;
	int op = (instruction >> 14) & 1;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	trn_jit(ctx, Q, size, Rm, op, Rn, Rd);
}

static void call_tbl_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int len = (instruction >> 13) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	tbl_interpreter(ctx, Q, Rm, len, Rn, Rd);
}

static void emit_tbl_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int len = (instruction >> 13) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rd = (instruction >> 0) & 31;
	tbl_jit(ctx, Q, Rm, len, Rn, Rd);
}

static void call_ld1r_no_offset_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	ld1r_no_offset_interpreter(ctx, Q, size, Rn, Rt);
}

static void emit_ld1r_no_offset_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	ld1r_no_offset_jit(ctx, Q, size, Rn, Rt);
}

static void call_ld1r_post_index_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	ld1r_post_index_interpreter(ctx, Q, Rm, size, Rn, Rt);
}

static void emit_ld1r_post_index_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	ld1r_post_index_jit(ctx, Q, Rm, size, Rn, Rt);
}

static void call_ld1_single_structure_no_offset_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int opcode = (instruction >> 14) & 3;
	int S = (instruction >> 12) & 1;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	ld1_single_structure_no_offset_interpreter(ctx, Q, opcode, S, size, Rn, Rt);
}

static bool help_decode_ld1_single_structure_no_offset(uint32_t instruction)
{
	if (((instruction >> 14) & 3) == 3) return false;
	return true;
}

static void emit_ld1_single_structure_no_offset_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int opcode = (instruction >> 14) & 3;
	int S = (instruction >> 12) & 1;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	ld1_single_structure_no_offset_jit(ctx, Q, opcode, S, size, Rn, Rt);
}

static void call_ld1_single_structure_post_index_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int opcode = (instruction >> 14) & 3;
	int S = (instruction >> 12) & 1;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	ld1_single_structure_post_index_interpreter(ctx, Q, Rm, opcode, S, size, Rn, Rt);
}

static bool help_decode_ld1_single_structure_post_index(uint32_t instruction)
{
	if (((instruction >> 14) & 3) == 3) return false;
	return true;
}

static void emit_ld1_single_structure_post_index_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int opcode = (instruction >> 14) & 3;
	int S = (instruction >> 12) & 1;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	ld1_single_structure_post_index_jit(ctx, Q, Rm, opcode, S, size, Rn, Rt);
}

static void call_st2_multiple_structures_no_offset_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	st2_multiple_structures_no_offset_interpreter(ctx, Q, size, Rn, Rt);
}

static void emit_st2_multiple_structures_no_offset_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	st2_multiple_structures_no_offset_jit(ctx, Q, size, Rn, Rt);
}

static void call_st2_multiple_structures_post_index_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	st2_multiple_structures_post_index_interpreter(ctx, Q, Rm, size, Rn, Rt);
}

static void emit_st2_multiple_structures_post_index_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	st2_multiple_structures_post_index_jit(ctx, Q, Rm, size, Rn, Rt);
}

static void call_st1_single_structure_no_offset_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int opcode = (instruction >> 14) & 3;
	int S = (instruction >> 12) & 1;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	st1_single_structure_no_offset_interpreter(ctx, Q, opcode, S, size, Rn, Rt);
}

static void emit_st1_single_structure_no_offset_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int opcode = (instruction >> 14) & 3;
	int S = (instruction >> 12) & 1;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	st1_single_structure_no_offset_jit(ctx, Q, opcode, S, size, Rn, Rt);
}

static void call_st1_single_structure_post_index_interpreter(interpreter_data* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int opcode = (instruction >> 14) & 3;
	int S = (instruction >> 12) & 1;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	st1_single_structure_post_index_interpreter(ctx, Q, Rm, opcode, S, size, Rn, Rt);
}

static void emit_st1_single_structure_post_index_jit(ssa_emit_context* ctx, uint32_t instruction)
{
	int Q = (instruction >> 30) & 1;
	int Rm = (instruction >> 16) & 31;
	int opcode = (instruction >> 14) & 3;
	int S = (instruction >> 12) & 1;
	int size = (instruction >> 10) & 3;
	int Rn = (instruction >> 5) & 31;
	int Rt = (instruction >> 0) & 31;
	st1_single_structure_post_index_jit(ctx, Q, Rm, opcode, S, size, Rn, Rt);
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

void init_aarch64_decoder(guest_process* process)
{
	append_table(process, "---100010-----------------------", (void*)emit_add_subtract_imm12_jit, (void*)call_add_subtract_imm12_interpreter,nullptr, "add_subtract_imm12");
	append_table(process, "---01011--0---------------------", (void*)emit_add_subtract_shifted_jit, (void*)call_add_subtract_shifted_interpreter,nullptr, "add_subtract_shifted");
	append_table(process, "---01011001---------------------", (void*)emit_add_subtract_extended_jit, (void*)call_add_subtract_extended_interpreter,nullptr, "add_subtract_extended");
	append_table(process, "---11010000-----000000----------", (void*)emit_add_subtract_carry_jit, (void*)call_add_subtract_carry_interpreter,nullptr, "add_subtract_carry");
	append_table(process, "-0011010110-----0010------------", (void*)emit_shift_variable_jit, (void*)call_shift_variable_interpreter,nullptr, "shift_variable");
	append_table(process, "10011011-01---------------------", (void*)emit_multiply_with_32_jit, (void*)call_multiply_with_32_interpreter,nullptr, "multiply_with_32");
	append_table(process, "10011011-10------11111----------", (void*)emit_multiply_hi_jit, (void*)call_multiply_hi_interpreter,nullptr, "multiply_hi");
	append_table(process, "-0011011000---------------------", (void*)emit_multiply_additive_jit, (void*)call_multiply_additive_interpreter,nullptr, "multiply_additive");
	append_table(process, "-0011010110-----00001-----------", (void*)emit_divide_jit, (void*)call_divide_interpreter,nullptr, "divide");
	append_table(process, "-101101011000000000000----------", (void*)emit_rbit_jit, (void*)call_rbit_interpreter,nullptr, "rbit");
	append_table(process, "-101101011000000000001----------", (void*)emit_rev16_jit, (void*)call_rev16_interpreter,nullptr, "rev16");
	append_table(process, "-10110101100000000001-----------", (void*)emit_reverse_jit, (void*)call_reverse_interpreter,nullptr, "reverse");
	append_table(process, "-10110101100000000010-----------", (void*)emit_count_leading_jit, (void*)call_count_leading_interpreter,nullptr, "count_leading");
	append_table(process, "-00100111-0---------------------", (void*)emit_extr_jit, (void*)call_extr_interpreter,nullptr, "extr");
	append_table(process, "---100110-----------------------", (void*)emit_bitfield_jit, (void*)call_bitfield_interpreter,nullptr, "bitfield");
	append_table(process, "---100100-----------------------", (void*)emit_logical_immediate_jit, (void*)call_logical_immediate_interpreter,nullptr, "logical_immediate");
	append_table(process, "---01010------------------------", (void*)emit_logical_shifted_jit, (void*)call_logical_shifted_interpreter,nullptr, "logical_shifted");
	append_table(process, "---11010100---------0-----------", (void*)emit_conditional_select_jit, (void*)call_conditional_select_interpreter,nullptr, "conditional_select");
	append_table(process, "--111010010----------0-----0----", (void*)emit_conditional_compare_jit, (void*)call_conditional_compare_interpreter,nullptr, "conditional_compare");
	append_table(process, "---100101-----------------------", (void*)emit_move_wide_immediate_jit, (void*)call_move_wide_immediate_interpreter,nullptr, "move_wide_immediate");
	append_table(process, "---10000------------------------", (void*)emit_pc_rel_addressing_jit, (void*)call_pc_rel_addressing_interpreter,nullptr, "pc_rel_addressing");
	append_table(process, "1101011000-11111000000-----00000", (void*)emit_branch_register_jit, (void*)call_branch_register_interpreter,nullptr, "branch_register");
	append_table(process, "1101011001011111000000-----00000", (void*)emit_return_register_jit, (void*)call_return_register_interpreter,nullptr, "return_register");
	append_table(process, "-011011-------------------------", (void*)emit_test_bit_branch_jit, (void*)call_test_bit_branch_interpreter,nullptr, "test_bit_branch");
	append_table(process, "-011010-------------------------", (void*)emit_compare_and_branch_jit, (void*)call_compare_and_branch_interpreter,nullptr, "compare_and_branch");
	append_table(process, "-00101--------------------------", (void*)emit_b_unconditional_jit, (void*)call_b_unconditional_interpreter,nullptr, "b_unconditional");
	append_table(process, "01010100-------------------0----", (void*)emit_b_conditional_jit, (void*)call_b_conditional_interpreter,nullptr, "b_conditional");
	append_table(process, "11010100000----------------00001", (void*)emit_svc_jit, (void*)call_svc_interpreter,nullptr, "svc");
	append_table(process, "110101010001--------------------", (void*)emit_msr_register_jit, (void*)call_msr_register_interpreter,nullptr, "msr_register");
	append_table(process, "110101010011--------------------", (void*)emit_mrs_register_jit, (void*)call_mrs_register_interpreter,nullptr, "mrs_register");
	append_table(process, "11010101000000110010-------11111", (void*)emit_hints_jit, (void*)call_hints_interpreter,nullptr, "hints");
	append_table(process, "1101010100-01-------------------", (void*)emit_sys_jit, (void*)call_sys_interpreter,nullptr, "sys");
	append_table(process, "11010101000000110011------------", (void*)emit_barriers_jit, (void*)call_barriers_interpreter,nullptr, "barriers");
	append_table(process, "--111-00--0---------01----------", (void*)emit_load_store_register_post_jit, (void*)call_load_store_register_post_interpreter,nullptr, "load_store_register_post");
	append_table(process, "--111-00--0---------11----------", (void*)emit_load_store_register_pre_jit, (void*)call_load_store_register_pre_interpreter,nullptr, "load_store_register_pre");
	append_table(process, "--111-00--0---------00----------", (void*)emit_load_store_register_unscaled_jit, (void*)call_load_store_register_unscaled_interpreter,nullptr, "load_store_register_unscaled");
	append_table(process, "--101-010-----------------------", (void*)emit_load_store_register_pair_imm_offset_jit, (void*)call_load_store_register_pair_imm_offset_interpreter,nullptr, "load_store_register_pair_imm_offset");
	append_table(process, "--101-001-----------------------", (void*)emit_load_store_register_pair_imm_post_jit, (void*)call_load_store_register_pair_imm_post_interpreter,nullptr, "load_store_register_pair_imm_post");
	append_table(process, "--101-011-----------------------", (void*)emit_load_store_register_pair_imm_pre_jit, (void*)call_load_store_register_pair_imm_pre_interpreter,nullptr, "load_store_register_pair_imm_pre");
	append_table(process, "--111-01------------------------", (void*)emit_load_store_register_imm_unsigned_jit, (void*)call_load_store_register_imm_unsigned_interpreter,nullptr, "load_store_register_imm_unsigned");
	append_table(process, "--111-00--1---------10----------", (void*)emit_load_store_register_offset_jit, (void*)call_load_store_register_offset_interpreter,nullptr, "load_store_register_offset");
	append_table(process, "--001000--0------11111----------", (void*)emit_load_store_exclusive_ordered_jit, (void*)call_load_store_exclusive_ordered_interpreter,nullptr, "load_store_exclusive_ordered");
	append_table(process, "-0-11110--0---------------------", (void*)emit_conversion_between_floating_point_and_fixed_point_jit, (void*)call_conversion_between_floating_point_and_fixed_point_interpreter,nullptr, "conversion_between_floating_point_and_fixed_point");
	append_table(process, "00011110--10001--10000----------", (void*)emit_fcvt_jit, (void*)call_fcvt_interpreter,nullptr, "fcvt");
	append_table(process, "-0011110--11100-000000----------", (void*)emit_fcvtz_scalar_integer_jit, (void*)call_fcvtz_scalar_integer_interpreter,nullptr, "fcvtz_scalar_integer");
	append_table(process, "-0011110--10000-000000----------", (void*)emit_fcvtn_scalar_integer_jit, (void*)call_fcvtn_scalar_integer_interpreter,nullptr, "fcvtn_scalar_integer");
	append_table(process, "-0011110--10010-000000----------", (void*)emit_fcvta_scalar_integer_jit, (void*)call_fcvta_scalar_integer_interpreter,nullptr, "fcvta_scalar_integer");
	append_table(process, "-0011110--11000-000000----------", (void*)emit_fcvtm_scalar_integer_jit, (void*)call_fcvtm_scalar_integer_interpreter,nullptr, "fcvtm_scalar_integer");
	append_table(process, "00011110--100100110000----------", (void*)emit_frintp_scalar_jit, (void*)call_frintp_scalar_interpreter,nullptr, "frintp_scalar");
	append_table(process, "00011110--100101010000----------", (void*)emit_frintm_scalar_jit, (void*)call_frintm_scalar_interpreter,nullptr, "frintm_scalar");
	append_table(process, "-0011110--10100-000000----------", (void*)emit_fcvtp_scalar_integer_jit, (void*)call_fcvtp_scalar_integer_interpreter,nullptr, "fcvtp_scalar_integer");
	append_table(process, "0-0011100-1-----110101----------", (void*)emit_fadd_vector_jit, (void*)call_fadd_vector_interpreter,nullptr, "fadd_vector");
	append_table(process, "0-1011100-1-----110111----------", (void*)emit_fmul_vector_jit, (void*)call_fmul_vector_interpreter,nullptr, "fmul_vector");
	append_table(process, "0-0011101-1-----110101----------", (void*)emit_fsub_vector_jit, (void*)call_fsub_vector_interpreter,nullptr, "fsub_vector");
	append_table(process, "0-1011100-1-----111111----------", (void*)emit_fdiv_vector_jit, (void*)call_fdiv_vector_interpreter,nullptr, "fdiv_vector");
	append_table(process, "0-001110--1-----110011----------", (void*)emit_fmul_accumulate_vector_jit, (void*)call_fmul_accumulate_vector_interpreter,nullptr, "fmul_accumulate_vector");
	append_table(process, "0-1011100-1-----110101----------", (void*)emit_faddp_vector_jit, (void*)call_faddp_vector_interpreter,nullptr, "faddp_vector");
	append_table(process, "0-1011101-100001110110----------", (void*)emit_frsqrte_vector_jit, (void*)call_frsqrte_vector_interpreter,nullptr, "frsqrte_vector");
	append_table(process, "0-0011101-1-----111111----------", (void*)emit_frsqrts_vector_jit, (void*)call_frsqrts_vector_interpreter,nullptr, "frsqrts_vector");
	append_table(process, "0-0011100-1-----111111----------", (void*)emit_frecps_vector_jit, (void*)call_frecps_vector_interpreter,nullptr, "frecps_vector");
	append_table(process, "010111111-------1001-0----------", (void*)emit_fmul_scalar_by_element_jit, (void*)call_fmul_scalar_by_element_interpreter,nullptr, "fmul_scalar_by_element");
	append_table(process, "0-0011111-------1001-0----------", (void*)emit_fmul_vector_by_element_jit, (void*)call_fmul_vector_by_element_interpreter,nullptr, "fmul_vector_by_element");
	append_table(process, "010111111-------0-01-0----------", (void*)emit_fmul_accumulate_scalar_jit, (void*)call_fmul_accumulate_scalar_interpreter,nullptr, "fmul_accumulate_scalar");
	append_table(process, "0-0011111-------0-01-0----------", (void*)emit_fmul_accumulate_element_jit, (void*)call_fmul_accumulate_element_interpreter,nullptr, "fmul_accumulate_element");
	append_table(process, "011111100-110000110110----------", (void*)emit_faddp_scalar_jit, (void*)call_faddp_scalar_interpreter,nullptr, "faddp_scalar");
	append_table(process, "0-0011101-100000110110----------", (void*)emit_fcmeq_vector_zero_jit, (void*)call_fcmeq_vector_zero_interpreter,nullptr, "fcmeq_vector_zero");
	append_table(process, "0-0011101-100000110010----------", (void*)emit_fcmgt_vector_zero_jit, (void*)call_fcmgt_vector_zero_interpreter,nullptr, "fcmgt_vector_zero");
	append_table(process, "0-1011101-100000110010----------", (void*)emit_fcmge_vector_zero_jit, (void*)call_fcmge_vector_zero_interpreter,nullptr, "fcmge_vector_zero");
	append_table(process, "0-0011100-1-----111001----------", (void*)emit_fcmeq_vector_register_jit, (void*)call_fcmeq_vector_register_interpreter,nullptr, "fcmeq_vector_register");
	append_table(process, "0-1011101-1-----111001----------", (void*)emit_fcmgt_vector_register_jit, (void*)call_fcmgt_vector_register_interpreter,nullptr, "fcmgt_vector_register");
	append_table(process, "0-1011100-1-----111001----------", (void*)emit_fcmge_vector_register_jit, (void*)call_fcmge_vector_register_interpreter,nullptr, "fcmge_vector_register");
	append_table(process, "00011110--1-----001010----------", (void*)emit_fadd_scalar_jit, (void*)call_fadd_scalar_interpreter,nullptr, "fadd_scalar");
	append_table(process, "00011110--1-----001110----------", (void*)emit_fsub_scalar_jit, (void*)call_fsub_scalar_interpreter,nullptr, "fsub_scalar");
	append_table(process, "00011110--1-----000010----------", (void*)emit_fmul_scalar_jit, (void*)call_fmul_scalar_interpreter,nullptr, "fmul_scalar");
	append_table(process, "00011110--1-----000110----------", (void*)emit_fdiv_scalar_jit, (void*)call_fdiv_scalar_interpreter,nullptr, "fdiv_scalar");
	append_table(process, "00011110--1-----010010----------", (void*)emit_fmax_scalar_jit, (void*)call_fmax_scalar_interpreter,nullptr, "fmax_scalar");
	append_table(process, "00011110--1-----010110----------", (void*)emit_fmin_scalar_jit, (void*)call_fmin_scalar_interpreter,nullptr, "fmin_scalar");
	append_table(process, "00011110--1-----011010----------", (void*)emit_fmaxnm_scalar_jit, (void*)call_fmaxnm_scalar_interpreter,nullptr, "fmaxnm_scalar");
	append_table(process, "00011110--1-----011110----------", (void*)emit_fminnm_scalar_jit, (void*)call_fminnm_scalar_interpreter,nullptr, "fminnm_scalar");
	append_table(process, "00011110--1-----100010----------", (void*)emit_fnmul_scalar_jit, (void*)call_fnmul_scalar_interpreter,nullptr, "fnmul_scalar");
	append_table(process, "00011110--100000110000----------", (void*)emit_fabs_scalar_jit, (void*)call_fabs_scalar_interpreter,nullptr, "fabs_scalar");
	append_table(process, "00011110--100001010000----------", (void*)emit_fneg_scalar_jit, (void*)call_fneg_scalar_interpreter,nullptr, "fneg_scalar");
	append_table(process, "0-1011101-100000111110----------", (void*)emit_fneg_vector_jit, (void*)call_fneg_vector_interpreter,nullptr, "fneg_vector");
	append_table(process, "00011110--100001110000----------", (void*)emit_fsqrt_scalar_jit, (void*)call_fsqrt_scalar_interpreter,nullptr, "fsqrt_scalar");
	append_table(process, "0-1011101-100001111110----------", (void*)emit_fsqrt_vector_jit, (void*)call_fsqrt_vector_interpreter,nullptr, "fsqrt_vector");
	append_table(process, "0-0011101-100001110110----------", (void*)emit_frecpe_vector_jit, (void*)call_frecpe_vector_interpreter,nullptr, "frecpe_vector");
	append_table(process, "011111101-100001110110----------", (void*)emit_frsqrte_scalar_jit, (void*)call_frsqrte_scalar_interpreter,nullptr, "frsqrte_scalar");
	append_table(process, "00011110--1--------10000000-----", (void*)emit_fmov_scalar_immediate_jit, (void*)call_fmov_scalar_immediate_interpreter,nullptr, "fmov_scalar_immediate");
	append_table(process, "0-001110000-----000011----------", (void*)emit_dup_general_jit, (void*)call_dup_general_interpreter,nullptr, "dup_general");
	append_table(process, "01011110000-----000001----------", (void*)emit_dup_element_scalar_jit, (void*)call_dup_element_scalar_interpreter,nullptr, "dup_element_scalar");
	append_table(process, "0-001110000-----000001----------", (void*)emit_dup_element_vector_jit, (void*)call_dup_element_vector_interpreter,nullptr, "dup_element_vector");
	append_table(process, "0-001110000-----001-11----------", (void*)emit_move_to_gp_jit, (void*)call_move_to_gp_interpreter,nullptr, "move_to_gp");
	append_table(process, "01001110000-----000111----------", (void*)emit_ins_general_jit, (void*)call_ins_general_interpreter,nullptr, "ins_general");
	append_table(process, "01101110000-----0----1----------", (void*)emit_ins_element_jit, (void*)call_ins_element_interpreter,nullptr, "ins_element");
	append_table(process, "0--0111100000-------01----------", (void*)emit_movi_immediate_jit, (void*)call_movi_immediate_interpreter,nullptr, "movi_immediate");
	append_table(process, "-0011110--10-11-000000----------", (void*)emit_fmov_general_jit, (void*)call_fmov_general_interpreter,nullptr, "fmov_general");
	append_table(process, "-0011110--10001-000000----------", (void*)emit_convert_to_float_gp_jit, (void*)call_convert_to_float_gp_interpreter,nullptr, "convert_to_float_gp");
	append_table(process, "01-111100-100001110110----------", (void*)emit_convert_to_float_vector_scalar_jit, (void*)call_convert_to_float_vector_scalar_interpreter,nullptr, "convert_to_float_vector_scalar");
	append_table(process, "0--011100-100001110110----------", (void*)emit_convert_to_float_vector_jit, (void*)call_convert_to_float_vector_interpreter,nullptr, "convert_to_float_vector");
	append_table(process, "0-0011110-------010101----------", (void*)emit_shl_immedaite_jit, (void*)call_shl_immedaite_interpreter,(void*)help_decode_shl_immedaite, "shl_immedaite");
	append_table(process, "0-0011110-------000001----------", (void*)emit_sshr_vector_jit, (void*)call_sshr_vector_interpreter,(void*)help_decode_sshr_vector, "sshr_vector");
	append_table(process, "0--011110-------101001----------", (void*)emit_shll_shll2_jit, (void*)call_shll_shll2_interpreter,(void*)help_decode_shll_shll2, "shll_shll2");
	append_table(process, "0-0011110-------100001----------", (void*)emit_shrn_jit, (void*)call_shrn_interpreter,(void*)help_decode_shrn, "shrn");
	append_table(process, "0-001110--100000000010----------", (void*)emit_rev64_vector_jit, (void*)call_rev64_vector_interpreter,nullptr, "rev64_vector");
	append_table(process, "0-101110--100000101110----------", (void*)emit_neg_vector_jit, (void*)call_neg_vector_interpreter,nullptr, "neg_vector");
	append_table(process, "0-10111000100000010110----------", (void*)emit_not_vector_jit, (void*)call_not_vector_interpreter,nullptr, "not_vector");
	append_table(process, "0-001110--100000101110----------", (void*)emit_abs_vector_jit, (void*)call_abs_vector_interpreter,nullptr, "abs_vector");
	append_table(process, "0-001111--------1000-0----------", (void*)emit_mul_vector_index_jit, (void*)call_mul_vector_index_interpreter,nullptr, "mul_vector_index");
	append_table(process, "0-001110--1-----100111----------", (void*)emit_mul_vector_jit, (void*)call_mul_vector_interpreter,nullptr, "mul_vector");
	append_table(process, "0-101110000-----0----0----------", (void*)emit_ext_jit, (void*)call_ext_interpreter,nullptr, "ext");
	append_table(process, "0--01110--1-----001101----------", (void*)emit_compare_above_jit, (void*)call_compare_above_interpreter,nullptr, "compare_above");
	append_table(process, "0--01110--1-----010001----------", (void*)emit_shl_vector_jit, (void*)call_shl_vector_interpreter,nullptr, "shl_vector");
	append_table(process, "0-001110--1-----100001----------", (void*)emit_add_vector_jit, (void*)call_add_vector_interpreter,nullptr, "add_vector");
	append_table(process, "0--01110--110000001110----------", (void*)emit_addlv_jit, (void*)call_addlv_interpreter,nullptr, "addlv");
	append_table(process, "0-001110--100000010110----------", (void*)emit_cnt_jit, (void*)call_cnt_interpreter,nullptr, "cnt");
	append_table(process, "0-0011101-1-----000111----------", (void*)emit_orr_orn_vector_jit, (void*)call_orr_orn_vector_interpreter,nullptr, "orr_orn_vector");
	append_table(process, "0-101110011-----000111----------", (void*)emit_bsl_vector_jit, (void*)call_bsl_vector_interpreter,nullptr, "bsl_vector");
	append_table(process, "0-0011100-1-----000111----------", (void*)emit_and_bic_vector_jit, (void*)call_and_bic_vector_interpreter,nullptr, "and_bic_vector");
	append_table(process, "0-101110001-----000111----------", (void*)emit_eor_vector_jit, (void*)call_eor_vector_interpreter,nullptr, "eor_vector");
	append_table(process, "0-001110--100001001010----------", (void*)emit_xnt_xnt2_jit, (void*)call_xnt_xnt2_interpreter,nullptr, "xnt_xnt2");
	append_table(process, "0-001110--0-----0-1110----------", (void*)emit_zip_jit, (void*)call_zip_interpreter,nullptr, "zip");
	append_table(process, "0-001110--0-----0-1010----------", (void*)emit_trn_jit, (void*)call_trn_interpreter,nullptr, "trn");
	append_table(process, "0-001110000-----0--000----------", (void*)emit_tbl_jit, (void*)call_tbl_interpreter,nullptr, "tbl");
	append_table(process, "0-001101010000001100------------", (void*)emit_ld1r_no_offset_jit, (void*)call_ld1r_no_offset_interpreter,nullptr, "ld1r_no_offset");
	append_table(process, "0-001101110-----1100------------", (void*)emit_ld1r_post_index_jit, (void*)call_ld1r_post_index_interpreter,nullptr, "ld1r_post_index");
	append_table(process, "0-00110101000000--0-------------", (void*)emit_ld1_single_structure_no_offset_jit, (void*)call_ld1_single_structure_no_offset_interpreter,(void*)help_decode_ld1_single_structure_no_offset, "ld1_single_structure_no_offset");
	append_table(process, "0-001101110-------0-------------", (void*)emit_ld1_single_structure_post_index_jit, (void*)call_ld1_single_structure_post_index_interpreter,(void*)help_decode_ld1_single_structure_post_index, "ld1_single_structure_post_index");
	append_table(process, "0-001100000000001000------------", (void*)emit_st2_multiple_structures_no_offset_jit, (void*)call_st2_multiple_structures_no_offset_interpreter,nullptr, "st2_multiple_structures_no_offset");
	append_table(process, "0-001100100-----1000------------", (void*)emit_st2_multiple_structures_post_index_jit, (void*)call_st2_multiple_structures_post_index_interpreter,nullptr, "st2_multiple_structures_post_index");
	append_table(process, "0-00110100000000--0-------------", (void*)emit_st1_single_structure_no_offset_jit, (void*)call_st1_single_structure_no_offset_interpreter,nullptr, "st1_single_structure_no_offset");
	append_table(process, "0-001101100-------0-------------", (void*)emit_st1_single_structure_post_index_jit, (void*)call_st1_single_structure_post_index_interpreter,nullptr, "st1_single_structure_post_index");
	append_table(process, "00011110--1---------11----------", (void*)emit_floating_point_conditional_select_jit, (void*)call_floating_point_conditional_select_interpreter,nullptr, "floating_point_conditional_select");
	append_table(process, "00011110--1-----001000-----0-000", (void*)emit_fcmp_jit, (void*)call_fcmp_interpreter,nullptr, "fcmp");
	append_table(process, "00011110--1---------01-----0----", (void*)emit_fccmp_jit, (void*)call_fccmp_interpreter,nullptr, "fccmp");
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
	if ((((uint64_t)set_flags && (uint64_t)use_x86_interpreter(ctx))))
	{
		if ((is_add))
		{
			d = x86_add_set_flags_interpreter<O>(ctx,n,m);
		}
		else
		{
			d = x86_subtract_set_flags_interpreter<O>(ctx,n,m);
		}
		return d;
	}
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

uint64_t select_interpreter(interpreter_data* ctx, uint64_t condition, uint64_t yes, uint64_t no)
{
	if ((condition))
	return yes;
	return no;
}

uint64_t create_mask_interpreter(interpreter_data* ctx, uint64_t bits)
{
	if ((((uint64_t)bits >= (uint64_t)64ULL)))
	return -1ULL;
	return ((uint64_t)(((uint64_t)1ULL << (uint64_t)bits)) - (uint64_t)1ULL);
}

uint64_t shift_left_check_interpreter(interpreter_data* ctx, uint64_t to_shift, uint64_t shift, uint64_t size)
{
	uint64_t result = 0ULL;
	if ((((uint64_t)(sign_extend((uint64_t)shift) >= sign_extend((uint64_t)size)))))
	{
		result = 0ULL;
	}
	else
	{
		result = ((uint64_t)to_shift << (uint64_t)shift);
	}
	return result;
}

uint64_t get_x86_rounding_mode_interpreter(interpreter_data* ctx, uint64_t rounding)
{
	uint64_t rounding_control;
	if ((((uint64_t)rounding == (uint64_t)FPRounding_TIEEVEN)))
	{
		rounding_control = 0ULL;
	}
	else if ((((uint64_t)rounding == (uint64_t)FPRounding_NEGINF)))
	{
		rounding_control = 1ULL;
	}
	else if ((((uint64_t)rounding == (uint64_t)FPRounding_POSINF)))
	{
		rounding_control = 2ULL;
	}
	else if ((((uint64_t)rounding == (uint64_t)FPRounding_ZERO)))
	{
		rounding_control = 3ULL;
	}
	else
	{
		undefined_interpreter(ctx);
	}
	return rounding_control;
}

uint64_t shift_right_check_interpreter(interpreter_data* ctx, uint64_t to_shift, uint64_t shift, uint64_t size, uint64_t is_unsigned)
{
	uint64_t result = 0ULL;
	if ((((uint64_t)(sign_extend((uint64_t)shift) >= sign_extend((uint64_t)size)))))
	{
		if ((((uint64_t)(((uint64_t)(sign_extend((uint64_t)to_shift) < sign_extend((uint64_t)0ULL)))) && (uint64_t)!is_unsigned)))
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
		if ((is_unsigned))
		{
			result = ((uint64_t)to_shift >> (uint64_t)shift);
		}
		else
		{
			result = ((uint64_t)(sign_extend((uint64_t)to_shift) >> sign_extend((uint64_t)shift)));
		}
	}
	return result;
}

uint64_t reverse_interpreter(interpreter_data* ctx, uint128_t word, uint64_t M, uint64_t N)
{
	uint128_t result = 0;
	uint64_t swsize = M;
	uint64_t sw = ((uint64_t)N / (uint64_t)swsize);
	for (uint64_t s = 0; s < (sw); s++)
	{
		uint128_t::insert(&result, (((uint64_t)(((uint64_t)sw - (uint64_t)1ULL)) - (uint64_t)s)), swsize, (uint128_t::extract(word, s, swsize)));
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

uint64_t VPart_interpreter(interpreter_data* ctx, uint64_t n, uint64_t part, uint64_t width)
{
	return uint128_t::extract(V_interpreter(ctx,n), part, width);
}

uint64_t get_from_concacted_vector_interpreter(interpreter_data* ctx, uint128_t top, uint128_t bottom, uint64_t index, uint64_t element_count, uint64_t element_size)
{
	uint128_t working = bottom;
	if ((((uint64_t)index >= (uint64_t)element_count)))
	{
		index = ((uint64_t)index - (uint64_t)element_count);
		working = top;
	}
	return uint128_t::extract(working, index, element_size);
}

uint64_t call_float_binary_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t fpcr, uint64_t N, uint64_t function)
{
	return call_interpreter(ctx,operand1,operand2,fpcr,(uint64_t)N,(uint64_t)0ULL,(uint64_t)0ULL,function);
}

uint64_t call_float_unary_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t fpcr, uint64_t N, uint64_t function)
{
	return call_interpreter(ctx,operand,fpcr,(uint64_t)N,(uint64_t)0ULL,(uint64_t)0ULL,(uint64_t)0ULL,function);
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

uint128_t replicate_vector_interpreter(interpreter_data* ctx, uint128_t source, uint64_t v_size, uint64_t count)
{
	uint128_t result = 0;
	for (uint64_t e = 0; e < (count); e++)
	{
		uint128_t::insert(&result, e, v_size, (uint128_t::extract(source, 0ULL, 64ULL)));
	}
	return result;
}

void st_interpreter(interpreter_data* ctx, uint64_t wback, uint64_t Q, uint64_t L, uint64_t opcode, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rt)
{
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t ebytes = ((uint64_t)esize / (uint64_t)8ULL);
	uint64_t rpt = 1ULL;
	uint64_t selem = 2ULL;
	uint64_t address = XSP_interpreter(ctx,Rn);
	uint64_t offs = 0ULL;
	uint64_t t = Rt;
	uint64_t m = Rm;
	if ((((uint64_t)ebytes * (uint64_t)8ULL)) == 8ULL)
	{
		for (uint64_t r = 0; r < (rpt); r++)
		{
			for (uint64_t e = 0; e < (elements); e++)
			{
				uint64_t tt = ((uint64_t)(((uint64_t)t + (uint64_t)r)) % (uint64_t)32ULL);
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,tt);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					mem_interpreter<uint8_t>(ctx,eaddr,(uint8_t)uint128_t::extract(rval, e, esize));
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					tt = ((uint64_t)(((uint64_t)tt + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
		}
		if ((wback))
		{
			uint64_t _offs = offs;
			if ((((uint64_t)Rm != (uint64_t)31ULL)))
			{
				_offs = X_interpreter(ctx,Rm);
			}
			address = ((uint64_t)address + (uint64_t)_offs);
			XSP_interpreter(ctx,Rn,address);
		}
	}
	if ((((uint64_t)ebytes * (uint64_t)8ULL)) == 16ULL)
	{
		for (uint64_t r = 0; r < (rpt); r++)
		{
			for (uint64_t e = 0; e < (elements); e++)
			{
				uint64_t tt = ((uint64_t)(((uint64_t)t + (uint64_t)r)) % (uint64_t)32ULL);
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,tt);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					mem_interpreter<uint16_t>(ctx,eaddr,(uint16_t)uint128_t::extract(rval, e, esize));
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					tt = ((uint64_t)(((uint64_t)tt + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
		}
		if ((wback))
		{
			uint64_t _offs = offs;
			if ((((uint64_t)Rm != (uint64_t)31ULL)))
			{
				_offs = X_interpreter(ctx,Rm);
			}
			address = ((uint64_t)address + (uint64_t)_offs);
			XSP_interpreter(ctx,Rn,address);
		}
	}
	if ((((uint64_t)ebytes * (uint64_t)8ULL)) == 32ULL)
	{
		for (uint64_t r = 0; r < (rpt); r++)
		{
			for (uint64_t e = 0; e < (elements); e++)
			{
				uint64_t tt = ((uint64_t)(((uint64_t)t + (uint64_t)r)) % (uint64_t)32ULL);
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,tt);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					mem_interpreter<uint32_t>(ctx,eaddr,(uint32_t)uint128_t::extract(rval, e, esize));
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					tt = ((uint64_t)(((uint64_t)tt + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
		}
		if ((wback))
		{
			uint64_t _offs = offs;
			if ((((uint64_t)Rm != (uint64_t)31ULL)))
			{
				_offs = X_interpreter(ctx,Rm);
			}
			address = ((uint64_t)address + (uint64_t)_offs);
			XSP_interpreter(ctx,Rn,address);
		}
	}
	if ((((uint64_t)ebytes * (uint64_t)8ULL)) == 64ULL)
	{
		for (uint64_t r = 0; r < (rpt); r++)
		{
			for (uint64_t e = 0; e < (elements); e++)
			{
				uint64_t tt = ((uint64_t)(((uint64_t)t + (uint64_t)r)) % (uint64_t)32ULL);
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,tt);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					mem_interpreter<uint64_t>(ctx,eaddr,(uint64_t)uint128_t::extract(rval, e, esize));
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					tt = ((uint64_t)(((uint64_t)tt + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
		}
		if ((wback))
		{
			uint64_t _offs = offs;
			if ((((uint64_t)Rm != (uint64_t)31ULL)))
			{
				_offs = X_interpreter(ctx,Rm);
			}
			address = ((uint64_t)address + (uint64_t)_offs);
			XSP_interpreter(ctx,Rn,address);
		}
	}
	if ((((uint64_t)ebytes * (uint64_t)8ULL)) == 128ULL)
	{
		for (uint64_t r = 0; r < (rpt); r++)
		{
			for (uint64_t e = 0; e < (elements); e++)
			{
				uint64_t tt = ((uint64_t)(((uint64_t)t + (uint64_t)r)) % (uint64_t)32ULL);
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,tt);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					mem_interpreter<uint128_t>(ctx,eaddr,(uint128_t)uint128_t::extract(rval, e, esize));
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					tt = ((uint64_t)(((uint64_t)tt + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
		}
		if ((wback))
		{
			uint64_t _offs = offs;
			if ((((uint64_t)Rm != (uint64_t)31ULL)))
			{
				_offs = X_interpreter(ctx,Rm);
			}
			address = ((uint64_t)address + (uint64_t)_offs);
			XSP_interpreter(ctx,Rn,address);
		}
	}
	
}

void memory_1_interpreter(interpreter_data* ctx, uint64_t wback, uint64_t Q, uint64_t L, uint64_t R, uint64_t Rm, uint64_t o2, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt, uint64_t is_load)
{
	uint64_t scale = bits_c_interpreter(ctx,opcode,2ULL,1ULL);
	uint64_t selem = ((uint64_t)(((uint64_t)(((uint64_t)bit_c_interpreter(ctx,opcode,0ULL) << (uint64_t)1ULL)) | (uint64_t)R)) + (uint64_t)1ULL);
	uint64_t replicate = 0ULL;
	uint64_t index;
	if ((((uint64_t)scale == (uint64_t)3ULL)))
	{
		scale = size;
		replicate = 1ULL;
	}
	else if ((((uint64_t)scale == (uint64_t)0ULL)))
	{
		index = Q;
		index = ((uint64_t)(((uint64_t)index << (uint64_t)1ULL)) | (uint64_t)S);
		index = ((uint64_t)(((uint64_t)index << (uint64_t)2ULL)) | (uint64_t)size);
	}
	else if ((((uint64_t)scale == (uint64_t)1ULL)))
	{
		index = Q;
		index = ((uint64_t)(((uint64_t)index << (uint64_t)1ULL)) | (uint64_t)S);
		index = ((uint64_t)(((uint64_t)index << (uint64_t)1ULL)) | (uint64_t)bit_c_interpreter(ctx,size,1ULL));
	}
	else if ((((uint64_t)scale == (uint64_t)2ULL)))
	{
		if ((((uint64_t)(((uint64_t)size & (uint64_t)1ULL)) == (uint64_t)0ULL)))
		{
			index = ((uint64_t)(((uint64_t)Q << (uint64_t)1ULL)) | (uint64_t)S);
		}
		else
		{
			index = Q;
			scale = 3ULL;
		}
	}
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)scale);
	uint64_t ebytes = ((uint64_t)esize / (uint64_t)8ULL);
	uint64_t address = XSP_interpreter(ctx,Rn);
	uint64_t offs = 0ULL;
	if ((((uint64_t)ebytes * (uint64_t)8ULL)) == 8ULL)
	{
		uint64_t t = Rt;
		if ((replicate))
		{
			for (uint64_t s = 0; s < (selem); s++)
			{
				uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
				uint8_t element = mem_interpreter<uint8_t>(ctx,eaddr);
				V_interpreter(ctx,t,replicate_vector_interpreter(ctx,(uint128_t)element,esize,((uint64_t)datasize / (uint64_t)esize)));
				offs = ((uint64_t)offs + (uint64_t)ebytes);
				t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
			}
		}
		else
		{
			if ((is_load))
			{
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,t);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					uint128_t::insert(&rval, index, esize, mem_interpreter<uint8_t>(ctx,eaddr));
					V_interpreter(ctx,t,rval);
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
			else
			{
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,t);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					mem_interpreter<uint8_t>(ctx,eaddr,(uint8_t)uint128_t::extract(rval, index, esize));
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
		}
		if ((wback))
		{
			uint64_t _offs = offs;
			if ((((uint64_t)Rm != (uint64_t)31ULL)))
			{
				_offs = X_interpreter(ctx,Rm);
			}
			address = ((uint64_t)address + (uint64_t)_offs);
			XSP_interpreter(ctx,Rn,address);
		}
	}
	if ((((uint64_t)ebytes * (uint64_t)8ULL)) == 16ULL)
	{
		uint64_t t = Rt;
		if ((replicate))
		{
			for (uint64_t s = 0; s < (selem); s++)
			{
				uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
				uint16_t element = mem_interpreter<uint16_t>(ctx,eaddr);
				V_interpreter(ctx,t,replicate_vector_interpreter(ctx,(uint128_t)element,esize,((uint64_t)datasize / (uint64_t)esize)));
				offs = ((uint64_t)offs + (uint64_t)ebytes);
				t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
			}
		}
		else
		{
			if ((is_load))
			{
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,t);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					uint128_t::insert(&rval, index, esize, mem_interpreter<uint16_t>(ctx,eaddr));
					V_interpreter(ctx,t,rval);
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
			else
			{
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,t);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					mem_interpreter<uint16_t>(ctx,eaddr,(uint16_t)uint128_t::extract(rval, index, esize));
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
		}
		if ((wback))
		{
			uint64_t _offs = offs;
			if ((((uint64_t)Rm != (uint64_t)31ULL)))
			{
				_offs = X_interpreter(ctx,Rm);
			}
			address = ((uint64_t)address + (uint64_t)_offs);
			XSP_interpreter(ctx,Rn,address);
		}
	}
	if ((((uint64_t)ebytes * (uint64_t)8ULL)) == 32ULL)
	{
		uint64_t t = Rt;
		if ((replicate))
		{
			for (uint64_t s = 0; s < (selem); s++)
			{
				uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
				uint32_t element = mem_interpreter<uint32_t>(ctx,eaddr);
				V_interpreter(ctx,t,replicate_vector_interpreter(ctx,(uint128_t)element,esize,((uint64_t)datasize / (uint64_t)esize)));
				offs = ((uint64_t)offs + (uint64_t)ebytes);
				t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
			}
		}
		else
		{
			if ((is_load))
			{
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,t);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					uint128_t::insert(&rval, index, esize, mem_interpreter<uint32_t>(ctx,eaddr));
					V_interpreter(ctx,t,rval);
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
			else
			{
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,t);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					mem_interpreter<uint32_t>(ctx,eaddr,(uint32_t)uint128_t::extract(rval, index, esize));
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
		}
		if ((wback))
		{
			uint64_t _offs = offs;
			if ((((uint64_t)Rm != (uint64_t)31ULL)))
			{
				_offs = X_interpreter(ctx,Rm);
			}
			address = ((uint64_t)address + (uint64_t)_offs);
			XSP_interpreter(ctx,Rn,address);
		}
	}
	if ((((uint64_t)ebytes * (uint64_t)8ULL)) == 64ULL)
	{
		uint64_t t = Rt;
		if ((replicate))
		{
			for (uint64_t s = 0; s < (selem); s++)
			{
				uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
				uint64_t element = mem_interpreter<uint64_t>(ctx,eaddr);
				V_interpreter(ctx,t,replicate_vector_interpreter(ctx,(uint128_t)element,esize,((uint64_t)datasize / (uint64_t)esize)));
				offs = ((uint64_t)offs + (uint64_t)ebytes);
				t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
			}
		}
		else
		{
			if ((is_load))
			{
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,t);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					uint128_t::insert(&rval, index, esize, mem_interpreter<uint64_t>(ctx,eaddr));
					V_interpreter(ctx,t,rval);
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
			else
			{
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,t);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					mem_interpreter<uint64_t>(ctx,eaddr,(uint64_t)uint128_t::extract(rval, index, esize));
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
		}
		if ((wback))
		{
			uint64_t _offs = offs;
			if ((((uint64_t)Rm != (uint64_t)31ULL)))
			{
				_offs = X_interpreter(ctx,Rm);
			}
			address = ((uint64_t)address + (uint64_t)_offs);
			XSP_interpreter(ctx,Rn,address);
		}
	}
	if ((((uint64_t)ebytes * (uint64_t)8ULL)) == 128ULL)
	{
		uint64_t t = Rt;
		if ((replicate))
		{
			for (uint64_t s = 0; s < (selem); s++)
			{
				uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
				uint128_t element = mem_interpreter<uint128_t>(ctx,eaddr);
				V_interpreter(ctx,t,replicate_vector_interpreter(ctx,(uint128_t)element,esize,((uint64_t)datasize / (uint64_t)esize)));
				offs = ((uint64_t)offs + (uint64_t)ebytes);
				t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
			}
		}
		else
		{
			if ((is_load))
			{
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,t);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					uint128_t::insert(&rval, index, esize, mem_interpreter<uint128_t>(ctx,eaddr));
					V_interpreter(ctx,t,rval);
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
			else
			{
				for (uint64_t s = 0; s < (selem); s++)
				{
					uint128_t rval = V_interpreter(ctx,t);
					uint64_t eaddr = ((uint64_t)address + (uint64_t)offs);
					mem_interpreter<uint128_t>(ctx,eaddr,(uint128_t)uint128_t::extract(rval, index, esize));
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
		}
		if ((wback))
		{
			uint64_t _offs = offs;
			if ((((uint64_t)Rm != (uint64_t)31ULL)))
			{
				_offs = X_interpreter(ctx,Rm);
			}
			address = ((uint64_t)address + (uint64_t)_offs);
			XSP_interpreter(ctx,Rn,address);
		}
	}
	
}

uint64_t bits_r_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t top, uint64_t bottom)
{
	top = ((uint64_t)top + (uint64_t)1ULL);
	uint64_t size = ((uint64_t)top - (uint64_t)bottom);
	uint64_t mask = ((uint64_t)(((uint64_t)1ULL << (uint64_t)size)) - (uint64_t)1ULL);
	return ((uint64_t)(((uint64_t)operand >> (uint64_t)bottom)) & (uint64_t)mask);
}

uint64_t infinity_interpreter(interpreter_data* ctx, uint64_t sign, uint64_t N)
{
	uint64_t result = ((uint64_t)sign << (uint64_t)(((uint64_t)N - (uint64_t)1ULL)));
	if ((((uint64_t)N == (uint64_t)32ULL)))
	{
		result = ((uint64_t)result | (uint64_t)(((uint64_t)255ULL << (uint64_t)23ULL)));
	}
	else if ((((uint64_t)N == (uint64_t)64ULL)))
	{
		result = ((uint64_t)result | (uint64_t)(((uint64_t)2047ULL << (uint64_t)52ULL)));
	}
	else
	{
		undefined_interpreter(ctx);
	}
	return result;
}

uint64_t float_is_nan_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t N)
{
	uint8_t result = 0ULL;
	uint64_t exp;
	uint64_t frac;
	if ((((uint64_t)N == (uint64_t)32ULL)))
	{
		exp = bits_r_interpreter(ctx,operand,30ULL,23ULL);
		frac = bits_r_interpreter(ctx,operand,22ULL,0ULL);
		if ((((uint64_t)((uint64_t)exp == (uint64_t)255ULL) && (uint64_t)((uint64_t)frac != (uint64_t)0ULL))))
		{
			result = 1ULL;
		}
	}
	else if ((((uint64_t)N == (uint64_t)64ULL)))
	{
		exp = bits_r_interpreter(ctx,operand,62ULL,52ULL);
		frac = bits_r_interpreter(ctx,operand,51ULL,0ULL);
		if ((((uint64_t)((uint64_t)exp == (uint64_t)2047ULL) && (uint64_t)((uint64_t)frac != (uint64_t)0ULL))))
		{
			result = 1ULL;
		}
	}
	else
	{
		undefined_interpreter(ctx);
	}
	return result;
}

uint64_t float_imm_interpreter(interpreter_data* ctx, uint64_t source, uint64_t N)
{
	if ((((uint64_t)N == (uint64_t)32ULL)))
	{
		return convert_to_float<uint32_t, uint64_t>(source, 0);
	}
	else if ((((uint64_t)N == (uint64_t)64ULL)))
	{
		return convert_to_float<uint64_t, uint64_t>(source, 0);
	}
	else
	{
		undefined_interpreter(ctx);
	}
}

template <typename F>
F create_fixed_from_fbits_interpreter(interpreter_data* ctx, uint64_t fbits, uint64_t N)
{
	uint64_t working;
	if ((((uint64_t)fbits == (uint64_t)64ULL)))
	{
		return (undefined_value());
	}
	else if ((((uint64_t)fbits > (uint64_t)64ULL)))
	{
		undefined_interpreter(ctx);
	}
	return float_imm_interpreter(ctx,(uint64_t)((uint64_t)1ULL << (uint64_t)fbits),N);
}

uint64_t FPAdd_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t o1 = operand1;
			uint32_t o2 = operand2;
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			uint64_t o1 = operand1;
			uint64_t o2 = operand2;
			return (undefined_value());
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPAdd_I);
}

uint64_t FPSub_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t o1 = operand1;
			uint32_t o2 = operand2;
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			uint64_t o1 = operand1;
			uint64_t o2 = operand2;
			return (undefined_value());
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPSub_I);
}

uint64_t FPMul_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t o1 = operand1;
			uint32_t o2 = operand2;
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			uint64_t o1 = operand1;
			uint64_t o2 = operand2;
			return (undefined_value());
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPMul_I);
}

uint64_t FPNMul_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	uint64_t result = FPMul_interpreter(ctx,operand1,operand2,FPCR,N);
	return FPNeg_interpreter(ctx,result,FPCR,N);
}

uint64_t FPDiv_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t o1 = operand1;
			uint32_t o2 = operand2;
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			uint64_t o1 = operand1;
			uint64_t o2 = operand2;
			return (undefined_value());
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPDiv_I);
}

uint64_t FPMax_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t o1 = operand1;
			uint32_t o2 = operand2;
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			uint64_t o1 = operand1;
			uint64_t o2 = operand2;
			return (undefined_value());
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPMax_I);
}

uint64_t FPMin_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t o1 = operand1;
			uint32_t o2 = operand2;
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			uint64_t o1 = operand1;
			uint64_t o2 = operand2;
			return (undefined_value());
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPMin_I);
}

uint64_t FPMaxNum_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t type_1_nan = float_is_nan_interpreter(ctx,operand1,N);
		uint64_t type_2_nan = float_is_nan_interpreter(ctx,operand2,N);
		if ((((uint64_t)type_1_nan && (uint64_t)!type_2_nan)))
		{
			operand1 = infinity_interpreter(ctx,1ULL,N);
		}
		else if ((((uint64_t)!type_1_nan && (uint64_t)type_2_nan)))
		{
			operand2 = infinity_interpreter(ctx,1ULL,N);
		}
		if (N == 32ULL)
		{
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			return (undefined_value());
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPMaxNum_I);
}

uint64_t FPMinNum_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t type_1_nan = float_is_nan_interpreter(ctx,operand1,N);
		uint64_t type_2_nan = float_is_nan_interpreter(ctx,operand2,N);
		if ((((uint64_t)type_1_nan && (uint64_t)!type_2_nan)))
		{
			operand1 = infinity_interpreter(ctx,0ULL,N);
		}
		else if ((((uint64_t)!type_1_nan && (uint64_t)type_2_nan)))
		{
			operand2 = infinity_interpreter(ctx,0ULL,N);
		}
		if (N == 32ULL)
		{
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			return (undefined_value());
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPMinNum_I);
}

uint64_t FPCompare_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t o1 = operand1;
			uint32_t o2 = operand2;
			uint32_t result = 0ULL;
			uint64_t type_1_nan = float_is_nan_interpreter(ctx,(uint64_t)o1,N);
			uint64_t type_2_nan = float_is_nan_interpreter(ctx,(uint64_t)o2,N);
			if ((((uint64_t)type_1_nan | (uint64_t)type_2_nan)))
			{
				result = 3ULL;
			}
			else
			{
				if (((undefined_value())))
				{
					result = 6ULL;
				}
				else if (((undefined_value())))
				{
					result = 8ULL;
				}
				else
				{
					result = 2ULL;
				}
			}
			return result;
		}
		if (N == 64ULL)
		{
			uint64_t o1 = operand1;
			uint64_t o2 = operand2;
			uint64_t result = 0ULL;
			uint64_t type_1_nan = float_is_nan_interpreter(ctx,(uint64_t)o1,N);
			uint64_t type_2_nan = float_is_nan_interpreter(ctx,(uint64_t)o2,N);
			if ((((uint64_t)type_1_nan | (uint64_t)type_2_nan)))
			{
				result = 3ULL;
			}
			else
			{
				if (((undefined_value())))
				{
					result = 6ULL;
				}
				else if (((undefined_value())))
				{
					result = 8ULL;
				}
				else
				{
					result = 2ULL;
				}
			}
			return result;
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPCompare_I);
}

uint64_t FPRSqrtStepFused_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t o1 = ((uint64_t)operand1 ^ (uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)N - (uint64_t)1ULL)))));
			uint32_t o2 = operand2;
			uint32_t three = float_imm_interpreter(ctx,(uint64_t)3ULL,N);
			uint32_t two = float_imm_interpreter(ctx,(uint64_t)2ULL,N);
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			uint64_t o1 = ((uint64_t)operand1 ^ (uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)N - (uint64_t)1ULL)))));
			uint64_t o2 = operand2;
			uint64_t three = float_imm_interpreter(ctx,(uint64_t)3ULL,N);
			uint64_t two = float_imm_interpreter(ctx,(uint64_t)2ULL,N);
			return (undefined_value());
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPRSqrtStepFused_I);
}

uint64_t FPRecipStepFused_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t o1 = ((uint64_t)operand1 ^ (uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)N - (uint64_t)1ULL)))));
			uint32_t o2 = operand2;
			uint32_t two = float_imm_interpreter(ctx,(uint64_t)2ULL,N);
			return ((undefined_value()));
		}
		if (N == 64ULL)
		{
			uint64_t o1 = ((uint64_t)operand1 ^ (uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)N - (uint64_t)1ULL)))));
			uint64_t o2 = operand2;
			uint64_t two = float_imm_interpreter(ctx,(uint64_t)2ULL,N);
			return ((undefined_value()));
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPRecipStepFused_I);
}

uint64_t FPCompareEQ_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t o1 = operand1;
			uint32_t o2 = operand2;
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			uint64_t o1 = operand1;
			uint64_t o2 = operand2;
			return (undefined_value());
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPCompareEQ_I);
}

uint64_t FPCompareGT_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t o1 = operand1;
			uint32_t o2 = operand2;
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			uint64_t o1 = operand1;
			uint64_t o2 = operand2;
			return (undefined_value());
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPCompareGT_I);
}

uint64_t FPCompareGE_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t o1 = operand1;
			uint32_t o2 = operand2;
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			uint64_t o1 = operand1;
			uint64_t o2 = operand2;
			return (undefined_value());
		}
		
	}
	return call_float_binary_interpreter(ctx,operand1,operand2,FPCR,N,(uint64_t)FPCompareGE_I);
}

uint64_t FPSqrt_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			return undefined_value();
		}
		if (N == 64ULL)
		{
			return undefined_value();
		}
		
	}
	return call_float_unary_interpreter(ctx,operand,FPCR,N,(uint64_t)FPSqrt_I);
}

uint64_t FPNeg_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t FPCR, uint64_t N)
{
	if ((use_fast_float_interpreter(ctx)))
	{
		return ((uint64_t)operand ^ (uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)N - (uint64_t)1ULL)))));
	}
	return call_float_unary_interpreter(ctx,operand,FPCR,N,(uint64_t)FPNeg_I);
}

uint64_t FPAbs_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t mask = ((uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)N - (uint64_t)1ULL)))) - (uint64_t)1ULL);
		return ((uint64_t)operand & (uint64_t)mask);
	}
	return call_float_unary_interpreter(ctx,operand,FPCR,N,(uint64_t)FPAbs_I);
}

uint64_t FPRSqrtEstimate_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t one = float_imm_interpreter(ctx,(uint64_t)1ULL,N);
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			uint64_t one = float_imm_interpreter(ctx,(uint64_t)1ULL,N);
			return (undefined_value());
		}
		
	}
	return call_float_unary_interpreter(ctx,operand,FPCR,N,(uint64_t)FPRSqrtEstimate_I);
}

uint64_t FPRecipEstimate_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		if (N == 32ULL)
		{
			uint32_t one = float_imm_interpreter(ctx,(uint64_t)1ULL,N);
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			uint64_t one = float_imm_interpreter(ctx,(uint64_t)1ULL,N);
			return (undefined_value());
		}
		
	}
	return call_float_unary_interpreter(ctx,operand,FPCR,N,(uint64_t)FPRecipEstimate_I);
}

uint64_t FixedToFP_interpreter(interpreter_data* ctx, uint64_t source, uint64_t fracbits, uint64_t is_unsigned, uint64_t to, uint64_t from)
{
	if ((((uint64_t)((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)to != (uint64_t)16ULL)) && (uint64_t)((uint64_t)from != (uint64_t)16ULL))))
	{
		if (to == 32ULL)
		{
			if (from == 32ULL)
			{
				if ((((uint64_t)from == (uint64_t)32ULL)))
				{
					source = ((uint64_t)source & (uint64_t)4294967295ULL);
				}
				uint32_t power = create_fixed_from_fbits_interpreter<uint32_t>(ctx,fracbits,to);
				uint32_t working_result;
				if ((is_unsigned))
				{
					working_result = convert_to_float<uint32_t, uint32_t>((uint32_t)source, 0);
				}
				else
				{
					working_result = convert_to_float<uint32_t, uint32_t>((uint32_t)source, 1);
				}
				if ((((uint64_t)fracbits == (uint64_t)0ULL)))
				{
					return working_result;
				}
				return (undefined_value());
			}
			if (from == 64ULL)
			{
				if ((((uint64_t)from == (uint64_t)32ULL)))
				{
					source = ((uint64_t)source & (uint64_t)4294967295ULL);
				}
				uint32_t power = create_fixed_from_fbits_interpreter<uint32_t>(ctx,fracbits,to);
				uint32_t working_result;
				if ((is_unsigned))
				{
					working_result = convert_to_float<uint32_t, uint64_t>((uint64_t)source, 0);
				}
				else
				{
					working_result = convert_to_float<uint32_t, uint64_t>((uint64_t)source, 1);
				}
				if ((((uint64_t)fracbits == (uint64_t)0ULL)))
				{
					return working_result;
				}
				return (undefined_value());
			}
			
		}
		if (to == 64ULL)
		{
			if (from == 32ULL)
			{
				if ((((uint64_t)from == (uint64_t)32ULL)))
				{
					source = ((uint64_t)source & (uint64_t)4294967295ULL);
				}
				uint64_t power = create_fixed_from_fbits_interpreter<uint64_t>(ctx,fracbits,to);
				uint64_t working_result;
				if ((is_unsigned))
				{
					working_result = convert_to_float<uint64_t, uint32_t>((uint32_t)source, 0);
				}
				else
				{
					working_result = convert_to_float<uint64_t, uint32_t>((uint32_t)source, 1);
				}
				if ((((uint64_t)fracbits == (uint64_t)0ULL)))
				{
					return working_result;
				}
				return (undefined_value());
			}
			if (from == 64ULL)
			{
				if ((((uint64_t)from == (uint64_t)32ULL)))
				{
					source = ((uint64_t)source & (uint64_t)4294967295ULL);
				}
				uint64_t power = create_fixed_from_fbits_interpreter<uint64_t>(ctx,fracbits,to);
				uint64_t working_result;
				if ((is_unsigned))
				{
					working_result = convert_to_float<uint64_t, uint64_t>((uint64_t)source, 0);
				}
				else
				{
					working_result = convert_to_float<uint64_t, uint64_t>((uint64_t)source, 1);
				}
				if ((((uint64_t)fracbits == (uint64_t)0ULL)))
				{
					return working_result;
				}
				return (undefined_value());
			}
			
		}
		
	}
	return call_interpreter(ctx,source,(uint64_t)fracbits,(uint64_t)is_unsigned,(uint64_t)to,(uint64_t)from,(uint64_t)0ULL,(uint64_t)FixedToFP_I);
}

uint64_t FPToFixed_interpreter(interpreter_data* ctx, uint64_t source, uint64_t fracbits, uint64_t is_unsigned, uint64_t round, uint64_t to, uint64_t from)
{
	if ((((uint64_t)((uint64_t)((uint64_t)((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)to != (uint64_t)16ULL)) && (uint64_t)((uint64_t)from != (uint64_t)16ULL)) && (uint64_t)((uint64_t)round != (uint64_t)FPRounding_ODD)) && (uint64_t)use_x86_sse41_interpreter(ctx))))
	{
		if (from == 32ULL)
		{
			if (to == 32ULL)
			{
				uint64_t max_i;
				uint64_t min_i;
				uint32_t max;
				uint32_t min;
				if ((is_unsigned))
				{
					min = 0ULL;
					max = convert_to_float<uint32_t, uint64_t>(create_mask_interpreter(ctx,to), 0);
					min_i = 0ULL;
					max_i = create_mask_interpreter(ctx,to);
				}
				else
				{
					min = convert_to_float<uint32_t, uint32_t>((uint32_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)to - (uint64_t)1ULL)))), 1);
					max = convert_to_float<uint32_t, uint32_t>((uint32_t)(create_mask_interpreter(ctx,((uint64_t)to - (uint64_t)1ULL))), 1);
					min_i = (uint32_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)to - (uint64_t)1ULL))));
					max_i = (uint32_t)(create_mask_interpreter(ctx,((uint64_t)to - (uint64_t)1ULL)));
				}
				uint32_t working = source;
				if ((((uint64_t)fracbits != (uint64_t)0ULL)))
				{
					uint32_t power = create_fixed_from_fbits_interpreter<uint32_t>(ctx,fracbits,from);
					working = (undefined_value());
				}
				uint64_t rounding_control;
				if ((((uint64_t)round == (uint64_t)FPRounding_TIEEVEN)))
				{
					rounding_control = 0ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_NEGINF)))
				{
					rounding_control = 1ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_POSINF)))
				{
					rounding_control = 2ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_ZERO)))
				{
					rounding_control = 3ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_TIEAWAY)))
				{
					rounding_control = 2ULL;
					if (((undefined_value())))
					{
						working = ((undefined_value()));
					}
				}
				else
				{
					undefined_interpreter(ctx);
				}
				if ((((uint64_t)from == (uint64_t)32ULL)))
				{
					working = intrinsic_binary_imm_interpreter<uint128_t>(ctx,(uint64_t)x86_roundss,(uint128_t)working,rounding_control);
				}
				else
				{
					working = intrinsic_binary_imm_interpreter<uint128_t>(ctx,(uint64_t)x86_roundsd,(uint128_t)working,rounding_control);
				}
				uint32_t result = 0ULL;
				if (((undefined_value())))
				{
					result = (uint32_t)max_i;
				}
				else if (((undefined_value())))
				{
					result = (uint32_t)min_i;
				}
				else if ((is_unsigned))
				{
					uint32_t s_max_i = (uint32_t)(create_mask_interpreter(ctx,((uint64_t)to - (uint64_t)1ULL)));
					uint32_t s_max = convert_to_float<uint32_t, uint32_t>(s_max_i, 1);
					if ((((uint32_t)(sign_extend((uint32_t)working) > sign_extend((uint32_t)s_max)))))
					{
						uint32_t difference = (undefined_value());
						working = (undefined_value());
						result = (uint32_t)(convert_to_float<uint32_t, uint32_t>(difference, 1));
						result = ((uint32_t)result + (uint32_t)(convert_to_float<uint32_t, uint32_t>(working, 1)));
					}
					else
					{
						result = (uint32_t)(convert_to_float<uint32_t, uint32_t>(working, 1));
					}
				}
				else
				{
					result = (uint32_t)(convert_to_float<uint32_t, uint32_t>(working, 1));
				}
				return result;
			}
			if (to == 64ULL)
			{
				uint64_t max_i;
				uint64_t min_i;
				uint32_t max;
				uint32_t min;
				if ((is_unsigned))
				{
					min = 0ULL;
					max = convert_to_float<uint32_t, uint64_t>(create_mask_interpreter(ctx,to), 0);
					min_i = 0ULL;
					max_i = create_mask_interpreter(ctx,to);
				}
				else
				{
					min = convert_to_float<uint32_t, uint64_t>((uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)to - (uint64_t)1ULL)))), 1);
					max = convert_to_float<uint32_t, uint64_t>((uint64_t)(create_mask_interpreter(ctx,((uint64_t)to - (uint64_t)1ULL))), 1);
					min_i = (uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)to - (uint64_t)1ULL))));
					max_i = (uint64_t)(create_mask_interpreter(ctx,((uint64_t)to - (uint64_t)1ULL)));
				}
				uint32_t working = source;
				if ((((uint64_t)fracbits != (uint64_t)0ULL)))
				{
					uint32_t power = create_fixed_from_fbits_interpreter<uint32_t>(ctx,fracbits,from);
					working = (undefined_value());
				}
				uint64_t rounding_control;
				if ((((uint64_t)round == (uint64_t)FPRounding_TIEEVEN)))
				{
					rounding_control = 0ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_NEGINF)))
				{
					rounding_control = 1ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_POSINF)))
				{
					rounding_control = 2ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_ZERO)))
				{
					rounding_control = 3ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_TIEAWAY)))
				{
					rounding_control = 2ULL;
					if (((undefined_value())))
					{
						working = ((undefined_value()));
					}
				}
				else
				{
					undefined_interpreter(ctx);
				}
				if ((((uint64_t)from == (uint64_t)32ULL)))
				{
					working = intrinsic_binary_imm_interpreter<uint128_t>(ctx,(uint64_t)x86_roundss,(uint128_t)working,rounding_control);
				}
				else
				{
					working = intrinsic_binary_imm_interpreter<uint128_t>(ctx,(uint64_t)x86_roundsd,(uint128_t)working,rounding_control);
				}
				uint64_t result = 0ULL;
				if (((undefined_value())))
				{
					result = (uint64_t)max_i;
				}
				else if (((undefined_value())))
				{
					result = (uint64_t)min_i;
				}
				else if ((is_unsigned))
				{
					uint64_t s_max_i = (uint64_t)(create_mask_interpreter(ctx,((uint64_t)to - (uint64_t)1ULL)));
					uint32_t s_max = convert_to_float<uint32_t, uint64_t>(s_max_i, 1);
					if ((((uint32_t)(sign_extend((uint32_t)working) > sign_extend((uint32_t)s_max)))))
					{
						uint32_t difference = (undefined_value());
						working = (undefined_value());
						result = (uint64_t)(convert_to_float<uint64_t, uint32_t>(difference, 1));
						result = ((uint64_t)result + (uint64_t)(convert_to_float<uint64_t, uint32_t>(working, 1)));
					}
					else
					{
						result = (uint64_t)(convert_to_float<uint64_t, uint32_t>(working, 1));
					}
				}
				else
				{
					result = (uint64_t)(convert_to_float<uint64_t, uint32_t>(working, 1));
				}
				return result;
			}
			
		}
		if (from == 64ULL)
		{
			if (to == 32ULL)
			{
				uint64_t max_i;
				uint64_t min_i;
				uint64_t max;
				uint64_t min;
				if ((is_unsigned))
				{
					min = 0ULL;
					max = convert_to_float<uint64_t, uint64_t>(create_mask_interpreter(ctx,to), 0);
					min_i = 0ULL;
					max_i = create_mask_interpreter(ctx,to);
				}
				else
				{
					min = convert_to_float<uint64_t, uint32_t>((uint32_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)to - (uint64_t)1ULL)))), 1);
					max = convert_to_float<uint64_t, uint32_t>((uint32_t)(create_mask_interpreter(ctx,((uint64_t)to - (uint64_t)1ULL))), 1);
					min_i = (uint32_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)to - (uint64_t)1ULL))));
					max_i = (uint32_t)(create_mask_interpreter(ctx,((uint64_t)to - (uint64_t)1ULL)));
				}
				uint64_t working = source;
				if ((((uint64_t)fracbits != (uint64_t)0ULL)))
				{
					uint64_t power = create_fixed_from_fbits_interpreter<uint64_t>(ctx,fracbits,from);
					working = (undefined_value());
				}
				uint64_t rounding_control;
				if ((((uint64_t)round == (uint64_t)FPRounding_TIEEVEN)))
				{
					rounding_control = 0ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_NEGINF)))
				{
					rounding_control = 1ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_POSINF)))
				{
					rounding_control = 2ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_ZERO)))
				{
					rounding_control = 3ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_TIEAWAY)))
				{
					rounding_control = 2ULL;
					if (((undefined_value())))
					{
						working = ((undefined_value()));
					}
				}
				else
				{
					undefined_interpreter(ctx);
				}
				if ((((uint64_t)from == (uint64_t)32ULL)))
				{
					working = intrinsic_binary_imm_interpreter<uint128_t>(ctx,(uint64_t)x86_roundss,(uint128_t)working,rounding_control);
				}
				else
				{
					working = intrinsic_binary_imm_interpreter<uint128_t>(ctx,(uint64_t)x86_roundsd,(uint128_t)working,rounding_control);
				}
				uint32_t result = 0ULL;
				if (((undefined_value())))
				{
					result = (uint32_t)max_i;
				}
				else if (((undefined_value())))
				{
					result = (uint32_t)min_i;
				}
				else if ((is_unsigned))
				{
					uint32_t s_max_i = (uint32_t)(create_mask_interpreter(ctx,((uint64_t)to - (uint64_t)1ULL)));
					uint64_t s_max = convert_to_float<uint64_t, uint32_t>(s_max_i, 1);
					if ((((uint64_t)(sign_extend((uint64_t)working) > sign_extend((uint64_t)s_max)))))
					{
						uint64_t difference = (undefined_value());
						working = (undefined_value());
						result = (uint32_t)(convert_to_float<uint32_t, uint64_t>(difference, 1));
						result = ((uint32_t)result + (uint32_t)(convert_to_float<uint32_t, uint64_t>(working, 1)));
					}
					else
					{
						result = (uint32_t)(convert_to_float<uint32_t, uint64_t>(working, 1));
					}
				}
				else
				{
					result = (uint32_t)(convert_to_float<uint32_t, uint64_t>(working, 1));
				}
				return result;
			}
			if (to == 64ULL)
			{
				uint64_t max_i;
				uint64_t min_i;
				uint64_t max;
				uint64_t min;
				if ((is_unsigned))
				{
					min = 0ULL;
					max = convert_to_float<uint64_t, uint64_t>(create_mask_interpreter(ctx,to), 0);
					min_i = 0ULL;
					max_i = create_mask_interpreter(ctx,to);
				}
				else
				{
					min = convert_to_float<uint64_t, uint64_t>((uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)to - (uint64_t)1ULL)))), 1);
					max = convert_to_float<uint64_t, uint64_t>((uint64_t)(create_mask_interpreter(ctx,((uint64_t)to - (uint64_t)1ULL))), 1);
					min_i = (uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)to - (uint64_t)1ULL))));
					max_i = (uint64_t)(create_mask_interpreter(ctx,((uint64_t)to - (uint64_t)1ULL)));
				}
				uint64_t working = source;
				if ((((uint64_t)fracbits != (uint64_t)0ULL)))
				{
					uint64_t power = create_fixed_from_fbits_interpreter<uint64_t>(ctx,fracbits,from);
					working = (undefined_value());
				}
				uint64_t rounding_control;
				if ((((uint64_t)round == (uint64_t)FPRounding_TIEEVEN)))
				{
					rounding_control = 0ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_NEGINF)))
				{
					rounding_control = 1ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_POSINF)))
				{
					rounding_control = 2ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_ZERO)))
				{
					rounding_control = 3ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_TIEAWAY)))
				{
					rounding_control = 2ULL;
					if (((undefined_value())))
					{
						working = ((undefined_value()));
					}
				}
				else
				{
					undefined_interpreter(ctx);
				}
				if ((((uint64_t)from == (uint64_t)32ULL)))
				{
					working = intrinsic_binary_imm_interpreter<uint128_t>(ctx,(uint64_t)x86_roundss,(uint128_t)working,rounding_control);
				}
				else
				{
					working = intrinsic_binary_imm_interpreter<uint128_t>(ctx,(uint64_t)x86_roundsd,(uint128_t)working,rounding_control);
				}
				uint64_t result = 0ULL;
				if (((undefined_value())))
				{
					result = (uint64_t)max_i;
				}
				else if (((undefined_value())))
				{
					result = (uint64_t)min_i;
				}
				else if ((is_unsigned))
				{
					uint64_t s_max_i = (uint64_t)(create_mask_interpreter(ctx,((uint64_t)to - (uint64_t)1ULL)));
					uint64_t s_max = convert_to_float<uint64_t, uint64_t>(s_max_i, 1);
					if ((((uint64_t)(sign_extend((uint64_t)working) > sign_extend((uint64_t)s_max)))))
					{
						uint64_t difference = (undefined_value());
						working = (undefined_value());
						result = (uint64_t)(convert_to_float<uint64_t, uint64_t>(difference, 1));
						result = ((uint64_t)result + (uint64_t)(convert_to_float<uint64_t, uint64_t>(working, 1)));
					}
					else
					{
						result = (uint64_t)(convert_to_float<uint64_t, uint64_t>(working, 1));
					}
				}
				else
				{
					result = (uint64_t)(convert_to_float<uint64_t, uint64_t>(working, 1));
				}
				return result;
			}
			
		}
		
	}
	return call_interpreter(ctx,source,(uint64_t)fracbits,(uint64_t)is_unsigned,(uint64_t)round,(uint64_t)to,(uint64_t)from,(uint64_t)FPToFixed_I);
}

uint64_t FPConvert_interpreter(interpreter_data* ctx, uint64_t source, uint64_t to, uint64_t from)
{
	if ((((uint64_t)to == (uint64_t)from)))
	{
		undefined_interpreter(ctx);
	}
	if ((((uint64_t)((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)to != (uint64_t)16ULL)) && (uint64_t)((uint64_t)from != (uint64_t)16ULL))))
	{
		uint64_t result;
		if ((((uint64_t)((uint64_t)to == (uint64_t)32ULL) && (uint64_t)((uint64_t)from == (uint64_t)64ULL))))
		{
			result = intrinsic_unary_interpreter<uint128_t>(ctx,(uint64_t)x86_cvtsd2ss,(uint128_t)source);
		}
		else
		{
			result = intrinsic_unary_interpreter<uint128_t>(ctx,(uint64_t)x86_cvtss2sd,(uint128_t)source);
		}
		return result;
	}
	return call_interpreter(ctx,source,(uint64_t)0ULL,(uint64_t)0ULL,(uint64_t)to,(uint64_t)from,(uint64_t)0ULL,(uint64_t)FPConvert_I);
}

uint64_t FPRoundInt_interpreter(interpreter_data* ctx, uint64_t source, uint64_t fpcr, uint64_t rounding, uint64_t N)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
	}
	return call_interpreter(ctx,source,fpcr,(uint64_t)rounding,(uint64_t)N,(uint64_t)0ULL,(uint64_t)0ULL,(uint64_t)FPRoundInt_I);
}

uint64_t FPMulAdd_interpreter(interpreter_data* ctx, uint64_t addend, uint64_t element1, uint64_t element2, uint64_t fpcr, uint64_t N)
{
	if ((use_fast_float_interpreter(ctx)))
	{
		if (N == 32ULL)
		{
			uint32_t o3 = addend;
			uint32_t o1 = element1;
			uint32_t o2 = element2;
			return (undefined_value());
		}
		if (N == 64ULL)
		{
			uint64_t o3 = addend;
			uint64_t o1 = element1;
			uint64_t o2 = element2;
			return (undefined_value());
		}
		
	}
	return call_interpreter(ctx,addend,element1,element2,fpcr,(uint64_t)N,(uint64_t)0ULL,(uint64_t)FPMulAdd_I);
}

void float_unary_scalar_interpreter(interpreter_data* ctx, uint64_t Rd, uint64_t Rn, uint64_t fsize, uint64_t float_function)
{
	uint64_t operand = V_interpreter(ctx,Rn);
	uint64_t N = get_flt_size_interpreter(ctx,fsize);
	uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
	uint64_t element_result = ((uint64_t(*)(void*,uint64_t,uint64_t,uint64_t))float_function)(ctx,operand,fpcr_state,N);
	uint128_t vector_result = 0;
	uint128_t::insert(&vector_result, 0ULL, N, element_result);
	V_interpreter(ctx,Rd,vector_result);
}

void float_unary_vector_interpreter(interpreter_data* ctx, uint64_t Rd, uint64_t Rn, uint64_t Q, uint64_t sz, uint64_t float_function)
{
	uint128_t operand = V_interpreter(ctx,Rn);
	uint128_t result = 0;
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint64_t working = uint128_t::extract(operand, e, esize);
		uint64_t element_result = ((uint64_t(*)(void*,uint64_t,uint64_t,uint64_t))float_function)(ctx,working,fpcr_state,esize);
		uint128_t::insert(&result, e, esize, element_result);
	}
	V_interpreter(ctx,Rd,result);
}

void float_binary_scalar_interpreter(interpreter_data* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t fsize, uint64_t float_function)
{
	uint64_t operand1 = V_interpreter(ctx,Rn);
	uint64_t operand2 = V_interpreter(ctx,Rm);
	uint64_t N = get_flt_size_interpreter(ctx,fsize);
	uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
	uint64_t element_result = ((uint64_t(*)(void*,uint64_t,uint64_t,uint64_t,uint64_t))float_function)(ctx,operand1,operand2,fpcr_state,N);
	uint128_t vector_result = 0;
	uint128_t::insert(&vector_result, 0ULL, N, element_result);
	V_interpreter(ctx,Rd,vector_result);
}

void float_binary_vector_interpreter(interpreter_data* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t Q, uint64_t sz, uint64_t float_function)
{
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t result = 0;
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint64_t element1 = uint128_t::extract(operand1, e, esize);
		uint64_t element2 = uint128_t::extract(operand2, e, esize);
		uint128_t::insert(&result, e, esize, ((uint64_t(*)(void*,uint64_t,uint64_t,uint64_t,uint64_t))float_function)(ctx,element1,element2,fpcr_state,esize));
	}
	V_interpreter(ctx,Rd,result);
}

void frint_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd, uint64_t rounding)
{
	uint128_t operand = V_interpreter(ctx,Rn);
	uint64_t esize = get_flt_size_interpreter(ctx,ftype);
	if ((((uint64_t)((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx)) && (uint64_t)((uint64_t)rounding != (uint64_t)FPRounding_TIEAWAY))))
	{
		uint64_t rounding_control = get_x86_rounding_mode_interpreter(ctx,rounding);
		uint128_t result = intrinsic_binary_imm_interpreter<uint128_t>(ctx,select_interpreter(ctx,((uint64_t)esize == (uint64_t)64ULL),(uint64_t)x86_roundsd,(uint64_t)x86_roundss),operand,rounding_control);
		result = clear_vector_scalar_interpreter(ctx,result,esize);
		V_interpreter(ctx,Rd,result);
	}
	else
	{
		uint128_t result = 0;
		uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
		uint64_t working = FPRoundInt_interpreter(ctx,(uint64_t)operand,fpcr_state,rounding,esize);
		uint128_t::insert(&result, 0ULL, esize, working);
		V_interpreter(ctx,Rd,result);
	}
}

void intrinsic_float_binary_vector_interpreter(interpreter_data* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t Q, uint64_t sz, uint64_t float_instruction, uint64_t double_instruction)
{
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t result;
	if ((sz))
	{
		result = intrinsic_binary_interpreter<uint128_t>(ctx,double_instruction,operand1,operand2);
	}
	else
	{
		result = intrinsic_binary_interpreter<uint128_t>(ctx,float_instruction,operand1,operand2);
	}
	if ((!Q))
	{
		uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
	}
	V_interpreter(ctx,Rd,result);
}

void intrinsic_float_binary_scalar_interpreter(interpreter_data* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t fsize, uint64_t half_instruction, uint64_t float_instruction, uint64_t double_instruction)
{
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t result;
	uint64_t esize = get_flt_size_interpreter(ctx,fsize);
	if ((((uint64_t)esize == (uint64_t)64ULL)))
	{
		result = intrinsic_binary_interpreter<uint128_t>(ctx,double_instruction,operand1,operand2);
	}
	else if ((((uint64_t)esize == (uint64_t)32ULL)))
	{
		result = intrinsic_binary_interpreter<uint128_t>(ctx,float_instruction,operand1,operand2);
		uint128_t::insert(&result, 1ULL, 32ULL, (uint64_t)0ULL);
	}
	else if ((((uint64_t)esize == (uint64_t)16ULL)))
	{
		result = intrinsic_binary_interpreter<uint128_t>(ctx,half_instruction,operand1,operand2);
		uint128_t::insert(&result, 1ULL, 16ULL, (uint64_t)0ULL);
		uint128_t::insert(&result, 1ULL, 32ULL, (uint64_t)0ULL);
	}
	else
	{
		undefined_interpreter(ctx);
	}
	uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
	V_interpreter(ctx,Rd,result);
}

void x86_sse_logic_vector_interpreter(interpreter_data* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t Q, uint64_t invert, uint64_t primary_instruction)
{
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	if ((invert))
	{
		uint128_t one = uint128_t(-1, -1);
		operand2 = intrinsic_binary_interpreter<uint128_t>(ctx,(uint64_t)x86_xorps,operand2,one);
	}
	uint128_t result = intrinsic_binary_interpreter<uint128_t>(ctx,primary_instruction,operand1,operand2);
	if ((!Q))
	{
		uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
	}
	V_interpreter(ctx,Rd,result);
}

uint128_t sse_copy_to_xmm_from_xmm_element_interpreter(interpreter_data* ctx, uint128_t source, uint64_t size, uint64_t index)
{
	if ((((uint64_t)size <= (uint64_t)16ULL)))
	{
		uint64_t source_element = uint128_t::extract(source, size, index);
		return sse_coppy_gp_across_lanes_interpreter(ctx,source_element,size);
	}
	if ((((uint64_t)size == (uint64_t)32ULL)))
	{
		return intrinsic_ternary_imm_interpreter<uint128_t>(ctx,(uint64_t)x86_shufps,source,source,((uint64_t)((uint64_t)((uint64_t)index | (uint64_t)(((uint64_t)index << (uint64_t)2ULL))) | (uint64_t)(((uint64_t)index << (uint64_t)4ULL))) | (uint64_t)(((uint64_t)index << (uint64_t)6ULL))));
	}
	else if ((((uint64_t)size == (uint64_t)64ULL)))
	{
		return intrinsic_ternary_imm_interpreter<uint128_t>(ctx,(uint64_t)x86_shufpd,source,source,((uint64_t)index | (uint64_t)(((uint64_t)index << (uint64_t)1ULL))));
	}
	else
	{
		undefined_interpreter(ctx);
	}
}

uint128_t sse_coppy_gp_across_lanes_interpreter(interpreter_data* ctx, uint64_t source, uint64_t size)
{
	if ((((uint64_t)size == (uint64_t)8ULL)))
	{
		source = ((uint64_t)source & (uint64_t)255ULL);
		source = ((uint64_t)((uint64_t)((uint64_t)source | (uint64_t)(((uint64_t)source << (uint64_t)8ULL))) | (uint64_t)(((uint64_t)source << (uint64_t)16ULL))) | (uint64_t)(((uint64_t)source << (uint64_t)24ULL)));
		size = 32ULL;
	}
	else if ((((uint64_t)size == (uint64_t)16ULL)))
	{
		source = ((uint64_t)source & (uint64_t)65535ULL);
		source = ((uint64_t)source | (uint64_t)(((uint64_t)source << (uint64_t)16ULL)));
		size = 32ULL;
	}
	uint128_t working_element = source;
	if ((((uint64_t)size == (uint64_t)32ULL)))
	{
		return intrinsic_ternary_imm_interpreter<uint128_t>(ctx,(uint64_t)x86_shufps,working_element,working_element,0ULL);
	}
	else if ((((uint64_t)size == (uint64_t)64ULL)))
	{
		return intrinsic_ternary_imm_interpreter<uint128_t>(ctx,(uint64_t)x86_shufpd,working_element,working_element,0ULL);
	}
	else
	{
		undefined_interpreter(ctx);
	}
}

void floating_point_multiply_scalar_element_interpreter(interpreter_data* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t sz, uint64_t index)
{
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	if ((use_x86_sse_interpreter(ctx)))
	{
		operand2 = sse_copy_to_xmm_from_xmm_element_interpreter(ctx,operand2,esize,index);
		uint64_t multiply_instruction;
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		multiply_instruction = (uint64_t)x86_mulss;
		else
		multiply_instruction = (uint64_t)x86_mulsd;
		uint128_t result = intrinsic_binary_interpreter<uint128_t>(ctx,multiply_instruction,operand1,operand2);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			uint128_t::insert(&result, 1ULL, 32ULL, (uint64_t)0ULL);
		}
		uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		V_interpreter(ctx,Rd,result);
	}
	else
	{
		uint128_t result = 0;
		uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
		uint64_t product = FPMul_interpreter(ctx,(uint64_t)operand1,uint128_t::extract(operand2, index, esize),fpcr_state,esize);
		uint128_t::insert(&result, 0ULL, esize, product);
		V_interpreter(ctx,Rd,result);
	}
}

void floating_point_multiply_vector_element_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t sz, uint64_t index)
{
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	if ((use_x86_sse_interpreter(ctx)))
	{
		operand2 = sse_copy_to_xmm_from_xmm_element_interpreter(ctx,operand2,esize,index);
		uint64_t multiply_instruction;
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		multiply_instruction = (uint64_t)x86_mulps;
		else
		multiply_instruction = (uint64_t)x86_mulpd;
		uint128_t result = intrinsic_binary_interpreter<uint128_t>(ctx,multiply_instruction,operand1,operand2);
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		}
		V_interpreter(ctx,Rd,result);
	}
	else
	{
		uint128_t result = 0;
		uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
		uint64_t element2 = uint128_t::extract(operand2, index, esize);
		uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint128_t::insert(&result, e, esize, FPMul_interpreter(ctx,element1,element2,fpcr_state,esize));
		}
		V_interpreter(ctx,Rd,result);
	}
}

void floating_point_multiply_accumulate_scalar_element_interpreter(interpreter_data* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t neg, uint64_t sz, uint64_t index)
{
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t operand3 = V_interpreter(ctx,Rd);
	uint128_t result;
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx))))
	{
		uint64_t add_instruction;
		uint64_t subtract_instruction;
		uint64_t multiply_instruction;
		operand2 = uint128_t::extract(operand2, index, esize);
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			add_instruction = (uint64_t)x86_addsd;
			subtract_instruction = (uint64_t)x86_subsd;
			multiply_instruction = (uint64_t)x86_mulsd;
		}
		else
		{
			add_instruction = (uint64_t)x86_addss;
			subtract_instruction = (uint64_t)x86_subss;
			multiply_instruction = (uint64_t)x86_mulss;
		}
		result = intrinsic_binary_interpreter<uint128_t>(ctx,multiply_instruction,operand1,operand2);
		if ((neg))
		{
			result = intrinsic_binary_interpreter<uint128_t>(ctx,subtract_instruction,operand3,result);
		}
		else
		{
			result = intrinsic_binary_interpreter<uint128_t>(ctx,add_instruction,operand3,result);
		}
		uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			uint128_t::insert(&result, 1ULL, 32ULL, (uint64_t)0ULL);
		}
	}
	else
	{
		result = 0;
		uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
		uint64_t element1 = operand1;
		if ((neg))
		{
			element1 = FPNeg_interpreter(ctx,element1,fpcr_state,esize);
		}
		uint64_t product_accumalant = FPMulAdd_interpreter(ctx,(uint64_t)operand3,element1,uint128_t::extract(operand2, index, esize),fpcr_state,esize);
		uint128_t::insert(&result, 0ULL, esize, product_accumalant);
	}
	V_interpreter(ctx,Rd,result);
}

void floating_point_multiply_accumulate_vector_element_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t neg, uint64_t sz, uint64_t index)
{
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t operand3 = V_interpreter(ctx,Rd);
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint128_t result;
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx))))
	{
		uint64_t add_instruction;
		uint64_t subtract_instruction;
		uint64_t multiply_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			add_instruction = (uint64_t)x86_addpd;
			subtract_instruction = (uint64_t)x86_subpd;
			multiply_instruction = (uint64_t)x86_mulpd;
		}
		else
		{
			add_instruction = (uint64_t)x86_addps;
			subtract_instruction = (uint64_t)x86_subps;
			multiply_instruction = (uint64_t)x86_mulps;
		}
		operand2 = sse_copy_to_xmm_from_xmm_element_interpreter(ctx,operand2,esize,index);
		result = intrinsic_binary_interpreter<uint128_t>(ctx,multiply_instruction,operand1,operand2);
		if ((neg))
		{
			result = intrinsic_binary_interpreter<uint128_t>(ctx,subtract_instruction,operand3,result);
		}
		else
		{
			result = intrinsic_binary_interpreter<uint128_t>(ctx,add_instruction,operand3,result);
		}
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		}
	}
	else
	{
		result = 0;
		uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
		uint64_t element2 = uint128_t::extract(operand2, index, esize);
		uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element3 = uint128_t::extract(operand3, e, esize);
			if ((neg))
			{
				element1 = FPNeg_interpreter(ctx,element1,fpcr_state,esize);
			}
			uint128_t::insert(&result, e, esize, FPMulAdd_interpreter(ctx,element3,element1,element2,fpcr_state,esize));
		}
	}
	V_interpreter(ctx,Rd,result);
}

void fcm_vector_interpreter(interpreter_data* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t mode, uint64_t Q, uint64_t sz)
{
	uint128_t n = V_interpreter(ctx,Rn);
	uint128_t m;
	if ((((uint64_t)Rm == (uint64_t)-1ULL)))
	{
		m = 0;
	}
	else
	{
		m = V_interpreter(ctx,Rm);
	}
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	if ((use_x86_sse_interpreter(ctx)))
	{
		uint128_t result;
		uint64_t operation = select_interpreter(ctx,((uint64_t)esize == (uint64_t)64ULL),(uint64_t)x86_cmppd,(uint64_t)x86_cmpps);
		uint64_t control;
		if ((((uint64_t)mode == (uint64_t)0ULL)))
		{
			control = 0ULL;
		}
		else if ((((uint64_t)mode == (uint64_t)1ULL)))
		{
			control = 6ULL;
		}
		else if ((((uint64_t)mode == (uint64_t)2ULL)))
		{
			control = 5ULL;
		}
		result = intrinsic_ternary_imm_interpreter<uint128_t>(ctx,operation,n,m,control);
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		}
		V_interpreter(ctx,Rd,result);
	}
	else
	{
		uint128_t result = 0;
		uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
		uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(n, e, esize);
			uint64_t element2 = uint128_t::extract(m, e, esize);
			uint64_t element_result;
			if ((((uint64_t)mode == (uint64_t)0ULL)))
			{
				element_result = ((uint64_t)0ULL - (uint64_t)FPCompareEQ_interpreter(ctx,element1,element2,fpcr_state,esize));
			}
			else if ((((uint64_t)mode == (uint64_t)1ULL)))
			{
				element_result = ((uint64_t)0ULL - (uint64_t)FPCompareGT_interpreter(ctx,element1,element2,fpcr_state,esize));
			}
			else if ((((uint64_t)mode == (uint64_t)2ULL)))
			{
				element_result = ((uint64_t)0ULL - (uint64_t)FPCompareGE_interpreter(ctx,element1,element2,fpcr_state,esize));
			}
			uint128_t::insert(&result, e, esize, element_result);
		}
		V_interpreter(ctx,Rd,result);
	}
}

uint128_t clear_vector_scalar_interpreter(interpreter_data* ctx, uint128_t working, uint64_t fltsize)
{
	if ((((uint64_t)fltsize == (uint64_t)32ULL)))
	{
		uint128_t::insert(&working, 1ULL, 32ULL, (uint64_t)0ULL);
	}
	uint128_t::insert(&working, 1ULL, 64ULL, (uint64_t)0ULL);
	return working;
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
		uint32_t result = add_subtract_carry_impl_interpreter<uint32_t>(ctx,operand1,operand2,((uint64_t)S == (uint64_t)1ULL),((uint64_t)op == (uint64_t)0ULL),(uint32_t)_sys_interpreter(ctx,(uint64_t)nzcv_c));
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = X_interpreter(ctx,Rm);
		uint64_t result = add_subtract_carry_impl_interpreter<uint64_t>(ctx,operand1,operand2,((uint64_t)S == (uint64_t)1ULL),((uint64_t)op == (uint64_t)0ULL),(uint64_t)_sys_interpreter(ctx,(uint64_t)nzcv_c));
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

uint64_t create_rbit_mask_interpreter(interpreter_data* ctx, uint64_t index)
{
	index = ((uint64_t)1ULL << (uint64_t)index);
	uint64_t mask = ((uint64_t)(((uint64_t)(((uint64_t)1ULL << (uint64_t)index)) - (uint64_t)1ULL)) << (uint64_t)index);
	mask = replicate_c_interpreter(ctx,mask,((uint64_t)index * (uint64_t)2ULL),((uint64_t)((uint64_t)64ULL / (uint64_t)index) / (uint64_t)2ULL));
	return mask;
}

void rbit_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	if (sf == 0ULL)
	{
		uint32_t operand = X_interpreter(ctx,Rn);
		uint32_t result = operand;
		uint64_t max = ((uint64_t)5ULL + (uint64_t)sf);
		for (uint64_t i = 0; i < (max); i++)
		{
			uint64_t n_mask = create_rbit_mask_interpreter(ctx,i);
			uint64_t i_mask = ~n_mask;
			uint64_t shift = ((uint64_t)1ULL << (uint64_t)i);
			result = ((uint32_t)(((uint32_t)(((uint32_t)result & (uint32_t)n_mask)) >> (uint32_t)shift)) | (uint32_t)(((uint32_t)(((uint32_t)result & (uint32_t)i_mask)) << (uint32_t)shift)));
		}
		X_interpreter(ctx,Rd,(uint64_t)result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand = X_interpreter(ctx,Rn);
		uint64_t result = operand;
		uint64_t max = ((uint64_t)5ULL + (uint64_t)sf);
		for (uint64_t i = 0; i < (max); i++)
		{
			uint64_t n_mask = create_rbit_mask_interpreter(ctx,i);
			uint64_t i_mask = ~n_mask;
			uint64_t shift = ((uint64_t)1ULL << (uint64_t)i);
			result = ((uint64_t)(((uint64_t)(((uint64_t)result & (uint64_t)n_mask)) >> (uint64_t)shift)) | (uint64_t)(((uint64_t)(((uint64_t)result & (uint64_t)i_mask)) << (uint64_t)shift)));
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
		if ((use_x86_lzcnt_interpreter(ctx)))
		{
			if ((s))
			{
				uint32_t nhigh = ((uint32_t)operand >> (uint32_t)1ULL);
				uint32_t mask = -1ULL;
				mask = ((uint32_t)mask >> (uint32_t)1ULL);
				uint32_t nlow = ((uint32_t)operand & (uint32_t)mask);
				result = intrinsic_unary_interpreter<uint32_t>(ctx,(uint64_t)x86_lzcnt,(((uint32_t)nhigh ^ (uint32_t)nlow)));
				result = ((uint32_t)result - (uint32_t)1ULL);
			}
			else
			{
				result = intrinsic_unary_interpreter<uint32_t>(ctx,(uint64_t)x86_lzcnt,operand);
			}
		}
		else
		{
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
		if ((use_x86_lzcnt_interpreter(ctx)))
		{
			if ((s))
			{
				uint64_t nhigh = ((uint64_t)operand >> (uint64_t)1ULL);
				uint64_t mask = -1ULL;
				mask = ((uint64_t)mask >> (uint64_t)1ULL);
				uint64_t nlow = ((uint64_t)operand & (uint64_t)mask);
				result = intrinsic_unary_interpreter<uint64_t>(ctx,(uint64_t)x86_lzcnt,(((uint64_t)nhigh ^ (uint64_t)nlow)));
				result = ((uint64_t)result - (uint64_t)1ULL);
			}
			else
			{
				result = intrinsic_unary_interpreter<uint64_t>(ctx,(uint64_t)x86_lzcnt,operand);
			}
		}
		else
		{
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
			_sys_interpreter(ctx,(uint64_t)nzcv_n,(uint64_t)((uint32_t)(sign_extend((uint32_t)result) < sign_extend((uint32_t)0ULL))));
			_sys_interpreter(ctx,(uint64_t)nzcv_z,(uint64_t)((uint32_t)result == (uint32_t)0ULL));
			_sys_interpreter(ctx,(uint64_t)nzcv_c,(uint64_t)0ULL);
			_sys_interpreter(ctx,(uint64_t)nzcv_v,(uint64_t)0ULL);
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
			_sys_interpreter(ctx,(uint64_t)nzcv_n,(uint64_t)((uint64_t)(sign_extend((uint64_t)result) < sign_extend((uint64_t)0ULL))));
			_sys_interpreter(ctx,(uint64_t)nzcv_z,(uint64_t)((uint64_t)result == (uint64_t)0ULL));
			_sys_interpreter(ctx,(uint64_t)nzcv_c,(uint64_t)0ULL);
			_sys_interpreter(ctx,(uint64_t)nzcv_v,(uint64_t)0ULL);
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
				_sys_interpreter(ctx,(uint64_t)nzcv_n,(uint64_t)((uint32_t)(sign_extend((uint32_t)result) < sign_extend((uint32_t)0ULL))));
				_sys_interpreter(ctx,(uint64_t)nzcv_z,(uint64_t)((uint32_t)result == (uint32_t)0ULL));
				_sys_interpreter(ctx,(uint64_t)nzcv_c,(uint64_t)0ULL);
				_sys_interpreter(ctx,(uint64_t)nzcv_v,(uint64_t)0ULL);
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
				_sys_interpreter(ctx,(uint64_t)nzcv_n,(uint64_t)((uint64_t)(sign_extend((uint64_t)result) < sign_extend((uint64_t)0ULL))));
				_sys_interpreter(ctx,(uint64_t)nzcv_z,(uint64_t)((uint64_t)result == (uint64_t)0ULL));
				_sys_interpreter(ctx,(uint64_t)nzcv_c,(uint64_t)0ULL);
				_sys_interpreter(ctx,(uint64_t)nzcv_v,(uint64_t)0ULL);
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
			_sys_interpreter(ctx,(uint64_t)nzcv_n,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,(uint64_t)nzcv_z,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,(uint64_t)nzcv_c,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,(uint64_t)nzcv_v,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL));
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
			_sys_interpreter(ctx,(uint64_t)nzcv_n,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,(uint64_t)nzcv_z,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,(uint64_t)nzcv_c,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL));
			_sys_interpreter(ctx,(uint64_t)nzcv_v,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL));
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

void msr_register_interpreter(interpreter_data* ctx, uint64_t imm15, uint64_t Rt)
{
	uint64_t operand = X_interpreter(ctx,Rt);
	if ((((uint64_t)imm15 == (uint64_t)23072ULL)))
	{
		_sys_interpreter(ctx,(uint64_t)fpcr,operand);
	}
	else if ((((uint64_t)imm15 == (uint64_t)23073ULL)))
	{
		_sys_interpreter(ctx,(uint64_t)fpsr,operand);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24194ULL)))
	{
		_sys_interpreter(ctx,(uint64_t)thread_local_1,operand);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24195ULL)))
	{
		_sys_interpreter(ctx,(uint64_t)thread_local_0,operand);
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
		operand = _sys_interpreter(ctx,(uint64_t)fpcr);
	}
	else if ((((uint64_t)imm15 == (uint64_t)23073ULL)))
	{
		operand = _sys_interpreter(ctx,(uint64_t)fpsr);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24194ULL)))
	{
		operand = _sys_interpreter(ctx,(uint64_t)thread_local_1);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24195ULL)))
	{
		operand = _sys_interpreter(ctx,(uint64_t)thread_local_0);
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
		_sys_interpreter(ctx,(uint64_t)exclusive_address,(uint64_t)-1ULL);
		_sys_interpreter(ctx,(uint64_t)exclusive_value,(uint64_t)-1ULL);
	}
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

uint64_t exclusive_address_mask_interpreter(interpreter_data* ctx)
{
	return ~(((uint64_t)(((uint64_t)4ULL << (uint64_t)4ULL)) - (uint64_t)1ULL));
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
			_sys_interpreter(ctx,(uint64_t)exclusive_address,((uint64_t)address & (uint64_t)exclusive_address_mask_interpreter(ctx)));
			_sys_interpreter(ctx,(uint64_t)exclusive_value,(uint64_t)value);
		}
		X_interpreter(ctx,Rt,(uint64_t)value);
	}
	if (datasize == 16ULL)
	{
		uint16_t value = mem_interpreter<uint16_t>(ctx,address);
		if ((is_exclusive))
		{
			_sys_interpreter(ctx,(uint64_t)exclusive_address,((uint64_t)address & (uint64_t)exclusive_address_mask_interpreter(ctx)));
			_sys_interpreter(ctx,(uint64_t)exclusive_value,(uint64_t)value);
		}
		X_interpreter(ctx,Rt,(uint64_t)value);
	}
	if (datasize == 32ULL)
	{
		uint32_t value = mem_interpreter<uint32_t>(ctx,address);
		if ((is_exclusive))
		{
			_sys_interpreter(ctx,(uint64_t)exclusive_address,((uint64_t)address & (uint64_t)exclusive_address_mask_interpreter(ctx)));
			_sys_interpreter(ctx,(uint64_t)exclusive_value,(uint64_t)value);
		}
		X_interpreter(ctx,Rt,(uint64_t)value);
	}
	if (datasize == 64ULL)
	{
		uint64_t value = mem_interpreter<uint64_t>(ctx,address);
		if ((is_exclusive))
		{
			_sys_interpreter(ctx,(uint64_t)exclusive_address,((uint64_t)address & (uint64_t)exclusive_address_mask_interpreter(ctx)));
			_sys_interpreter(ctx,(uint64_t)exclusive_value,(uint64_t)value);
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
			uint64_t mask = exclusive_address_mask_interpreter(ctx);
			uint64_t _exclusive_address = _sys_interpreter(ctx,(uint64_t)exclusive_address);
			if ((((uint64_t)_exclusive_address == (uint64_t)(((uint64_t)address & (uint64_t)mask)))))
			{
				uint8_t to_swap = X_interpreter(ctx,Rt);
				uint8_t expecting = _sys_interpreter(ctx,(uint64_t)exclusive_value);
				uint8_t cas_success = compare_and_swap_interpreter(ctx,address,(uint64_t)expecting,(uint64_t)to_swap,datasize);
				X_interpreter(ctx,Rs,(uint64_t)((uint8_t)(((uint8_t)cas_success ^ (uint8_t)1ULL)) & (uint8_t)1ULL));
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
			uint64_t mask = exclusive_address_mask_interpreter(ctx);
			uint64_t _exclusive_address = _sys_interpreter(ctx,(uint64_t)exclusive_address);
			if ((((uint64_t)_exclusive_address == (uint64_t)(((uint64_t)address & (uint64_t)mask)))))
			{
				uint16_t to_swap = X_interpreter(ctx,Rt);
				uint16_t expecting = _sys_interpreter(ctx,(uint64_t)exclusive_value);
				uint16_t cas_success = compare_and_swap_interpreter(ctx,address,(uint64_t)expecting,(uint64_t)to_swap,datasize);
				X_interpreter(ctx,Rs,(uint64_t)((uint16_t)(((uint16_t)cas_success ^ (uint16_t)1ULL)) & (uint16_t)1ULL));
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
			uint64_t mask = exclusive_address_mask_interpreter(ctx);
			uint64_t _exclusive_address = _sys_interpreter(ctx,(uint64_t)exclusive_address);
			if ((((uint64_t)_exclusive_address == (uint64_t)(((uint64_t)address & (uint64_t)mask)))))
			{
				uint32_t to_swap = X_interpreter(ctx,Rt);
				uint32_t expecting = _sys_interpreter(ctx,(uint64_t)exclusive_value);
				uint32_t cas_success = compare_and_swap_interpreter(ctx,address,(uint64_t)expecting,(uint64_t)to_swap,datasize);
				X_interpreter(ctx,Rs,(uint64_t)((uint32_t)(((uint32_t)cas_success ^ (uint32_t)1ULL)) & (uint32_t)1ULL));
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
			uint64_t mask = exclusive_address_mask_interpreter(ctx);
			uint64_t _exclusive_address = _sys_interpreter(ctx,(uint64_t)exclusive_address);
			if ((((uint64_t)_exclusive_address == (uint64_t)(((uint64_t)address & (uint64_t)mask)))))
			{
				uint64_t to_swap = X_interpreter(ctx,Rt);
				uint64_t expecting = _sys_interpreter(ctx,(uint64_t)exclusive_value);
				uint64_t cas_success = compare_and_swap_interpreter(ctx,address,(uint64_t)expecting,(uint64_t)to_swap,datasize);
				X_interpreter(ctx,Rs,(uint64_t)((uint64_t)(((uint64_t)cas_success ^ (uint64_t)1ULL)) & (uint64_t)1ULL));
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
		result = FPToFixed_interpreter(ctx,source,fracbits,((uint64_t)opcode == (uint64_t)1ULL),(uint64_t)FPRounding_ZERO,intsize,fltsize);
		X_interpreter(ctx,Rd,result);
	}
	else
	{
		undefined_with_interpreter(ctx,100ULL);
	}
}

void fcvt_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t opc, uint64_t Rn, uint64_t Rd)
{
	uint64_t srcsize = get_flt_size_interpreter(ctx,ftype);
	uint64_t dstsize = get_flt_size_interpreter(ctx,opc);
	uint64_t operand = V_interpreter(ctx,Rn);
	V_interpreter(ctx,Rd,(uint128_t)FPConvert_interpreter(ctx,operand,dstsize,srcsize));
}

void fcvtz_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_interpreter(ctx,sf,ftype,Rd,Rn,(uint64_t)FPRounding_ZERO,U,0ULL);
}

void fcvtn_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_interpreter(ctx,sf,ftype,Rd,Rn,(uint64_t)FPRounding_TIEEVEN,U,0ULL);
}

void fcvta_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_interpreter(ctx,sf,ftype,Rd,Rn,(uint64_t)FPRounding_TIEAWAY,U,0ULL);
}

void fcvtm_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_interpreter(ctx,sf,ftype,Rd,Rn,(uint64_t)FPRounding_NEGINF,U,0ULL);
}

void frintp_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd)
{
	frint_interpreter(ctx,ftype,Rn,Rd,(uint64_t)FPRounding_POSINF);
}

void frintm_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd)
{
	frint_interpreter(ctx,ftype,Rn,Rd,(uint64_t)FPRounding_NEGINF);
}

void fcvtp_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_interpreter(ctx,sf,ftype,Rd,Rn,(uint64_t)FPRounding_POSINF,U,0ULL);
}

void fadd_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx))))
	{
		intrinsic_float_binary_vector_interpreter(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)x86_addps,(uint64_t)x86_addpd);
		return;
	}
	float_binary_vector_interpreter(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)FPAdd_interpreter);
}

void fmul_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx))))
	{
		intrinsic_float_binary_vector_interpreter(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)x86_mulps,(uint64_t)x86_mulpd);
		return;
	}
	float_binary_vector_interpreter(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)FPMul_interpreter);
}

void fsub_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx))))
	{
		intrinsic_float_binary_vector_interpreter(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)x86_subps,(uint64_t)x86_subpd);
		return;
	}
	float_binary_vector_interpreter(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)FPSub_interpreter);
}

void fdiv_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx))))
	{
		intrinsic_float_binary_vector_interpreter(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)x86_divps,(uint64_t)x86_divpd);
		return;
	}
	float_binary_vector_interpreter(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)FPDiv_interpreter);
}

void fmul_accumulate_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t neg, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t operand3 = V_interpreter(ctx,Rd);
	uint128_t result;
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx))))
	{
		uint64_t add_instruction;
		uint64_t subtract_instruction;
		uint64_t multiply_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			add_instruction = (uint64_t)x86_addpd;
			subtract_instruction = (uint64_t)x86_subpd;
			multiply_instruction = (uint64_t)x86_mulpd;
		}
		else
		{
			add_instruction = (uint64_t)x86_addps;
			subtract_instruction = (uint64_t)x86_subps;
			multiply_instruction = (uint64_t)x86_mulps;
		}
		result = intrinsic_binary_interpreter<uint128_t>(ctx,multiply_instruction,operand1,operand2);
		if ((neg))
		{
			result = intrinsic_binary_interpreter<uint128_t>(ctx,subtract_instruction,operand3,result);
		}
		else
		{
			result = intrinsic_binary_interpreter<uint128_t>(ctx,add_instruction,operand3,result);
		}
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		}
	}
	else
	{
		result = 0;
		uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element2 = uint128_t::extract(operand2, e, esize);
			uint64_t element3 = uint128_t::extract(operand3, e, esize);
			if ((neg))
			{
				element1 = FPNeg_interpreter(ctx,element1,fpcr_state,esize);
			}
			uint128_t::insert(&result, e, esize, FPMulAdd_interpreter(ctx,element3,element1,element2,fpcr_state,esize));
		}
	}
	V_interpreter(ctx,Rd,result);
}

void faddp_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint128_t lo = V_interpreter(ctx,Rn);
	uint128_t hi = V_interpreter(ctx,Rm);
	uint128_t result = 0;
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint64_t element1 = get_from_concacted_vector_interpreter(ctx,hi,lo,((uint64_t)2ULL * (uint64_t)e),((uint64_t)datasize / (uint64_t)esize),esize);
		uint64_t element2 = get_from_concacted_vector_interpreter(ctx,hi,lo,((uint64_t)(((uint64_t)2ULL * (uint64_t)e)) + (uint64_t)1ULL),((uint64_t)datasize / (uint64_t)esize),esize);
		uint128_t::insert(&result, e, esize, FPAdd_interpreter(ctx,element1,element2,fpcr_state,esize));
	}
	V_interpreter(ctx,Rd,result);
}

void frsqrte_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t operand = V_interpreter(ctx,Rn);
	if ((use_x86_sse_interpreter(ctx)))
	{
		uint128_t one = sse_coppy_gp_across_lanes_interpreter(ctx,float_imm_interpreter(ctx,(uint64_t)1ULL,esize),esize);
		uint64_t sqrt_instruction;
		uint64_t divide_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			sqrt_instruction = (uint64_t)x86_sqrtpd;
			divide_instruction = (uint64_t)x86_divpd;
		}
		else
		{
			sqrt_instruction = (uint64_t)x86_sqrtps;
			divide_instruction = (uint64_t)x86_divps;
		}
		uint128_t sqrt_result = intrinsic_unary_interpreter<uint128_t>(ctx,sqrt_instruction,operand);
		uint128_t reciprocal_result = intrinsic_binary_interpreter<uint128_t>(ctx,divide_instruction,one,sqrt_result);
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			uint128_t::insert(&reciprocal_result, 1ULL, 64ULL, (uint64_t)0ULL);
		}
		V_interpreter(ctx,Rd,reciprocal_result);
	}
	else
	{
		uint128_t result = 0;
		uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element = uint128_t::extract(operand, e, esize);
			element = FPRSqrtEstimate_interpreter(ctx,element,fpcr_state,esize);
			uint128_t::insert(&result, e, esize, element);
		}
		V_interpreter(ctx,Rd,result);
	}
}

void frsqrts_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	if ((use_x86_sse_interpreter(ctx)))
	{
		uint128_t two = sse_coppy_gp_across_lanes_interpreter(ctx,float_imm_interpreter(ctx,(uint64_t)2ULL,esize),esize);
		uint128_t three = sse_coppy_gp_across_lanes_interpreter(ctx,float_imm_interpreter(ctx,(uint64_t)3ULL,esize),esize);
		uint128_t negator = sse_coppy_gp_across_lanes_interpreter(ctx,(uint64_t)((uint64_t)1ULL << (uint64_t)(((uint64_t)esize - (uint64_t)1ULL))),esize);
		operand1 = intrinsic_binary_interpreter<uint128_t>(ctx,(uint64_t)x86_xorps,operand1,negator);
		uint64_t add_instruction;
		uint64_t multiply_instruction;
		uint64_t divide_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			add_instruction = (uint64_t)x86_addpd;
			multiply_instruction = (uint64_t)x86_mulpd;
			divide_instruction = (uint64_t)x86_divpd;
		}
		else
		{
			add_instruction = (uint64_t)x86_addps;
			multiply_instruction = (uint64_t)x86_mulps;
			divide_instruction = (uint64_t)x86_divps;
		}
		uint128_t mul_result = intrinsic_binary_interpreter<uint128_t>(ctx,multiply_instruction,operand1,operand2);
		uint128_t add_result = intrinsic_binary_interpreter<uint128_t>(ctx,add_instruction,three,mul_result);
		uint128_t div_result = intrinsic_binary_interpreter<uint128_t>(ctx,divide_instruction,add_result,two);
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			uint128_t::insert(&div_result, 1ULL, 64ULL, (uint64_t)0ULL);
		}
		V_interpreter(ctx,Rd,div_result);
	}
	else
	{
		uint128_t result = 0;
		uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element2 = uint128_t::extract(operand2, e, esize);
			uint128_t::insert(&result, e, esize, FPRSqrtStepFused_interpreter(ctx,element1,element2,fpcr_state,esize));
		}
		V_interpreter(ctx,Rd,result);
	}
}

void frecps_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	if ((use_x86_sse_interpreter(ctx)))
	{
		uint128_t negator = sse_coppy_gp_across_lanes_interpreter(ctx,(uint64_t)((uint64_t)1ULL << (uint64_t)(((uint64_t)esize - (uint64_t)1ULL))),esize);
		uint128_t two = sse_coppy_gp_across_lanes_interpreter(ctx,float_imm_interpreter(ctx,(uint64_t)2ULL,esize),esize);
		operand1 = intrinsic_binary_interpreter<uint128_t>(ctx,(uint64_t)x86_xorps,operand1,negator);
		uint64_t add_instruction;
		uint64_t multiply_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			add_instruction = (uint64_t)x86_addpd;
			multiply_instruction = (uint64_t)x86_mulpd;
		}
		else
		{
			add_instruction = (uint64_t)x86_addps;
			multiply_instruction = (uint64_t)x86_mulps;
		}
		uint128_t result = intrinsic_binary_interpreter<uint128_t>(ctx,add_instruction,two,(uint128_t)intrinsic_binary_interpreter<uint128_t>(ctx,multiply_instruction,operand1,operand2));
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		}
		V_interpreter(ctx,Rd,result);
	}
	else
	{
		uint128_t result = 0;
		uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element2 = uint128_t::extract(operand2, e, esize);
			uint128_t::insert(&result, e, esize, FPRecipStepFused_interpreter(ctx,element1,element2,fpcr_state,esize));
		}
		V_interpreter(ctx,Rd,result);
	}
}

void fmul_scalar_by_element_interpreter(interpreter_data* ctx, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t H, uint64_t Rn, uint64_t Rd)
{
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)H);
	uint64_t index;
	uint64_t rm_hi = M;
	uint64_t test = ((uint64_t)(((uint64_t)sz << (uint64_t)1ULL)) | (uint64_t)L);
	if ((((uint64_t)(((uint64_t)test >> (uint64_t)1ULL)) == (uint64_t)0ULL)))
	{
		index = ((uint64_t)L | (uint64_t)(((uint64_t)H << (uint64_t)1ULL)));
	}
	else if ((((uint64_t)test == (uint64_t)2ULL)))
	{
		index = H;
	}
	else
	{
		undefined_interpreter(ctx);
	}
	floating_point_multiply_scalar_element_interpreter(ctx,Rd,Rn,((uint64_t)Rm | (uint64_t)(((uint64_t)rm_hi << (uint64_t)4ULL))),sz,index);
}

void fmul_vector_by_element_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t H, uint64_t Rn, uint64_t Rd)
{
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)H);
	uint64_t index;
	uint64_t rm_hi = M;
	uint64_t test = ((uint64_t)(((uint64_t)sz << (uint64_t)1ULL)) | (uint64_t)L);
	if ((((uint64_t)(((uint64_t)test >> (uint64_t)1ULL)) == (uint64_t)0ULL)))
	{
		index = ((uint64_t)L | (uint64_t)(((uint64_t)H << (uint64_t)1ULL)));
	}
	else if ((((uint64_t)test == (uint64_t)2ULL)))
	{
		index = H;
	}
	else
	{
		undefined_interpreter(ctx);
	}
	floating_point_multiply_vector_element_interpreter(ctx,Q,Rd,Rn,((uint64_t)Rm | (uint64_t)(((uint64_t)rm_hi << (uint64_t)4ULL))),sz,index);
}

void fmul_accumulate_scalar_interpreter(interpreter_data* ctx, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t neg, uint64_t H, uint64_t Rn, uint64_t Rd)
{
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)H);
	uint64_t index;
	uint64_t rm_hi = M;
	uint64_t test = ((uint64_t)(((uint64_t)sz << (uint64_t)1ULL)) | (uint64_t)L);
	if ((((uint64_t)(((uint64_t)test >> (uint64_t)1ULL)) == (uint64_t)0ULL)))
	{
		index = ((uint64_t)L | (uint64_t)(((uint64_t)H << (uint64_t)1ULL)));
	}
	else if ((((uint64_t)test == (uint64_t)2ULL)))
	{
		index = H;
	}
	else
	{
		undefined_interpreter(ctx);
	}
	floating_point_multiply_accumulate_scalar_element_interpreter(ctx,Rd,Rn,((uint64_t)Rm | (uint64_t)(((uint64_t)rm_hi << (uint64_t)4ULL))),neg,sz,index);
}

void fmul_accumulate_element_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t neg, uint64_t H, uint64_t Rn, uint64_t Rd)
{
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)H);
	uint64_t index;
	uint64_t rm_hi = M;
	uint64_t test = ((uint64_t)(((uint64_t)sz << (uint64_t)1ULL)) | (uint64_t)L);
	if ((((uint64_t)(((uint64_t)test >> (uint64_t)1ULL)) == (uint64_t)0ULL)))
	{
		index = ((uint64_t)L | (uint64_t)(((uint64_t)H << (uint64_t)1ULL)));
	}
	else if ((((uint64_t)test == (uint64_t)2ULL)))
	{
		index = H;
	}
	else
	{
		undefined_interpreter(ctx);
	}
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,((uint64_t)Rm | (uint64_t)(((uint64_t)rm_hi << (uint64_t)4ULL))));
	uint128_t operand3 = V_interpreter(ctx,Rd);
	floating_point_multiply_accumulate_vector_element_interpreter(ctx,Q,Rd,Rn,((uint64_t)Rm | (uint64_t)(((uint64_t)rm_hi << (uint64_t)4ULL))),neg,sz,index);
}

void faddp_scalar_interpreter(interpreter_data* ctx, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)esize * (uint64_t)2ULL);
	uint128_t operand = V_interpreter(ctx,Rn);
	if ((use_x86_sse_interpreter(ctx)))
	{
		uint128_t result = intrinsic_binary_interpreter<uint128_t>(ctx,select_interpreter(ctx,((uint64_t)esize == (uint64_t)64ULL),(uint64_t)x86_haddpd,(uint64_t)x86_haddps),operand,operand);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			uint128_t::insert(&result, 1ULL, 32ULL, (uint64_t)0ULL);
		}
		uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		V_interpreter(ctx,Rd,result);
	}
	else
	{
		uint128_t result = 0;
		uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
		uint64_t bottom = uint128_t::extract(operand, 0ULL, esize);
		uint64_t top = uint128_t::extract(operand, 1ULL, esize);
		uint128_t::insert(&result, 0ULL, esize, FPAdd_interpreter(ctx,bottom,top,fpcr_state,esize));
		V_interpreter(ctx,Rd,result);
	}
}

void fcmeq_vector_zero_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	fcm_vector_interpreter(ctx,Rd,Rn,-1ULL,0ULL,Q,sz);
}

void fcmgt_vector_zero_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	fcm_vector_interpreter(ctx,Rd,Rn,-1ULL,1ULL,Q,sz);
}

void fcmge_vector_zero_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	fcm_vector_interpreter(ctx,Rd,Rn,-1ULL,2ULL,Q,sz);
}

void fcmeq_vector_register_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	fcm_vector_interpreter(ctx,Rd,Rn,Rm,0ULL,Q,sz);
}

void fcmgt_vector_register_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	fcm_vector_interpreter(ctx,Rd,Rn,Rm,1ULL,Q,sz);
}

void fcmge_vector_register_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	fcm_vector_interpreter(ctx,Rd,Rn,Rm,2ULL,Q,sz);
}

void fadd_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		intrinsic_float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,-1ULL,(uint64_t)x86_addss,(uint64_t)x86_addsd);
		return;
	}
	float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPAdd_interpreter);
}

void fsub_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		intrinsic_float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,-1ULL,(uint64_t)x86_subss,(uint64_t)x86_subsd);
		return;
	}
	float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPSub_interpreter);
}

void fmul_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		intrinsic_float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,-1ULL,(uint64_t)x86_mulss,(uint64_t)x86_mulsd);
		return;
	}
	float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPMul_interpreter);
}

void fdiv_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		intrinsic_float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,-1ULL,(uint64_t)x86_divss,(uint64_t)x86_divsd);
		return;
	}
	float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPDiv_interpreter);
}

void fmax_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		intrinsic_float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,-1ULL,(uint64_t)x86_maxss,(uint64_t)x86_maxsd);
		return;
	}
	float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPMax_interpreter);
}

void fmin_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		intrinsic_float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,-1ULL,(uint64_t)x86_minss,(uint64_t)x86_minsd);
		return;
	}
	float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPMin_interpreter);
}

void fmaxnm_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPMaxNum_interpreter);
}

void fminnm_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPMinNum_interpreter);
}

void fnmul_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		uint128_t operand1 = V_interpreter(ctx,Rn);
		uint128_t operand2 = V_interpreter(ctx,Rm);
		uint64_t esize = get_flt_size_interpreter(ctx,ftype);
		uint128_t negator = (uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)esize - (uint64_t)1ULL))));
		uint64_t multiply_instruction;
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			multiply_instruction = (uint64_t)x86_mulss;
		}
		else if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			multiply_instruction = (uint64_t)x86_mulsd;
		}
		else
		{
			undefined_interpreter(ctx);
		}
		operand2 = intrinsic_binary_interpreter<uint128_t>(ctx,(uint64_t)x86_xorps,operand2,negator);
		uint128_t result = intrinsic_binary_interpreter<uint128_t>(ctx,multiply_instruction,operand1,operand2);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			uint128_t::insert(&result, 1ULL, 32ULL, (uint64_t)0ULL);
		}
		uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		V_interpreter(ctx,Rd,result);
		return;
	}
	float_binary_scalar_interpreter(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPNMul_interpreter);
}

void fabs_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx))))
	{
		uint64_t esize = get_flt_size_interpreter(ctx,ftype);
		uint128_t operand = V_interpreter(ctx,Rn);
		uint128_t mask = (uint64_t)(((uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)esize - (uint64_t)1ULL)))) - (uint64_t)1ULL));
		uint128_t result = intrinsic_binary_interpreter<uint128_t>(ctx,(uint64_t)x86_pand,operand,mask);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			uint128_t::insert(&result, 1ULL, 32ULL, (uint64_t)0ULL);
		}
		uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		V_interpreter(ctx,Rd,result);
		return;
	}
	float_unary_scalar_interpreter(ctx,Rd,Rn,ftype,(uint64_t)FPAbs_interpreter);
}

void fneg_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx))))
	{
		uint64_t esize = get_flt_size_interpreter(ctx,ftype);
		uint128_t operand = V_interpreter(ctx,Rn);
		uint128_t mask = (uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)esize - (uint64_t)1ULL))));
		uint128_t result = intrinsic_binary_interpreter<uint128_t>(ctx,(uint64_t)x86_xorps,operand,mask);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			uint128_t::insert(&result, 1ULL, 32ULL, (uint64_t)0ULL);
		}
		uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		V_interpreter(ctx,Rd,result);
		return;
	}
	float_unary_scalar_interpreter(ctx,Rd,Rn,ftype,(uint64_t)FPNeg_interpreter);
}

void fneg_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx))))
	{
		uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
		uint128_t operand = V_interpreter(ctx,Rn);
		uint128_t mask = sse_coppy_gp_across_lanes_interpreter(ctx,(uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)esize - (uint64_t)1ULL)))),esize);
		uint128_t result = intrinsic_binary_interpreter<uint128_t>(ctx,(uint64_t)x86_xorps,operand,mask);
		if ((((uint64_t)Q == (uint64_t)0ULL)))
		{
			uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		}
		V_interpreter(ctx,Rd,result);
		return;
	}
	float_unary_vector_interpreter(ctx,Rd,Rn,Q,sz,(uint64_t)FPNeg_interpreter);
}

void fsqrt_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		uint64_t esize = get_flt_size_interpreter(ctx,ftype);
		uint128_t operand = V_interpreter(ctx,Rn);
		uint64_t sqrt_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			sqrt_instruction = (uint64_t)x86_sqrtsd;
		}
		else
		{
			sqrt_instruction = (uint64_t)x86_sqrtss;
		}
		uint128_t result = intrinsic_unary_interpreter<uint128_t>(ctx,sqrt_instruction,operand);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			uint128_t::insert(&result, 1ULL, 32ULL, (uint64_t)0ULL);
		}
		uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		V_interpreter(ctx,Rd,result);
		return;
	}
	float_unary_scalar_interpreter(ctx,Rd,Rn,ftype,(uint64_t)FPSqrt_interpreter);
}

void fsqrt_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx))))
	{
		uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
		uint128_t operand = V_interpreter(ctx,Rn);
		uint64_t sqrt_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			sqrt_instruction = (uint64_t)x86_sqrtpd;
		}
		else
		{
			sqrt_instruction = (uint64_t)x86_sqrtps;
		}
		uint128_t result = intrinsic_unary_interpreter<uint128_t>(ctx,sqrt_instruction,operand);
		if ((((uint64_t)Q == (uint64_t)0ULL)))
		{
			uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		}
		V_interpreter(ctx,Rd,result);
		return;
	}
	float_unary_vector_interpreter(ctx,Rd,Rn,Q,sz,(uint64_t)FPSqrt_interpreter);
}

void frecpe_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx))))
	{
		uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
		uint128_t operand = V_interpreter(ctx,Rn);
		uint128_t one = sse_coppy_gp_across_lanes_interpreter(ctx,float_imm_interpreter(ctx,(uint64_t)1ULL,esize),esize);
		uint64_t divide_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			divide_instruction = (uint64_t)x86_divpd;
		}
		else
		{
			divide_instruction = (uint64_t)x86_divps;
		}
		uint128_t result = intrinsic_binary_interpreter<uint128_t>(ctx,divide_instruction,one,operand);
		if ((((uint64_t)Q == (uint64_t)0ULL)))
		{
			uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		}
		V_interpreter(ctx,Rd,result);
		return;
	}
	float_unary_vector_interpreter(ctx,Rd,Rn,Q,sz,(uint64_t)FPRecipEstimate_interpreter);
}

void frsqrte_scalar_interpreter(interpreter_data* ctx, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t operand = V_interpreter(ctx,Rn);
	if ((((uint64_t)use_fast_float_interpreter(ctx) && (uint64_t)use_x86_sse_interpreter(ctx))))
	{
		uint128_t one = float_imm_interpreter(ctx,(uint64_t)1ULL,esize);
		uint64_t sqrt_instruction;
		uint64_t divide_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			sqrt_instruction = (uint64_t)x86_sqrtsd;
			divide_instruction = (uint64_t)x86_divsd;
		}
		else
		{
			sqrt_instruction = (uint64_t)x86_sqrtss;
			divide_instruction = (uint64_t)x86_divss;
		}
		uint128_t sqrt_result = intrinsic_unary_interpreter<uint128_t>(ctx,sqrt_instruction,(uint128_t)operand);
		uint128_t reciprocal_result = intrinsic_binary_interpreter<uint128_t>(ctx,divide_instruction,one,sqrt_result);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			uint128_t::insert(&reciprocal_result, 1ULL, 32ULL, (uint64_t)0ULL);
		}
		uint128_t::insert(&reciprocal_result, 1ULL, 64ULL, (uint64_t)0ULL);
		V_interpreter(ctx,Rd,reciprocal_result);
	}
	else
	{
		uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
		uint64_t result = FPRSqrtEstimate_interpreter(ctx,operand,fpcr_state,esize);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			result = ((uint64_t)result & (uint64_t)4294967295ULL);
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

uint64_t _compare_and_swap_interpreter(interpreter_data* ctx, uint64_t physical_address, uint64_t expecting, uint64_t to_swap, uint64_t size)
{
	if ((use_x86_interpreter(ctx)))
	{
		if ((((uint64_t)size != (uint64_t)64ULL)))
		{
			uint64_t mask = ((uint64_t)(((uint64_t)1ULL << (uint64_t)size)) - (uint64_t)1ULL);
			uint64_t masking_value = *(uint64_t*)physical_address;
			masking_value = ((uint64_t)masking_value & (uint64_t)~mask);
			expecting = ((uint64_t)(((uint64_t)expecting & (uint64_t)mask)) | (uint64_t)masking_value);
			to_swap = ((uint64_t)(((uint64_t)to_swap & (uint64_t)mask)) | (uint64_t)masking_value);
		}
		return intrinsic_ternary_interpreter<uint64_t>(ctx,(uint64_t)x86_cmpxchg,physical_address,expecting,to_swap);
	}
	return call_interpreter(ctx,physical_address,expecting,to_swap,(uint64_t)size,(uint64_t)0ULL,(uint64_t)0ULL,(uint64_t)compare_and_swap_interpreter_cpp);
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

void dup_general_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_interpreter(ctx,bits_c_interpreter(ctx,imm5,3ULL,0ULL));
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t element = X_interpreter(ctx,Rn);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t result;
	if ((use_x86_sse_interpreter(ctx)))
	{
		result = sse_coppy_gp_across_lanes_interpreter(ctx,element,esize);
		if ((!Q))
		{
			uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		}
	}
	else
	{
		result = 0;
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint128_t::insert(&result, e, esize, element);
		}
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
			uint32_t result = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				result = (uint32_t)sign_extend((uint8_t)result);
			}
			X_interpreter(ctx,Rd,(uint64_t)result);
		}
		if (datasize == 64ULL)
		{
			uint64_t result = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				result = (uint64_t)sign_extend((uint8_t)result);
			}
			X_interpreter(ctx,Rd,(uint64_t)result);
		}
		
	}
	if (esize == 16ULL)
	{
		if (datasize == 32ULL)
		{
			uint32_t result = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				result = (uint32_t)sign_extend((uint16_t)result);
			}
			X_interpreter(ctx,Rd,(uint64_t)result);
		}
		if (datasize == 64ULL)
		{
			uint64_t result = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				result = (uint64_t)sign_extend((uint16_t)result);
			}
			X_interpreter(ctx,Rd,(uint64_t)result);
		}
		
	}
	if (esize == 32ULL)
	{
		if (datasize == 32ULL)
		{
			uint32_t result = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				result = (uint32_t)sign_extend((uint32_t)result);
			}
			X_interpreter(ctx,Rd,(uint64_t)result);
		}
		if (datasize == 64ULL)
		{
			uint64_t result = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				result = (uint64_t)sign_extend((uint32_t)result);
			}
			X_interpreter(ctx,Rd,(uint64_t)result);
		}
		
	}
	if (esize == 64ULL)
	{
		if (datasize == 32ULL)
		{
			uint32_t result = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				result = (uint32_t)sign_extend((uint64_t)result);
			}
			X_interpreter(ctx,Rd,(uint64_t)result);
		}
		if (datasize == 64ULL)
		{
			uint64_t result = uint128_t::extract(operand, index, esize);
			if ((!U))
			{
				result = (uint64_t)sign_extend((uint64_t)result);
			}
			X_interpreter(ctx,Rd,(uint64_t)result);
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
		uint64_t result = X_interpreter(ctx,Rn);
		if (size == 32ULL)
		{
			VPart_interpreter(ctx,Rd,part,size,(uint64_t)(uint32_t)result);
		}
		if (size == 64ULL)
		{
			VPart_interpreter(ctx,Rd,part,size,(uint64_t)(uint64_t)result);
		}
		
	}
	else
	{
		uint128_t v = V_interpreter(ctx,Rn);
		uint64_t result = uint128_t::extract(v, part, size);
		X_interpreter(ctx,Rd,result);
	}
}

void convert_to_float_gp_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_float_interpreter(ctx,sf,ftype,U,Rn,Rd,0ULL);
}

void convert_to_float_vector_scalar_interpreter(interpreter_data* ctx, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	convert_to_float_interpreter(ctx,sz,sz,U,Rn,Rd,1ULL);
}

void convert_to_float_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t operand = V_interpreter(ctx,Rn);
	uint128_t result = 0;
	if (esize == 16ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element = uint128_t::extract(operand, e, esize);
			uint64_t working;
			if ((U))
			{
				working = convert_to_float<uint16_t, uint16_t>((uint16_t)element, 0);
			}
			else
			{
				working = convert_to_float<uint16_t, uint16_t>((uint16_t)element, 1);
			}
			uint128_t::insert(&result, e, esize, working);
		}
		V_interpreter(ctx,Rd,result);
	}
	if (esize == 32ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element = uint128_t::extract(operand, e, esize);
			uint64_t working;
			if ((U))
			{
				working = convert_to_float<uint32_t, uint32_t>((uint32_t)element, 0);
			}
			else
			{
				working = convert_to_float<uint32_t, uint32_t>((uint32_t)element, 1);
			}
			uint128_t::insert(&result, e, esize, working);
		}
		V_interpreter(ctx,Rd,result);
	}
	if (esize == 64ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element = uint128_t::extract(operand, e, esize);
			uint64_t working;
			if ((U))
			{
				working = convert_to_float<uint64_t, uint64_t>((uint64_t)element, 0);
			}
			else
			{
				working = convert_to_float<uint64_t, uint64_t>((uint64_t)element, 1);
			}
			uint128_t::insert(&result, e, esize, working);
		}
		V_interpreter(ctx,Rd,result);
	}
	
}

void shl_immedaite_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)highest_bit_set_c_interpreter(ctx,immh,32ULL));
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t shift = ((uint64_t)(((uint64_t)(((uint64_t)immh << (uint64_t)3ULL)) | (uint64_t)immb)) - (uint64_t)esize);
	uint128_t operand = V_interpreter(ctx,Rn);
	uint128_t result = 0;
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint64_t element = uint128_t::extract(operand, e, esize);
		element = ((uint64_t)element << (uint64_t)shift);
		uint128_t::insert(&result, e, esize, element);
	}
	V_interpreter(ctx,Rd,result);
}

void sshr_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)highest_bit_set_c_interpreter(ctx,immh,32ULL));
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t shift = ((uint64_t)(((uint64_t)esize * (uint64_t)2ULL)) - (uint64_t)(((uint64_t)(((uint64_t)immh << (uint64_t)3ULL)) | (uint64_t)immb)));
	uint128_t operand = V_interpreter(ctx,Rn);
	uint128_t result = 0;
	if (esize == 8ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element = uint128_t::extract(operand, e, esize);
			element = shift_right_check_interpreter(ctx,(uint64_t)sign_extend((uint8_t)element),(uint64_t)shift,esize,0ULL);
			uint128_t::insert(&result, e, esize, element);
		}
		V_interpreter(ctx,Rd,result);
	}
	if (esize == 16ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element = uint128_t::extract(operand, e, esize);
			element = shift_right_check_interpreter(ctx,(uint64_t)sign_extend((uint16_t)element),(uint64_t)shift,esize,0ULL);
			uint128_t::insert(&result, e, esize, element);
		}
		V_interpreter(ctx,Rd,result);
	}
	if (esize == 32ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element = uint128_t::extract(operand, e, esize);
			element = shift_right_check_interpreter(ctx,(uint64_t)sign_extend((uint32_t)element),(uint64_t)shift,esize,0ULL);
			uint128_t::insert(&result, e, esize, element);
		}
		V_interpreter(ctx,Rd,result);
	}
	if (esize == 64ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element = uint128_t::extract(operand, e, esize);
			element = shift_right_check_interpreter(ctx,(uint64_t)sign_extend((uint64_t)element),(uint64_t)shift,esize,0ULL);
			uint128_t::insert(&result, e, esize, element);
		}
		V_interpreter(ctx,Rd,result);
	}
	
}

void shll_shll2_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)highest_bit_set_c_interpreter(ctx,bits_c_interpreter(ctx,immh,2ULL,0ULL),32ULL));
	uint64_t datasize = 64ULL;
	uint64_t part = Q;
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t shift = ((uint64_t)(((uint64_t)(((uint64_t)immh << (uint64_t)3ULL)) | (uint64_t)immb)) - (uint64_t)esize);
	uint64_t operand = VPart_interpreter(ctx,Rn,part,datasize);
	uint128_t result = 0;
	uint128_t working_vector = 0;
	uint128_t::insert(&working_vector, 0ULL, datasize, operand);
	if (esize == 8ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element = uint128_t::extract(working_vector, e, esize);
			if ((!U))
			{
				element = (uint64_t)sign_extend((uint8_t)element);
			}
			element = ((uint64_t)element << (uint64_t)shift);
			uint128_t::insert(&result, e, (((uint64_t)2ULL * (uint64_t)esize)), element);
		}
		V_interpreter(ctx,Rd,result);
	}
	if (esize == 16ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element = uint128_t::extract(working_vector, e, esize);
			if ((!U))
			{
				element = (uint64_t)sign_extend((uint16_t)element);
			}
			element = ((uint64_t)element << (uint64_t)shift);
			uint128_t::insert(&result, e, (((uint64_t)2ULL * (uint64_t)esize)), element);
		}
		V_interpreter(ctx,Rd,result);
	}
	if (esize == 32ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element = uint128_t::extract(working_vector, e, esize);
			if ((!U))
			{
				element = (uint64_t)sign_extend((uint32_t)element);
			}
			element = ((uint64_t)element << (uint64_t)shift);
			uint128_t::insert(&result, e, (((uint64_t)2ULL * (uint64_t)esize)), element);
		}
		V_interpreter(ctx,Rd,result);
	}
	if (esize == 64ULL)
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element = uint128_t::extract(working_vector, e, esize);
			if ((!U))
			{
				element = (uint64_t)sign_extend((uint64_t)element);
			}
			element = ((uint64_t)element << (uint64_t)shift);
			uint128_t::insert(&result, e, (((uint64_t)2ULL * (uint64_t)esize)), element);
		}
		V_interpreter(ctx,Rd,result);
	}
	
}

void shrn_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)highest_bit_set_c_interpreter(ctx,bits_c_interpreter(ctx,immh,2ULL,0ULL),32ULL));
	uint64_t datasize = 64ULL;
	uint64_t part = Q;
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t shift = ((uint64_t)(((uint64_t)2ULL * (uint64_t)esize)) - (uint64_t)(((uint64_t)(((uint64_t)immh << (uint64_t)3ULL)) | (uint64_t)immb)));
	uint128_t operand = V_interpreter(ctx,Rn);
	uint128_t result = 0;
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint64_t element = shift_right_check_interpreter(ctx,uint128_t::extract(operand, e, (((uint64_t)2ULL * (uint64_t)esize))),(uint64_t)shift,((uint64_t)2ULL * (uint64_t)esize),1ULL);
		uint128_t::insert(&result, e, esize, element);
	}
	VPart_interpreter(ctx,Rd,part,datasize,(uint64_t)result);
}

void rev64_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd)
{
	uint64_t csize = 64ULL;
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t containers = ((uint64_t)datasize / (uint64_t)csize);
	uint128_t operand = V_interpreter(ctx,Rn);
	uint128_t result = 0;
	for (uint64_t c = 0; c < (containers); c++)
	{
		uint64_t container = uint128_t::extract(operand, c, csize);
		uint128_t::insert(&result, c, csize, reverse_interpreter(ctx,(uint128_t)container,esize,csize));
	}
	V_interpreter(ctx,Rd,result);
}

void neg_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd)
{
	uint128_t operand = V_interpreter(ctx,Rn);
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t result = 0;
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint128_t::insert(&result, e, esize, -(uint128_t::extract(operand, e, esize)));
	}
	V_interpreter(ctx,Rd,result);
}

void not_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rn, uint64_t Rd)
{
	uint128_t operand = V_interpreter(ctx,Rn);
	uint64_t esize = 8ULL;
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t result = 0;
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint128_t::insert(&result, e, esize, ~(uint128_t::extract(operand, e, esize)));
	}
	V_interpreter(ctx,Rd,result);
}

void abs_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd)
{
	uint128_t operand = V_interpreter(ctx,Rn);
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t result = 0;
	for (uint64_t e = 0; e < (elements); e++)
	{
		if (esize == 8ULL)
		{
			uint8_t working = (uint128_t::extract(operand, e, esize));
			if ((((uint8_t)(sign_extend((uint8_t)working) < sign_extend((uint8_t)0ULL)))))
			{
				working = -working;
			}
			uint128_t::insert(&result, e, esize, working);
		}
		if (esize == 16ULL)
		{
			uint16_t working = (uint128_t::extract(operand, e, esize));
			if ((((uint16_t)(sign_extend((uint16_t)working) < sign_extend((uint16_t)0ULL)))))
			{
				working = -working;
			}
			uint128_t::insert(&result, e, esize, working);
		}
		if (esize == 32ULL)
		{
			uint32_t working = (uint128_t::extract(operand, e, esize));
			if ((((uint32_t)(sign_extend((uint32_t)working) < sign_extend((uint32_t)0ULL)))))
			{
				working = -working;
			}
			uint128_t::insert(&result, e, esize, working);
		}
		if (esize == 64ULL)
		{
			uint64_t working = (uint128_t::extract(operand, e, esize));
			if ((((uint64_t)(sign_extend((uint64_t)working) < sign_extend((uint64_t)0ULL)))))
			{
				working = -working;
			}
			uint128_t::insert(&result, e, esize, working);
		}
		
	}
	V_interpreter(ctx,Rd,result);
}

void mul_vector_index_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t L, uint64_t M, uint64_t Rm, uint64_t H, uint64_t Rn, uint64_t Rd)
{
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)H);
	uint64_t index;
	uint64_t rm_hi;
	if ((((uint64_t)size == (uint64_t)1ULL)))
	{
		index = H;
		index = ((uint64_t)L | (uint64_t)(((uint64_t)index << (uint64_t)1ULL)));
		index = ((uint64_t)M | (uint64_t)(((uint64_t)index << (uint64_t)1ULL)));
		rm_hi = 0ULL;
	}
	else if ((((uint64_t)size == (uint64_t)2ULL)))
	{
		index = H;
		index = ((uint64_t)L | (uint64_t)(((uint64_t)index << (uint64_t)1ULL)));
		rm_hi = M;
	}
	else
	{
		undefined_interpreter(ctx);
	}
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,((uint64_t)Rm | (uint64_t)(((uint64_t)rm_hi << (uint64_t)4ULL))));
	uint128_t result = 0;
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t element2 = uint128_t::extract(operand2, index, esize);
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint64_t element1 = uint128_t::extract(operand1, e, esize);
		uint64_t product = ((uint64_t)element1 * (uint64_t)element2);
		uint128_t::insert(&result, e, esize, product);
	}
	V_interpreter(ctx,Rd,result);
}

void mul_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t result = 0;
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint64_t element1 = uint128_t::extract(operand1, e, esize);
		uint64_t element2 = uint128_t::extract(operand2, e, esize);
		uint64_t working = ((uint64_t)element1 * (uint64_t)element2);
		uint128_t::insert(&result, e, esize, working);
	}
	V_interpreter(ctx,Rd,result);
}

void ext_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t imm4, uint64_t Rn, uint64_t Rd)
{
	uint128_t lo = V_interpreter(ctx,Rn);
	uint128_t hi = V_interpreter(ctx,Rm);
	uint64_t vector_size = ((uint64_t)8ULL << (uint64_t)Q);
	uint64_t start = imm4;
	uint128_t result = 0;
	for (uint64_t e = 0; e < (vector_size); e++)
	{
		uint8_t working = get_from_concacted_vector_interpreter(ctx,hi,lo,((uint64_t)start + (uint64_t)e),vector_size,8ULL);
		uint128_t::insert(&result, e, 8ULL, working);
	}
	V_interpreter(ctx,Rd,result);
}

void compare_above_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t result = 0;
	for (uint64_t e = 0; e < (elements); e++)
	{
		if (esize == 8ULL)
		{
			uint8_t element1 = uint128_t::extract(operand1, e, esize);
			uint8_t element2 = uint128_t::extract(operand2, e, esize);
			uint8_t working;
			if ((U))
			{
				working = ((uint8_t)element1 > (uint8_t)element2);
			}
			else
			{
				working = ((uint8_t)(sign_extend((uint8_t)element1) > sign_extend((uint8_t)element2)));
			}
			uint128_t::insert(&result, e, esize, (((uint8_t)0ULL - (uint8_t)working)));
		}
		if (esize == 16ULL)
		{
			uint16_t element1 = uint128_t::extract(operand1, e, esize);
			uint16_t element2 = uint128_t::extract(operand2, e, esize);
			uint16_t working;
			if ((U))
			{
				working = ((uint16_t)element1 > (uint16_t)element2);
			}
			else
			{
				working = ((uint16_t)(sign_extend((uint16_t)element1) > sign_extend((uint16_t)element2)));
			}
			uint128_t::insert(&result, e, esize, (((uint16_t)0ULL - (uint16_t)working)));
		}
		if (esize == 32ULL)
		{
			uint32_t element1 = uint128_t::extract(operand1, e, esize);
			uint32_t element2 = uint128_t::extract(operand2, e, esize);
			uint32_t working;
			if ((U))
			{
				working = ((uint32_t)element1 > (uint32_t)element2);
			}
			else
			{
				working = ((uint32_t)(sign_extend((uint32_t)element1) > sign_extend((uint32_t)element2)));
			}
			uint128_t::insert(&result, e, esize, (((uint32_t)0ULL - (uint32_t)working)));
		}
		if (esize == 64ULL)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element2 = uint128_t::extract(operand2, e, esize);
			uint64_t working;
			if ((U))
			{
				working = ((uint64_t)element1 > (uint64_t)element2);
			}
			else
			{
				working = ((uint64_t)(sign_extend((uint64_t)element1) > sign_extend((uint64_t)element2)));
			}
			uint128_t::insert(&result, e, esize, (((uint64_t)0ULL - (uint64_t)working)));
		}
		
	}
	V_interpreter(ctx,Rd,result);
}

void shl_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t result = 0;
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint64_t element = uint128_t::extract(operand1, e, esize);
		uint64_t shift = (uint64_t)sign_extend((uint8_t)(uint128_t::extract(operand2, e, esize)));
		if ((!U))
		{
			if (esize == 8ULL)
			{
				element = (uint64_t)sign_extend((uint8_t)element);
			}
			if (esize == 16ULL)
			{
				element = (uint64_t)sign_extend((uint16_t)element);
			}
			if (esize == 32ULL)
			{
				element = (uint64_t)sign_extend((uint32_t)element);
			}
			if (esize == 64ULL)
			{
				element = (uint64_t)sign_extend((uint64_t)element);
			}
			
		}
		if ((((uint64_t)(sign_extend((uint64_t)shift) >= sign_extend((uint64_t)0ULL)))))
		{
			element = shift_left_check_interpreter(ctx,element,shift,esize);
		}
		else
		{
			shift = -shift;
			element = shift_right_check_interpreter(ctx,element,shift,esize,U);
		}
		uint128_t::insert(&result, e, esize, element);
	}
	V_interpreter(ctx,Rd,result);
}

void add_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t result;
	if ((use_x86_sse2_interpreter(ctx)))
	{
		uint64_t add_instruction;
		if ((((uint64_t)esize == (uint64_t)8ULL)))
		add_instruction = (uint64_t)x86_paddb;
		else if ((((uint64_t)esize == (uint64_t)16ULL)))
		add_instruction = (uint64_t)x86_paddw;
		else if ((((uint64_t)esize == (uint64_t)32ULL)))
		add_instruction = (uint64_t)x86_paddd;
		else if ((((uint64_t)esize == (uint64_t)64ULL)))
		add_instruction = (uint64_t)x86_paddq;
		else
		undefined_interpreter(ctx);
		result = intrinsic_binary_interpreter<uint128_t>(ctx,add_instruction,operand1,operand2);
		if ((((uint64_t)Q == (uint64_t)0ULL)))
		{
			uint128_t::insert(&result, 1ULL, 64ULL, (uint64_t)0ULL);
		}
	}
	else
	{
		result = 0;
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element2 = uint128_t::extract(operand2, e, esize);
			uint128_t::insert(&result, e, esize, (((uint64_t)element1 + (uint64_t)element2)));
		}
	}
	V_interpreter(ctx,Rd,result);
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
		if ((use_x86_interpreter(ctx)))
		{
			count = intrinsic_unary_interpreter<uint16_t>(ctx,(uint64_t)x86_popcnt,(uint16_t)working);
		}
		else
		{
			for (uint64_t b = 0; b < (esize); b++)
			{
				uint8_t bit = ((uint8_t)(((uint8_t)working >> (uint8_t)b)) & (uint8_t)1ULL);
				if ((bit))
				{
					count = (((uint8_t)count + (uint8_t)1ULL));
				}
			}
		}
		uint128_t::insert(&result, e, esize, count);
	}
	V_interpreter(ctx,Rd,result);
}

void orr_orn_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t invert, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((use_x86_sse_interpreter(ctx)))
	{
		x86_sse_logic_vector_interpreter(ctx,Rd,Rn,Rm,Q,invert,(uint64_t)x86_orps);
	}
	else
	{
		uint64_t esize = 64ULL;
		uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
		uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
		uint128_t operand1 = V_interpreter(ctx,Rn);
		uint128_t operand2 = V_interpreter(ctx,Rm);
		uint128_t result = 0;
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element2 = uint128_t::extract(operand2, e, esize);
			if ((invert))
			{
				element2 = ~element2;
			}
			uint128_t::insert(&result, e, esize, (((uint64_t)element1 | (uint64_t)element2)));
		}
		V_interpreter(ctx,Rd,result);
	}
}

void bsl_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = 64ULL;
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t operand1 = V_interpreter(ctx,Rm);
	uint128_t operand2 = V_interpreter(ctx,Rd);
	uint128_t operand3 = V_interpreter(ctx,Rn);
	if ((use_x86_sse_interpreter(ctx)))
	{
		uint128_t xor_result = intrinsic_binary_interpreter<uint128_t>(ctx,(uint64_t)x86_xorps,operand1,operand3);
		uint128_t and_result = intrinsic_binary_interpreter<uint128_t>(ctx,(uint64_t)x86_pand,xor_result,operand2);
		uint128_t final_result = intrinsic_binary_interpreter<uint128_t>(ctx,(uint64_t)x86_xorps,operand1,and_result);
		if ((((uint64_t)Q == (uint64_t)0ULL)))
		{
			uint128_t::insert(&final_result, 1ULL, 64ULL, (uint64_t)0ULL);
		}
		V_interpreter(ctx,Rd,final_result);
	}
	else
	{
		uint128_t result = 0;
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element2 = uint128_t::extract(operand2, e, esize);
			uint64_t element3 = uint128_t::extract(operand3, e, esize);
			uint128_t::insert(&result, e, esize, (((uint64_t)element1 ^ (uint64_t)(((uint64_t)(((uint64_t)element1 ^ (uint64_t)element3)) & (uint64_t)element2)))));
		}
		V_interpreter(ctx,Rd,result);
	}
}

void and_bic_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t invert, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((use_x86_sse2_interpreter(ctx)))
	{
		if ((invert))
		{
			x86_sse_logic_vector_interpreter(ctx,Rd,Rm,Rn,Q,0ULL,(uint64_t)x86_pandn);
		}
		else
		{
			x86_sse_logic_vector_interpreter(ctx,Rd,Rn,Rm,Q,0ULL,(uint64_t)x86_pand);
		}
	}
	else
	{
		uint64_t esize = 64ULL;
		uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
		uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
		uint128_t operand1 = V_interpreter(ctx,Rn);
		uint128_t operand2 = V_interpreter(ctx,Rm);
		uint128_t result = 0;
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element2 = uint128_t::extract(operand2, e, esize);
			if ((invert))
			{
				element2 = ~element2;
			}
			uint128_t::insert(&result, e, esize, (((uint64_t)element1 & (uint64_t)element2)));
		}
		V_interpreter(ctx,Rd,result);
	}
}

void eor_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((use_x86_sse_interpreter(ctx)))
	{
		x86_sse_logic_vector_interpreter(ctx,Rd,Rn,Rm,Q,0ULL,(uint64_t)x86_xorps);
	}
	else
	{
		uint64_t esize = 64ULL;
		uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
		uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
		uint128_t operand1 = V_interpreter(ctx,Rn);
		uint128_t operand2 = V_interpreter(ctx,Rm);
		uint128_t result = 0;
		for (uint64_t e = 0; e < (elements); e++)
		{
			uint64_t element1 = uint128_t::extract(operand1, e, esize);
			uint64_t element2 = uint128_t::extract(operand2, e, esize);
			uint128_t::insert(&result, e, esize, (((uint64_t)element1 ^ (uint64_t)element2)));
		}
		V_interpreter(ctx,Rd,result);
	}
}

void xnt_xnt2_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = 64ULL;
	uint64_t part = Q;
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint128_t operand = V_interpreter(ctx,Rn);
	uint128_t result = 0;
	uint64_t mask = -1ULL;
	if ((((uint64_t)esize != (uint64_t)64ULL)))
	{
		mask = ((uint64_t)(((uint64_t)1ULL << (uint64_t)esize)) - (uint64_t)1ULL);
	}
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint64_t element = uint128_t::extract(operand, e, (((uint64_t)2ULL * (uint64_t)esize)));
		uint128_t::insert(&result, e, esize, (((uint64_t)element & (uint64_t)mask)));
	}
	VPart_interpreter(ctx,Rd,part,datasize,(uint64_t)result);
}

void zip_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t op, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t part = op;
	uint64_t pairs = ((uint64_t)elements / (uint64_t)2ULL);
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t result = 0;
	uint64_t base = ((uint64_t)part * (uint64_t)pairs);
	for (uint64_t p = 0; p < (pairs); p++)
	{
		uint128_t::insert(&result, (((uint64_t)((uint64_t)2ULL * (uint64_t)p) + (uint64_t)0ULL)), esize, (uint128_t::extract(operand1, (((uint64_t)base + (uint64_t)p)), esize)));
		uint128_t::insert(&result, (((uint64_t)((uint64_t)2ULL * (uint64_t)p) + (uint64_t)1ULL)), esize, (uint128_t::extract(operand2, (((uint64_t)base + (uint64_t)p)), esize)));
	}
	V_interpreter(ctx,Rd,result);
}

void trn_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t op, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t part = op;
	uint64_t pairs = ((uint64_t)elements / (uint64_t)2ULL);
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t result = 0;
	for (uint64_t p = 0; p < (pairs); p++)
	{
		uint128_t::insert(&result, (((uint64_t)((uint64_t)2ULL * (uint64_t)p) + (uint64_t)0ULL)), esize, (uint128_t::extract(operand1, (((uint64_t)((uint64_t)2ULL * (uint64_t)p) + (uint64_t)part)), esize)));
		uint128_t::insert(&result, (((uint64_t)((uint64_t)2ULL * (uint64_t)p) + (uint64_t)1ULL)), esize, (uint128_t::extract(operand2, (((uint64_t)((uint64_t)2ULL * (uint64_t)p) + (uint64_t)part)), esize)));
	}
	V_interpreter(ctx,Rd,result);
}

void tbl_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t len, uint64_t Rn, uint64_t Rd)
{
	store_context_interpreter(ctx);
	uint64_t vector_context = get_vector_context_interpreter(ctx);
	call_interpreter(ctx,vector_context,(uint64_t)Rd,(uint64_t)Rn,(uint64_t)len,(uint64_t)Rm,(uint64_t)Q,(uint64_t)table_lookup_fallback);
	load_context_interpreter(ctx);
}

void ld1r_no_offset_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	memory_1_interpreter(ctx,0ULL,Q,1ULL,0ULL,0ULL,0ULL,6ULL,0ULL,size,Rn,Rt,1ULL);
}

void ld1r_post_index_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	memory_1_interpreter(ctx,1ULL,Q,1ULL,0ULL,Rm,0ULL,6ULL,0ULL,size,Rn,Rt,1ULL);
}

void ld1_single_structure_no_offset_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	memory_1_interpreter(ctx,0ULL,Q,1ULL,0ULL,0ULL,0ULL,((uint64_t)opcode << (uint64_t)1ULL),S,size,Rn,Rt,1ULL);
}

void ld1_single_structure_post_index_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	memory_1_interpreter(ctx,1ULL,Q,1ULL,0ULL,Rm,0ULL,((uint64_t)opcode << (uint64_t)1ULL),S,size,Rn,Rt,1ULL);
}

void st2_multiple_structures_no_offset_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	st_interpreter(ctx,0ULL,Q,0ULL,4ULL,size,0ULL,Rn,Rt);
}

void st2_multiple_structures_post_index_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	st_interpreter(ctx,1ULL,Q,0ULL,4ULL,size,Rm,Rn,Rt);
}

void st1_single_structure_no_offset_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	memory_1_interpreter(ctx,0ULL,Q,1ULL,0ULL,0ULL,0ULL,((uint64_t)opcode << (uint64_t)1ULL),S,size,Rn,Rt,0ULL);
}

void st1_single_structure_post_index_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	memory_1_interpreter(ctx,1ULL,Q,1ULL,0ULL,Rm,0ULL,((uint64_t)opcode << (uint64_t)1ULL),S,size,Rn,Rt,0ULL);
}

void floating_point_conditional_select_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t Rd)
{
	uint64_t fltsize = get_flt_size_interpreter(ctx,ftype);
	uint128_t operand1 = V_interpreter(ctx,Rn);
	uint128_t operand2 = V_interpreter(ctx,Rm);
	uint128_t result = 0;
	if ((condition_holds_interpreter(ctx,cond)))
	{
		result = operand1;
	}
	else
	{
		result = operand2;
	}
	result = clear_vector_scalar_interpreter(ctx,result,fltsize);
	V_interpreter(ctx,Rd,result);
}

void fcmp_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t opc)
{
	uint64_t datasize = get_flt_size_interpreter(ctx,ftype);
	uint64_t cmp_with_zero = ((uint64_t)opc == (uint64_t)1ULL);
	uint64_t operand1 = V_interpreter(ctx,Rn);
	uint64_t operand2;
	uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
	if ((cmp_with_zero))
	{
		operand2 = 0ULL;
	}
	else
	{
		operand2 = V_interpreter(ctx,Rm);
	}
	uint64_t nzcv = FPCompare_interpreter(ctx,operand1,operand2,fpcr_state,datasize);
	_sys_interpreter(ctx,(uint64_t)nzcv_n,((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL));
	_sys_interpreter(ctx,(uint64_t)nzcv_z,((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL));
	_sys_interpreter(ctx,(uint64_t)nzcv_c,((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL));
	_sys_interpreter(ctx,(uint64_t)nzcv_v,((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL));
}

void fccmp_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t nzcv)
{
	uint64_t datasize = get_flt_size_interpreter(ctx,ftype);
	uint64_t operand1 = V_interpreter(ctx,Rn);
	uint64_t operand2 = V_interpreter(ctx,Rm);
	uint64_t fpcr_state = _sys_interpreter(ctx,(uint64_t)fpcr);
	if ((condition_holds_interpreter(ctx,cond)))
	{
		uint64_t success_nzcv = FPCompare_interpreter(ctx,operand1,operand2,fpcr_state,datasize);
		_sys_interpreter(ctx,(uint64_t)nzcv_n,((uint64_t)(((uint64_t)success_nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL));
		_sys_interpreter(ctx,(uint64_t)nzcv_z,((uint64_t)(((uint64_t)success_nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL));
		_sys_interpreter(ctx,(uint64_t)nzcv_c,((uint64_t)(((uint64_t)success_nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL));
		_sys_interpreter(ctx,(uint64_t)nzcv_v,((uint64_t)(((uint64_t)success_nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL));
	}
	else
	{
		_sys_interpreter(ctx,(uint64_t)nzcv_n,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL));
		_sys_interpreter(ctx,(uint64_t)nzcv_z,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL));
		_sys_interpreter(ctx,(uint64_t)nzcv_c,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL));
		_sys_interpreter(ctx,(uint64_t)nzcv_v,(uint64_t)((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL));
	}
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
	if ((((uint64_t)set_flags && (uint64_t)use_x86_jit(ctx))))
	{
		if ((is_add))
		{
			d = x86_add_set_flags_jit(ctx,O,n,m);
		}
		else
		{
			d = x86_subtract_set_flags_jit(ctx,O,n,m);
		}
		return d;
	}
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

uint64_t select_jit(ssa_emit_context* ctx, uint64_t condition, uint64_t yes, uint64_t no)
{
	if ((condition))
	return yes;
	return no;
}

ir_operand create_mask_jit(ssa_emit_context* ctx, uint64_t bits)
{
	if ((((uint64_t)bits >= (uint64_t)64ULL)))
	return ir_operand::create_con(-1ULL, int64);
	return ir_operand::create_con(((uint64_t)(((uint64_t)1ULL << (uint64_t)bits)) - (uint64_t)1ULL), int64);
}

ir_operand shift_left_check_jit(ssa_emit_context* ctx, ir_operand to_shift, ir_operand shift, uint64_t size)
{
	ir_operand result = ssa_emit_context::emit_ssa(ctx, ir_move, ir_operand::create_con(0ULL, int64));
	{
	        ir_operand end = ir_operation_block::create_label(ctx->ir);
	        ir_operand yes = ir_operation_block::create_label(ctx->ir);
	
	        ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_equal_signed, shift, ir_operand::create_con(size, int64));
	
	        ir_operation_block::jump_if(ctx->ir,yes, condition);
	    	{
			ssa_emit_context::move(ctx,result,ssa_emit_context::emit_ssa(ctx, ir_shift_left, to_shift, copy_new_raw_size(ctx, shift, int64)));
		}
	        
	        ir_operation_block::jump(ctx->ir,end);
	        ir_operation_block::mark_label(ctx->ir, yes);
	
	    	{
			ssa_emit_context::move(ctx,result,ir_operand::create_con(0ULL, int64));
		}
	
	        ir_operation_block::mark_label(ctx->ir, end);
	    }
	return result;
}

uint64_t get_x86_rounding_mode_jit(ssa_emit_context* ctx, uint64_t rounding)
{
	uint64_t rounding_control;
	if ((((uint64_t)rounding == (uint64_t)FPRounding_TIEEVEN)))
	{
		rounding_control = 0ULL;
	}
	else if ((((uint64_t)rounding == (uint64_t)FPRounding_NEGINF)))
	{
		rounding_control = 1ULL;
	}
	else if ((((uint64_t)rounding == (uint64_t)FPRounding_POSINF)))
	{
		rounding_control = 2ULL;
	}
	else if ((((uint64_t)rounding == (uint64_t)FPRounding_ZERO)))
	{
		rounding_control = 3ULL;
	}
	else
	{
		undefined_jit(ctx);
	}
	return rounding_control;
}

ir_operand shift_right_check_jit(ssa_emit_context* ctx, ir_operand to_shift, ir_operand shift, uint64_t size, uint64_t is_unsigned)
{
	ir_operand result = ssa_emit_context::emit_ssa(ctx, ir_move, ir_operand::create_con(0ULL, int64));
	{
	        ir_operand end = ir_operation_block::create_label(ctx->ir);
	        ir_operand yes = ir_operation_block::create_label(ctx->ir);
	
	        ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_equal_signed, shift, ir_operand::create_con(size, int64));
	
	        ir_operation_block::jump_if(ctx->ir,yes, condition);
	    	{
			if ((is_unsigned))
			{
				ssa_emit_context::move(ctx,result,ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, to_shift, copy_new_raw_size(ctx, shift, int64)));
			}
			else
			{
				ssa_emit_context::move(ctx,result,ssa_emit_context::emit_ssa(ctx, ir_shift_right_signed, to_shift, copy_new_raw_size(ctx, shift, int64)));
			}
		}
	        
	        ir_operation_block::jump(ctx->ir,end);
	        ir_operation_block::mark_label(ctx->ir, yes);
	
	    	{
			{
			        ir_operand end = ir_operation_block::create_label(ctx->ir);
			        ir_operand yes = ir_operation_block::create_label(ctx->ir);
			
			        ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, to_shift, ir_operand::create_con(0ULL, int64)), ir_operand::create_con(!is_unsigned, int64));
			
			        ir_operation_block::jump_if(ctx->ir,yes, condition);
			    	{
					ssa_emit_context::move(ctx,result,ir_operand::create_con(0ULL, int64));
				}
			        
			        ir_operation_block::jump(ctx->ir,end);
			        ir_operation_block::mark_label(ctx->ir, yes);
			
			    	{
					ssa_emit_context::move(ctx,result,ir_operand::create_con(-1ULL, int64));
				}
			
			        ir_operation_block::mark_label(ctx->ir, end);
			    }
		}
	
	        ir_operation_block::mark_label(ctx->ir, end);
	    }
	return result;
}

ir_operand reverse_jit(ssa_emit_context* ctx, ir_operand word, uint64_t M, uint64_t N)
{
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	uint64_t swsize = M;
	uint64_t sw = ((uint64_t)N / (uint64_t)swsize);
	for (uint64_t s = 0; s < (sw); s++)
	{
		ssa_emit_context::vector_insert(ctx,result, (((uint64_t)(((uint64_t)sw - (uint64_t)1ULL)) - (uint64_t)s)), swsize, ssa_emit_context::vector_extract(ctx,word, s, swsize));
	}
	return copy_new_raw_size(ctx, result, int64);
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

ir_operand VPart_jit(ssa_emit_context* ctx, uint64_t n, uint64_t part, uint64_t width)
{
	return ssa_emit_context::vector_extract(ctx,V_jit(ctx,n), part, width);
}

ir_operand get_from_concacted_vector_jit(ssa_emit_context* ctx, ir_operand top, ir_operand bottom, uint64_t index, uint64_t element_count, uint64_t element_size)
{
	ir_operand working = bottom;
	if ((((uint64_t)index >= (uint64_t)element_count)))
	{
		index = ((uint64_t)index - (uint64_t)element_count);
		working = top;
	}
	return ssa_emit_context::vector_extract(ctx,working, index, element_size);
}

ir_operand call_float_binary_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand fpcr, uint64_t N, uint64_t function)
{
	return call_jit(ctx,operand1,operand2,fpcr,ir_operand::create_con(N, int64),ir_operand::create_con(0ULL, int64),ir_operand::create_con(0ULL, int64),function);
}

ir_operand call_float_unary_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand fpcr, uint64_t N, uint64_t function)
{
	return call_jit(ctx,operand,fpcr,ir_operand::create_con(N, int64),ir_operand::create_con(0ULL, int64),ir_operand::create_con(0ULL, int64),ir_operand::create_con(0ULL, int64),function);
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

ir_operand replicate_vector_jit(ssa_emit_context* ctx, ir_operand source, uint64_t v_size, uint64_t count)
{
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t e = 0; e < (count); e++)
	{
		ssa_emit_context::vector_insert(ctx,result, e, v_size, ssa_emit_context::vector_extract(ctx,source, 0ULL, 64ULL));
	}
	return result;
}

void st_jit(ssa_emit_context* ctx, uint64_t wback, uint64_t Q, uint64_t L, uint64_t opcode, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rt)
{
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t ebytes = ((uint64_t)esize / (uint64_t)8ULL);
	uint64_t rpt = 1ULL;
	uint64_t selem = 2ULL;
	ir_operand address = XSP_jit(ctx,Rn);
	uint64_t offs = 0ULL;
	uint64_t t = Rt;
	uint64_t m = Rm;
	uint64_t O = (((uint64_t)ebytes * (uint64_t)8ULL)) == 8ULL ? int8 : (((uint64_t)ebytes * (uint64_t)8ULL)) == 16ULL ? int16 : (((uint64_t)ebytes * (uint64_t)8ULL)) == 32ULL ? int32 : (((uint64_t)ebytes * (uint64_t)8ULL)) == 64ULL ? int64 : (((uint64_t)ebytes * (uint64_t)8ULL)) == 128ULL ? int128 : 0;
	{
		for (uint64_t r = 0; r < (rpt); r++)
		{
			for (uint64_t e = 0; e < (elements); e++)
			{
				uint64_t tt = ((uint64_t)(((uint64_t)t + (uint64_t)r)) % (uint64_t)32ULL);
				for (uint64_t s = 0; s < (selem); s++)
				{
					ir_operand rval = V_jit(ctx,tt);
					ir_operand eaddr = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offs, int64));
					mem_jit(ctx,O,eaddr,copy_new_raw_size(ctx, ssa_emit_context::vector_extract(ctx,rval, e, esize), O));
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					tt = ((uint64_t)(((uint64_t)tt + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
		}
		if ((wback))
		{
			ir_operand _offs = ir_operand::create_con(offs, int64);
			if ((((uint64_t)Rm != (uint64_t)31ULL)))
			{
				_offs = X_jit(ctx,Rm);
			}
			address = ssa_emit_context::emit_ssa(ctx, ir_add, address, _offs);
			XSP_jit(ctx,Rn,address);
		}
	}
}

void memory_1_jit(ssa_emit_context* ctx, uint64_t wback, uint64_t Q, uint64_t L, uint64_t R, uint64_t Rm, uint64_t o2, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt, uint64_t is_load)
{
	uint64_t scale = bits_c_jit(ctx,opcode,2ULL,1ULL);
	uint64_t selem = ((uint64_t)(((uint64_t)(((uint64_t)bit_c_jit(ctx,opcode,0ULL) << (uint64_t)1ULL)) | (uint64_t)R)) + (uint64_t)1ULL);
	uint64_t replicate = 0ULL;
	uint64_t index;
	if ((((uint64_t)scale == (uint64_t)3ULL)))
	{
		scale = size;
		replicate = 1ULL;
	}
	else if ((((uint64_t)scale == (uint64_t)0ULL)))
	{
		index = Q;
		index = ((uint64_t)(((uint64_t)index << (uint64_t)1ULL)) | (uint64_t)S);
		index = ((uint64_t)(((uint64_t)index << (uint64_t)2ULL)) | (uint64_t)size);
	}
	else if ((((uint64_t)scale == (uint64_t)1ULL)))
	{
		index = Q;
		index = ((uint64_t)(((uint64_t)index << (uint64_t)1ULL)) | (uint64_t)S);
		index = ((uint64_t)(((uint64_t)index << (uint64_t)1ULL)) | (uint64_t)bit_c_jit(ctx,size,1ULL));
	}
	else if ((((uint64_t)scale == (uint64_t)2ULL)))
	{
		if ((((uint64_t)(((uint64_t)size & (uint64_t)1ULL)) == (uint64_t)0ULL)))
		{
			index = ((uint64_t)(((uint64_t)Q << (uint64_t)1ULL)) | (uint64_t)S);
		}
		else
		{
			index = Q;
			scale = 3ULL;
		}
	}
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)scale);
	uint64_t ebytes = ((uint64_t)esize / (uint64_t)8ULL);
	ir_operand address = XSP_jit(ctx,Rn);
	uint64_t offs = 0ULL;
	uint64_t O = (((uint64_t)ebytes * (uint64_t)8ULL)) == 8ULL ? int8 : (((uint64_t)ebytes * (uint64_t)8ULL)) == 16ULL ? int16 : (((uint64_t)ebytes * (uint64_t)8ULL)) == 32ULL ? int32 : (((uint64_t)ebytes * (uint64_t)8ULL)) == 64ULL ? int64 : (((uint64_t)ebytes * (uint64_t)8ULL)) == 128ULL ? int128 : 0;
	{
		uint64_t t = Rt;
		if ((replicate))
		{
			for (uint64_t s = 0; s < (selem); s++)
			{
				ir_operand eaddr = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offs, int64));
				ir_operand element = copy_new_raw_size(ctx, mem_jit(ctx,O,eaddr), O);
				V_jit(ctx,t,replicate_vector_jit(ctx,copy_new_raw_size(ctx, element, int128),esize,((uint64_t)datasize / (uint64_t)esize)));
				offs = ((uint64_t)offs + (uint64_t)ebytes);
				t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
			}
		}
		else
		{
			if ((is_load))
			{
				for (uint64_t s = 0; s < (selem); s++)
				{
					ir_operand rval = V_jit(ctx,t);
					ir_operand eaddr = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offs, int64));
					ssa_emit_context::vector_insert(ctx,rval, index, esize, mem_jit(ctx,O,eaddr));
					V_jit(ctx,t,rval);
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
			else
			{
				for (uint64_t s = 0; s < (selem); s++)
				{
					ir_operand rval = V_jit(ctx,t);
					ir_operand eaddr = ssa_emit_context::emit_ssa(ctx, ir_add, address, ir_operand::create_con(offs, int64));
					mem_jit(ctx,O,eaddr,copy_new_raw_size(ctx, ssa_emit_context::vector_extract(ctx,rval, index, esize), O));
					offs = ((uint64_t)offs + (uint64_t)ebytes);
					t = ((uint64_t)(((uint64_t)t + (uint64_t)1ULL)) % (uint64_t)32ULL);
				}
			}
		}
		if ((wback))
		{
			ir_operand _offs = ir_operand::create_con(offs, int64);
			if ((((uint64_t)Rm != (uint64_t)31ULL)))
			{
				_offs = X_jit(ctx,Rm);
			}
			address = ssa_emit_context::emit_ssa(ctx, ir_add, address, _offs);
			XSP_jit(ctx,Rn,address);
		}
	}
}

ir_operand bits_r_jit(ssa_emit_context* ctx, ir_operand operand, uint64_t top, uint64_t bottom)
{
	top = ((uint64_t)top + (uint64_t)1ULL);
	uint64_t size = ((uint64_t)top - (uint64_t)bottom);
	uint64_t mask = ((uint64_t)(((uint64_t)1ULL << (uint64_t)size)) - (uint64_t)1ULL);
	return ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, operand, ir_operand::create_con(bottom, int64)), ir_operand::create_con(mask, int64));
}

ir_operand infinity_jit(ssa_emit_context* ctx, uint64_t sign, uint64_t N)
{
	ir_operand result = ir_operand::create_con(((uint64_t)sign << (uint64_t)(((uint64_t)N - (uint64_t)1ULL))), int64);
	if ((((uint64_t)N == (uint64_t)32ULL)))
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, result, ir_operand::create_con((((uint64_t)255ULL << (uint64_t)23ULL)), int64));
	}
	else if ((((uint64_t)N == (uint64_t)64ULL)))
	{
		result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, result, ir_operand::create_con((((uint64_t)2047ULL << (uint64_t)52ULL)), int64));
	}
	else
	{
		undefined_jit(ctx);
	}
	return result;
}

ir_operand float_is_nan_jit(ssa_emit_context* ctx, ir_operand operand, uint64_t N)
{
	ir_operand result = ssa_emit_context::emit_ssa(ctx, ir_move, ir_operand::create_con(0ULL, int8));
	ir_operand exp;
	ir_operand frac;
	if ((((uint64_t)N == (uint64_t)32ULL)))
	{
		exp = bits_r_jit(ctx,operand,30ULL,23ULL);
		frac = bits_r_jit(ctx,operand,22ULL,0ULL);
		{
		        ir_operand end = ir_operation_block::create_label(ctx->ir);
		        ir_operand yes = ir_operation_block::create_label(ctx->ir);
		
		        ir_operand condition = ssa_emit_context::emit_ssa(ctx,ir_bitwise_exclusive_or,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, exp, ir_operand::create_con(255ULL, int64)), ssa_emit_context::emit_ssa(ctx, ir_compare_not_equal, frac, ir_operand::create_con(0ULL, int64))), ir_operand::create_con(1));
		
		        ir_operation_block::jump_if(ctx->ir,end, condition);
		
		    	{
				ssa_emit_context::move(ctx,result,ir_operand::create_con(1ULL, int8));
			}
		
		        ir_operation_block::mark_label(ctx->ir, end);
		    }
	}
	else if ((((uint64_t)N == (uint64_t)64ULL)))
	{
		exp = bits_r_jit(ctx,operand,62ULL,52ULL);
		frac = bits_r_jit(ctx,operand,51ULL,0ULL);
		{
		        ir_operand end = ir_operation_block::create_label(ctx->ir);
		        ir_operand yes = ir_operation_block::create_label(ctx->ir);
		
		        ir_operand condition = ssa_emit_context::emit_ssa(ctx,ir_bitwise_exclusive_or,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, exp, ir_operand::create_con(2047ULL, int64)), ssa_emit_context::emit_ssa(ctx, ir_compare_not_equal, frac, ir_operand::create_con(0ULL, int64))), ir_operand::create_con(1));
		
		        ir_operation_block::jump_if(ctx->ir,end, condition);
		
		    	{
				ssa_emit_context::move(ctx,result,ir_operand::create_con(1ULL, int8));
			}
		
		        ir_operation_block::mark_label(ctx->ir, end);
		    }
	}
	else
	{
		undefined_jit(ctx);
	}
	return copy_new_raw_size(ctx, result, int64);
}

ir_operand float_imm_jit(ssa_emit_context* ctx, ir_operand source, uint64_t N)
{
	if ((((uint64_t)N == (uint64_t)32ULL)))
	{
		return copy_new_raw_size(ctx, ssa_emit_context::convert_to_float(ctx,source,int32,int64, 0), int64);
	}
	else if ((((uint64_t)N == (uint64_t)64ULL)))
	{
		return ssa_emit_context::convert_to_float(ctx,source,int64,int64, 0);
	}
	else
	{
		undefined_jit(ctx);
	}
}

ir_operand create_fixed_from_fbits_jit(ssa_emit_context* ctx,uint64_t F, uint64_t fbits, uint64_t N)
{
	uint64_t working;
	if ((((uint64_t)fbits == (uint64_t)64ULL)))
	{
		return ssa_emit_context::emit_ssa(ctx, ir_floating_point_multiply, copy_new_raw_size(ctx, float_imm_jit(ctx,ir_operand::create_con(((uint64_t)1ULL << (uint64_t)63ULL), int64),N), F), copy_new_raw_size(ctx, float_imm_jit(ctx,ir_operand::create_con(2ULL, int64),N), F));
	}
	else if ((((uint64_t)fbits > (uint64_t)64ULL)))
	{
		undefined_jit(ctx);
	}
	return copy_new_raw_size(ctx, float_imm_jit(ctx,ir_operand::create_con(((uint64_t)1ULL << (uint64_t)fbits), int64),N), F);
}

ir_operand FPAdd_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand o1 = copy_new_raw_size(ctx, operand1, F);
			ir_operand o2 = copy_new_raw_size(ctx, operand2, F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_add, o1, o2), int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPAdd_I);
}

ir_operand FPSub_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand o1 = copy_new_raw_size(ctx, operand1, F);
			ir_operand o2 = copy_new_raw_size(ctx, operand2, F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_subtract, o1, o2), int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPSub_I);
}

ir_operand FPMul_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand o1 = copy_new_raw_size(ctx, operand1, F);
			ir_operand o2 = copy_new_raw_size(ctx, operand2, F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_multiply, o1, o2), int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPMul_I);
}

ir_operand FPNMul_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	ir_operand result = FPMul_jit(ctx,operand1,operand2,FPCR,N);
	return FPNeg_jit(ctx,result,FPCR,N);
}

ir_operand FPDiv_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand o1 = copy_new_raw_size(ctx, operand1, F);
			ir_operand o2 = copy_new_raw_size(ctx, operand2, F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_divide, o1, o2), int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPDiv_I);
}

ir_operand FPMax_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand o1 = copy_new_raw_size(ctx, operand1, F);
			ir_operand o2 = copy_new_raw_size(ctx, operand2, F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_select_max, o1, o2), int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPMax_I);
}

ir_operand FPMin_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand o1 = copy_new_raw_size(ctx, operand1, F);
			ir_operand o2 = copy_new_raw_size(ctx, operand2, F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_select_min, o1, o2), int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPMin_I);
}

ir_operand FPMaxNum_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		ir_operand type_1_nan = float_is_nan_jit(ctx,operand1,N);
		ir_operand type_2_nan = float_is_nan_jit(ctx,operand2,N);
		{
		        ir_operand end = ir_operation_block::create_label(ctx->ir);
		        ir_operand yes = ir_operation_block::create_label(ctx->ir);
		
		        ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, type_1_nan, ssa_emit_context::emit_ssa(ctx, ir_logical_not, type_2_nan));
		
		        ir_operation_block::jump_if(ctx->ir,yes, condition);
		    	{
			        ir_operand end = ir_operation_block::create_label(ctx->ir);
			        ir_operand yes = ir_operation_block::create_label(ctx->ir);
			
			        ir_operand condition = ssa_emit_context::emit_ssa(ctx,ir_bitwise_exclusive_or,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_logical_not, type_1_nan), type_2_nan), ir_operand::create_con(1));
			
			        ir_operation_block::jump_if(ctx->ir,end, condition);
			
			    	{
					ssa_emit_context::move(ctx,operand2,infinity_jit(ctx,1ULL,N));
				}
			
			        ir_operation_block::mark_label(ctx->ir, end);
			    }
		        
		        ir_operation_block::jump(ctx->ir,end);
		        ir_operation_block::mark_label(ctx->ir, yes);
		
		    	{
				ssa_emit_context::move(ctx,operand1,infinity_jit(ctx,1ULL,N));
			}
		
		        ir_operation_block::mark_label(ctx->ir, end);
		    }
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_select_max, copy_new_raw_size(ctx, operand1, F), copy_new_raw_size(ctx, operand2, F)), int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPMaxNum_I);
}

ir_operand FPMinNum_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		ir_operand type_1_nan = float_is_nan_jit(ctx,operand1,N);
		ir_operand type_2_nan = float_is_nan_jit(ctx,operand2,N);
		{
		        ir_operand end = ir_operation_block::create_label(ctx->ir);
		        ir_operand yes = ir_operation_block::create_label(ctx->ir);
		
		        ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, type_1_nan, ssa_emit_context::emit_ssa(ctx, ir_logical_not, type_2_nan));
		
		        ir_operation_block::jump_if(ctx->ir,yes, condition);
		    	{
			        ir_operand end = ir_operation_block::create_label(ctx->ir);
			        ir_operand yes = ir_operation_block::create_label(ctx->ir);
			
			        ir_operand condition = ssa_emit_context::emit_ssa(ctx,ir_bitwise_exclusive_or,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_logical_not, type_1_nan), type_2_nan), ir_operand::create_con(1));
			
			        ir_operation_block::jump_if(ctx->ir,end, condition);
			
			    	{
					ssa_emit_context::move(ctx,operand2,infinity_jit(ctx,0ULL,N));
				}
			
			        ir_operation_block::mark_label(ctx->ir, end);
			    }
		        
		        ir_operation_block::jump(ctx->ir,end);
		        ir_operation_block::mark_label(ctx->ir, yes);
		
		    	{
				ssa_emit_context::move(ctx,operand1,infinity_jit(ctx,0ULL,N));
			}
		
		        ir_operation_block::mark_label(ctx->ir, end);
		    }
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_select_min, copy_new_raw_size(ctx, operand1, F), copy_new_raw_size(ctx, operand2, F)), int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPMinNum_I);
}

ir_operand FPCompare_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand o1 = copy_new_raw_size(ctx, operand1, F);
			ir_operand o2 = copy_new_raw_size(ctx, operand2, F);
			ir_operand result = ssa_emit_context::emit_ssa(ctx, ir_move, ir_operand::create_con(0ULL, F));
			ir_operand type_1_nan = float_is_nan_jit(ctx,copy_new_raw_size(ctx, o1, int64),N);
			ir_operand type_2_nan = float_is_nan_jit(ctx,copy_new_raw_size(ctx, o2, int64),N);
			{
			        ir_operand end = ir_operation_block::create_label(ctx->ir);
			        ir_operand yes = ir_operation_block::create_label(ctx->ir);
			
			        ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, type_1_nan, type_2_nan);
			
			        ir_operation_block::jump_if(ctx->ir,yes, condition);
			    	{
					{
					        ir_operand end = ir_operation_block::create_label(ctx->ir);
					        ir_operand yes = ir_operation_block::create_label(ctx->ir);
					
					        ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_floating_point_compare_equal, o1, o2);
					
					        ir_operation_block::jump_if(ctx->ir,yes, condition);
					    	{
						        ir_operand end = ir_operation_block::create_label(ctx->ir);
						        ir_operand yes = ir_operation_block::create_label(ctx->ir);
						
						        ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_floating_point_compare_less, o1, o2);
						
						        ir_operation_block::jump_if(ctx->ir,yes, condition);
						    	{
								ssa_emit_context::move(ctx,result,ir_operand::create_con(2ULL, F));
							}
						        
						        ir_operation_block::jump(ctx->ir,end);
						        ir_operation_block::mark_label(ctx->ir, yes);
						
						    	{
								ssa_emit_context::move(ctx,result,ir_operand::create_con(8ULL, F));
							}
						
						        ir_operation_block::mark_label(ctx->ir, end);
						    }
					        
					        ir_operation_block::jump(ctx->ir,end);
					        ir_operation_block::mark_label(ctx->ir, yes);
					
					    	{
							ssa_emit_context::move(ctx,result,ir_operand::create_con(6ULL, F));
						}
					
					        ir_operation_block::mark_label(ctx->ir, end);
					    }
				}
			        
			        ir_operation_block::jump(ctx->ir,end);
			        ir_operation_block::mark_label(ctx->ir, yes);
			
			    	{
					ssa_emit_context::move(ctx,result,ir_operand::create_con(3ULL, F));
				}
			
			        ir_operation_block::mark_label(ctx->ir, end);
			    }
			return copy_new_raw_size(ctx, result, int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPCompare_I);
}

ir_operand FPRSqrtStepFused_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand o1 = copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, operand1, ir_operand::create_con((((uint64_t)1ULL << (uint64_t)(((uint64_t)N - (uint64_t)1ULL)))), int64)), F);
			ir_operand o2 = copy_new_raw_size(ctx, operand2, F);
			ir_operand three = copy_new_raw_size(ctx, float_imm_jit(ctx,ir_operand::create_con(3ULL, int64),N), F);
			ir_operand two = copy_new_raw_size(ctx, float_imm_jit(ctx,ir_operand::create_con(2ULL, int64),N), F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_divide, ssa_emit_context::emit_ssa(ctx, ir_floating_point_add, three, ssa_emit_context::emit_ssa(ctx, ir_floating_point_multiply, o1, o2)), two), int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPRSqrtStepFused_I);
}

ir_operand FPRecipStepFused_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand o1 = copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, operand1, ir_operand::create_con((((uint64_t)1ULL << (uint64_t)(((uint64_t)N - (uint64_t)1ULL)))), int64)), F);
			ir_operand o2 = copy_new_raw_size(ctx, operand2, F);
			ir_operand two = copy_new_raw_size(ctx, float_imm_jit(ctx,ir_operand::create_con(2ULL, int64),N), F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_add, two, ssa_emit_context::emit_ssa(ctx, ir_floating_point_multiply, o1, o2)), int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPRecipStepFused_I);
}

ir_operand FPCompareEQ_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand o1 = copy_new_raw_size(ctx, operand1, F);
			ir_operand o2 = copy_new_raw_size(ctx, operand2, F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_compare_equal, o1, o2), int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPCompareEQ_I);
}

ir_operand FPCompareGT_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand o1 = copy_new_raw_size(ctx, operand1, F);
			ir_operand o2 = copy_new_raw_size(ctx, operand2, F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_compare_greater, o1, o2), int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPCompareGT_I);
}

ir_operand FPCompareGE_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand o1 = copy_new_raw_size(ctx, operand1, F);
			ir_operand o2 = copy_new_raw_size(ctx, operand2, F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_compare_greater_equal, o1, o2), int64);
		}
	}
	return call_float_binary_jit(ctx,operand1,operand2,FPCR,N,(uint64_t)FPCompareGE_I);
}

ir_operand FPSqrt_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_square_root, copy_new_raw_size(ctx, operand, F)), int64);
		}
	}
	return call_float_unary_jit(ctx,operand,FPCR,N,(uint64_t)FPSqrt_I);
}

ir_operand FPNeg_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N)
{
	if ((use_fast_float_jit(ctx)))
	{
		return ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, operand, ir_operand::create_con((((uint64_t)1ULL << (uint64_t)(((uint64_t)N - (uint64_t)1ULL)))), int64));
	}
	return call_float_unary_jit(ctx,operand,FPCR,N,(uint64_t)FPNeg_I);
}

ir_operand FPAbs_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		ir_operand mask = ir_operand::create_con(((uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)N - (uint64_t)1ULL)))) - (uint64_t)1ULL), int64);
		return ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, operand, mask);
	}
	return call_float_unary_jit(ctx,operand,FPCR,N,(uint64_t)FPAbs_I);
}

ir_operand FPRSqrtEstimate_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand one = copy_new_raw_size(ctx, float_imm_jit(ctx,ir_operand::create_con(1ULL, int64),N), F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_divide, one, ssa_emit_context::emit_ssa(ctx, ir_floating_point_square_root, copy_new_raw_size(ctx, operand, F))), int64);
		}
	}
	return call_float_unary_jit(ctx,operand,FPCR,N,(uint64_t)FPRSqrtEstimate_I);
}

ir_operand FPRecipEstimate_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand one = copy_new_raw_size(ctx, float_imm_jit(ctx,ir_operand::create_con(1ULL, int64),N), F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_divide, one, copy_new_raw_size(ctx, operand, F)), int64);
		}
	}
	return call_float_unary_jit(ctx,operand,FPCR,N,(uint64_t)FPRecipEstimate_I);
}

ir_operand FixedToFP_jit(ssa_emit_context* ctx, ir_operand source, uint64_t fracbits, uint64_t is_unsigned, uint64_t to, uint64_t from)
{
	if ((((uint64_t)((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)to != (uint64_t)16ULL)) && (uint64_t)((uint64_t)from != (uint64_t)16ULL))))
	{
		uint64_t F = to == 32ULL ? int32 : to == 64ULL ? int64 : 0;
		{
			uint64_t I = from == 32ULL ? int32 : from == 64ULL ? int64 : 0;
			{
				if ((((uint64_t)from == (uint64_t)32ULL)))
				{
					source = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, source, ir_operand::create_con(4294967295ULL, int64));
				}
				ir_operand power = copy_new_raw_size(ctx, create_fixed_from_fbits_jit(ctx,F,fracbits,to), F);
				ir_operand working_result;
				if ((is_unsigned))
				{
					working_result = ssa_emit_context::convert_to_float(ctx,copy_new_raw_size(ctx, source, I),F,I, 0);
				}
				else
				{
					working_result = ssa_emit_context::convert_to_float(ctx,copy_new_raw_size(ctx, source, I),F,I, 1);
				}
				if ((((uint64_t)fracbits == (uint64_t)0ULL)))
				{
					return copy_new_raw_size(ctx, working_result, int64);
				}
				return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_divide, working_result, power), int64);
			}
		}
	}
	return call_jit(ctx,source,ir_operand::create_con(fracbits, int64),ir_operand::create_con(is_unsigned, int64),ir_operand::create_con(to, int64),ir_operand::create_con(from, int64),ir_operand::create_con(0ULL, int64),(uint64_t)FixedToFP_I);
}

ir_operand FPToFixed_jit(ssa_emit_context* ctx, ir_operand source, uint64_t fracbits, uint64_t is_unsigned, uint64_t round, uint64_t to, uint64_t from)
{
	if ((((uint64_t)((uint64_t)((uint64_t)((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)to != (uint64_t)16ULL)) && (uint64_t)((uint64_t)from != (uint64_t)16ULL)) && (uint64_t)((uint64_t)round != (uint64_t)FPRounding_ODD)) && (uint64_t)use_x86_sse41_jit(ctx))))
	{
		uint64_t F = from == 32ULL ? int32 : from == 64ULL ? int64 : 0;
		{
			uint64_t I = to == 32ULL ? int32 : to == 64ULL ? int64 : 0;
			{
				ir_operand max_i;
				ir_operand min_i;
				ir_operand max;
				ir_operand min;
				if ((is_unsigned))
				{
					min = ir_operand::create_con(0ULL, F);
					max = ssa_emit_context::convert_to_float(ctx,create_mask_jit(ctx,to),F,int64, 0);
					min_i = ir_operand::create_con(0ULL, int64);
					max_i = create_mask_jit(ctx,to);
				}
				else
				{
					min = ssa_emit_context::convert_to_float(ctx,ir_operand::create_con((((uint64_t)1ULL << (uint64_t)(((uint64_t)to - (uint64_t)1ULL)))), I),F,I, 1);
					max = ssa_emit_context::convert_to_float(ctx,copy_new_raw_size(ctx, create_mask_jit(ctx,((uint64_t)to - (uint64_t)1ULL)), I),F,I, 1);
					min_i = copy_new_raw_size(ctx, ir_operand::create_con((((uint64_t)1ULL << (uint64_t)(((uint64_t)to - (uint64_t)1ULL)))), I), int64);
					max_i = copy_new_raw_size(ctx, copy_new_raw_size(ctx, create_mask_jit(ctx,((uint64_t)to - (uint64_t)1ULL)), I), int64);
				}
				ir_operand working = ssa_emit_context::emit_ssa(ctx, ir_move, copy_new_raw_size(ctx, source, F));
				if ((((uint64_t)fracbits != (uint64_t)0ULL)))
				{
					ir_operand power = copy_new_raw_size(ctx, create_fixed_from_fbits_jit(ctx,F,fracbits,from), F);
					working = ssa_emit_context::emit_ssa(ctx, ir_floating_point_multiply, power, working);
				}
				uint64_t rounding_control;
				if ((((uint64_t)round == (uint64_t)FPRounding_TIEEVEN)))
				{
					rounding_control = 0ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_NEGINF)))
				{
					rounding_control = 1ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_POSINF)))
				{
					rounding_control = 2ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_ZERO)))
				{
					rounding_control = 3ULL;
				}
				else if ((((uint64_t)round == (uint64_t)FPRounding_TIEAWAY)))
				{
					rounding_control = 2ULL;
					{
					        ir_operand end = ir_operation_block::create_label(ctx->ir);
					        ir_operand yes = ir_operation_block::create_label(ctx->ir);
					
					        ir_operand condition = ssa_emit_context::emit_ssa(ctx,ir_bitwise_exclusive_or,ssa_emit_context::emit_ssa(ctx, ir_floating_point_compare_less_equal, working, ir_operand::create_con(0ULL, F)), ir_operand::create_con(1));
					
					        ir_operation_block::jump_if(ctx->ir,end, condition);
					
					    	{
							ssa_emit_context::move(ctx,working,ssa_emit_context::emit_ssa(ctx, ir_floating_point_subtract, working, copy_new_raw_size(ctx, float_imm_jit(ctx,ir_operand::create_con(1ULL, int64),from), F)));
						}
					
					        ir_operation_block::mark_label(ctx->ir, end);
					    }
				}
				else
				{
					undefined_jit(ctx);
				}
				if ((((uint64_t)from == (uint64_t)32ULL)))
				{
					working = copy_new_raw_size(ctx, intrinsic_binary_imm_jit(ctx,int128,(uint64_t)x86_roundss,copy_new_raw_size(ctx, working, int128),rounding_control), F);
				}
				else
				{
					working = copy_new_raw_size(ctx, intrinsic_binary_imm_jit(ctx,int128,(uint64_t)x86_roundsd,copy_new_raw_size(ctx, working, int128),rounding_control), F);
				}
				ir_operand result = ssa_emit_context::emit_ssa(ctx, ir_move, ir_operand::create_con(0ULL, I));
				{
				        ir_operand end = ir_operation_block::create_label(ctx->ir);
				        ir_operand yes = ir_operation_block::create_label(ctx->ir);
				
				        ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_floating_point_compare_greater_equal, working, max);
				
				        ir_operation_block::jump_if(ctx->ir,yes, condition);
				    	{
					        ir_operand end = ir_operation_block::create_label(ctx->ir);
					        ir_operand yes = ir_operation_block::create_label(ctx->ir);
					
					        ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_floating_point_compare_less_equal, working, min);
					
					        ir_operation_block::jump_if(ctx->ir,yes, condition);
					    	if ((is_unsigned))
						{
							ir_operand s_max_i = copy_new_raw_size(ctx, create_mask_jit(ctx,((uint64_t)to - (uint64_t)1ULL)), I);
							ir_operand s_max = ssa_emit_context::convert_to_float(ctx,s_max_i,F,I, 1);
							{
							        ir_operand end = ir_operation_block::create_label(ctx->ir);
							        ir_operand yes = ir_operation_block::create_label(ctx->ir);
							
							        ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_signed, working, s_max);
							
							        ir_operation_block::jump_if(ctx->ir,yes, condition);
							    	{
									ssa_emit_context::move(ctx,result,copy_new_raw_size(ctx, ssa_emit_context::convert_to_integer(ctx,working,I,F, 1), I));
								}
							        
							        ir_operation_block::jump(ctx->ir,end);
							        ir_operation_block::mark_label(ctx->ir, yes);
							
							    	{
									ir_operand difference = ssa_emit_context::emit_ssa(ctx, ir_floating_point_subtract, working, s_max);
									ssa_emit_context::move(ctx,working,ssa_emit_context::emit_ssa(ctx, ir_floating_point_subtract, working, difference));
									ssa_emit_context::move(ctx,result,copy_new_raw_size(ctx, ssa_emit_context::convert_to_integer(ctx,difference,I,F, 1), I));
									ssa_emit_context::move(ctx,result,ssa_emit_context::emit_ssa(ctx, ir_add, result, copy_new_raw_size(ctx, ssa_emit_context::convert_to_integer(ctx,working,I,F, 1), I)));
								}
							
							        ir_operation_block::mark_label(ctx->ir, end);
							    }
						}
						else
						{
							ssa_emit_context::move(ctx,result,copy_new_raw_size(ctx, ssa_emit_context::convert_to_integer(ctx,working,I,F, 1), I));
						}
					        
					        ir_operation_block::jump(ctx->ir,end);
					        ir_operation_block::mark_label(ctx->ir, yes);
					
					    	{
							ssa_emit_context::move(ctx,result,copy_new_raw_size(ctx, min_i, I));
						}
					
					        ir_operation_block::mark_label(ctx->ir, end);
					    }
				        
				        ir_operation_block::jump(ctx->ir,end);
				        ir_operation_block::mark_label(ctx->ir, yes);
				
				    	{
						ssa_emit_context::move(ctx,result,copy_new_raw_size(ctx, max_i, I));
					}
				
				        ir_operation_block::mark_label(ctx->ir, end);
				    }
				return copy_new_raw_size(ctx, result, int64);
			}
		}
	}
	return call_jit(ctx,source,ir_operand::create_con(fracbits, int64),ir_operand::create_con(is_unsigned, int64),ir_operand::create_con(round, int64),ir_operand::create_con(to, int64),ir_operand::create_con(from, int64),(uint64_t)FPToFixed_I);
}

ir_operand FPConvert_jit(ssa_emit_context* ctx, ir_operand source, uint64_t to, uint64_t from)
{
	if ((((uint64_t)to == (uint64_t)from)))
	{
		undefined_jit(ctx);
	}
	if ((((uint64_t)((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)to != (uint64_t)16ULL)) && (uint64_t)((uint64_t)from != (uint64_t)16ULL))))
	{
		ir_operand result;
		if ((((uint64_t)((uint64_t)to == (uint64_t)32ULL) && (uint64_t)((uint64_t)from == (uint64_t)64ULL))))
		{
			result = copy_new_raw_size(ctx, intrinsic_unary_jit(ctx,int128,(uint64_t)x86_cvtsd2ss,copy_new_raw_size(ctx, source, int128)), int64);
		}
		else
		{
			result = copy_new_raw_size(ctx, intrinsic_unary_jit(ctx,int128,(uint64_t)x86_cvtss2sd,copy_new_raw_size(ctx, source, int128)), int64);
		}
		return result;
	}
	return call_jit(ctx,source,ir_operand::create_con(0ULL, int64),ir_operand::create_con(0ULL, int64),ir_operand::create_con(to, int64),ir_operand::create_con(from, int64),ir_operand::create_con(0ULL, int64),(uint64_t)FPConvert_I);
}

ir_operand FPRoundInt_jit(ssa_emit_context* ctx, ir_operand source, ir_operand fpcr, uint64_t rounding, uint64_t N)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)((uint64_t)N != (uint64_t)16ULL))))
	{
	}
	return call_jit(ctx,source,fpcr,ir_operand::create_con(rounding, int64),ir_operand::create_con(N, int64),ir_operand::create_con(0ULL, int64),ir_operand::create_con(0ULL, int64),(uint64_t)FPRoundInt_I);
}

ir_operand FPMulAdd_jit(ssa_emit_context* ctx, ir_operand addend, ir_operand element1, ir_operand element2, ir_operand fpcr, uint64_t N)
{
	if ((use_fast_float_jit(ctx)))
	{
		uint64_t F = N == 32ULL ? int32 : N == 64ULL ? int64 : 0;
		{
			ir_operand o3 = copy_new_raw_size(ctx, addend, F);
			ir_operand o1 = copy_new_raw_size(ctx, element1, F);
			ir_operand o2 = copy_new_raw_size(ctx, element2, F);
			return copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_floating_point_add, o3, ssa_emit_context::emit_ssa(ctx, ir_floating_point_multiply, o1, o2)), int64);
		}
	}
	return call_jit(ctx,addend,element1,element2,fpcr,ir_operand::create_con(N, int64),ir_operand::create_con(0ULL, int64),(uint64_t)FPMulAdd_I);
}

void float_unary_scalar_jit(ssa_emit_context* ctx, uint64_t Rd, uint64_t Rn, uint64_t fsize, uint64_t float_function)
{
	ir_operand operand = copy_new_raw_size(ctx, V_jit(ctx,Rn), int64);
	uint64_t N = get_flt_size_jit(ctx,fsize);
	ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
	ir_operand element_result = ((ir_operand(*)(void*,ir_operand,ir_operand,uint64_t))float_function)(ctx,operand,fpcr_state,N);
	ir_operand vector_result = ssa_emit_context::vector_zero(ctx);
	ssa_emit_context::vector_insert(ctx,vector_result, 0ULL, N, element_result);
	V_jit(ctx,Rd,vector_result);
}

void float_unary_vector_jit(ssa_emit_context* ctx, uint64_t Rd, uint64_t Rn, uint64_t Q, uint64_t sz, uint64_t float_function)
{
	ir_operand operand = V_jit(ctx,Rn);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ir_operand working = ssa_emit_context::vector_extract(ctx,operand, e, esize);
		ir_operand element_result = ((ir_operand(*)(void*,ir_operand,ir_operand,uint64_t))float_function)(ctx,working,fpcr_state,esize);
		ssa_emit_context::vector_insert(ctx,result, e, esize, element_result);
	}
	V_jit(ctx,Rd,result);
}

void float_binary_scalar_jit(ssa_emit_context* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t fsize, uint64_t float_function)
{
	ir_operand operand1 = copy_new_raw_size(ctx, V_jit(ctx,Rn), int64);
	ir_operand operand2 = copy_new_raw_size(ctx, V_jit(ctx,Rm), int64);
	uint64_t N = get_flt_size_jit(ctx,fsize);
	ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
	ir_operand element_result = ((ir_operand(*)(void*,ir_operand,ir_operand,ir_operand,uint64_t))float_function)(ctx,operand1,operand2,fpcr_state,N);
	ir_operand vector_result = ssa_emit_context::vector_zero(ctx);
	ssa_emit_context::vector_insert(ctx,vector_result, 0ULL, N, element_result);
	V_jit(ctx,Rd,vector_result);
}

void float_binary_vector_jit(ssa_emit_context* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t Q, uint64_t sz, uint64_t float_function)
{
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
		ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, e, esize);
		ssa_emit_context::vector_insert(ctx,result, e, esize, ((ir_operand(*)(void*,ir_operand,ir_operand,ir_operand,uint64_t))float_function)(ctx,element1,element2,fpcr_state,esize));
	}
	V_jit(ctx,Rd,result);
}

void frint_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd, uint64_t rounding)
{
	ir_operand operand = V_jit(ctx,Rn);
	uint64_t esize = get_flt_size_jit(ctx,ftype);
	if ((((uint64_t)((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx)) && (uint64_t)((uint64_t)rounding != (uint64_t)FPRounding_TIEAWAY))))
	{
		uint64_t rounding_control = get_x86_rounding_mode_jit(ctx,rounding);
		ir_operand result = copy_new_raw_size(ctx, intrinsic_binary_imm_jit(ctx,int128,select_jit(ctx,((uint64_t)esize == (uint64_t)64ULL),(uint64_t)x86_roundsd,(uint64_t)x86_roundss),operand,rounding_control), int128);
		result = clear_vector_scalar_jit(ctx,result,esize);
		V_jit(ctx,Rd,result);
	}
	else
	{
		ir_operand result = ssa_emit_context::vector_zero(ctx);
		ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
		ir_operand working = FPRoundInt_jit(ctx,copy_new_raw_size(ctx, operand, int64),fpcr_state,rounding,esize);
		ssa_emit_context::vector_insert(ctx,result, 0ULL, esize, working);
		V_jit(ctx,Rd,result);
	}
}

void intrinsic_float_binary_vector_jit(ssa_emit_context* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t Q, uint64_t sz, uint64_t float_instruction, uint64_t double_instruction)
{
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand result;
	if ((sz))
	{
		result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,double_instruction,operand1,operand2), int128);
	}
	else
	{
		result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,float_instruction,operand1,operand2), int128);
	}
	if ((!Q))
	{
		ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
	}
	V_jit(ctx,Rd,result);
}

void intrinsic_float_binary_scalar_jit(ssa_emit_context* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t fsize, uint64_t half_instruction, uint64_t float_instruction, uint64_t double_instruction)
{
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand result;
	uint64_t esize = get_flt_size_jit(ctx,fsize);
	if ((((uint64_t)esize == (uint64_t)64ULL)))
	{
		result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,double_instruction,operand1,operand2), int128);
	}
	else if ((((uint64_t)esize == (uint64_t)32ULL)))
	{
		result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,float_instruction,operand1,operand2), int128);
		ssa_emit_context::vector_insert(ctx,result, 1ULL, 32ULL, ir_operand::create_con(0ULL, int64));
	}
	else if ((((uint64_t)esize == (uint64_t)16ULL)))
	{
		result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,half_instruction,operand1,operand2), int128);
		ssa_emit_context::vector_insert(ctx,result, 1ULL, 16ULL, ir_operand::create_con(0ULL, int64));
		ssa_emit_context::vector_insert(ctx,result, 1ULL, 32ULL, ir_operand::create_con(0ULL, int64));
	}
	else
	{
		undefined_jit(ctx);
	}
	ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
	V_jit(ctx,Rd,result);
}

void x86_sse_logic_vector_jit(ssa_emit_context* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t Q, uint64_t invert, uint64_t primary_instruction)
{
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	if ((invert))
	{
		ir_operand one = ssa_emit_context::vector_one(ctx);
		operand2 = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,(uint64_t)x86_xorps,operand2,one), int128);
	}
	ir_operand result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,primary_instruction,operand1,operand2), int128);
	if ((!Q))
	{
		ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
	}
	V_jit(ctx,Rd,result);
}

ir_operand sse_copy_to_xmm_from_xmm_element_jit(ssa_emit_context* ctx, ir_operand source, uint64_t size, uint64_t index)
{
	if ((((uint64_t)size <= (uint64_t)16ULL)))
	{
		ir_operand source_element = ssa_emit_context::vector_extract(ctx,source, size, index);
		return sse_coppy_gp_across_lanes_jit(ctx,source_element,size);
	}
	if ((((uint64_t)size == (uint64_t)32ULL)))
	{
		return copy_new_raw_size(ctx, intrinsic_ternary_imm_jit(ctx,int128,(uint64_t)x86_shufps,source,source,((uint64_t)((uint64_t)((uint64_t)index | (uint64_t)(((uint64_t)index << (uint64_t)2ULL))) | (uint64_t)(((uint64_t)index << (uint64_t)4ULL))) | (uint64_t)(((uint64_t)index << (uint64_t)6ULL)))), int128);
	}
	else if ((((uint64_t)size == (uint64_t)64ULL)))
	{
		return copy_new_raw_size(ctx, intrinsic_ternary_imm_jit(ctx,int128,(uint64_t)x86_shufpd,source,source,((uint64_t)index | (uint64_t)(((uint64_t)index << (uint64_t)1ULL)))), int128);
	}
	else
	{
		undefined_jit(ctx);
	}
}

ir_operand sse_coppy_gp_across_lanes_jit(ssa_emit_context* ctx, ir_operand source, uint64_t size)
{
	if ((((uint64_t)size == (uint64_t)8ULL)))
	{
		source = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, source, ir_operand::create_con(255ULL, int64));
		source = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, source, ssa_emit_context::emit_ssa(ctx, ir_shift_left, source, ir_operand::create_con(8ULL, int64))), ssa_emit_context::emit_ssa(ctx, ir_shift_left, source, ir_operand::create_con(16ULL, int64))), ssa_emit_context::emit_ssa(ctx, ir_shift_left, source, ir_operand::create_con(24ULL, int64)));
		size = 32ULL;
	}
	else if ((((uint64_t)size == (uint64_t)16ULL)))
	{
		source = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, source, ir_operand::create_con(65535ULL, int64));
		source = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, source, ssa_emit_context::emit_ssa(ctx, ir_shift_left, source, ir_operand::create_con(16ULL, int64)));
		size = 32ULL;
	}
	ir_operand working_element = copy_new_raw_size(ctx, source, int128);
	if ((((uint64_t)size == (uint64_t)32ULL)))
	{
		return copy_new_raw_size(ctx, intrinsic_ternary_imm_jit(ctx,int128,(uint64_t)x86_shufps,working_element,working_element,0ULL), int128);
	}
	else if ((((uint64_t)size == (uint64_t)64ULL)))
	{
		return copy_new_raw_size(ctx, intrinsic_ternary_imm_jit(ctx,int128,(uint64_t)x86_shufpd,working_element,working_element,0ULL), int128);
	}
	else
	{
		undefined_jit(ctx);
	}
}

void floating_point_multiply_scalar_element_jit(ssa_emit_context* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t sz, uint64_t index)
{
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	if ((use_x86_sse_jit(ctx)))
	{
		operand2 = sse_copy_to_xmm_from_xmm_element_jit(ctx,operand2,esize,index);
		uint64_t multiply_instruction;
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		multiply_instruction = (uint64_t)x86_mulss;
		else
		multiply_instruction = (uint64_t)x86_mulsd;
		ir_operand result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,multiply_instruction,operand1,operand2), int128);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 32ULL, ir_operand::create_con(0ULL, int64));
		}
		ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		V_jit(ctx,Rd,result);
	}
	else
	{
		ir_operand result = ssa_emit_context::vector_zero(ctx);
		ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
		ir_operand product = FPMul_jit(ctx,copy_new_raw_size(ctx, operand1, int64),ssa_emit_context::vector_extract(ctx,operand2, index, esize),fpcr_state,esize);
		ssa_emit_context::vector_insert(ctx,result, 0ULL, esize, product);
		V_jit(ctx,Rd,result);
	}
}

void floating_point_multiply_vector_element_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t sz, uint64_t index)
{
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	if ((use_x86_sse_jit(ctx)))
	{
		operand2 = sse_copy_to_xmm_from_xmm_element_jit(ctx,operand2,esize,index);
		uint64_t multiply_instruction;
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		multiply_instruction = (uint64_t)x86_mulps;
		else
		multiply_instruction = (uint64_t)x86_mulpd;
		ir_operand result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,multiply_instruction,operand1,operand2), int128);
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		}
		V_jit(ctx,Rd,result);
	}
	else
	{
		ir_operand result = ssa_emit_context::vector_zero(ctx);
		uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
		ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, index, esize);
		ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
			ssa_emit_context::vector_insert(ctx,result, e, esize, FPMul_jit(ctx,element1,element2,fpcr_state,esize));
		}
		V_jit(ctx,Rd,result);
	}
}

void floating_point_multiply_accumulate_scalar_element_jit(ssa_emit_context* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t neg, uint64_t sz, uint64_t index)
{
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand operand3 = V_jit(ctx,Rd);
	ir_operand result;
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx))))
	{
		uint64_t add_instruction;
		uint64_t subtract_instruction;
		uint64_t multiply_instruction;
		operand2 = copy_new_raw_size(ctx, ssa_emit_context::vector_extract(ctx,operand2, index, esize), int128);
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			add_instruction = (uint64_t)x86_addsd;
			subtract_instruction = (uint64_t)x86_subsd;
			multiply_instruction = (uint64_t)x86_mulsd;
		}
		else
		{
			add_instruction = (uint64_t)x86_addss;
			subtract_instruction = (uint64_t)x86_subss;
			multiply_instruction = (uint64_t)x86_mulss;
		}
		result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,multiply_instruction,operand1,operand2), int128);
		if ((neg))
		{
			result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,subtract_instruction,operand3,result), int128);
		}
		else
		{
			result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,add_instruction,operand3,result), int128);
		}
		ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 32ULL, ir_operand::create_con(0ULL, int64));
		}
	}
	else
	{
		result = ssa_emit_context::vector_zero(ctx);
		ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
		ir_operand element1 = copy_new_raw_size(ctx, operand1, int64);
		if ((neg))
		{
			element1 = FPNeg_jit(ctx,element1,fpcr_state,esize);
		}
		ir_operand product_accumalant = FPMulAdd_jit(ctx,copy_new_raw_size(ctx, operand3, int64),element1,ssa_emit_context::vector_extract(ctx,operand2, index, esize),fpcr_state,esize);
		ssa_emit_context::vector_insert(ctx,result, 0ULL, esize, product_accumalant);
	}
	V_jit(ctx,Rd,result);
}

void floating_point_multiply_accumulate_vector_element_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t neg, uint64_t sz, uint64_t index)
{
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand operand3 = V_jit(ctx,Rd);
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	ir_operand result;
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx))))
	{
		uint64_t add_instruction;
		uint64_t subtract_instruction;
		uint64_t multiply_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			add_instruction = (uint64_t)x86_addpd;
			subtract_instruction = (uint64_t)x86_subpd;
			multiply_instruction = (uint64_t)x86_mulpd;
		}
		else
		{
			add_instruction = (uint64_t)x86_addps;
			subtract_instruction = (uint64_t)x86_subps;
			multiply_instruction = (uint64_t)x86_mulps;
		}
		operand2 = sse_copy_to_xmm_from_xmm_element_jit(ctx,operand2,esize,index);
		result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,multiply_instruction,operand1,operand2), int128);
		if ((neg))
		{
			result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,subtract_instruction,operand3,result), int128);
		}
		else
		{
			result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,add_instruction,operand3,result), int128);
		}
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		}
	}
	else
	{
		result = ssa_emit_context::vector_zero(ctx);
		uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
		ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, index, esize);
		ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
			ir_operand element3 = ssa_emit_context::vector_extract(ctx,operand3, e, esize);
			if ((neg))
			{
				element1 = FPNeg_jit(ctx,element1,fpcr_state,esize);
			}
			ssa_emit_context::vector_insert(ctx,result, e, esize, FPMulAdd_jit(ctx,element3,element1,element2,fpcr_state,esize));
		}
	}
	V_jit(ctx,Rd,result);
}

void fcm_vector_jit(ssa_emit_context* ctx, uint64_t Rd, uint64_t Rn, uint64_t Rm, uint64_t mode, uint64_t Q, uint64_t sz)
{
	ir_operand n = V_jit(ctx,Rn);
	ir_operand m;
	if ((((uint64_t)Rm == (uint64_t)-1ULL)))
	{
		m = ssa_emit_context::vector_zero(ctx);
	}
	else
	{
		m = V_jit(ctx,Rm);
	}
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	if ((use_x86_sse_jit(ctx)))
	{
		ir_operand result;
		uint64_t operation = select_jit(ctx,((uint64_t)esize == (uint64_t)64ULL),(uint64_t)x86_cmppd,(uint64_t)x86_cmpps);
		uint64_t control;
		if ((((uint64_t)mode == (uint64_t)0ULL)))
		{
			control = 0ULL;
		}
		else if ((((uint64_t)mode == (uint64_t)1ULL)))
		{
			control = 6ULL;
		}
		else if ((((uint64_t)mode == (uint64_t)2ULL)))
		{
			control = 5ULL;
		}
		result = copy_new_raw_size(ctx, intrinsic_ternary_imm_jit(ctx,int128,operation,n,m,control), int128);
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		}
		V_jit(ctx,Rd,result);
	}
	else
	{
		ir_operand result = ssa_emit_context::vector_zero(ctx);
		uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
		ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element1 = ssa_emit_context::vector_extract(ctx,n, e, esize);
			ir_operand element2 = ssa_emit_context::vector_extract(ctx,m, e, esize);
			ir_operand element_result;
			if ((((uint64_t)mode == (uint64_t)0ULL)))
			{
				element_result = ssa_emit_context::emit_ssa(ctx, ir_subtract, ir_operand::create_con(0ULL, int64), FPCompareEQ_jit(ctx,element1,element2,fpcr_state,esize));
			}
			else if ((((uint64_t)mode == (uint64_t)1ULL)))
			{
				element_result = ssa_emit_context::emit_ssa(ctx, ir_subtract, ir_operand::create_con(0ULL, int64), FPCompareGT_jit(ctx,element1,element2,fpcr_state,esize));
			}
			else if ((((uint64_t)mode == (uint64_t)2ULL)))
			{
				element_result = ssa_emit_context::emit_ssa(ctx, ir_subtract, ir_operand::create_con(0ULL, int64), FPCompareGE_jit(ctx,element1,element2,fpcr_state,esize));
			}
			ssa_emit_context::vector_insert(ctx,result, e, esize, element_result);
		}
		V_jit(ctx,Rd,result);
	}
}

ir_operand clear_vector_scalar_jit(ssa_emit_context* ctx, ir_operand working, uint64_t fltsize)
{
	if ((((uint64_t)fltsize == (uint64_t)32ULL)))
	{
		ssa_emit_context::vector_insert(ctx,working, 1ULL, 32ULL, ir_operand::create_con(0ULL, int64));
	}
	ssa_emit_context::vector_insert(ctx,working, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
	return working;
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
		ir_operand result = copy_new_raw_size(ctx, add_subtract_carry_impl_jit(ctx,O,operand1,operand2,((uint64_t)S == (uint64_t)1ULL),((uint64_t)op == (uint64_t)0ULL),copy_new_raw_size(ctx, _sys_jit(ctx,(uint64_t)nzcv_c), O)), O);
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

uint64_t create_rbit_mask_jit(ssa_emit_context* ctx, uint64_t index)
{
	index = ((uint64_t)1ULL << (uint64_t)index);
	uint64_t mask = ((uint64_t)(((uint64_t)(((uint64_t)1ULL << (uint64_t)index)) - (uint64_t)1ULL)) << (uint64_t)index);
	mask = replicate_c_jit(ctx,mask,((uint64_t)index * (uint64_t)2ULL),((uint64_t)((uint64_t)64ULL / (uint64_t)index) / (uint64_t)2ULL));
	return mask;
}

void rbit_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rn, uint64_t Rd)
{
	uint64_t datasize = ((uint64_t)32ULL << (uint64_t)sf);
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : 0;
	{
		ir_operand operand = copy_new_raw_size(ctx, X_jit(ctx,Rn), O);
		ir_operand result = operand;
		uint64_t max = ((uint64_t)5ULL + (uint64_t)sf);
		for (uint64_t i = 0; i < (max); i++)
		{
			uint64_t n_mask = create_rbit_mask_jit(ctx,i);
			uint64_t i_mask = ~n_mask;
			uint64_t shift = ((uint64_t)1ULL << (uint64_t)i);
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, result, ir_operand::create_con(n_mask, O)), ir_operand::create_con(shift, O)), ssa_emit_context::emit_ssa(ctx, ir_shift_left, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, result, ir_operand::create_con(i_mask, O)), ir_operand::create_con(shift, O)));
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
		if ((use_x86_lzcnt_jit(ctx)))
		{
			if ((s))
			{
				ir_operand nhigh = ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, operand, ir_operand::create_con(1ULL, O));
				ir_operand mask = ir_operand::create_con(-1ULL, O);
				mask = ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, mask, ir_operand::create_con(1ULL, O));
				ir_operand nlow = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, operand, mask);
				result = copy_new_raw_size(ctx, intrinsic_unary_jit(ctx,O,(uint64_t)x86_lzcnt,ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, nhigh, nlow)), O);
				result = ssa_emit_context::emit_ssa(ctx, ir_subtract, result, ir_operand::create_con(1ULL, O));
			}
			else
			{
				result = copy_new_raw_size(ctx, intrinsic_unary_jit(ctx,O,(uint64_t)x86_lzcnt,operand), O);
			}
		}
		else
		{
			for (uint64_t i = 0; i < (datasize); i++)
			{
				ir_operand working = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, operand, ir_operand::create_con((((uint64_t)((uint64_t)datasize - (uint64_t)i) - (uint64_t)1ULL)), O)), ir_operand::create_con(1ULL, O));
				if ((s))
				{
					{
					        ir_operand end = ir_operation_block::create_label(ctx->ir);
					        ir_operand yes = ir_operation_block::create_label(ctx->ir);
					
					        ir_operand condition = ssa_emit_context::emit_ssa(ctx,ir_bitwise_exclusive_or,ssa_emit_context::emit_ssa(ctx, ir_compare_not_equal, working, sig_bit), ir_operand::create_con(1));
					
					        ir_operation_block::jump_if(ctx->ir,end, condition);
					
					    	{
							ssa_emit_context::move(ctx,done,ir_operand::create_con(1ULL, O));
						}
					
					        ir_operation_block::mark_label(ctx->ir, end);
					    }
				}
				else {
				        ir_operand end = ir_operation_block::create_label(ctx->ir);
				        ir_operand yes = ir_operation_block::create_label(ctx->ir);
				
				        ir_operand condition = ssa_emit_context::emit_ssa(ctx,ir_bitwise_exclusive_or,working, ir_operand::create_con(1));
				
				        ir_operation_block::jump_if(ctx->ir,end, condition);
				
				    	{
						ssa_emit_context::move(ctx,done,ir_operand::create_con(1ULL, O));
					}
				
				        ir_operation_block::mark_label(ctx->ir, end);
				    }
				{
				        ir_operand end = ir_operation_block::create_label(ctx->ir);
				        ir_operand yes = ir_operation_block::create_label(ctx->ir);
				
				        ir_operand condition = ssa_emit_context::emit_ssa(ctx,ir_bitwise_exclusive_or,ssa_emit_context::emit_ssa(ctx, ir_logical_not, done), ir_operand::create_con(1));
				
				        ir_operation_block::jump_if(ctx->ir,end, condition);
				
				    	{
						ssa_emit_context::move(ctx,result,ssa_emit_context::emit_ssa(ctx, ir_add, result, ir_operand::create_con(1ULL, O)));
					}
				
				        ir_operation_block::mark_label(ctx->ir, end);
				    }
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
			_sys_jit(ctx,(uint64_t)nzcv_n,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, result, ir_operand::create_con(0ULL, O)), int64));
			_sys_jit(ctx,(uint64_t)nzcv_z,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, result, ir_operand::create_con(0ULL, O)), int64));
			_sys_jit(ctx,(uint64_t)nzcv_c,ir_operand::create_con(0ULL, int64));
			_sys_jit(ctx,(uint64_t)nzcv_v,ir_operand::create_con(0ULL, int64));
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
				_sys_jit(ctx,(uint64_t)nzcv_n,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, result, ir_operand::create_con(0ULL, O)), int64));
				_sys_jit(ctx,(uint64_t)nzcv_z,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, result, ir_operand::create_con(0ULL, O)), int64));
				_sys_jit(ctx,(uint64_t)nzcv_c,ir_operand::create_con(0ULL, int64));
				_sys_jit(ctx,(uint64_t)nzcv_v,ir_operand::create_con(0ULL, int64));
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
				_sys_jit(ctx,(uint64_t)nzcv_n,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL), int64));
				_sys_jit(ctx,(uint64_t)nzcv_z,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL), int64));
				_sys_jit(ctx,(uint64_t)nzcv_c,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL), int64));
				_sys_jit(ctx,(uint64_t)nzcv_v,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL), int64));
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

void msr_register_jit(ssa_emit_context* ctx, uint64_t imm15, uint64_t Rt)
{
	ir_operand operand = X_jit(ctx,Rt);
	if ((((uint64_t)imm15 == (uint64_t)23072ULL)))
	{
		_sys_jit(ctx,(uint64_t)fpcr,operand);
	}
	else if ((((uint64_t)imm15 == (uint64_t)23073ULL)))
	{
		_sys_jit(ctx,(uint64_t)fpsr,operand);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24194ULL)))
	{
		_sys_jit(ctx,(uint64_t)thread_local_1,operand);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24195ULL)))
	{
		_sys_jit(ctx,(uint64_t)thread_local_0,operand);
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
		operand = _sys_jit(ctx,(uint64_t)fpcr);
	}
	else if ((((uint64_t)imm15 == (uint64_t)23073ULL)))
	{
		operand = _sys_jit(ctx,(uint64_t)fpsr);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24194ULL)))
	{
		operand = _sys_jit(ctx,(uint64_t)thread_local_1);
	}
	else if ((((uint64_t)imm15 == (uint64_t)24195ULL)))
	{
		operand = _sys_jit(ctx,(uint64_t)thread_local_0);
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
		_sys_jit(ctx,(uint64_t)exclusive_address,ir_operand::create_con(-1ULL, int64));
		_sys_jit(ctx,(uint64_t)exclusive_value,ir_operand::create_con(-1ULL, int64));
	}
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

ir_operand exclusive_address_mask_jit(ssa_emit_context* ctx)
{
	return ir_operand::create_con(~(((uint64_t)(((uint64_t)4ULL << (uint64_t)4ULL)) - (uint64_t)1ULL)), int64);
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
			_sys_jit(ctx,(uint64_t)exclusive_address,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, address, exclusive_address_mask_jit(ctx)));
			_sys_jit(ctx,(uint64_t)exclusive_value,copy_new_raw_size(ctx, value, int64));
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
			ir_operand mask = exclusive_address_mask_jit(ctx);
			ir_operand _exclusive_address = _sys_jit(ctx,(uint64_t)exclusive_address);
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
					ir_operand expecting = copy_new_raw_size(ctx, _sys_jit(ctx,(uint64_t)exclusive_value), S);
					ir_operand cas_success = copy_new_raw_size(ctx, compare_and_swap_jit(ctx,address,copy_new_raw_size(ctx, expecting, int64),copy_new_raw_size(ctx, to_swap, int64),datasize), S);
					X_jit(ctx,Rs,copy_new_raw_size(ctx, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, cas_success, ir_operand::create_con(1ULL, S)), ir_operand::create_con(1ULL, S)), int64));
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
		result = FPToFixed_jit(ctx,source,fracbits,((uint64_t)opcode == (uint64_t)1ULL),(uint64_t)FPRounding_ZERO,intsize,fltsize);
		X_jit(ctx,Rd,result);
	}
	else
	{
		undefined_with_jit(ctx,100ULL);
	}
}

void fcvt_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t opc, uint64_t Rn, uint64_t Rd)
{
	uint64_t srcsize = get_flt_size_jit(ctx,ftype);
	uint64_t dstsize = get_flt_size_jit(ctx,opc);
	ir_operand operand = copy_new_raw_size(ctx, V_jit(ctx,Rn), int64);
	V_jit(ctx,Rd,copy_new_raw_size(ctx, FPConvert_jit(ctx,operand,dstsize,srcsize), int128));
}

void fcvtz_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_jit(ctx,sf,ftype,Rd,Rn,(uint64_t)FPRounding_ZERO,U,0ULL);
}

void fcvtn_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_jit(ctx,sf,ftype,Rd,Rn,(uint64_t)FPRounding_TIEEVEN,U,0ULL);
}

void fcvta_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_jit(ctx,sf,ftype,Rd,Rn,(uint64_t)FPRounding_TIEAWAY,U,0ULL);
}

void fcvtm_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_jit(ctx,sf,ftype,Rd,Rn,(uint64_t)FPRounding_NEGINF,U,0ULL);
}

void frintp_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd)
{
	frint_jit(ctx,ftype,Rn,Rd,(uint64_t)FPRounding_POSINF);
}

void frintm_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd)
{
	frint_jit(ctx,ftype,Rn,Rd,(uint64_t)FPRounding_NEGINF);
}

void fcvtp_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_int_jit(ctx,sf,ftype,Rd,Rn,(uint64_t)FPRounding_POSINF,U,0ULL);
}

void fadd_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx))))
	{
		intrinsic_float_binary_vector_jit(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)x86_addps,(uint64_t)x86_addpd);
		return;
	}
	float_binary_vector_jit(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)FPAdd_jit);
}

void fmul_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx))))
	{
		intrinsic_float_binary_vector_jit(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)x86_mulps,(uint64_t)x86_mulpd);
		return;
	}
	float_binary_vector_jit(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)FPMul_jit);
}

void fsub_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx))))
	{
		intrinsic_float_binary_vector_jit(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)x86_subps,(uint64_t)x86_subpd);
		return;
	}
	float_binary_vector_jit(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)FPSub_jit);
}

void fdiv_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx))))
	{
		intrinsic_float_binary_vector_jit(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)x86_divps,(uint64_t)x86_divpd);
		return;
	}
	float_binary_vector_jit(ctx,Rd,Rn,Rm,Q,sz,(uint64_t)FPDiv_jit);
}

void fmul_accumulate_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t neg, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand operand3 = V_jit(ctx,Rd);
	ir_operand result;
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx))))
	{
		uint64_t add_instruction;
		uint64_t subtract_instruction;
		uint64_t multiply_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			add_instruction = (uint64_t)x86_addpd;
			subtract_instruction = (uint64_t)x86_subpd;
			multiply_instruction = (uint64_t)x86_mulpd;
		}
		else
		{
			add_instruction = (uint64_t)x86_addps;
			subtract_instruction = (uint64_t)x86_subps;
			multiply_instruction = (uint64_t)x86_mulps;
		}
		result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,multiply_instruction,operand1,operand2), int128);
		if ((neg))
		{
			result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,subtract_instruction,operand3,result), int128);
		}
		else
		{
			result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,add_instruction,operand3,result), int128);
		}
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		}
	}
	else
	{
		result = ssa_emit_context::vector_zero(ctx);
		ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
			ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, e, esize);
			ir_operand element3 = ssa_emit_context::vector_extract(ctx,operand3, e, esize);
			if ((neg))
			{
				element1 = FPNeg_jit(ctx,element1,fpcr_state,esize);
			}
			ssa_emit_context::vector_insert(ctx,result, e, esize, FPMulAdd_jit(ctx,element3,element1,element2,fpcr_state,esize));
		}
	}
	V_jit(ctx,Rd,result);
}

void faddp_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	ir_operand lo = V_jit(ctx,Rn);
	ir_operand hi = V_jit(ctx,Rm);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ir_operand element1 = get_from_concacted_vector_jit(ctx,hi,lo,((uint64_t)2ULL * (uint64_t)e),((uint64_t)datasize / (uint64_t)esize),esize);
		ir_operand element2 = get_from_concacted_vector_jit(ctx,hi,lo,((uint64_t)(((uint64_t)2ULL * (uint64_t)e)) + (uint64_t)1ULL),((uint64_t)datasize / (uint64_t)esize),esize);
		ssa_emit_context::vector_insert(ctx,result, e, esize, FPAdd_jit(ctx,element1,element2,fpcr_state,esize));
	}
	V_jit(ctx,Rd,result);
}

void frsqrte_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand operand = V_jit(ctx,Rn);
	if ((use_x86_sse_jit(ctx)))
	{
		ir_operand one = sse_coppy_gp_across_lanes_jit(ctx,float_imm_jit(ctx,ir_operand::create_con(1ULL, int64),esize),esize);
		uint64_t sqrt_instruction;
		uint64_t divide_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			sqrt_instruction = (uint64_t)x86_sqrtpd;
			divide_instruction = (uint64_t)x86_divpd;
		}
		else
		{
			sqrt_instruction = (uint64_t)x86_sqrtps;
			divide_instruction = (uint64_t)x86_divps;
		}
		ir_operand sqrt_result = copy_new_raw_size(ctx, intrinsic_unary_jit(ctx,int128,sqrt_instruction,operand), int128);
		ir_operand reciprocal_result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,divide_instruction,one,sqrt_result), int128);
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			ssa_emit_context::vector_insert(ctx,reciprocal_result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		}
		V_jit(ctx,Rd,reciprocal_result);
	}
	else
	{
		ir_operand result = ssa_emit_context::vector_zero(ctx);
		ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element = ssa_emit_context::vector_extract(ctx,operand, e, esize);
			element = FPRSqrtEstimate_jit(ctx,element,fpcr_state,esize);
			ssa_emit_context::vector_insert(ctx,result, e, esize, element);
		}
		V_jit(ctx,Rd,result);
	}
}

void frsqrts_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	if ((use_x86_sse_jit(ctx)))
	{
		ir_operand two = sse_coppy_gp_across_lanes_jit(ctx,float_imm_jit(ctx,ir_operand::create_con(2ULL, int64),esize),esize);
		ir_operand three = sse_coppy_gp_across_lanes_jit(ctx,float_imm_jit(ctx,ir_operand::create_con(3ULL, int64),esize),esize);
		ir_operand negator = sse_coppy_gp_across_lanes_jit(ctx,ir_operand::create_con(((uint64_t)1ULL << (uint64_t)(((uint64_t)esize - (uint64_t)1ULL))), int64),esize);
		operand1 = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,(uint64_t)x86_xorps,operand1,negator), int128);
		uint64_t add_instruction;
		uint64_t multiply_instruction;
		uint64_t divide_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			add_instruction = (uint64_t)x86_addpd;
			multiply_instruction = (uint64_t)x86_mulpd;
			divide_instruction = (uint64_t)x86_divpd;
		}
		else
		{
			add_instruction = (uint64_t)x86_addps;
			multiply_instruction = (uint64_t)x86_mulps;
			divide_instruction = (uint64_t)x86_divps;
		}
		ir_operand mul_result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,multiply_instruction,operand1,operand2), int128);
		ir_operand add_result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,add_instruction,three,mul_result), int128);
		ir_operand div_result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,divide_instruction,add_result,two), int128);
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			ssa_emit_context::vector_insert(ctx,div_result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		}
		V_jit(ctx,Rd,div_result);
	}
	else
	{
		ir_operand result = ssa_emit_context::vector_zero(ctx);
		ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
			ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, e, esize);
			ssa_emit_context::vector_insert(ctx,result, e, esize, FPRSqrtStepFused_jit(ctx,element1,element2,fpcr_state,esize));
		}
		V_jit(ctx,Rd,result);
	}
}

void frecps_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	if ((use_x86_sse_jit(ctx)))
	{
		ir_operand negator = sse_coppy_gp_across_lanes_jit(ctx,ir_operand::create_con(((uint64_t)1ULL << (uint64_t)(((uint64_t)esize - (uint64_t)1ULL))), int64),esize);
		ir_operand two = sse_coppy_gp_across_lanes_jit(ctx,float_imm_jit(ctx,ir_operand::create_con(2ULL, int64),esize),esize);
		operand1 = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,(uint64_t)x86_xorps,operand1,negator), int128);
		uint64_t add_instruction;
		uint64_t multiply_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			add_instruction = (uint64_t)x86_addpd;
			multiply_instruction = (uint64_t)x86_mulpd;
		}
		else
		{
			add_instruction = (uint64_t)x86_addps;
			multiply_instruction = (uint64_t)x86_mulps;
		}
		ir_operand result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,add_instruction,two,copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,multiply_instruction,operand1,operand2), int128)), int128);
		if ((((uint64_t)datasize == (uint64_t)64ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		}
		V_jit(ctx,Rd,result);
	}
	else
	{
		ir_operand result = ssa_emit_context::vector_zero(ctx);
		ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
			ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, e, esize);
			ssa_emit_context::vector_insert(ctx,result, e, esize, FPRecipStepFused_jit(ctx,element1,element2,fpcr_state,esize));
		}
		V_jit(ctx,Rd,result);
	}
}

void fmul_scalar_by_element_jit(ssa_emit_context* ctx, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t H, uint64_t Rn, uint64_t Rd)
{
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)H);
	uint64_t index;
	uint64_t rm_hi = M;
	uint64_t test = ((uint64_t)(((uint64_t)sz << (uint64_t)1ULL)) | (uint64_t)L);
	if ((((uint64_t)(((uint64_t)test >> (uint64_t)1ULL)) == (uint64_t)0ULL)))
	{
		index = ((uint64_t)L | (uint64_t)(((uint64_t)H << (uint64_t)1ULL)));
	}
	else if ((((uint64_t)test == (uint64_t)2ULL)))
	{
		index = H;
	}
	else
	{
		undefined_jit(ctx);
	}
	floating_point_multiply_scalar_element_jit(ctx,Rd,Rn,((uint64_t)Rm | (uint64_t)(((uint64_t)rm_hi << (uint64_t)4ULL))),sz,index);
}

void fmul_vector_by_element_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t H, uint64_t Rn, uint64_t Rd)
{
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)H);
	uint64_t index;
	uint64_t rm_hi = M;
	uint64_t test = ((uint64_t)(((uint64_t)sz << (uint64_t)1ULL)) | (uint64_t)L);
	if ((((uint64_t)(((uint64_t)test >> (uint64_t)1ULL)) == (uint64_t)0ULL)))
	{
		index = ((uint64_t)L | (uint64_t)(((uint64_t)H << (uint64_t)1ULL)));
	}
	else if ((((uint64_t)test == (uint64_t)2ULL)))
	{
		index = H;
	}
	else
	{
		undefined_jit(ctx);
	}
	floating_point_multiply_vector_element_jit(ctx,Q,Rd,Rn,((uint64_t)Rm | (uint64_t)(((uint64_t)rm_hi << (uint64_t)4ULL))),sz,index);
}

void fmul_accumulate_scalar_jit(ssa_emit_context* ctx, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t neg, uint64_t H, uint64_t Rn, uint64_t Rd)
{
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)H);
	uint64_t index;
	uint64_t rm_hi = M;
	uint64_t test = ((uint64_t)(((uint64_t)sz << (uint64_t)1ULL)) | (uint64_t)L);
	if ((((uint64_t)(((uint64_t)test >> (uint64_t)1ULL)) == (uint64_t)0ULL)))
	{
		index = ((uint64_t)L | (uint64_t)(((uint64_t)H << (uint64_t)1ULL)));
	}
	else if ((((uint64_t)test == (uint64_t)2ULL)))
	{
		index = H;
	}
	else
	{
		undefined_jit(ctx);
	}
	floating_point_multiply_accumulate_scalar_element_jit(ctx,Rd,Rn,((uint64_t)Rm | (uint64_t)(((uint64_t)rm_hi << (uint64_t)4ULL))),neg,sz,index);
}

void fmul_accumulate_element_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t neg, uint64_t H, uint64_t Rn, uint64_t Rd)
{
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)H);
	uint64_t index;
	uint64_t rm_hi = M;
	uint64_t test = ((uint64_t)(((uint64_t)sz << (uint64_t)1ULL)) | (uint64_t)L);
	if ((((uint64_t)(((uint64_t)test >> (uint64_t)1ULL)) == (uint64_t)0ULL)))
	{
		index = ((uint64_t)L | (uint64_t)(((uint64_t)H << (uint64_t)1ULL)));
	}
	else if ((((uint64_t)test == (uint64_t)2ULL)))
	{
		index = H;
	}
	else
	{
		undefined_jit(ctx);
	}
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,((uint64_t)Rm | (uint64_t)(((uint64_t)rm_hi << (uint64_t)4ULL))));
	ir_operand operand3 = V_jit(ctx,Rd);
	floating_point_multiply_accumulate_vector_element_jit(ctx,Q,Rd,Rn,((uint64_t)Rm | (uint64_t)(((uint64_t)rm_hi << (uint64_t)4ULL))),neg,sz,index);
}

void faddp_scalar_jit(ssa_emit_context* ctx, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)esize * (uint64_t)2ULL);
	ir_operand operand = V_jit(ctx,Rn);
	if ((use_x86_sse_jit(ctx)))
	{
		ir_operand result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,select_jit(ctx,((uint64_t)esize == (uint64_t)64ULL),(uint64_t)x86_haddpd,(uint64_t)x86_haddps),operand,operand), int128);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 32ULL, ir_operand::create_con(0ULL, int64));
		}
		ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		V_jit(ctx,Rd,result);
	}
	else
	{
		ir_operand result = ssa_emit_context::vector_zero(ctx);
		ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
		ir_operand bottom = ssa_emit_context::vector_extract(ctx,operand, 0ULL, esize);
		ir_operand top = ssa_emit_context::vector_extract(ctx,operand, 1ULL, esize);
		ssa_emit_context::vector_insert(ctx,result, 0ULL, esize, FPAdd_jit(ctx,bottom,top,fpcr_state,esize));
		V_jit(ctx,Rd,result);
	}
}

void fcmeq_vector_zero_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	fcm_vector_jit(ctx,Rd,Rn,-1ULL,0ULL,Q,sz);
}

void fcmgt_vector_zero_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	fcm_vector_jit(ctx,Rd,Rn,-1ULL,1ULL,Q,sz);
}

void fcmge_vector_zero_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	fcm_vector_jit(ctx,Rd,Rn,-1ULL,2ULL,Q,sz);
}

void fcmeq_vector_register_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	fcm_vector_jit(ctx,Rd,Rn,Rm,0ULL,Q,sz);
}

void fcmgt_vector_register_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	fcm_vector_jit(ctx,Rd,Rn,Rm,1ULL,Q,sz);
}

void fcmge_vector_register_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	fcm_vector_jit(ctx,Rd,Rn,Rm,2ULL,Q,sz);
}

void fadd_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		intrinsic_float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,-1ULL,(uint64_t)x86_addss,(uint64_t)x86_addsd);
		return;
	}
	float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPAdd_jit);
}

void fsub_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		intrinsic_float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,-1ULL,(uint64_t)x86_subss,(uint64_t)x86_subsd);
		return;
	}
	float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPSub_jit);
}

void fmul_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		intrinsic_float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,-1ULL,(uint64_t)x86_mulss,(uint64_t)x86_mulsd);
		return;
	}
	float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPMul_jit);
}

void fdiv_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		intrinsic_float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,-1ULL,(uint64_t)x86_divss,(uint64_t)x86_divsd);
		return;
	}
	float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPDiv_jit);
}

void fmax_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		intrinsic_float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,-1ULL,(uint64_t)x86_maxss,(uint64_t)x86_maxsd);
		return;
	}
	float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPMax_jit);
}

void fmin_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		intrinsic_float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,-1ULL,(uint64_t)x86_minss,(uint64_t)x86_minsd);
		return;
	}
	float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPMin_jit);
}

void fmaxnm_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPMaxNum_jit);
}

void fminnm_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPMinNum_jit);
}

void fnmul_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		ir_operand operand1 = V_jit(ctx,Rn);
		ir_operand operand2 = V_jit(ctx,Rm);
		uint64_t esize = get_flt_size_jit(ctx,ftype);
		ir_operand negator = copy_new_raw_size(ctx, ir_operand::create_con((((uint64_t)1ULL << (uint64_t)(((uint64_t)esize - (uint64_t)1ULL)))), int64), int128);
		uint64_t multiply_instruction;
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			multiply_instruction = (uint64_t)x86_mulss;
		}
		else if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			multiply_instruction = (uint64_t)x86_mulsd;
		}
		else
		{
			undefined_jit(ctx);
		}
		operand2 = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,(uint64_t)x86_xorps,operand2,negator), int128);
		ir_operand result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,multiply_instruction,operand1,operand2), int128);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 32ULL, ir_operand::create_con(0ULL, int64));
		}
		ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		V_jit(ctx,Rd,result);
		return;
	}
	float_binary_scalar_jit(ctx,Rd,Rn,Rm,ftype,(uint64_t)FPNMul_jit);
}

void fabs_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx))))
	{
		uint64_t esize = get_flt_size_jit(ctx,ftype);
		ir_operand operand = V_jit(ctx,Rn);
		ir_operand mask = copy_new_raw_size(ctx, ir_operand::create_con((((uint64_t)(((uint64_t)1ULL << (uint64_t)(((uint64_t)esize - (uint64_t)1ULL)))) - (uint64_t)1ULL)), int64), int128);
		ir_operand result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,(uint64_t)x86_pand,operand,mask), int128);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 32ULL, ir_operand::create_con(0ULL, int64));
		}
		ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		V_jit(ctx,Rd,result);
		return;
	}
	float_unary_scalar_jit(ctx,Rd,Rn,ftype,(uint64_t)FPAbs_jit);
}

void fneg_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx))))
	{
		uint64_t esize = get_flt_size_jit(ctx,ftype);
		ir_operand operand = V_jit(ctx,Rn);
		ir_operand mask = copy_new_raw_size(ctx, ir_operand::create_con((((uint64_t)1ULL << (uint64_t)(((uint64_t)esize - (uint64_t)1ULL)))), int64), int128);
		ir_operand result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,(uint64_t)x86_xorps,operand,mask), int128);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 32ULL, ir_operand::create_con(0ULL, int64));
		}
		ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		V_jit(ctx,Rd,result);
		return;
	}
	float_unary_scalar_jit(ctx,Rd,Rn,ftype,(uint64_t)FPNeg_jit);
}

void fneg_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx))))
	{
		uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
		ir_operand operand = V_jit(ctx,Rn);
		ir_operand mask = sse_coppy_gp_across_lanes_jit(ctx,ir_operand::create_con((((uint64_t)1ULL << (uint64_t)(((uint64_t)esize - (uint64_t)1ULL)))), int64),esize);
		ir_operand result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,(uint64_t)x86_xorps,operand,mask), int128);
		if ((((uint64_t)Q == (uint64_t)0ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		}
		V_jit(ctx,Rd,result);
		return;
	}
	float_unary_vector_jit(ctx,Rd,Rn,Q,sz,(uint64_t)FPNeg_jit);
}

void fsqrt_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx)) && (uint64_t)((uint64_t)ftype != (uint64_t)3ULL))))
	{
		uint64_t esize = get_flt_size_jit(ctx,ftype);
		ir_operand operand = V_jit(ctx,Rn);
		uint64_t sqrt_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			sqrt_instruction = (uint64_t)x86_sqrtsd;
		}
		else
		{
			sqrt_instruction = (uint64_t)x86_sqrtss;
		}
		ir_operand result = copy_new_raw_size(ctx, intrinsic_unary_jit(ctx,int128,sqrt_instruction,operand), int128);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 32ULL, ir_operand::create_con(0ULL, int64));
		}
		ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		V_jit(ctx,Rd,result);
		return;
	}
	float_unary_scalar_jit(ctx,Rd,Rn,ftype,(uint64_t)FPSqrt_jit);
}

void fsqrt_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx))))
	{
		uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
		ir_operand operand = V_jit(ctx,Rn);
		uint64_t sqrt_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			sqrt_instruction = (uint64_t)x86_sqrtpd;
		}
		else
		{
			sqrt_instruction = (uint64_t)x86_sqrtps;
		}
		ir_operand result = copy_new_raw_size(ctx, intrinsic_unary_jit(ctx,int128,sqrt_instruction,operand), int128);
		if ((((uint64_t)Q == (uint64_t)0ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		}
		V_jit(ctx,Rd,result);
		return;
	}
	float_unary_vector_jit(ctx,Rd,Rn,Q,sz,(uint64_t)FPSqrt_jit);
}

void frecpe_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx))))
	{
		uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
		ir_operand operand = V_jit(ctx,Rn);
		ir_operand one = sse_coppy_gp_across_lanes_jit(ctx,float_imm_jit(ctx,ir_operand::create_con(1ULL, int64),esize),esize);
		uint64_t divide_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			divide_instruction = (uint64_t)x86_divpd;
		}
		else
		{
			divide_instruction = (uint64_t)x86_divps;
		}
		ir_operand result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,divide_instruction,one,operand), int128);
		if ((((uint64_t)Q == (uint64_t)0ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		}
		V_jit(ctx,Rd,result);
		return;
	}
	float_unary_vector_jit(ctx,Rd,Rn,Q,sz,(uint64_t)FPRecipEstimate_jit);
}

void frsqrte_scalar_jit(ssa_emit_context* ctx, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	ir_operand operand = copy_new_raw_size(ctx, V_jit(ctx,Rn), int64);
	if ((((uint64_t)use_fast_float_jit(ctx) && (uint64_t)use_x86_sse_jit(ctx))))
	{
		ir_operand one = copy_new_raw_size(ctx, float_imm_jit(ctx,ir_operand::create_con(1ULL, int64),esize), int128);
		uint64_t sqrt_instruction;
		uint64_t divide_instruction;
		if ((((uint64_t)esize == (uint64_t)64ULL)))
		{
			sqrt_instruction = (uint64_t)x86_sqrtsd;
			divide_instruction = (uint64_t)x86_divsd;
		}
		else
		{
			sqrt_instruction = (uint64_t)x86_sqrtss;
			divide_instruction = (uint64_t)x86_divss;
		}
		ir_operand sqrt_result = copy_new_raw_size(ctx, intrinsic_unary_jit(ctx,int128,sqrt_instruction,copy_new_raw_size(ctx, operand, int128)), int128);
		ir_operand reciprocal_result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,divide_instruction,one,sqrt_result), int128);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			ssa_emit_context::vector_insert(ctx,reciprocal_result, 1ULL, 32ULL, ir_operand::create_con(0ULL, int64));
		}
		ssa_emit_context::vector_insert(ctx,reciprocal_result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		V_jit(ctx,Rd,reciprocal_result);
	}
	else
	{
		ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
		ir_operand result = FPRSqrtEstimate_jit(ctx,operand,fpcr_state,esize);
		if ((((uint64_t)esize == (uint64_t)32ULL)))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, result, ir_operand::create_con(4294967295ULL, int64));
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

ir_operand _compare_and_swap_jit(ssa_emit_context* ctx, ir_operand physical_address, ir_operand expecting, ir_operand to_swap, uint64_t size)
{
	if ((use_x86_jit(ctx)))
	{
		if ((((uint64_t)size != (uint64_t)64ULL)))
		{
			ir_operand mask = ir_operand::create_con(((uint64_t)(((uint64_t)1ULL << (uint64_t)size)) - (uint64_t)1ULL), int64);
			ir_operand masking_value = ssa_emit_context::emit_ssa(ctx, ir_load, physical_address, int64);
			masking_value = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, masking_value, ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, mask));
			expecting = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, expecting, mask), masking_value);
			to_swap = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, to_swap, mask), masking_value);
		}
		return copy_new_raw_size(ctx, intrinsic_ternary_jit(ctx,int64,(uint64_t)x86_cmpxchg,physical_address,expecting,to_swap), int64);
	}
	return call_jit(ctx,physical_address,expecting,to_swap,ir_operand::create_con(size, int64),ir_operand::create_con(0ULL, int64),ir_operand::create_con(0ULL, int64),(uint64_t)compare_and_swap_interpreter_cpp);
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

void dup_general_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd)
{
	uint64_t size = lowest_bit_set_c_jit(ctx,bits_c_jit(ctx,imm5,3ULL,0ULL));
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	ir_operand element = X_jit(ctx,Rn);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand result;
	if ((use_x86_sse_jit(ctx)))
	{
		result = sse_coppy_gp_across_lanes_jit(ctx,element,esize);
		if ((!Q))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		}
	}
	else
	{
		result = ssa_emit_context::vector_zero(ctx);
		for (uint64_t e = 0; e < (elements); e++)
		{
			ssa_emit_context::vector_insert(ctx,result, e, esize, element);
		}
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
			ir_operand result = copy_new_raw_size(ctx, ssa_emit_context::vector_extract(ctx,operand, index, esize), R);
			if ((!U))
			{
				result = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, result, S), R);
			}
			X_jit(ctx,Rd,copy_new_raw_size(ctx, result, int64));
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
		ir_operand result = X_jit(ctx,Rn);
		uint64_t O = size == 32ULL ? int32 : size == 64ULL ? int64 : 0;
		{
			VPart_jit(ctx,Rd,part,size,copy_new_raw_size(ctx, copy_new_raw_size(ctx, result, O), int64));
		}
	}
	else
	{
		ir_operand v = V_jit(ctx,Rn);
		ir_operand result = ssa_emit_context::vector_extract(ctx,v, part, size);
		X_jit(ctx,Rd,result);
	}
}

void convert_to_float_gp_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd)
{
	convert_to_float_jit(ctx,sf,ftype,U,Rn,Rd,0ULL);
}

void convert_to_float_vector_scalar_jit(ssa_emit_context* ctx, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	convert_to_float_jit(ctx,sz,sz,U,Rn,Rd,1ULL);
}

void convert_to_float_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)32ULL << (uint64_t)sz);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand operand = V_jit(ctx,Rn);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	uint64_t F = esize == 16ULL ? int16 : esize == 32ULL ? int32 : esize == 64ULL ? int64 : 0;
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element = ssa_emit_context::vector_extract(ctx,operand, e, esize);
			ir_operand working;
			if ((U))
			{
				working = copy_new_raw_size(ctx, ssa_emit_context::convert_to_float(ctx,copy_new_raw_size(ctx, element, F),F,F, 0), int64);
			}
			else
			{
				working = copy_new_raw_size(ctx, ssa_emit_context::convert_to_float(ctx,copy_new_raw_size(ctx, element, F),F,F, 1), int64);
			}
			ssa_emit_context::vector_insert(ctx,result, e, esize, working);
		}
		V_jit(ctx,Rd,result);
	}
}

void shl_immedaite_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)highest_bit_set_c_jit(ctx,immh,32ULL));
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t shift = ((uint64_t)(((uint64_t)(((uint64_t)immh << (uint64_t)3ULL)) | (uint64_t)immb)) - (uint64_t)esize);
	ir_operand operand = V_jit(ctx,Rn);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ir_operand element = ssa_emit_context::vector_extract(ctx,operand, e, esize);
		element = ssa_emit_context::emit_ssa(ctx, ir_shift_left, element, ir_operand::create_con(shift, int64));
		ssa_emit_context::vector_insert(ctx,result, e, esize, element);
	}
	V_jit(ctx,Rd,result);
}

void sshr_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)highest_bit_set_c_jit(ctx,immh,32ULL));
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t shift = ((uint64_t)(((uint64_t)esize * (uint64_t)2ULL)) - (uint64_t)(((uint64_t)(((uint64_t)immh << (uint64_t)3ULL)) | (uint64_t)immb)));
	ir_operand operand = V_jit(ctx,Rn);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	uint64_t O = esize == 8ULL ? int8 : esize == 16ULL ? int16 : esize == 32ULL ? int32 : esize == 64ULL ? int64 : 0;
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element = ssa_emit_context::vector_extract(ctx,operand, e, esize);
			element = shift_right_check_jit(ctx,ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, element, O), int64),ir_operand::create_con(shift, int64),esize,0ULL);
			ssa_emit_context::vector_insert(ctx,result, e, esize, element);
		}
		V_jit(ctx,Rd,result);
	}
}

void shll_shll2_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)highest_bit_set_c_jit(ctx,bits_c_jit(ctx,immh,2ULL,0ULL),32ULL));
	uint64_t datasize = 64ULL;
	uint64_t part = Q;
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t shift = ((uint64_t)(((uint64_t)(((uint64_t)immh << (uint64_t)3ULL)) | (uint64_t)immb)) - (uint64_t)esize);
	ir_operand operand = VPart_jit(ctx,Rn,part,datasize);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	ir_operand working_vector = ssa_emit_context::vector_zero(ctx);
	ssa_emit_context::vector_insert(ctx,working_vector, 0ULL, datasize, operand);
	uint64_t O = esize == 8ULL ? int8 : esize == 16ULL ? int16 : esize == 32ULL ? int32 : esize == 64ULL ? int64 : 0;
	{
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element = ssa_emit_context::vector_extract(ctx,working_vector, e, esize);
			if ((!U))
			{
				element = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, element, O), int64);
			}
			element = ssa_emit_context::emit_ssa(ctx, ir_shift_left, element, ir_operand::create_con(shift, int64));
			ssa_emit_context::vector_insert(ctx,result, e, (((uint64_t)2ULL * (uint64_t)esize)), element);
		}
		V_jit(ctx,Rd,result);
	}
}

void shrn_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)highest_bit_set_c_jit(ctx,bits_c_jit(ctx,immh,2ULL,0ULL),32ULL));
	uint64_t datasize = 64ULL;
	uint64_t part = Q;
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t shift = ((uint64_t)(((uint64_t)2ULL * (uint64_t)esize)) - (uint64_t)(((uint64_t)(((uint64_t)immh << (uint64_t)3ULL)) | (uint64_t)immb)));
	ir_operand operand = V_jit(ctx,Rn);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ir_operand element = shift_right_check_jit(ctx,ssa_emit_context::vector_extract(ctx,operand, e, (((uint64_t)2ULL * (uint64_t)esize))),ir_operand::create_con(shift, int64),((uint64_t)2ULL * (uint64_t)esize),1ULL);
		ssa_emit_context::vector_insert(ctx,result, e, esize, element);
	}
	VPart_jit(ctx,Rd,part,datasize,copy_new_raw_size(ctx, result, int64));
}

void rev64_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd)
{
	uint64_t csize = 64ULL;
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t containers = ((uint64_t)datasize / (uint64_t)csize);
	ir_operand operand = V_jit(ctx,Rn);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t c = 0; c < (containers); c++)
	{
		ir_operand container = ssa_emit_context::vector_extract(ctx,operand, c, csize);
		ssa_emit_context::vector_insert(ctx,result, c, csize, reverse_jit(ctx,copy_new_raw_size(ctx, container, int128),esize,csize));
	}
	V_jit(ctx,Rd,result);
}

void neg_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd)
{
	ir_operand operand = V_jit(ctx,Rn);
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_negate, ssa_emit_context::vector_extract(ctx,operand, e, esize)));
	}
	V_jit(ctx,Rd,result);
}

void not_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rn, uint64_t Rd)
{
	ir_operand operand = V_jit(ctx,Rn);
	uint64_t esize = 8ULL;
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, ssa_emit_context::vector_extract(ctx,operand, e, esize)));
	}
	V_jit(ctx,Rd,result);
}

void abs_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd)
{
	ir_operand operand = V_jit(ctx,Rn);
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint64_t O = esize == 8ULL ? int8 : esize == 16ULL ? int16 : esize == 32ULL ? int32 : esize == 64ULL ? int64 : 0;
		{
			ir_operand working = ssa_emit_context::emit_ssa(ctx, ir_move, copy_new_raw_size(ctx, ssa_emit_context::vector_extract(ctx,operand, e, esize), O));
			{
			        ir_operand end = ir_operation_block::create_label(ctx->ir);
			        ir_operand yes = ir_operation_block::create_label(ctx->ir);
			
			        ir_operand condition = ssa_emit_context::emit_ssa(ctx,ir_bitwise_exclusive_or,ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, working, ir_operand::create_con(0ULL, O)), ir_operand::create_con(1));
			
			        ir_operation_block::jump_if(ctx->ir,end, condition);
			
			    	{
					ssa_emit_context::move(ctx,working,ssa_emit_context::emit_ssa(ctx, ir_negate, working));
				}
			
			        ir_operation_block::mark_label(ctx->ir, end);
			    }
			ssa_emit_context::vector_insert(ctx,result, e, esize, working);
		}
	}
	V_jit(ctx,Rd,result);
}

void mul_vector_index_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t L, uint64_t M, uint64_t Rm, uint64_t H, uint64_t Rn, uint64_t Rd)
{
	uint64_t idxdsize = ((uint64_t)64ULL << (uint64_t)H);
	uint64_t index;
	uint64_t rm_hi;
	if ((((uint64_t)size == (uint64_t)1ULL)))
	{
		index = H;
		index = ((uint64_t)L | (uint64_t)(((uint64_t)index << (uint64_t)1ULL)));
		index = ((uint64_t)M | (uint64_t)(((uint64_t)index << (uint64_t)1ULL)));
		rm_hi = 0ULL;
	}
	else if ((((uint64_t)size == (uint64_t)2ULL)))
	{
		index = H;
		index = ((uint64_t)L | (uint64_t)(((uint64_t)index << (uint64_t)1ULL)));
		rm_hi = M;
	}
	else
	{
		undefined_jit(ctx);
	}
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,((uint64_t)Rm | (uint64_t)(((uint64_t)rm_hi << (uint64_t)4ULL))));
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, index, esize);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
		ir_operand product = ssa_emit_context::emit_ssa(ctx, ir_multiply, element1, element2);
		ssa_emit_context::vector_insert(ctx,result, e, esize, product);
	}
	V_jit(ctx,Rd,result);
}

void mul_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
		ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, e, esize);
		ir_operand working = ssa_emit_context::emit_ssa(ctx, ir_multiply, element1, element2);
		ssa_emit_context::vector_insert(ctx,result, e, esize, working);
	}
	V_jit(ctx,Rd,result);
}

void ext_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t imm4, uint64_t Rn, uint64_t Rd)
{
	ir_operand lo = V_jit(ctx,Rn);
	ir_operand hi = V_jit(ctx,Rm);
	uint64_t vector_size = ((uint64_t)8ULL << (uint64_t)Q);
	uint64_t start = imm4;
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t e = 0; e < (vector_size); e++)
	{
		ir_operand working = copy_new_raw_size(ctx, get_from_concacted_vector_jit(ctx,hi,lo,((uint64_t)start + (uint64_t)e),vector_size,8ULL), int8);
		ssa_emit_context::vector_insert(ctx,result, e, 8ULL, working);
	}
	V_jit(ctx,Rd,result);
}

void compare_above_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t e = 0; e < (elements); e++)
	{
		uint64_t O = esize == 8ULL ? int8 : esize == 16ULL ? int16 : esize == 32ULL ? int32 : esize == 64ULL ? int64 : 0;
		{
			ir_operand element1 = copy_new_raw_size(ctx, ssa_emit_context::vector_extract(ctx,operand1, e, esize), O);
			ir_operand element2 = copy_new_raw_size(ctx, ssa_emit_context::vector_extract(ctx,operand2, e, esize), O);
			ir_operand working;
			if ((U))
			{
				working = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_unsigned, element1, element2);
			}
			else
			{
				working = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_signed, element1, element2);
			}
			ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_subtract, ir_operand::create_con(0ULL, O), working));
		}
	}
	V_jit(ctx,Rd,result);
}

void shl_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t e = 0; e < (elements); e++)
	{
		ir_operand element = ssa_emit_context::emit_ssa(ctx, ir_move, ssa_emit_context::vector_extract(ctx,operand1, e, esize));
		ir_operand shift = ssa_emit_context::emit_ssa(ctx, ir_move, ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, ssa_emit_context::vector_extract(ctx,operand2, e, esize), int8), int64));
		if ((!U))
		{
			uint64_t O = esize == 8ULL ? int8 : esize == 16ULL ? int16 : esize == 32ULL ? int32 : esize == 64ULL ? int64 : 0;
			{
				element = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,copy_new_raw_size(ctx, element, O), int64);
			}
		}
		{
		        ir_operand end = ir_operation_block::create_label(ctx->ir);
		        ir_operand yes = ir_operation_block::create_label(ctx->ir);
		
		        ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_compare_greater_equal_signed, shift, ir_operand::create_con(0ULL, int64));
		
		        ir_operation_block::jump_if(ctx->ir,yes, condition);
		    	{
				ssa_emit_context::move(ctx,shift,ssa_emit_context::emit_ssa(ctx, ir_negate, shift));
				ssa_emit_context::move(ctx,element,shift_right_check_jit(ctx,element,shift,esize,U));
			}
		        
		        ir_operation_block::jump(ctx->ir,end);
		        ir_operation_block::mark_label(ctx->ir, yes);
		
		    	{
				ssa_emit_context::move(ctx,element,shift_left_check_jit(ctx,element,shift,esize));
			}
		
		        ir_operation_block::mark_label(ctx->ir, end);
		    }
		ssa_emit_context::vector_insert(ctx,result, e, esize, element);
	}
	V_jit(ctx,Rd,result);
}

void add_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand result;
	if ((use_x86_sse2_jit(ctx)))
	{
		uint64_t add_instruction;
		if ((((uint64_t)esize == (uint64_t)8ULL)))
		add_instruction = (uint64_t)x86_paddb;
		else if ((((uint64_t)esize == (uint64_t)16ULL)))
		add_instruction = (uint64_t)x86_paddw;
		else if ((((uint64_t)esize == (uint64_t)32ULL)))
		add_instruction = (uint64_t)x86_paddd;
		else if ((((uint64_t)esize == (uint64_t)64ULL)))
		add_instruction = (uint64_t)x86_paddq;
		else
		undefined_jit(ctx);
		result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,add_instruction,operand1,operand2), int128);
		if ((((uint64_t)Q == (uint64_t)0ULL)))
		{
			ssa_emit_context::vector_insert(ctx,result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		}
	}
	else
	{
		result = ssa_emit_context::vector_zero(ctx);
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
			ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, e, esize);
			ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_add, element1, element2));
		}
	}
	V_jit(ctx,Rd,result);
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
		if ((use_x86_jit(ctx)))
		{
			count = copy_new_raw_size(ctx, intrinsic_unary_jit(ctx,int16,(uint64_t)x86_popcnt,copy_new_raw_size(ctx, working, int16)), int8);
		}
		else
		{
			for (uint64_t b = 0; b < (esize); b++)
			{
				ir_operand bit = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, working, ir_operand::create_con(b, int8)), ir_operand::create_con(1ULL, int8));
				{
				        ir_operand end = ir_operation_block::create_label(ctx->ir);
				        ir_operand yes = ir_operation_block::create_label(ctx->ir);
				
				        ir_operand condition = ssa_emit_context::emit_ssa(ctx,ir_bitwise_exclusive_or,bit, ir_operand::create_con(1));
				
				        ir_operation_block::jump_if(ctx->ir,end, condition);
				
				    	{
						ssa_emit_context::move(ctx,count,ssa_emit_context::emit_ssa(ctx, ir_add, count, ir_operand::create_con(1ULL, int8)));
					}
				
				        ir_operation_block::mark_label(ctx->ir, end);
				    }
			}
		}
		ssa_emit_context::vector_insert(ctx,result, e, esize, count);
	}
	V_jit(ctx,Rd,result);
}

void orr_orn_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t invert, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((use_x86_sse_jit(ctx)))
	{
		x86_sse_logic_vector_jit(ctx,Rd,Rn,Rm,Q,invert,(uint64_t)x86_orps);
	}
	else
	{
		uint64_t esize = 64ULL;
		uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
		uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
		ir_operand operand1 = V_jit(ctx,Rn);
		ir_operand operand2 = V_jit(ctx,Rm);
		ir_operand result = ssa_emit_context::vector_zero(ctx);
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
			ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, e, esize);
			if ((invert))
			{
				element2 = ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, element2);
			}
			ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, element1, element2));
		}
		V_jit(ctx,Rd,result);
	}
}

void bsl_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = 64ULL;
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand operand1 = V_jit(ctx,Rm);
	ir_operand operand2 = V_jit(ctx,Rd);
	ir_operand operand3 = V_jit(ctx,Rn);
	if ((use_x86_sse_jit(ctx)))
	{
		ir_operand xor_result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,(uint64_t)x86_xorps,operand1,operand3), int128);
		ir_operand and_result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,(uint64_t)x86_pand,xor_result,operand2), int128);
		ir_operand final_result = copy_new_raw_size(ctx, intrinsic_binary_jit(ctx,int128,(uint64_t)x86_xorps,operand1,and_result), int128);
		if ((((uint64_t)Q == (uint64_t)0ULL)))
		{
			ssa_emit_context::vector_insert(ctx,final_result, 1ULL, 64ULL, ir_operand::create_con(0ULL, int64));
		}
		V_jit(ctx,Rd,final_result);
	}
	else
	{
		ir_operand result = ssa_emit_context::vector_zero(ctx);
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
			ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, e, esize);
			ir_operand element3 = ssa_emit_context::vector_extract(ctx,operand3, e, esize);
			ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, element1, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, element1, element3), element2)));
		}
		V_jit(ctx,Rd,result);
	}
}

void and_bic_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t invert, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((use_x86_sse2_jit(ctx)))
	{
		if ((invert))
		{
			x86_sse_logic_vector_jit(ctx,Rd,Rm,Rn,Q,0ULL,(uint64_t)x86_pandn);
		}
		else
		{
			x86_sse_logic_vector_jit(ctx,Rd,Rn,Rm,Q,0ULL,(uint64_t)x86_pand);
		}
	}
	else
	{
		uint64_t esize = 64ULL;
		uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
		uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
		ir_operand operand1 = V_jit(ctx,Rn);
		ir_operand operand2 = V_jit(ctx,Rm);
		ir_operand result = ssa_emit_context::vector_zero(ctx);
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
			ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, e, esize);
			if ((invert))
			{
				element2 = ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, element2);
			}
			ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, element1, element2));
		}
		V_jit(ctx,Rd,result);
	}
}

void eor_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t Rn, uint64_t Rd)
{
	if ((use_x86_sse_jit(ctx)))
	{
		x86_sse_logic_vector_jit(ctx,Rd,Rn,Rm,Q,0ULL,(uint64_t)x86_xorps);
	}
	else
	{
		uint64_t esize = 64ULL;
		uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
		uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
		ir_operand operand1 = V_jit(ctx,Rn);
		ir_operand operand2 = V_jit(ctx,Rm);
		ir_operand result = ssa_emit_context::vector_zero(ctx);
		for (uint64_t e = 0; e < (elements); e++)
		{
			ir_operand element1 = ssa_emit_context::vector_extract(ctx,operand1, e, esize);
			ir_operand element2 = ssa_emit_context::vector_extract(ctx,operand2, e, esize);
			ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, element1, element2));
		}
		V_jit(ctx,Rd,result);
	}
}

void xnt_xnt2_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = 64ULL;
	uint64_t part = Q;
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	ir_operand operand = V_jit(ctx,Rn);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	ir_operand mask = ir_operand::create_con(-1ULL, int64);
	if ((((uint64_t)esize != (uint64_t)64ULL)))
	{
		mask = ir_operand::create_con(((uint64_t)(((uint64_t)1ULL << (uint64_t)esize)) - (uint64_t)1ULL), int64);
	}
	for (uint64_t e = 0; e < (elements); e++)
	{
		ir_operand element = ssa_emit_context::vector_extract(ctx,operand, e, (((uint64_t)2ULL * (uint64_t)esize)));
		ssa_emit_context::vector_insert(ctx,result, e, esize, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, element, mask));
	}
	VPart_jit(ctx,Rd,part,datasize,copy_new_raw_size(ctx, result, int64));
}

void zip_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t op, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t part = op;
	uint64_t pairs = ((uint64_t)elements / (uint64_t)2ULL);
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	uint64_t base = ((uint64_t)part * (uint64_t)pairs);
	for (uint64_t p = 0; p < (pairs); p++)
	{
		ssa_emit_context::vector_insert(ctx,result, (((uint64_t)((uint64_t)2ULL * (uint64_t)p) + (uint64_t)0ULL)), esize, ssa_emit_context::vector_extract(ctx,operand1, (((uint64_t)base + (uint64_t)p)), esize));
		ssa_emit_context::vector_insert(ctx,result, (((uint64_t)((uint64_t)2ULL * (uint64_t)p) + (uint64_t)1ULL)), esize, ssa_emit_context::vector_extract(ctx,operand2, (((uint64_t)base + (uint64_t)p)), esize));
	}
	V_jit(ctx,Rd,result);
}

void trn_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t op, uint64_t Rn, uint64_t Rd)
{
	uint64_t esize = ((uint64_t)8ULL << (uint64_t)size);
	uint64_t datasize = ((uint64_t)64ULL << (uint64_t)Q);
	uint64_t elements = ((uint64_t)datasize / (uint64_t)esize);
	uint64_t part = op;
	uint64_t pairs = ((uint64_t)elements / (uint64_t)2ULL);
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand result = ssa_emit_context::vector_zero(ctx);
	for (uint64_t p = 0; p < (pairs); p++)
	{
		ssa_emit_context::vector_insert(ctx,result, (((uint64_t)((uint64_t)2ULL * (uint64_t)p) + (uint64_t)0ULL)), esize, ssa_emit_context::vector_extract(ctx,operand1, (((uint64_t)((uint64_t)2ULL * (uint64_t)p) + (uint64_t)part)), esize));
		ssa_emit_context::vector_insert(ctx,result, (((uint64_t)((uint64_t)2ULL * (uint64_t)p) + (uint64_t)1ULL)), esize, ssa_emit_context::vector_extract(ctx,operand2, (((uint64_t)((uint64_t)2ULL * (uint64_t)p) + (uint64_t)part)), esize));
	}
	V_jit(ctx,Rd,result);
}

void tbl_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t len, uint64_t Rn, uint64_t Rd)
{
	store_context_jit(ctx);
	ir_operand vector_context = get_vector_context_jit(ctx);
	call_jit(ctx,vector_context,ir_operand::create_con(Rd, int64),ir_operand::create_con(Rn, int64),ir_operand::create_con(len, int64),ir_operand::create_con(Rm, int64),ir_operand::create_con(Q, int64),(uint64_t)table_lookup_fallback);
	load_context_jit(ctx);
}

void ld1r_no_offset_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	memory_1_jit(ctx,0ULL,Q,1ULL,0ULL,0ULL,0ULL,6ULL,0ULL,size,Rn,Rt,1ULL);
}

void ld1r_post_index_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	memory_1_jit(ctx,1ULL,Q,1ULL,0ULL,Rm,0ULL,6ULL,0ULL,size,Rn,Rt,1ULL);
}

void ld1_single_structure_no_offset_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	memory_1_jit(ctx,0ULL,Q,1ULL,0ULL,0ULL,0ULL,((uint64_t)opcode << (uint64_t)1ULL),S,size,Rn,Rt,1ULL);
}

void ld1_single_structure_post_index_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	memory_1_jit(ctx,1ULL,Q,1ULL,0ULL,Rm,0ULL,((uint64_t)opcode << (uint64_t)1ULL),S,size,Rn,Rt,1ULL);
}

void st2_multiple_structures_no_offset_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	st_jit(ctx,0ULL,Q,0ULL,4ULL,size,0ULL,Rn,Rt);
}

void st2_multiple_structures_post_index_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	st_jit(ctx,1ULL,Q,0ULL,4ULL,size,Rm,Rn,Rt);
}

void st1_single_structure_no_offset_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	memory_1_jit(ctx,0ULL,Q,1ULL,0ULL,0ULL,0ULL,((uint64_t)opcode << (uint64_t)1ULL),S,size,Rn,Rt,0ULL);
}

void st1_single_structure_post_index_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt)
{
	memory_1_jit(ctx,1ULL,Q,1ULL,0ULL,Rm,0ULL,((uint64_t)opcode << (uint64_t)1ULL),S,size,Rn,Rt,0ULL);
}

void floating_point_conditional_select_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t Rd)
{
	uint64_t fltsize = get_flt_size_jit(ctx,ftype);
	ir_operand operand1 = V_jit(ctx,Rn);
	ir_operand operand2 = V_jit(ctx,Rm);
	ir_operand result = ssa_emit_context::emit_ssa(ctx, ir_move, ssa_emit_context::vector_zero(ctx));
	{
	        ir_operand end = ir_operation_block::create_label(ctx->ir);
	        ir_operand yes = ir_operation_block::create_label(ctx->ir);
	
	        ir_operand condition = condition_holds_jit(ctx,cond);
	
	        ir_operation_block::jump_if(ctx->ir,yes, condition);
	    	{
			ssa_emit_context::move(ctx,result,operand2);
		}
	        
	        ir_operation_block::jump(ctx->ir,end);
	        ir_operation_block::mark_label(ctx->ir, yes);
	
	    	{
			ssa_emit_context::move(ctx,result,operand1);
		}
	
	        ir_operation_block::mark_label(ctx->ir, end);
	    }
	result = clear_vector_scalar_jit(ctx,result,fltsize);
	V_jit(ctx,Rd,result);
}

void fcmp_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t opc)
{
	uint64_t datasize = get_flt_size_jit(ctx,ftype);
	uint64_t cmp_with_zero = ((uint64_t)opc == (uint64_t)1ULL);
	ir_operand operand1 = copy_new_raw_size(ctx, V_jit(ctx,Rn), int64);
	ir_operand operand2;
	ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
	if ((cmp_with_zero))
	{
		operand2 = ir_operand::create_con(0ULL, int64);
	}
	else
	{
		operand2 = copy_new_raw_size(ctx, V_jit(ctx,Rm), int64);
	}
	ir_operand nzcv = FPCompare_jit(ctx,operand1,operand2,fpcr_state,datasize);
	_sys_jit(ctx,(uint64_t)nzcv_n,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, nzcv, ir_operand::create_con(3ULL, int64)), ir_operand::create_con(1ULL, int64)));
	_sys_jit(ctx,(uint64_t)nzcv_z,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, nzcv, ir_operand::create_con(2ULL, int64)), ir_operand::create_con(1ULL, int64)));
	_sys_jit(ctx,(uint64_t)nzcv_c,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, nzcv, ir_operand::create_con(1ULL, int64)), ir_operand::create_con(1ULL, int64)));
	_sys_jit(ctx,(uint64_t)nzcv_v,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, nzcv, ir_operand::create_con(0ULL, int64)), ir_operand::create_con(1ULL, int64)));
}

void fccmp_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t nzcv)
{
	uint64_t datasize = get_flt_size_jit(ctx,ftype);
	ir_operand operand1 = copy_new_raw_size(ctx, V_jit(ctx,Rn), int64);
	ir_operand operand2 = copy_new_raw_size(ctx, V_jit(ctx,Rm), int64);
	ir_operand fpcr_state = _sys_jit(ctx,(uint64_t)fpcr);
	{
	        ir_operand end = ir_operation_block::create_label(ctx->ir);
	        ir_operand yes = ir_operation_block::create_label(ctx->ir);
	
	        ir_operand condition = condition_holds_jit(ctx,cond);
	
	        ir_operation_block::jump_if(ctx->ir,yes, condition);
	    	{
			_sys_jit(ctx,(uint64_t)nzcv_n,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)3ULL)) & (uint64_t)1ULL), int64));
			_sys_jit(ctx,(uint64_t)nzcv_z,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)2ULL)) & (uint64_t)1ULL), int64));
			_sys_jit(ctx,(uint64_t)nzcv_c,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)1ULL)) & (uint64_t)1ULL), int64));
			_sys_jit(ctx,(uint64_t)nzcv_v,ir_operand::create_con(((uint64_t)(((uint64_t)nzcv >> (uint64_t)0ULL)) & (uint64_t)1ULL), int64));
		}
	        
	        ir_operation_block::jump(ctx->ir,end);
	        ir_operation_block::mark_label(ctx->ir, yes);
	
	    	{
			ir_operand success_nzcv = FPCompare_jit(ctx,operand1,operand2,fpcr_state,datasize);
			_sys_jit(ctx,(uint64_t)nzcv_n,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, success_nzcv, ir_operand::create_con(3ULL, int64)), ir_operand::create_con(1ULL, int64)));
			_sys_jit(ctx,(uint64_t)nzcv_z,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, success_nzcv, ir_operand::create_con(2ULL, int64)), ir_operand::create_con(1ULL, int64)));
			_sys_jit(ctx,(uint64_t)nzcv_c,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, success_nzcv, ir_operand::create_con(1ULL, int64)), ir_operand::create_con(1ULL, int64)));
			_sys_jit(ctx,(uint64_t)nzcv_v,ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, success_nzcv, ir_operand::create_con(0ULL, int64)), ir_operand::create_con(1ULL, int64)));
		}
	
	        ir_operation_block::mark_label(ctx->ir, end);
	    }
}

