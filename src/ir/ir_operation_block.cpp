#include "bit_register_allocations.h"
#include "debugging.h"
#include "ir.h"

#define OPERATION_ELEMENT intrusive_linked_list_element<ir_operation>
#define OPERATION_LIST intrusive_linked_list<ir_operation>

void ir_operation_block::create(ir_operation_block** result, arena_allocator* allocator) {
    ir_operation_block* working_result = arena_allocator::allocate_struct<ir_operation_block>(allocator, 1);

    working_result->allocator = allocator;

    ir_operation first_nop;
    ir_operation last_nop;

    create_raw_operation(allocator, &first_nop, ir_no_operation, 0, 0);
    create_raw_operation(allocator, &last_nop, ir_no_operation, 0, 0);

    working_result->label_index = 0;
    working_result->local_index = 0;

    working_result->operations = intrusive_linked_list<ir_operation>::create(allocator, first_nop, last_nop);

    *result = working_result;
}

void ir_operation_block::create_raw_operation(arena_allocator* allocator, ir_operation* result, uint64_t instruction, int destination_count, int source_count) {
    fast_array<ir_operand>::create(allocator, destination_count, &result->destinations);
    fast_array<ir_operand>::create(allocator, source_count, &result->sources);

    result->instruction = (ir_instructions)instruction;
    result->node_id = -1;
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emit(ir_operation_block* block, ir_operation operation, intrusive_linked_list_element<ir_operation>* point) {
    OPERATION_ELEMENT* working_element = OPERATION_LIST::create_element(block->operations, operation);

    if (point == nullptr) {
        OPERATION_LIST::insert_element(block->operations, working_element);
    } else {
        OPERATION_LIST::insert_element_after(block->operations, point, working_element);
    }

    return working_element;
}

void ir_operation_block::log(ir_operation_block* ctx) {
    std::string to_log = ir_operation_block::get_block_log(ctx);

    std::cout << to_log << std::endl;
}

void ir_operation_block::remove_redundant_moves(ir_operation_block* ctx) {
    for (auto i = ctx->operations->first; i != nullptr; i = i->next) {
        ir_operation* working_operation = &i->data;

        if (working_operation->instruction != ir_move) {
            continue;
        }

        ir_operand* des = &working_operation->destinations[0];
        ir_operand* src = &working_operation->sources[0];

        if (ir_operand::are_equal(*des, *src) && ir_operand::get_raw_size(des) >= int64) {
            working_operation->instruction = ir_no_operation;
            working_operation->destinations.count = 0;
            working_operation->sources.count = 0;
        }
    }
}

// NOTE: creating std::unordered_map contains a new somewhere. and the point of this
//		is to avoid news as much as possible.

// TODO: create a custom allocator group
static void remap_operands_impl(std::unordered_map<uint64_t, uint64_t>** remaps, fast_array<ir_operand>* operands, bool use_bit_register_allocations, bool ignore_unreamped_registers) {
    for (int i = 0; i < operands->count; ++i) {
        ir_operand* working = &operands->data[i];

        if (ir_operand::is_constant(working))
            continue;

        if (ir_operand::is_hardware(working))
            continue;

        uint64_t remap_index = working->meta_data & UINT32_MAX;
        uint64_t source_value = working->value;

        std::unordered_map<uint64_t, uint64_t>* working_remap = remaps[remap_index];

        if (working_remap->find(source_value) == working_remap->end()) {
            if (ignore_unreamped_registers) {
                continue;
            }

            int new_source_value = working_remap->size();

            (*working_remap)[source_value] = new_source_value;

            working->value = new_source_value;
        } else {
            working->value = (*working_remap)[source_value];
        }

        if (use_bit_register_allocations) {
            working->value |= ir_operand::is_vector(working) ? bit_register_allocations::vec_allocation : bit_register_allocations::gp_allocation;
        }
    }
}

static void remap_operands_operations_impl(std::unordered_map<uint64_t, uint64_t>** remaps, ir_operation* operation, bool use_bit_register_allocations, bool ignore_unreamped_registers) {
    remap_operands_impl(remaps, &operation->destinations, use_bit_register_allocations, ignore_unreamped_registers);
    remap_operands_impl(remaps, &operation->sources, use_bit_register_allocations, ignore_unreamped_registers);
}

void ir_operation_block::create_vector_gp_remap_scheme(arena_allocator* allocator, std::unordered_map<uint64_t, uint64_t>* remap_store, std::unordered_map<uint64_t, uint64_t>** remap_redirect) {
    remap_store[0] = std::unordered_map<uint64_t, uint64_t>();
    remap_store[1] = std::unordered_map<uint64_t, uint64_t>();

    for (int i = 0; i < ir_operand_meta::top; ++i) {
        if (i >= ir_operand_meta::int128) {
            remap_redirect[i] = &remap_store[1];
        } else {
            remap_redirect[i] = &remap_store[0];
        }
    }
}

void ir_operation_block::clamp_operands(ir_operation_block* ir, bool use_bit_register_allocations, int* size_counts) {
    std::unordered_map<uint64_t, uint64_t> remap_safe[2];
    std::unordered_map<uint64_t, uint64_t>* remap_redirection[ir_operand_meta::top];

    create_vector_gp_remap_scheme(ir->allocator, remap_safe, remap_redirection);

    for (auto i = ir->operations->first; i != ir->operations->last; i = i->next) {
        remap_operands_operations_impl(remap_redirection, &i->data, use_bit_register_allocations, false);
    }

    if (size_counts != nullptr) {
        size_counts[0] = remap_safe[0].size();
        size_counts[1] = remap_safe[1].size();
    }
}

void ir_operation_block::ssa_remap(ir_operation_block* ir, std::unordered_map<uint64_t, uint64_t>* remap_data) {
    std::unordered_map<uint64_t, uint64_t>* remap_redirection[ir_operand_meta::top];

    for (int i = 0; i < ir_operand_meta::top; ++i) {
        remap_redirection[i] = remap_data;
    }

    for (auto i = ir->operations->first; i != ir->operations->last; i = i->next) {
        remap_operands_operations_impl(remap_redirection, &i->data, false, true);
    }
}

bool ir_operation_block::is_label(ir_operation* operation) {
    switch (operation->instruction) {
    case ir_mark_label:
        return true;
    }

    return false;
}

bool ir_operation_block::is_jump(ir_operation* operation) {
    switch (operation->instruction) {
    case ir_jump_if:
    case ir_jump_if_equal:
    case ir_jump_if_not_equal:
    case ir_jump_if_less_signed:
    case ir_jump_if_less_equal_signed:
    case ir_jump_if_less_unsigned:
    case ir_jump_if_less_equal_unsigned:
    case ir_jump_if_greater_signed:
    case ir_jump_if_greater_equal_signed:
    case ir_jump_if_greater_unsigned:
    case ir_jump_if_greater_equal_unsigned:
        return true;
    }

    return false;
}

bool ir_operation_block::is_compare(ir_operation* operation) {
    switch (operation->instruction) {
    case ir_compare_equal:
    case ir_compare_greater_equal_signed:
    case ir_compare_greater_equal_unsigned:
    case ir_compare_greater_signed:
    case ir_compare_greater_unsigned:
    case ir_compare_less_equal_signed:
    case ir_compare_less_equal_unsigned:
    case ir_compare_less_signed:
    case ir_compare_less_unsigned:
    case ir_compare_not_equal: {
        return true;
    }; break;
    }

    return false;
}

bool ir_operation_block::ends_control_flow(ir_operation* operation) {
    if (is_jump(operation)) {
        return true;
    }

    switch (operation->instruction) {
    case ir_close_and_return:
    case ir_table_jump:
        return true;
    }

    return false;
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emit_with(ir_operation_block* ir, uint64_t instruction, ir_operand* destinations, int destination_count, ir_operand* sources, int source_count, intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;

    create_raw_operation(ir->allocator, &result, instruction, destination_count, source_count);

    fast_array<ir_operand>::copy_from(&result.destinations, destinations);
    fast_array<ir_operand>::copy_from(&result.sources, sources);

    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 1, 0);
    result.destinations[0] = destination_0;
    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 1, 1);
    result.destinations[0] = destination_0;
    result.sources[0] = source_0;
    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, ir_operand source_1, intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 1, 2);
    result.destinations[0] = destination_0;
    result.sources[0] = source_0;
    result.sources[1] = source_1;
    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, ir_operand source_1, ir_operand source_2, intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 1, 3);
    result.destinations[0] = destination_0;
    result.sources[0] = source_0;
    result.sources[1] = source_1;
    result.sources[2] = source_2;
    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, ir_operand source_1, ir_operand source_2, ir_operand source_3, intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 1, 4);
    result.destinations[0] = destination_0;
    result.sources[0] = source_0;
    result.sources[1] = source_1;
    result.sources[2] = source_2;
    result.sources[3] = source_3;
    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, ir_operand source_1, ir_operand source_2, ir_operand source_3, ir_operand source_4,
                                                                        intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 1, 5);
    result.destinations[0] = destination_0;
    result.sources[0] = source_0;
    result.sources[1] = source_1;
    result.sources[2] = source_2;
    result.sources[3] = source_3;
    result.sources[4] = source_4;
    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, ir_operand source_1, ir_operand source_2, ir_operand source_3, ir_operand source_4, ir_operand source_5,
                                                                        intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 1, 6);
    result.destinations[0] = destination_0;
    result.sources[0] = source_0;
    result.sources[1] = source_1;
    result.sources[2] = source_2;
    result.sources[3] = source_3;
    result.sources[4] = source_4;
    result.sources[5] = source_5;
    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emitds(ir_operation_block* ir, uint64_t instruction, ir_operand destination_0, ir_operand source_0, ir_operand source_1, ir_operand source_2, ir_operand source_3, ir_operand source_4, ir_operand source_5, ir_operand source_6,
                                                                        intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 1, 7);
    result.destinations[0] = destination_0;
    result.sources[0] = source_0;
    result.sources[1] = source_1;
    result.sources[2] = source_2;
    result.sources[3] = source_3;
    result.sources[4] = source_4;
    result.sources[5] = source_5;
    result.sources[6] = source_6;
    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emits(ir_operation_block* ir, uint64_t instruction, intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 0, 0);
    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emits(ir_operation_block* ir, uint64_t instruction, ir_operand source_0, intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 0, 1);
    result.sources[0] = source_0;
    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emits(ir_operation_block* ir, uint64_t instruction, ir_operand source_0, ir_operand source_1, intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 0, 2);
    result.sources[0] = source_0;
    result.sources[1] = source_1;
    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emits(ir_operation_block* ir, uint64_t instruction, ir_operand source_0, ir_operand source_1, ir_operand source_2, intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 0, 3);
    result.sources[0] = source_0;
    result.sources[1] = source_1;
    result.sources[2] = source_2;
    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emits(ir_operation_block* ir, uint64_t instruction, ir_operand source_0, ir_operand source_1, ir_operand source_2, ir_operand source_3, intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 0, 4);
    result.sources[0] = source_0;
    result.sources[1] = source_1;
    result.sources[2] = source_2;
    result.sources[3] = source_3;
    return emit(ir, result, point);
}

