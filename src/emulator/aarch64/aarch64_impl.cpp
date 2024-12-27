#include "aarch64_impl.h"
#include "string.h"

static void append_table(aarch64_process* process, std::string encoding, void* emit, void* interperate)
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

	fixed_length_decoder<uint32_t>::insert_entry(&process->decoder, instruction, mask, emit, interperate);
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

void init_aarch64_decoder(aarch64_process* process)
{
	append_table(process, "---100010-----------------------", (void*)emit_add_subtract_imm12_jit, (void*)call_add_subtract_imm12_interpreter);
	append_table(process, "---01011--0---------------------", (void*)emit_add_subtract_shifted_jit, (void*)call_add_subtract_shifted_interpreter);
	append_table(process, "---01011001---------------------", (void*)emit_add_subtract_extended_jit, (void*)call_add_subtract_extended_interpreter);
	append_table(process, "---01010------------------------", (void*)emit_logical_shifted_jit, (void*)call_logical_shifted_interpreter);
	append_table(process, "---100101-----------------------", (void*)emit_move_wide_immediate_jit, (void*)call_move_wide_immediate_interpreter);
	append_table(process, "1101011000011111000000-----00000", (void*)emit_br_jit, (void*)call_br_interpreter);
	append_table(process, "---10000------------------------", (void*)emit_pc_rel_addressing_jit, (void*)call_pc_rel_addressing_interpreter);
	append_table(process, "-0011010110-----00001-----------", (void*)emit_divide_jit, (void*)call_divide_interpreter);
}

uint64_t decode_add_subtract_imm_12_interpreter(interpreter_data* ctx, uint64_t source, uint64_t shift)
{
	return source << (shift * 12ULL);
}

template <typename O>
O add_subtract_impl_interpreter(interpreter_data* ctx, O n, O m, uint64_t set_flags, uint64_t is_add)
{
	O d;
	if ((is_add))
	{
		d = n + m;
	}
	else
	{
		d = n - m;
	}
	if ((set_flags))
	{
		_sys_interpreter(ctx,0ULL,(O)(sign_extend(d) < sign_extend(0ULL)));
		_sys_interpreter(ctx,1ULL,d == 0ULL);
		if ((is_add))
		{
			_sys_interpreter(ctx,2ULL,d < n);
			_sys_interpreter(ctx,3ULL,(O)(sign_extend(((d ^ n) & ~ (n ^ m))) < sign_extend(0ULL)));
		}
		else
		{
			_sys_interpreter(ctx,2ULL,n >= m);
			_sys_interpreter(ctx,3ULL,(O)(sign_extend(((d ^ n) & (n ^ m))) < sign_extend(0ULL)));
		}
	}
	return d;
}

