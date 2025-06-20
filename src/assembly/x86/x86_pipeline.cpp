#include "debugging.h"
#include "x86_assembler.h"
#include "x86_pipeline.h"
#include "x86_pre_allocator.h"

#include "ir/basic_register_allocator.h"
#include "ir/ssa.h"
#include "ir/undefined_behavior_check.h"

#include <fstream>
#include <iostream>

int id = 0;

void assemble_x86_64_pipeline(void** result_code, uint64_t* result_code_size, ir_operation_block* source_ir, abi working_abi, compiler_flags flags) {
    arena_allocator* allocator = source_ir->allocator;

    ir_operation_block* pre_allocated_code;
    ir_operation_block* register_allocated_code;

    ir_operation_block::create(&pre_allocated_code, allocator);
    ir_operation_block::create(&register_allocated_code, allocator);

    x86_pre_allocator_context pre_allocation_data;
    basic_register_allocator_context register_allocation_data;

    if (flags & check_undefined_behavior) {
        ir_operation_block* undefined_behavior_checked_code;
        ir_operation_block::create(&undefined_behavior_checked_code, allocator);

        run_undefined_behavior_check_pass(undefined_behavior_checked_code, source_ir);

        source_ir = undefined_behavior_checked_code;
    }

    bool use_lrsa_hints = false;

    if ((flags & optimize_basic_ssa) || (flags & optimize_group_pool_ssa)) {
        convert_to_ssa(source_ir, flags);

        use_lrsa_hints = true;
    }

    x86_pre_allocator_context::run_pass(&pre_allocation_data, pre_allocated_code, source_ir, working_abi.cpu, working_abi.os);

    // ir_operation_block::log(pre_allocated_code);

    // std::string code;
    // std::cin >> code;

    basic_register_allocator_context::run_pass(&register_allocation_data, register_allocated_code, pre_allocated_code,

                                               16, {ir_operand_meta::int64, (uint32_t)pre_allocation_data.opernad_counts[0]},

                                               16, {ir_operand_meta::int128, (uint32_t)pre_allocation_data.opernad_counts[0]},

                                               RSP(ir_operand_meta::int64),

                                               use_lrsa_hints);

    /*
    if (flags & compiler_flags::optimize_group_pool_ssa)
    {
        std::ofstream str;

        id++;

        str.open("/media/linvirt/partish/tmp/" + std::to_string(id));

        str << ir_operation_block::get_block_log(register_allocated_code);

        str.close();
    }
        */

    assemble_x86_64_code(result_code, result_code_size, register_allocated_code);
}
