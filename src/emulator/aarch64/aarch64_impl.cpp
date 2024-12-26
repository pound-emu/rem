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

void init_aarch64_decoder(aarch64_process* process)
{
	append_table(process, "---100010-----------------------", (void*)emit_add_subtract_imm12_jit, (void*)call_add_subtract_imm12_interpreter);
	append_table(process, "---100101-----------------------", (void*)emit_move_wide_immediate_jit, (void*)call_move_wide_immediate_interpreter);
	append_table(process, "1101011000011111000000-----00000", (void*)emit_br_jit, (void*)call_br_interpreter);
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
		_sys_interpreter(ctx,0ULL,sign_extend(d) < sign_extend(0ULL));
		_sys_interpreter(ctx,1ULL,d == 0ULL);
		if ((is_add))
		{
			_sys_interpreter(ctx,2ULL,d < n);
			_sys_interpreter(ctx,3ULL,sign_extend(((d ^ n) & ~ (n ^ m))) < sign_extend(0ULL));
		}
		else
		{
			_sys_interpreter(ctx,2ULL,n >= m);
			_sys_interpreter(ctx,3ULL,sign_extend(((d ^ n) & (n ^ m))) < sign_extend(0ULL));
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

void move_wide_immediate_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t hw, uint64_t imm16, uint64_t Rd)
{
	uint64_t shift = hw * 16ULL;
	uint64_t immediate = imm16 << shift;
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
	if ((sf == 0ULL))
	{
		result = (uint32_t)result;
	}
	X_interpreter(ctx,Rd,result);
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

void move_wide_immediate_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t hw, uint64_t imm16, uint64_t Rd)
{
	uint64_t shift = hw * 16ULL;
	uint64_t immediate = imm16 << shift;
	ir_operand result;
	if ((opc == 0ULL))
	{
		result = ir_operand::create_con(~ immediate, int64);
	}
	else if ((opc == 3ULL))
	{
		result = X_jit(ctx,Rd);
		result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_and, result, ir_operand::create_con(~ (65535ULL << shift), int64));
		result = ssa_emit_context::emit_ssa(ctx, ir_bitwise_or, result, ir_operand::create_con(immediate, int64));
	}
	else
	{
		result = ir_operand::create_con(immediate, int64);
	}
	if ((sf == 0ULL))
	{
		result = ir_operand::copy_new_raw_size(ir_operand::copy_new_raw_size(result, int32), int64);
	}
	X_jit(ctx,Rd,result);
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