intrusive_linked_list_element<ir_operation>* ir_operation_block::emits(ir_operation_block* ir, uint64_t instruction, ir_operand source_0, ir_operand source_1, ir_operand source_2, ir_operand source_3, ir_operand source_4, intrusive_linked_list_element<ir_operation>* point) {
    ir_operation result;
    ir_operation_block::create_raw_operation(ir->allocator, &result, instruction, 0, 5);
    result.sources[0] = source_0;
    result.sources[1] = source_1;
    result.sources[2] = source_2;
    result.sources[3] = source_3;
    result.sources[4] = source_4;
    return emit(ir, result, point);
}

ir_operand ir_operation_block::create_label(ir_operation_block* block) {
    uint64_t label_location = block->label_index;
    block->label_index++;

    ir_operand result_label = ir_operand::create_con(label_location, int64);

    return result_label;
}

void ir_operation_block::mark_label(ir_operation_block* block, ir_operand label) {
    ir_operation_block::emits(block, ir_mark_label, label);

    ir_operation_block::emits(block, ir_no_operation);
}

void ir_operation_block::jump_if(ir_operation_block* block, ir_operand label, ir_operand condition) {
    ir_operation_block::emits(block, ir_jump_if, label, condition);
}

void ir_operation_block::jump_if_not(ir_operation_block* block, ir_operand label, ir_operand condition) {
    // TODO
}