void add_subtract_imm12_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t sh, uint64_t imm12, uint64_t Rn, uint64_t Rd)
{
	if (sf == 0ULL)
	{
		uint32_t operand1 = XSP_interpreter(ctx,Rn);
		uint32_t operand2 = decode_add_subtract_imm_12_interpreter(ctx,imm12,sh);
		uint32_t d = add_subtract_impl_interpreter<uint32_t>(ctx,operand1,operand2,S,op == 0ULL);
		if ((S))
		{
			X_interpreter(ctx,Rd,d);
		}
		else
		{
			XSP_interpreter(ctx,Rd,d);
		}
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = XSP_interpreter(ctx,Rn);
		uint64_t operand2 = decode_add_subtract_imm_12_interpreter(ctx,imm12,sh);
		uint64_t d = add_subtract_impl_interpreter<uint64_t>(ctx,operand1,operand2,S,op == 0ULL);
		if ((S))
		{
			X_interpreter(ctx,Rd,d);
		}
		else
		{
			XSP_interpreter(ctx,Rd,d);
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
		uint32_t result = add_subtract_impl_interpreter<uint32_t>(ctx,operand1,operand2,S,op == 0ULL);
		X_interpreter(ctx,Rd,result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = a_shift_reg_interpreter<uint64_t>(ctx,Rm,shift,shift_ammount);
		uint64_t result = add_subtract_impl_interpreter<uint64_t>(ctx,operand1,operand2,S,op == 0ULL);
		X_interpreter(ctx,Rd,result);
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
		uint32_t result = add_subtract_impl_interpreter<uint32_t>(ctx,operand1,operand2,S,op == 0ULL);
		if ((S))
		{
			X_interpreter(ctx,Rd,result);
		}
		else
		{
			XSP_interpreter(ctx,Rd,result);
		}
	}
	if (sf == 1ULL)
	{
		uint64_t shift = imm3;
		uint64_t extend_type = option;
		uint64_t operand1 = XSP_interpreter(ctx,Rn);
		uint64_t operand2 = a_extend_reg_interpreter<uint64_t>(ctx,Rm,extend_type,shift);
		uint64_t result = add_subtract_impl_interpreter<uint64_t>(ctx,operand1,operand2,S,op == 0ULL);
		if ((S))
		{
			X_interpreter(ctx,Rd,result);
		}
		else
		{
			XSP_interpreter(ctx,Rd,result);
		}
	}
	
}

uint64_t sign_extend_interpreter(interpreter_data* ctx, uint64_t source, uint64_t count)
{
	uint64_t max = 64ULL;
	uint64_t shift = max - count;
	return (uint64_t)(sign_extend((source << shift)) >> sign_extend(shift));
}

template <typename O>
O a_shift_reg_interpreter(interpreter_data* ctx, uint64_t m, uint64_t shift_type, uint64_t ammount)
{
	O result = X_interpreter(ctx,m);
	if ((shift_type == 0ULL))
	{
		return result << ammount;
	}
	else if ((shift_type == 1ULL))
	{
		return result >> ammount;
	}
	else if ((shift_type == 2ULL))
	{
		return (O)(sign_extend(result) >> sign_extend(ammount));
	}
	else
	{
		return rotate_right(result,ammount);
	}
}

template <typename O>
O a_extend_reg_interpreter(interpreter_data* ctx, uint64_t m, uint64_t extend_type, uint64_t shift)
{
	O val = X_interpreter(ctx,m);
	if ((extend_type == 0ULL))
	{
		val = val & 255ULL;
	}
	else if ((extend_type == 1ULL))
	{
		val = val & 65535ULL;
	}
	else if ((extend_type == 2ULL))
	{
		val = val & 4294967295ULL;
	}
	else if ((extend_type == 4ULL))
	{
		val = (O)sign_extend((uint8_t)val);
	}
	else if ((extend_type == 5ULL))
	{
		val = (O)sign_extend((uint16_t)val);
	}
	else if ((extend_type == 6ULL))
	{
		val = (O)sign_extend((uint32_t)val);
	}
	return val << shift;
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
			operand2 = ~ operand2;
		}
		uint32_t result;
		if ((opc == 0ULL || opc == 3ULL))
		{
			result = operand1 & operand2;
			if ((opc == 3ULL))
			{
				_sys_interpreter(ctx,0ULL,(uint32_t)(sign_extend(result) < sign_extend(0ULL)));
				_sys_interpreter(ctx,1ULL,result == 0ULL);
				_sys_interpreter(ctx,2ULL,0ULL);
				_sys_interpreter(ctx,3ULL,0ULL);
			}
		}
		else if ((opc == 1ULL))
		{
			result = operand1 | operand2;
		}
		else if ((opc == 2ULL))
		{
			result = operand1 ^ operand2;
		}
		X_interpreter(ctx,Rd,result);
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = a_shift_reg_interpreter<uint64_t>(ctx,Rm,shift_type,shift_ammount);
		if ((N))
		{
			operand2 = ~ operand2;
		}
		uint64_t result;
		if ((opc == 0ULL || opc == 3ULL))
		{
			result = operand1 & operand2;
			if ((opc == 3ULL))
			{
				_sys_interpreter(ctx,0ULL,(uint64_t)(sign_extend(result) < sign_extend(0ULL)));
				_sys_interpreter(ctx,1ULL,result == 0ULL);
				_sys_interpreter(ctx,2ULL,0ULL);
				_sys_interpreter(ctx,3ULL,0ULL);
			}
		}
		else if ((opc == 1ULL))
		{
			result = operand1 | operand2;
		}
		else if ((opc == 2ULL))
		{
			result = operand1 ^ operand2;
		}
		X_interpreter(ctx,Rd,result);
	}
	
}

void move_wide_immediate_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t hw, uint64_t imm16, uint64_t Rd)
{
	uint64_t shift = hw * 16ULL;
	uint64_t immediate = imm16 << shift;
	if (sf == 0ULL)
	{
		uint32_t result;
		if ((opc == 0ULL))
		{
			result = ~ immediate;
		}
		else if ((opc == 3ULL))
		{
			result = X_interpreter(ctx,Rd);
			result = result & ~ (65535ULL << shift);
			result = result | immediate;
		}
		else
		{
			result = immediate;
		}
		X_interpreter(ctx,Rd,result);
	}
	if (sf == 1ULL)
	{
		uint64_t result;
		if ((opc == 0ULL))
		{
			result = ~ immediate;
		}
		else if ((opc == 3ULL))
		{
			result = X_interpreter(ctx,Rd);
			result = result & ~ (65535ULL << shift);
			result = result | immediate;
		}
		else
		{
			result = immediate;
		}
		X_interpreter(ctx,Rd,result);
	}
	
}

