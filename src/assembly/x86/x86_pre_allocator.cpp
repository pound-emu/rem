#include "x86_pre_allocator.h"
#include "ir/bit_register_allocations.h"
#include "ir/checks.h"
#include "tools/bit_tools.h"
#include "debugging.h"

static uint64_t get_context_size(uint64_t value)
{
	return (value + 32) & ~0b1111ULL;
}

static void emit_move(x86_pre_allocator_context* context, ir_operand destination, ir_operand source)
{
	assert_same_size({ destination, source });

	if (ir_operand::are_equal(destination, source) && ir_operand::get_raw_size(&destination) >= int64)
	{
		return;
	}

	if (ir_operand::get_raw_size(&destination) != ir_operand::get_raw_size(&source))
	{
		ir_operation_block::log(context->source_ir);

		throw_error();
	}

	assert(ir_operand::get_raw_size(&destination) == ir_operand::get_raw_size(&source));
	assert(!ir_operand::is_constant(&destination));

	ir_operation_block::emitds(context->ir, ir_instructions::ir_move, destination, source);
}

static ir_operand create_scrap_operand(x86_pre_allocator_context* context, uint64_t size)
{
	uint64_t result = context->scrap_index | bit_register_allocations::scrap_allocation;

	context->scrap_index++;

	return ir_operand::create_reg(result, size & UINT32_MAX);
}

static ir_operand copy_register(x86_pre_allocator_context* result, ir_operand source)
{
	assert_is_register(source);

	ir_operand destination = create_scrap_operand(result, source.meta_data);

	emit_move(result, destination, source);
	
	return destination;
}

static ir_operand register_or_constant(x86_pre_allocator_context* context, ir_operand* source, bool force_copy = false)
{
	if (ir_operand::is_constant(source))
	{
		ir_operand working_result = create_scrap_operand(context, source->meta_data);

		emit_move(context, working_result, *source);

		return working_result;
	}
	
	if (!force_copy)
		return *source;

	ir_operand copy = create_scrap_operand(context, source->meta_data);
	
	emit_move(context, copy, *source);

	return copy;
}

static ir_operand register_or_constant(x86_pre_allocator_context* context, ir_operand source, bool force_copy = false)
{
	return register_or_constant(context, &source, force_copy);
}

static ir_operand vector_from_register(x86_pre_allocator_context* context, ir_operand source)
{
	source = register_or_constant(context, source);
	ir_operand result = create_scrap_operand(context, int128);
	
	ir_operation_block::emitds(context->ir, x86_movq_to_vec, result, source);

	return result;
}

static void emit_throw_exception(x86_pre_allocator_context* context)
{
	ir_operand zero = register_or_constant(context, ir_operand::create_con(0));

	//TODO in the future, i want this to throw an actual exception
	//instead of reading a null pointer LOL
	ir_operation_block::emitds(context->ir, ir_load, zero, zero);
}

static void assert_binary_condition_true(x86_pre_allocator_context* context, ir_instructions condition, ir_operand x, ir_operand y)
{
	assert_same_size({x, y});

	ir_operand condition_result = create_scrap_operand(context, x.meta_data);
	
	x = register_or_constant(context, &x);	
	y = register_or_constant(context, &y);

	ir_operation_block::emitds(context->ir, condition, condition_result, x, y);

	ir_operand end = ir_operation_block::create_label(context->ir);

	ir_operation_block::jump_if(context->ir, end, condition_result);

	emit_throw_exception(context);

	ir_operation_block::mark_label(context->ir, end);
}

static void extend_source(x86_pre_allocator_context* context,ir_operand* source, uint64_t new_size, bool is_signed)
{
	ir_operation_block* ir = context->ir;

	ir_operand temp_location = create_scrap_operand(context, new_size);

	if (is_signed)
	{
		ir_operation_block::emitds(ir, ir_sign_extend, temp_location, *source);
	}
	else
	{
		emit_move(context, temp_location, ir_operand::copy_new_raw_size(*source, new_size));
		ir_operation_block::emitds(ir, ir_bitwise_and, temp_location, temp_location, ir_operand::create_con(255));	
	}

	*source = temp_location;	
}