static void get_used_operands(std::unordered_set<uint64_t>* result, ir_operand* operands, int operand_count) {
    for (int i = 0; i < operand_count; ++i) {
        ir_operand working = operands[i];

        if (ir_operand::is_constant(&working))
            continue;

        result->insert(working.value);
    }
}

void ir_operation_block::get_used_registers(std::unordered_set<uint64_t>* result, ir_operation_block* block) {
    for (auto i = block->operations->first; i != nullptr; i = i->next) {
        get_used_operands(result, i->data.destinations.data, i->data.destinations.count);
        get_used_operands(result, i->data.sources.data, i->data.sources.count);
    }
}

void ir_operation_block::jump(ir_operation_block* block, ir_operand label) {
    jump_if(block, label, ir_operand::create_con(1));
}

std::string get_string(ir_operand value) {
    uint64_t raw_size = ir_operand::get_raw_size(&value);

    std::string result = "(";

    if (ir_operand::is_hardware(&value)) {
        result += "H ";
    }

    if (ir_operand::is_constant(&value)) {
        result += "C ";
    } else {
        result += "R ";
    }

    switch (raw_size) {
    case ir_operand_meta::int8:
        result += "INT8";
        break;
    case ir_operand_meta::int16:
        result += "INT16";
        break;
    case ir_operand_meta::int32:
        result += "INT32";
        break;
    case ir_operand_meta::int64:
        result += "INT64";
        break;
    case ir_operand_meta::int128:
        result += "INT128";
        break;
    case (ir_operand_meta)UINT32_MAX:
        result += "GENERIC";
        break;
    default:
        throw_error();
    }

    result += " " + std::to_string(value.value);

    result += ")";

    return result;
}

std::string ir_operation_block::get_block_log(ir_operation_block* ir) {
    std::string result = "";

    for (auto i = ir->operations->first; i != nullptr; i = i->next) {
        ir_operation working_operation = i->data;

        if (working_operation.instruction == ir_no_operation)
            continue;

        std::string name = instruction_names[working_operation.instruction];

        result += name + " ";

        for (int o = 0; o < working_operation.destinations.count; ++o) {
            result += get_string(working_operation.destinations[o]) + " ";
        }

        for (int o = 0; o < working_operation.sources.count; ++o) {
            result += get_string(working_operation.sources[o]) + " ";
        }

        result += "\n";
    }

    return result;
}
