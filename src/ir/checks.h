#pragma once

#include <vector>
#include "debugging.h"
#include "ir.h"

static void assert_operand_count(ir_operation* operation, int destinations, int sources) {
    assert(operation->destinations.count == destinations);
    assert(operation->sources.count == sources);
}

static void assert_is_register(ir_operand operand) {
    assert(ir_operand::is_register(&operand));
}

static void assert_is_constant(ir_operand operand) {
    assert(ir_operand::is_constant(&operand));
}

static void assert_all_registers(ir_operand* operands, int count) {
    for (int i = 0; i < count; ++i) {
        assert_is_register(operands[i]);
    }
}

static void assert_is_size(ir_operand operand, int size) {
    assert(ir_operand::get_raw_size(&operand) == size);
}

static void assert_all_registers(ir_operation* operation) {
    assert_all_registers(operation->destinations.data, operation->destinations.count);
    assert_all_registers(operation->sources.data, operation->sources.count);
}

static void assert_same_registers(ir_operand left, ir_operand right) {
    assert(ir_operand::are_equal(left, right));
}

static void get_all_operands(ir_operand* operands, int count, std::vector<ir_operand*>* result) {
    for (int i = 0; i < count; ++i) {
        result->push_back(&operands[i]);
    }
}

static void get_all_operands(ir_operation* operation, std::vector<ir_operand*>* result) {
    get_all_operands(operation->destinations.data, operation->destinations.count, result);
    get_all_operands(operation->sources.data, operation->sources.count, result);
}

static void assert_registers_same_type(ir_operation* operation) {
    std::vector<ir_operand*> all_operands;

    get_all_operands(operation, &all_operands);

    uint64_t check_type = ir_operand::get_raw_size(all_operands[0]);

    for (int i = 1; i < all_operands.size(); ++i) {
        uint64_t working_type = ir_operand::get_raw_size(all_operands[i]);

        assert(check_type == working_type);
    }
}

// I'm pretty sure std::initializer_list does not allocate memory.
static void assert_same_size(std::initializer_list<ir_operand> operands) {
    // TODO in relase, this should not execute

    int count = operands.size();

    if (count == 0)
        return;

    uint64_t first_size = operands.begin()[0].meta_data & UINT32_MAX;

    for (int i = 1; i < count; ++i) {
        uint64_t working_size = operands.begin()[i].meta_data & UINT32_MAX;

        if (working_size != first_size) {
            throw_error();
        }
    }
}
