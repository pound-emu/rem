#include "interpreter.h"
template <typename O>
O add_subtract_impl(void* ctx, O n, O m, uint64_t set_flags, uint64_t is_add)
{
	O d;
	if (is_add)
	{
		d = n + m;
	}
	else 
	{
		d = n - m;
	}
	return d;
}

void add_subtract_imm12(void* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t sh, uint64_t imm12, uint64_t Rn, uint64_t Rd)
{
	if (sf == 0ULL)
	{
		uint32_t operand1 = XSP(ctx, Rn);
		uint32_t operand2 = imm12 << sh * 12ULL;
		uint32_t d = add_subtract_impl<uint32_t>(ctx, operand1, operand2, S, op);
		if (S && Rd != 31ULL)
		{
			X(ctx, Rd, d);
		}
		else 
		{
			XSP(ctx, Rd, d);
		}
	}
	if (sf == 1ULL)
	{
		uint64_t operand1 = XSP(ctx, Rn);
		uint64_t operand2 = imm12 << sh * 12ULL;
		uint64_t d = add_subtract_impl<uint64_t>(ctx, operand1, operand2, S, op);
		if (S && Rd != 31ULL)
		{
			X(ctx, Rd, d);
		}
		else 
		{
			XSP(ctx, Rd, d);
		}
	}
}

uint64_t XSP(void* ctx, uint64_t reg_index)
{
	if (reg_index == 31ULL)
	{
		return SP(ctx);
	}
	else 
	{
		return X(ctx, reg_index);
	}
}

void XSP(void* ctx, uint64_t reg_index, uint64_t value)
{
	if (reg_index == 31ULL)
	{
		SP(ctx, value);
	}
	else 
	{
		X(ctx, reg_index, value);
	}
}