static void emit_conditional_select(x86_pre_allocator_context* result, ir_operand destination, ir_operand condition, ir_operand source_0, ir_operand source_1)
{
	ir_operand working_destination = destination;

	ir_operand working_condition = register_or_constant(result, &condition);

	ir_operand working_source_0 = register_or_constant(result, &source_0);
	ir_operand working_source_1 = register_or_constant(result, &source_1);

	assert_same_size({ working_destination, working_condition, working_source_0, working_source_1 });

	bool extend_result_8 = false;

	if (working_destination.meta_data == int8)
	{
		working_destination.meta_data = int16;
		working_source_0.meta_data = int16;
		working_source_1.meta_data = int16;
	
		//We don't have to change working_condition's size!

		extend_result_8 = true;
	}

	ir_operation_block::emitds(result->ir, ir_conditional_select, working_destination, working_condition, working_source_0, working_source_1);

	if (extend_result_8)
	{
		working_destination = ir_operand::copy_new_raw_size(working_destination, int64);

		ir_operation_block::emitds(result->ir, ir_bitwise_and, working_destination, working_destination, ir_operand::create_con(255));
	}
}

static bool instruction_is_commutative(uint64_t instruction)
{
	switch (instruction)
	{
		case ir_add:
		case ir_multiply:
		case ir_bitwise_and:
		case ir_bitwise_or:
		case ir_bitwise_exclusive_or:
		{
			return true;
		};
	}

	return false;
}

static void swap(ir_operand* l, ir_operand* r)
{
	ir_operand tmp = *l;

	*l = *r;
	*r = tmp;
}

static void emit_d_n_f_d_n_m(x86_pre_allocator_context* result, ir_instructions instruction, ir_operand destination, ir_operand source_0, ir_operand source_1)
{
	assert_same_size({ destination, source_0, source_1 });

	if (instruction_is_commutative(instruction) && ir_operand::is_constant(&source_0) && !ir_operand::is_constant(&source_1))
	{
		swap(&source_0, &source_1);	
	}

	if ((ir_operand::is_constant(&source_1) && source_1.value < INT32_MAX) && !ir_operand::is_constant(&source_0))
	{
		if (!ir_operand::are_equal(destination, source_0))
		{
			emit_move(result,destination, source_0);
		}

		ir_operation_block::emitds(result->ir, instruction, destination, destination, source_1);

		return;
	}

	ir_operand working_destination = destination;
	ir_operand working_source_0 = register_or_constant(result, &source_0);
	ir_operand working_source_1 = register_or_constant(result, &source_1);

	if (ir_operand::are_equal(working_destination, working_source_0))
	{
		ir_operation_block::emitds(result->ir, instruction, working_destination, working_destination, working_source_1);
	}
	else if (instruction == ir_add && ir_operand::get_raw_size(&working_destination) > int16)
	{
		ir_operation_block::emitds(result->ir, x86_lea, working_destination, working_source_0, working_source_1);
	}
	else if (!ir_operand::are_equal(working_destination, working_source_1))
	{
		emit_move(result,destination, source_0);
		ir_operation_block::emitds(result->ir, instruction, working_destination, working_destination, working_source_1);
	}
	else
	{
		ir_operand working_scrap = create_scrap_operand(result, destination.meta_data);

		emit_move(result, working_scrap, working_source_0);
		ir_operation_block::emitds(result->ir, instruction, working_scrap, working_scrap, working_source_1);
		
		emit_move(result, working_destination, working_scrap);
	}
}

static void emit_floating_point_compare(x86_pre_allocator_context* result, uint64_t instruction, ir_operand destination, ir_operand source_0, ir_operand source_1)
{
	assert_same_size({ destination, source_0, source_1 });

	ir_operand working_destination = destination;

	ir_operand working_vector_0 = vector_from_register(result, source_0);
	ir_operand working_vector_1 = vector_from_register(result, source_1);

	ir_operation_block::emitds(result->ir, instruction, working_destination, working_vector_0, working_vector_1);
}

