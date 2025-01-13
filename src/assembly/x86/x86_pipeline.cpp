#include "x86_pipeline.h"
#include "x86_pre_allocator.h"
#include "x86_assembler.h"
#include "debugging.h"
#include "ir/ssa.h"

#include "ir/basic_register_allocator.h"
#include "ir/undefined_behavior_check.h"

void assemble_x86_64_pipeline(void** result_code, uint64_t* result_code_size, ir_operation_block* source_ir, bool optimize, abi working_abi, compiler_flags flags)
{
	arena_allocator* allocator = source_ir->allocator;
	
	ir_operation_block* pre_allocated_code; 
	ir_operation_block* register_allocated_code;
		
	ir_operation_block::create(&pre_allocated_code, allocator);
	ir_operation_block::create(&register_allocated_code, allocator);

	x86_pre_allocator_context pre_allocation_data;
	basic_register_allocator_context register_allocation_data;

	if (flags & check_undefined_behavior)
	{
		ir_operation_block* undefined_behavior_checked_code;		
		ir_operation_block::create(&undefined_behavior_checked_code, allocator);

		run_undefined_behavior_check_pass(undefined_behavior_checked_code, source_ir);

		source_ir = undefined_behavior_checked_code;
	}

	if (flags & optimize_ssa)
	{
		ssa_construct_and_optimize(source_ir, flags);
	}
	
	x86_pre_allocator_context::run_pass(&pre_allocation_data, pre_allocated_code, source_ir, working_abi.cpu,working_abi.os);
	
	basic_register_allocator_context::run_pass(
		&register_allocation_data,
		register_allocated_code,
		pre_allocated_code,

		16,
		{ 
			ir_operand_meta::int64, 
			(uint32_t)pre_allocation_data.opernad_counts[0] 
		},

		16,
		{ 
			ir_operand_meta::int128, 
			(uint32_t)pre_allocation_data.opernad_counts[0] 
		},
		
		RSP(ir_operand_meta::int64)
	);

	assemble_x86_64_code(result_code, result_code_size, register_allocated_code);
}