void branch_long_universal_interpreter(interpreter_data* ctx, uint64_t Rn, uint64_t link)
{
	uint64_t branch_location = X_interpreter(ctx,Rn);
	if ((link))
	{
		uint64_t link_address = _get_pc_interpreter(ctx) + 4ULL;
		X_interpreter(ctx,30ULL,link_address);
	}
	_branch_long_interpreter(ctx,branch_location);
}

void br_interpreter(interpreter_data* ctx, uint64_t Rn)
{
	branch_long_universal_interpreter(ctx,Rn,0ULL);
}

void pc_rel_addressing_interpreter(interpreter_data* ctx, uint64_t op, uint64_t immlo, uint64_t immhi, uint64_t Rd)
{
	uint64_t offset = sign_extend_interpreter(ctx,(immhi << 2ULL) | immlo,21ULL);
	uint64_t instruction_pc = _get_pc_interpreter(ctx);
	if ((op))
	{
		offset = offset << 12ULL;
		instruction_pc = instruction_pc & 4095ULL;
	}
	X_interpreter(ctx,Rd,instruction_pc + offset);
}

void divide_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t Rm, uint64_t o1, uint64_t Rn, uint64_t Rd)
{
	uint64_t is_signed = o1 == 1ULL;
	if (sf == 0ULL)
	{
		uint32_t operand1 = X_interpreter(ctx,Rn);
		uint32_t operand2 = X_interpreter(ctx,Rm);
		uint32_t result;
		if ((operand2 == 0ULL))
		{
			X_interpreter(ctx,Rd,0ULL);
		}
		else
		{
			if ((is_signed))
			{
				uint64_t min = 9223372036854775808ULL;
				uint64_t neg_one = 18446744073709551615ULL;
				if ((! sf))
				{
					min = min >> 32ULL;
					neg_one = neg_one >> 32ULL;
				}
				if ((operand1 == min && operand2 == (uint32_t)18446744073709551615ULL))
				{
					X_interpreter(ctx,Rd,min);
				}
				else
				{
					X_interpreter(ctx,Rd,(uint32_t)(sign_extend(operand1) / sign_extend(operand2)));
				}
			}
			else
			{
				X_interpreter(ctx,Rd,operand1 / operand2);
			}
		}
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = X_interpreter(ctx,Rn);
		uint64_t operand2 = X_interpreter(ctx,Rm);
		uint64_t result;
		if ((operand2 == 0ULL))
		{
			X_interpreter(ctx,Rd,0ULL);
		}
		else
		{
			if ((is_signed))
			{
				uint64_t min = 9223372036854775808ULL;
				uint64_t neg_one = 18446744073709551615ULL;
				if ((! sf))
				{
					min = min >> 32ULL;
					neg_one = neg_one >> 32ULL;
				}
				if ((operand1 == min && operand2 == (uint64_t)18446744073709551615ULL))
				{
					X_interpreter(ctx,Rd,min);
				}
				else
				{
					X_interpreter(ctx,Rd,(uint64_t)(sign_extend(operand1) / sign_extend(operand2)));
				}
			}
			else
			{
				X_interpreter(ctx,Rd,operand1 / operand2);
			}
		}
	}
	
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
	if ((reg_id == 31ULL))
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
	if ((reg_id == 31ULL))
	{
		return;
	}
	else
	{
		_x_interpreter(ctx,reg_id,value);
	}
}

uint64_t decode_add_subtract_imm_12_jit(ssa_emit_context* ctx, uint64_t source, uint64_t shift)
{
	return source << (shift * 12ULL);
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
		_sys_jit(ctx,0ULL,ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, d, ir_operand::create_con(0ULL, O)));
		_sys_jit(ctx,1ULL,ssa_emit_context::emit_ssa(ctx, ir_compare_equal, d, ir_operand::create_con(0ULL, O)));
		if ((is_add))
		{
			_sys_jit(ctx,2ULL,ssa_emit_context::emit_ssa(ctx, ir_compare_less_unsigned, d, n));
			_sys_jit(ctx,3ULL,ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, d, n), ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, n, m))), ir_operand::create_con(0ULL, O)));
		}
		else
		{
			_sys_jit(ctx,2ULL,ssa_emit_context::emit_ssa(ctx, ir_compare_greater_equal_unsigned, n, m));
			_sys_jit(ctx,3ULL,ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, d, n), ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, n, m)), ir_operand::create_con(0ULL, O)));
		}
	}
	return d;
}