static void emit_floating_point_binary(x86_pre_allocator_context* result, uint64_t instruction, ir_operand destination, ir_operand source_0, ir_operand source_1)
{
	assert_same_size({ destination, source_0, source_1 });

	ir_operand working_destination = destination;
	ir_operand working_source_0 = register_or_constant(result, &source_0);
	ir_operand working_source_1 = register_or_constant(result, &source_1);

	ir_operand working_vector_0 = vector_from_register(result, working_source_0);
	ir_operand working_vector_1 = vector_from_register(result, working_source_1);

	int working_instruction;

	switch (instruction)
	{
		case ir_floating_point_add: 		working_instruction = ir_operand::get_raw_size(&destination) == int64 ? x86_addsd : x86_addss; break;
		case ir_floating_point_subtract: 	working_instruction = ir_operand::get_raw_size(&destination) == int64 ? x86_subsd : x86_subss; break;
		case ir_floating_point_multiply: 	working_instruction = ir_operand::get_raw_size(&destination) == int64 ? x86_mulsd : x86_mulss; break;
		case ir_floating_point_divide: 		working_instruction = ir_operand::get_raw_size(&destination) == int64 ? x86_divsd : x86_divss; break;
		case ir_floating_point_select_min:	working_instruction = ir_operand::get_raw_size(&destination) == int64 ? x86_minsd : x86_minss; break;
		case ir_floating_point_select_max:	working_instruction = ir_operand::get_raw_size(&destination) == int64 ? x86_maxsd : x86_maxss; break;

		default: throw_error();
	}

	ir_operation_block::emitds(result->ir, working_instruction, working_vector_0, working_vector_0, working_vector_1);

	ir_operation_block::emitds(result->ir, x86_movq_to_gp, working_destination, working_vector_0);
}

static void emit_floating_point_unary(x86_pre_allocator_context* result, uint64_t instruction, ir_operand destination, ir_operand source_0)
{
	assert_same_size({ destination, source_0 });

	ir_operand working_destination = destination;
	ir_operand working_source_0 = register_or_constant(result, &source_0);

	ir_operand working_vector_0 = vector_from_register(result, working_source_0);

	int working_instruction;

	switch (instruction)
	{
		case ir_floating_point_square_root:	working_instruction = ir_operand::get_raw_size(&destination) == int64 ? x86_sqrtsd : x86_sqrtss; break;
		default: throw_error();
	}

	ir_operation_block::emitds(result->ir, working_instruction, working_vector_0, working_vector_0);

	ir_operation_block::emitds(result->ir, x86_movq_to_gp, working_destination, working_vector_0);
}

static void emit_compare(x86_pre_allocator_context* result, uint64_t instruction, ir_operand destination, ir_operand source_0, ir_operand source_1)
{
	assert_same_size({ destination, source_0, source_1 });

	ir_operand working_destination = destination;

	ir_operand working_source_0 = register_or_constant(result, &source_0);
	ir_operand working_source_1 = register_or_constant(result, &source_1);

	ir_operation_block::emitds(result->ir, instruction, working_destination, working_source_0, working_source_1);
}

static void emit_shift(x86_pre_allocator_context* result, uint64_t instruction, ir_operand destination, ir_operand source_0, ir_operand source_1)
{
	assert_same_size({ destination, source_0, source_1 });

	if ((ir_operand::is_constant(&source_1) && source_1.value < 255) && !ir_operand::is_constant(&source_0))
	{
		if (!ir_operand::are_equal(destination, source_0))
		{
			emit_move(result,destination, source_0);
		}

		ir_operation_block::emitds(result->ir, instruction, destination, destination, source_1);

		return;
	}

	ir_operand working_destination = destination;
	ir_operand working_source_0 = register_or_constant(result, &source_0);
	ir_operand working_source_1 = register_or_constant(result, &source_1);

	ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_lock, RCX(ir_operand_meta::int64));

	emit_move(result, RCX(destination.meta_data), working_source_1);
	emit_move(result, working_destination, working_source_0);
	ir_operation_block::emitds(result->ir, instruction, working_destination, working_destination, RCX(ir_operand_meta::int8));

	ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_unlock, RCX(ir_operand_meta::int64));
}

static void emit_double_shift_right(x86_pre_allocator_context* result, ir_operand destination, ir_operand source_0, ir_operand source_1, ir_operand shift)
{
	assert_same_size({ destination, source_0, source_1, shift });

	ir_operand working_destination = destination;
	ir_operand working_source_0 = register_or_constant(result, &source_0);
	ir_operand working_source_1 = register_or_constant(result, &source_1);
	ir_operand working_shift = register_or_constant(result, &shift);

	ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_lock, RCX(ir_operand_meta::int64));

	ir_operand working_scrap = create_scrap_operand(result, working_destination.meta_data);
	emit_move(result, working_scrap, working_source_0);

	emit_move(result, RCX(working_scrap.meta_data), working_shift);

	if (ir_operand::get_raw_size(&destination) == int8)
	{
		working_destination.meta_data = int16;
		working_source_0.meta_data = int16;
		working_source_1.meta_data = int16;
		working_shift.meta_data = int16;
		working_scrap.meta_data = int16;

		extend_source(result, &working_source_0, int16, false);

		emit_move(result, working_scrap, working_source_1);

		ir_operation_block::emitds(result->ir, ir_shift_left, working_scrap, working_scrap, ir_operand::create_con(8, int16));
		ir_operation_block::emitds(result->ir, ir_bitwise_or, working_scrap, working_scrap, working_source_0);
		ir_operation_block::emitds(result->ir, ir_shift_right_unsigned, working_scrap, working_scrap, RCX(working_scrap.meta_data));
	}
	else
	{
		ir_operation_block::emitds(result->ir, ir_instructions::ir_double_shift_right, working_scrap, working_scrap, working_source_1, RCX(working_scrap.meta_data));
	}

	emit_move(result, working_destination, working_scrap);

	ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_unlock, RCX(ir_operand_meta::int64));
}

