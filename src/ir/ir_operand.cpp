#include "ir.h"

#include "debugging.h"
#include "tools/bit_tools.h"

bool ir_operand::is_register(ir_operand* test) {
    return !is_constant(test);
}

bool ir_operand::is_constant(ir_operand* test) {
    return test->meta_data & ir_operand_meta::is_constant;
}

bool ir_operand::is_hardware(ir_operand* test) {
    if (is_constant(test)) {
        return false;
    }

    return test->meta_data & ir_operand_meta::is_hardware_register;
}

bool ir_operand::is_vector(ir_operand* test) {
    uint64_t size_test = test->meta_data & UINT32_MAX;

    return size_test >= ir_operand_meta::int128;
}

uint64_t ir_operand::get_masked_constant(ir_operand* test) {
    assert(is_constant(test));

    return test->value & get_mask_from_size(test->meta_data & UINT32_MAX);
}

uint64_t ir_operand::get_raw_size(ir_operand* test) {
    return test->meta_data & UINT32_MAX;
}

void ir_operand::set_raw_size(ir_operand* test, uint64_t new_size) {
    test->meta_data = (test->meta_data & ~(uint64_t)UINT32_MAX) | (new_size & UINT32_MAX);
}

uint64_t ir_operand::get_meta(ir_operand* test) {
    return test->meta_data & ~(uint64_t)UINT32_MAX;
}

ir_operand ir_operand::create_con(uint64_t value, uint64_t size) {
    ir_operand result;

    size = (size & UINT32_MAX);

    result.value = value & get_mask_from_size(size);
    result.meta_data = size | ir_operand_meta::is_constant;

    assert(!ir_operand::is_vector(&result));

    return result;
}

ir_operand ir_operand::create_reg(uint64_t value, uint64_t size) {
    ir_operand result;

    result.value = value;
    result.meta_data = (size & UINT32_MAX);

    return result;
}

ir_operand ir_operand::create_hardware_reg(uint64_t value, uint64_t size) {
    ir_operand result;

    result.value = value;
    result.meta_data = (size & UINT32_MAX) | ir_operand_meta::is_hardware_register;

    return result;
}

ir_operand ir_operand::copy_new_raw_size(ir_operand source, uint64_t new_size) {
    if (new_size & UINT32_MAX == source.meta_data & UINT32_MAX)
        return source;

    if (ir_operand::is_vector(&source) != ((new_size & UINT32_MAX) >= int128)) {
        throw_error();
    }

    source.meta_data = (new_size & UINT32_MAX) | (source.meta_data & ~(uint64_t)UINT32_MAX);

    return source;
}

void ir_operation_block::copy(ir_operation_block* result, ir_operation_block* source) {
    for (auto i = source->operations->first; i != nullptr; i = i->next) {
        if (i->data.instruction == ir_no_operation)
            continue;

        ir_operation_block::emit_with(result, i->data.instruction, i->data.destinations.data, i->data.destinations.count, i->data.sources.data, i->data.sources.count);
    }
}

bool ir_operand::are_equal(ir_operand left, ir_operand right) {
    bool type_equal = left.meta_data == right.meta_data;
    bool value_equal = left.value == right.value;

    return type_equal && value_equal;
}

bool ir_operand::same_size(ir_operand left, ir_operand right) {
    return ir_operand::get_raw_size(&left) == ir_operand::get_raw_size(&right);
}