void add_subtract_imm12_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t sh, uint64_t imm12, uint64_t Rn, uint64_t Rd)
{
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = ir_operand::copy_new_raw_size(XSP_jit(ctx,Rn), O);
		ir_operand operand2 = ir_operand::create_con(decode_add_subtract_imm_12_jit(ctx,imm12,sh), O);
		ir_operand d = ir_operand::copy_new_raw_size(add_subtract_impl_jit(ctx,O,operand1,operand2,S,op == 0ULL), O);
		if ((S))
		{
			X_jit(ctx,Rd,d);
		}
		else
		{
			XSP_jit(ctx,Rd,d);
		}
	}
}

void add_subtract_shifted_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t shift, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd)
{
	uint64_t shift_ammount = imm6;
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = ir_operand::copy_new_raw_size(X_jit(ctx,Rn), O);
		ir_operand operand2 = ir_operand::copy_new_raw_size(a_shift_reg_jit(ctx,O,Rm,shift,shift_ammount), O);
		ir_operand result = ir_operand::copy_new_raw_size(add_subtract_impl_jit(ctx,O,operand1,operand2,S,op == 0ULL), O);
		X_jit(ctx,Rd,result);
	}
}

void add_subtract_extended_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t option, uint64_t imm3, uint64_t Rn, uint64_t Rd)
{
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		uint64_t shift = imm3;
		uint64_t extend_type = option;
		ir_operand operand1 = ir_operand::copy_new_raw_size(XSP_jit(ctx,Rn), O);
		ir_operand operand2 = ir_operand::copy_new_raw_size(a_extend_reg_jit(ctx,O,Rm,extend_type,shift), O);
		ir_operand result = ir_operand::copy_new_raw_size(add_subtract_impl_jit(ctx,O,operand1,operand2,S,op == 0ULL), O);
		if ((S))
		{
			X_jit(ctx,Rd,result);
		}
		else
		{
			XSP_jit(ctx,Rd,result);
		}
	}
}

uint64_t sign_extend_jit(ssa_emit_context* ctx, uint64_t source, uint64_t count)
{
	uint64_t max = 64ULL;
	uint64_t shift = max - count;
	return (uint64_t)(sign_extend((source << shift)) >> sign_extend(shift));
}

ir_operand a_shift_reg_jit(ssa_emit_context* ctx,uint64_t O, uint64_t m, uint64_t shift_type, uint64_t ammount)
{
	ir_operand result = ir_operand::copy_new_raw_size(X_jit(ctx,m), O);
	if ((shift_type == 0ULL))
	{
		return ssa_emit_context::emit_ssa(ctx, ir_shift_left, result, ir_operand::create_con(ammount, O));
	}
	else if ((shift_type == 1ULL))
	{
		return ssa_emit_context::emit_ssa(ctx, ir_shift_right_unsigned, result, ir_operand::create_con(ammount, O));
	}
	else if ((shift_type == 2ULL))
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
	ir_operand val = ir_operand::copy_new_raw_size(X_jit(ctx,m), O);
	if ((extend_type == 0ULL))
	{
		val = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, val, ir_operand::create_con(255ULL, O));
	}
	else if ((extend_type == 1ULL))
	{
		val = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, val, ir_operand::create_con(65535ULL, O));
	}
	else if ((extend_type == 2ULL))
	{
		val = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, val, ir_operand::create_con(4294967295ULL, O));
	}
	else if ((extend_type == 4ULL))
	{
		val = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,ir_operand::copy_new_raw_size(val, int8), O);
	}
	else if ((extend_type == 5ULL))
	{
		val = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,ir_operand::copy_new_raw_size(val, int16), O);
	}
	else if ((extend_type == 6ULL))
	{
		val = ssa_emit_context::emit_ssa(ctx,ir_sign_extend,ir_operand::copy_new_raw_size(val, int32), O);
	}
	return ssa_emit_context::emit_ssa(ctx, ir_shift_left, val, ir_operand::create_con(shift, O));
}