static void emit_d_f_d_n(x86_pre_allocator_context* result, uint64_t instruction, ir_operand destination, ir_operand source)
{
	assert_same_size({destination, source});
	assert_is_register(destination);

	ir_operand working_destination = destination;

	if (!ir_operand::are_equal(destination, source))
	{
		ir_operand working_source_0 = register_or_constant(result, &source);

		emit_move(result, working_destination, working_source_0);
	}

	ir_operation_block::emitds(result->ir, instruction, working_destination, working_destination);
}

//TODO: This code is nasty, needs to be refactored.
static void emit_big_multiply_divide(x86_pre_allocator_context* result, uint64_t instruction, ir_operand destination, ir_operand source_0, ir_operand source_1)
{
	assert_same_size({ destination, source_0, source_1 });

	ir_operation_block* ir = result->ir;

	ir_operand working_destination = destination;
	ir_operand working_source_0 = register_or_constant(result, &source_0);
	ir_operand working_source_1 = register_or_constant(result, &source_1);

	bool extend_8 = ir_operand::get_raw_size(&working_destination) == int8;

	if (extend_8)
	{
		switch (instruction)
		{
			case ir_divide_signed:
			case ir_multiply_hi_signed:
			{
				extend_source(result, &working_source_0, int16, true);
				extend_source(result, &working_source_1, int16, true);
			}; break;

			default:
			{
				extend_source(result, &working_source_0, int16, false);
				extend_source(result, &working_source_1, int16, false);
			}; break;
		}

		working_destination.meta_data = int16;
	}

	ir_operand ax = RAX(working_destination.meta_data);
	ir_operand dx = RDX(working_destination.meta_data);

	ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_lock, ax);
	ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_lock, dx);

	emit_move(result, ax, working_source_0);

	uint64_t working_size = ir_operand::get_raw_size(&working_destination);

	if (instruction == ir_divide_signed)
	{
		switch (working_size)
		{
			case int16: ir_operation_block::emitds(ir, ir_instructions::x86_cwd, dx, ax); break;
			case int32: ir_operation_block::emitds(ir, ir_instructions::x86_cdq, dx, ax); break;
			case int64: ir_operation_block::emitds(ir, ir_instructions::x86_cqo, dx, ax); break;
			default: throw_error();
		}
	}
	else
	{
		emit_move(result, dx, ir_operand::create_con(0, dx.meta_data));
	}

	ir_operand working_destinations[] = { dx, ax };
	ir_operand working_sources[] = { ax, working_source_1 };

	ir_operation_block::emit_with(ir, instruction, working_destinations, 2, working_sources, 2);

	switch (instruction)
	{
	case ir_multiply_hi_signed:
	case ir_multiply_hi_unsigned:
	{
		if (extend_8)
		{
			ir_operation_block::emitds(result->ir, ir_instructions::ir_shift_right_unsigned, ax, ax, ir_operand::create_con(8, int8));
			emit_move(result, working_destination, ax);
		}
		else
		{
			emit_move(result, working_destination, dx);
		}
	}; break;

	default:
	{
		emit_move(result, working_destination, ax);
	}; break;
	}

	ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_unlock, dx);
	ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_unlock, ax);
}

static void emit_context_exit(x86_pre_allocator_context* result, uint64_t instruction, ir_operand new_location)
{
	int raw_size = ir_operand::get_raw_size(&new_location);

	new_location = register_or_constant(result, &new_location, raw_size != int64);

	new_location.meta_data = int64;

	if (raw_size <= int16)
	{
		ir_operation_block::emitds(result->ir,ir_bitwise_and, new_location, new_location, ir_operand::create_con((1 << (8 << raw_size)) - 1));
	}

	ir_operation_block::emits(result->ir, instruction, new_location , ir_operand::create_con(0));

	intrusive_linked_list< intrusive_linked_list_element<ir_operation>*>::insert_element(result->revisit_instructions, result->ir->operations->last->prev);
}

