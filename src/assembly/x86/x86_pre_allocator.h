#pragma once

#include "abi_information.h"
#include "ir/ir.h"

#define RAX(size) ir_operand::create_hardware_reg(0, size)
#define RCX(size) ir_operand::create_hardware_reg(1, size)
#define RDX(size) ir_operand::create_hardware_reg(2, size)
#define RBX(size) ir_operand::create_hardware_reg(3, size)
#define RSP(size) ir_operand::create_hardware_reg(4, size)
#define RBP(size) ir_operand::create_hardware_reg(5, size)
#define RSI(size) ir_operand::create_hardware_reg(6, size)
#define RDI(size) ir_operand::create_hardware_reg(7, size)
#define R8(size) ir_operand::create_hardware_reg(8, size)
#define R9(size) ir_operand::create_hardware_reg(9, size)

#define XMM(index) ir_operand::create_hardware_reg(index, int128)

struct x86_pre_allocator_context {
    uint64_t scrap_index;

    arena_allocator* allocator;
    ir_operation_block* ir;
    ir_operation_block* source_ir;
    intrusive_linked_list<intrusive_linked_list_element<ir_operation>*>* revisit_instructions;

    int opernad_counts[2];

    static void run_pass(x86_pre_allocator_context* pre_allocator_context, ir_operation_block* result_ir, ir_operation_block* source, cpu_information cpu_data, os_information os);
};