void logical_shifted_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t shift, uint64_t N, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd)
{
	uint64_t shift_type = shift;
	uint64_t shift_ammount = imm6;
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = ir_operand::copy_new_raw_size(X_jit(ctx,Rn), O);
		ir_operand operand2 = ir_operand::copy_new_raw_size(a_shift_reg_jit(ctx,O,Rm,shift_type,shift_ammount), O);
		if ((N))
		{
			operand2 = ssa_emit_context::emit_ssa(ctx, ir_bitwise_not, operand2);
		}
		ir_operand result;
		if ((opc == 0ULL || opc == 3ULL))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, operand1, operand2);
			if ((opc == 3ULL))
			{
				_sys_jit(ctx,0ULL,ssa_emit_context::emit_ssa(ctx, ir_compare_less_signed, result, ir_operand::create_con(0ULL, O)));
				_sys_jit(ctx,1ULL,ssa_emit_context::emit_ssa(ctx, ir_compare_equal, result, ir_operand::create_con(0ULL, O)));
				_sys_jit(ctx,2ULL,ir_operand::create_con(0ULL, int64));
				_sys_jit(ctx,3ULL,ir_operand::create_con(0ULL, int64));
			}
		}
		else if ((opc == 1ULL))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, operand1, operand2);
		}
		else if ((opc == 2ULL))
		{
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_exclusive_or, operand1, operand2);
		}
		X_jit(ctx,Rd,result);
	}
}

void move_wide_immediate_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t hw, uint64_t imm16, uint64_t Rd)
{
	uint64_t shift = hw * 16ULL;
	uint64_t immediate = imm16 << shift;
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand result;
		if ((opc == 0ULL))
		{
			result = ir_operand::create_con(~ immediate, O);
		}
		else if ((opc == 3ULL))
		{
			result = ir_operand::copy_new_raw_size(X_jit(ctx,Rd), O);
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, result, ir_operand::create_con(~ (65535ULL << shift), O));
			result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, result, ir_operand::create_con(immediate, O));
		}
		else
		{
			result = ir_operand::create_con(immediate, O);
		}
		X_jit(ctx,Rd,result);
	}
}

void branch_long_universal_jit(ssa_emit_context* ctx, uint64_t Rn, uint64_t link)
{
	ir_operand branch_location = X_jit(ctx,Rn);
	if ((link))
	{
		ir_operand link_address = ir_operand::create_con(_get_pc_jit(ctx) + 4ULL, int64);
		X_jit(ctx,30ULL,link_address);
	}
	_branch_long_jit(ctx,branch_location);
}

void br_jit(ssa_emit_context* ctx, uint64_t Rn)
{
	branch_long_universal_jit(ctx,Rn,0ULL);
}

void pc_rel_addressing_jit(ssa_emit_context* ctx, uint64_t op, uint64_t immlo, uint64_t immhi, uint64_t Rd)
{
	uint64_t offset = sign_extend_jit(ctx,(immhi << 2ULL) | immlo,21ULL);
	uint64_t instruction_pc = _get_pc_jit(ctx);
	if ((op))
	{
		offset = offset << 12ULL;
		instruction_pc = instruction_pc & 4095ULL;
	}
	X_jit(ctx,Rd,ir_operand::create_con(instruction_pc + offset, int64));
}

void divide_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rm, uint64_t o1, uint64_t Rn, uint64_t Rd)
{
	uint64_t is_signed = o1 == 1ULL;
	uint64_t O = sf == 0ULL ? int32 : sf == 1ULL ? int64 : throw 0;
	{
		ir_operand operand1 = ir_operand::copy_new_raw_size(X_jit(ctx,Rn), O);
		ir_operand operand2 = ir_operand::copy_new_raw_size(X_jit(ctx,Rm), O);
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
					uint64_t neg_one = 18446744073709551615ULL;
					if ((! sf))
					{
						min = min >> 32ULL;
						neg_one = neg_one >> 32ULL;
					}
					{
					    ir_operand end = ir_operation_block::create_label(ctx->ir);
					    ir_operand yes = ir_operation_block::create_label(ctx->ir);
					
					    ir_operand condition = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, ssa_emit_context::emit_ssa(ctx, ir_compare_equal, operand1, ir_operand::create_con(min, O)), ssa_emit_context::emit_ssa(ctx, ir_compare_equal, operand2, ir_operand::create_con(18446744073709551615ULL, O)));
					
					    ir_operation_block::jump_if(ctx->ir,yes, condition);
						{
							X_jit(ctx,Rd,ssa_emit_context::emit_ssa(ctx, ir_divide_signed, operand1, operand2));
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
					X_jit(ctx,Rd,ssa_emit_context::emit_ssa(ctx, ir_divide_unsigned, operand1, operand2));
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
	if ((reg_id == 31ULL))
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
	if ((reg_id == 31ULL))
	{
		return;
	}
	else
	{
		_x_jit(ctx,reg_id,value);
	}
}