static void emit_argument_load(x86_pre_allocator_context* result, ir_operation* instruction)
{
	assert_is_register(instruction->destinations[0]);
	assert_is_constant(instruction->sources[0]);

	ir_operation_block::emitds(result->ir, ir_instructions::ir_get_argument, instruction->destinations[0], RSP(ir_operand_meta::int64), ir_operand::create_con(0));

	intrusive_linked_list< intrusive_linked_list_element<ir_operation>*>::insert_element(result->revisit_instructions, result->ir->operations->last->prev);

	ir_operation_block::emitds(result->ir, ir_instructions::ir_load, instruction->destinations[0], instruction->destinations[0], ir_operand::create_con(instruction->sources[0].value * 8));
}

static void emit_with_possible_remaps(x86_pre_allocator_context* result, ir_operation* operation)
{
	ir_operand new_destinations[10];
	ir_operand new_sources[10];

	for (int i = 0; i < operation->destinations.count; ++i)
	{
		new_destinations[i] =  register_or_constant(result, operation->destinations[i]);
	}

	for (int i = 0; i < operation->sources.count; ++i)
	{
		new_sources[i] =  register_or_constant(result, operation->sources[i]);
	}

	ir_operation_block::emit_with(
		result->ir, 
		operation->instruction,
		new_destinations,		operation->destinations.count,
		new_sources,			operation->sources.count
	);
}

static void emit_as_is(x86_pre_allocator_context* result, ir_operation* operation)
{
	ir_operation_block* ir = result->ir;

	ir_operation_block::emit_with(
		ir, 
		operation->instruction,
		operation->destinations.data,	operation->destinations.count,
		operation->sources.data,		operation->sources.count
	);
}

static void emit_vector_insert(x86_pre_allocator_context* result, ir_operation* operation)
{
	assert_operand_count(operation, 1, 4);

	ir_operand destination = operation->destinations[0];
	ir_operand source_vector = operation->sources[0];
	
	ir_operand value = register_or_constant(result,operation->sources[1]);
	ir_operand index = operation->sources[2];
	ir_operand size = operation->sources[3];

	assert_is_register(source_vector);
	assert_is_size(source_vector, int128);

	assert(ir_operand::get_raw_size(&value) <= int64);

	assert_is_constant(index);
	assert_is_constant(size);

	if (!ir_operand::are_equal(destination, source_vector))
	{
		emit_move(result, destination, source_vector);
	}

	ir_operation_block::emitds(result->ir, ir_vector_insert, destination, destination, value, index, size);
}

static void emit_external_call(x86_pre_allocator_context* result, ir_operation* operation, os_information os)
{
	if (os == _linux)
	{
		ir_operand locks[] = {
			RDI(int64),
			RSI(int64),
			RDX(int64),
			RCX(int64),
			R8(int64),
			R9(int64),
		};

		int abi_count = (sizeof(locks) / sizeof(ir_operand));

		for (int i = 0; i < abi_count; ++i)
		{
			ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_lock, locks[i]);
		}

		ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_lock, RAX(int64));

		ir_operand function = register_or_constant(result, operation->sources[0]);

		for (int i = 1; i < operation->sources.count; ++i)
		{
			int abi_des = i - 1;

			if (abi_des >= abi_count)
			{
				throw_error();
			}

			ir_operand source = operation->sources[i];
			ir_operand abi_location = locks[abi_des];

			abi_location = ir_operand::copy_new_raw_size(abi_location, source.meta_data);

			emit_move(result, abi_location,source);
		}

		ir_operation_block::emitds(result->ir, ir_instructions::ir_external_call,  RAX(int64), function);

		emit_move(result,operation->destinations[0],RAX(operation->destinations[0].meta_data));

		ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_unlock, RAX(int64));

		for (int i = 0; i < abi_count; ++i)
		{
			ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_unlock, locks[i]);
		}
	}
	else
	{
		throw_error();
	}
}

static void emit_compare_and_swap(x86_pre_allocator_context* result, ir_operation* operation)
{
	assert_operand_count(operation, 1, 3);

	ir_operand destination = operation->destinations[0];
	ir_operand address = register_or_constant(result,operation->sources[0]);
	ir_operand expecting = register_or_constant(result,operation->sources[1]);
	ir_operand to_swap = register_or_constant(result,operation->sources[2]);

	assert_same_size({destination, expecting, to_swap});

	ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_lock, RAX(int64));

	ir_operand rax = RAX(expecting.meta_data);

	emit_move(result, rax, expecting);
	ir_operation_block::emitds(result->ir, ir_compare_and_swap, destination, address, rax, to_swap);

	ir_operation_block::emits(result->ir, ir_instructions::ir_register_allocator_p_unlock, RAX(int64));
}

static uint64_t create_float(double source, uint64_t size)
{
	switch (size)
	{
		case int32:
		{
			float tmp = source;

			return *(uint32_t*)&tmp;
		}

		case int64:
		{
			return *(uint64_t*)&source;
		}
		default: throw_error();
	}
}

static void emit_convert_to_float(x86_pre_allocator_context* result, ir_operation* operation)
{
	assert_operand_count(operation, 1, 1);

	ir_operand destination = operation->destinations[0];
	ir_operand source = register_or_constant(result,operation->sources[0]);

	bool is_signed = operation->instruction == ir_convert_to_float_signed;

	uint64_t d_size = ir_operand::get_raw_size(&destination);
	uint64_t s_size = ir_operand::get_raw_size(&source);

	if (is_signed || s_size <= int32)
	{
		ir_operand working_vector = create_scrap_operand(result, int128);

		if (!is_signed && s_size <= int32)
		{
			ir_operand temp = create_scrap_operand(result,source.meta_data);

			emit_move(result, temp, ir_operand::copy_new_raw_size(source, source.meta_data));

			temp = ir_operand::copy_new_raw_size(source, int64);

			source = temp;
		}

		if (d_size == int64)
		{
			ir_operation_block::emitds(result->ir, x86_cvtsi2sd, working_vector, source);
		}
		else if (d_size == int32)
		{
			ir_operation_block::emitds(result->ir, x86_cvtsi2ss, working_vector, source);
		}
		else
		{
			throw_error();
		}

		ir_operation_block::emitds(result->ir, x86_movq_to_gp, destination, working_vector);
	}
	else if (!is_signed && s_size == int64)
	{
		uint64_t convert_instruction = d_size == int64 ? x86_cvtsi2sd : x86_cvtsi2ss;
		uint64_t add_instruction = d_size == int64 ? x86_addsd : x86_addss;
		uint64_t mul_instruction = d_size == int64 ? x86_mulsd : x86_mulss;

		ir_operand lsb 			= copy_register(result, source);
		ir_operand high_part	= copy_register(result, source);

		ir_operand lsb_float 		= create_scrap_operand(result, int128);
		ir_operand high_part_float 	= create_scrap_operand(result, int128);
		ir_operand two 				= create_scrap_operand(result, int128);

		ir_operation_block::emitds(result->ir, x86_movq_to_vec, two, register_or_constant(result,ir_operand::create_con(create_float(2.0, d_size))));

		ir_operation_block::emitds(result->ir,ir_bitwise_and, lsb, lsb, ir_operand::create_con(1));
		ir_operation_block::emitds(result->ir,ir_shift_right_unsigned, high_part, high_part, ir_operand::create_con(1));
	
		ir_operation_block::emitds(result->ir, convert_instruction, lsb_float, lsb);	
		ir_operation_block::emitds(result->ir, convert_instruction, high_part_float, high_part);	

		ir_operation_block::emitds(result->ir, mul_instruction, high_part_float, high_part_float, two);	
		ir_operation_block::emitds(result->ir, add_instruction, high_part_float, high_part_float, lsb_float);

		ir_operation_block::emitds(result->ir, x86_movq_to_gp, destination, high_part_float);
	}

	if (d_size <= int32)
	{
		emit_move(result, destination, destination);
	}
}

static void emit_pre_allocation_instruction(x86_pre_allocator_context* pre_allocator_context, ir_operation* operation, os_information os)
{
	ir_instructions working_instruction = (ir_instructions)operation->instruction;

	switch (working_instruction)
	{
		//Binary Operations
		case ir_add:
		case ir_subtract:
		case ir_bitwise_and:
		case ir_bitwise_or:
		case ir_bitwise_exclusive_or:
		case ir_multiply:
		{
			assert_operand_count(operation, 1, 2);
			assert_is_register(operation->destinations[0]);

			emit_d_n_f_d_n_m(pre_allocator_context, operation->instruction, operation->destinations[0], operation->sources[0], operation->sources[1]);
		};  break;

		case ir_floating_point_add:
		case ir_floating_point_subtract:
		case ir_floating_point_multiply:
		case ir_floating_point_divide:
		case ir_floating_point_select_min:
		case ir_floating_point_select_max:
		{
			assert_operand_count(operation, 1, 2);
			assert_is_register(operation->destinations[0]);
			
			emit_floating_point_binary(pre_allocator_context, operation->instruction, operation->destinations[0], operation->sources[0], operation->sources[1]);
		}; break;

		case ir_floating_point_compare_equal:
		case ir_floating_point_compare_less:
		case ir_floating_point_compare_not_equal:
		case ir_floating_point_compare_greater:
		case ir_floating_point_compare_greater_equal:
		{
			assert_operand_count(operation, 1, 2);
			assert_is_register(operation->destinations[0]);
			
			emit_floating_point_compare(pre_allocator_context, operation->instruction, operation->destinations[0], operation->sources[0], operation->sources[1]);
		}; break;

		case ir_multiply_hi_signed:
		case ir_multiply_hi_unsigned:
		case ir_divide_unsigned:
		case ir_divide_signed:
		{
			assert_operand_count(operation, 1, 2);
			assert_is_register(operation->destinations[0]);

			emit_big_multiply_divide(pre_allocator_context, operation->instruction, operation->destinations[0], operation->sources[0], operation->sources[1]);
		}; break;

		case ir_compare_equal:
		case ir_compare_greater_equal_signed:
		case ir_compare_greater_equal_unsigned:
		case ir_compare_greater_signed:
		case ir_compare_greater_unsigned:
		case ir_compare_less_equal_signed:
		case ir_compare_less_equal_unsigned:
		case ir_compare_less_signed:
		case ir_compare_less_unsigned:
		case ir_compare_not_equal:
		{
			assert_operand_count(operation, 1, 2);
			assert_is_register(operation->destinations[0]);

			emit_compare(pre_allocator_context, operation->instruction, operation->destinations[0], operation->sources[0], operation->sources[1]);

		}; break;

		case ir_shift_left:
		case ir_shift_right_signed:
		case ir_shift_right_unsigned:
		case ir_rotate_right:
		{
			assert_operand_count(operation, 1, 2);
			assert_is_register(operation->destinations[0]);

			emit_shift(pre_allocator_context, operation->instruction, operation->destinations[0], operation->sources[0], operation->sources[1]);

		}; break;

		//ABI
		case ir_close_and_return:
		case ir_table_jump:
		{
			assert_operand_count(operation, 0, 1);

			emit_context_exit(pre_allocator_context, working_instruction, operation->sources[0]);

		}; break;

		case ir_get_argument:
		{
			assert_operand_count(operation, 1, 1);
			assert_is_constant(operation->sources[0]);

			emit_argument_load(pre_allocator_context, operation);
		}; break;

		//Unary Operations
		case ir_move:
		{
			assert_operand_count(operation, 1, 1);
			assert_is_register(operation->destinations[0]);

			emit_move(pre_allocator_context, operation->destinations[0], operation->sources[0]);

		}; break;

		case ir_bitwise_not:
		case ir_incrament: 
		case ir_negate:
		case ir_logical_not:
		{
			assert_operand_count(operation, 1, 1);
			assert_is_register(operation->destinations[0]);

			emit_d_f_d_n(pre_allocator_context, operation->instruction, operation->destinations[0], operation->sources[0]);

		}; break;

		case ir_floating_point_square_root:
		{
			assert_operand_count(operation, 1, 1);
			assert_is_register(operation->destinations[0]);

			emit_floating_point_unary(pre_allocator_context, operation->instruction, operation->destinations[0], operation->sources[0]);
		}; break;

		case ir_sign_extend:
		{
			assert_operand_count(operation, 1, 1);
			assert_is_register(operation->destinations[0]);

			if (ir_operand::get_raw_size(&operation->destinations[0]) == ir_operand::get_raw_size(&operation->sources[0]))
			{
				emit_move(pre_allocator_context, operation->destinations[0], operation->sources[0]);
			}
			else
			{
				ir_operand source = register_or_constant(pre_allocator_context, &operation->sources[0]);

				ir_operation_block::emitds(pre_allocator_context->ir, ir_instructions::ir_sign_extend, operation->destinations[0], source);
			}

		}; break;

		//Misc
		case ir_no_operation:
		{
			//Do nothing lolESS
		}; break;

		case ir_mark_label:
		{
			emit_as_is(pre_allocator_context, operation);
			
			assert(operation->sources[0].value < UINT32_MAX);
		}; break;

		case ir_load:
		case ir_jump_if:
		case ir_vector_zero:
		{
			emit_as_is(pre_allocator_context, operation);
		}; break;

		case ir_assert_false:
		case ir_assert_true:
		case ir_store:	//TODO this can be optimized
		{
			emit_with_possible_remaps(pre_allocator_context, operation);
		}; break;

		case ir_vector_extract:
		{
			assert_operand_count(operation, 1, 3);
			
			assert(!ir_operand::is_vector(&operation->destinations[0]));

			assert_is_size(operation->sources[0], int128);
			assert_is_constant(operation->sources[1]);
			assert_is_constant(operation->sources[2]);

			emit_as_is(pre_allocator_context, operation);
		}; break;

		case ir_vector_insert:
		{
			emit_vector_insert(pre_allocator_context, operation);
		}; break;

		case ir_conditional_select:
		{
			assert_operand_count(operation, 1, 3);
			assert_is_register(operation->destinations[0]);

			emit_conditional_select(pre_allocator_context, operation->destinations[0], operation->sources[0], operation->sources[1], operation->sources[2]);
		}; break;

		case ir_double_shift_right:
		{
			assert_operand_count(operation, 1, 3);
			assert_is_register(operation->destinations[0]);

			emit_double_shift_right(pre_allocator_context, operation->destinations[0], operation->sources[0], operation->sources[1], operation->sources[2]);
		}; break;

		case ir_external_call:
		{
			emit_external_call(pre_allocator_context, operation, os);
		}; break;

		case ir_compare_and_swap:
		{
			emit_compare_and_swap(pre_allocator_context, operation);
		}; break;

		case ir_convert_to_float_signed:
		case ir_convert_to_float_unsigned:
		{
			emit_convert_to_float(pre_allocator_context, operation);
		}; break;

		default: 
		
		throw_error();
	}

	for (int i = 0; i < operation->destinations.count; ++i)
	{
		ir_operand destination = operation->destinations[i];

		uint64_t raw_size = ir_operand::get_raw_size(&destination);

		if (raw_size == ir_operand_meta::int16 || raw_size == ir_operand_meta::int8)
		{
			ir_operand::set_raw_size(&destination, ir_operand_meta::int64);

			uint64_t mask = (1ULL << (8 << raw_size)) - 1;

			ir_operation_block::emitds(pre_allocator_context->ir, ir_instructions::ir_bitwise_and, destination, destination, ir_operand::create_con(mask, ir_operand_meta::int64));
		}
	}
}

void x86_pre_allocator_context::run_pass(x86_pre_allocator_context* pre_allocator_context, ir_operation_block* result_ir, ir_operation_block* source, cpu_information cpu_data, os_information os)
{
	assert(cpu_data == x86_64);

	ir_operation_block::clamp_operands(source, false);

	pre_allocator_context->allocator = result_ir->allocator;
	pre_allocator_context->scrap_index = 0;
	pre_allocator_context->ir = result_ir;
	pre_allocator_context->source_ir = source;

	pre_allocator_context->revisit_instructions = intrusive_linked_list<intrusive_linked_list_element<ir_operation>*>::create(pre_allocator_context->allocator, nullptr, nullptr);

	pre_allocator_context->ir->label_index = UINT32_MAX;

	for (auto i = source->operations->first; i != source->operations->last; i = i->next)
	{
		emit_pre_allocation_instruction(pre_allocator_context, &i->data, os);

		pre_allocator_context->scrap_index = 0;
	}

	ir_operation_block::clamp_operands(result_ir, false, pre_allocator_context->opernad_counts);

	uint64_t context_size = (pre_allocator_context->opernad_counts[0] * 8) + (pre_allocator_context->opernad_counts[1] * 16);

	context_size = get_context_size(context_size);

	ir_operation_block::emits(result_ir, ir_instructions::ir_open_context, ir_operand::create_con(context_size), result_ir->operations->first);

	for (auto i = pre_allocator_context->revisit_instructions->first; i != pre_allocator_context->revisit_instructions->last; i = i->next)
	{
		intrusive_linked_list_element<ir_operation>* working_element = i->data;

		if (working_element == nullptr)
			continue;

		switch (working_element->data.instruction)
		{
		case ir_get_argument:
		{
			working_element->data.instruction = ir_load; 
			working_element->data.sources[1].value = context_size + 8;
		} break;
		
		case ir_close_and_return:
		case ir_table_jump:
		{
			working_element->data.sources[1].value = context_size; 
		} break;
		default: throw_error();
		}
	}

	ir_operation_block::emits(result_ir, ir_instructions::ir_register_allocator_p_lock, RSP(ir_operand_meta::int64), result_ir->operations->first);
}
