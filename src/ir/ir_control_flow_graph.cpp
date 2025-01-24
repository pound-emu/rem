#include "ir.h"
#include "debugging.h"
#include "checks.h"

static ir_control_flow_node* create_and_insert_node(intrusive_linked_list<ir_control_flow_node*>* list, intrusive_linked_list_element<ir_operation>* first_instruction, intrusive_linked_list_element<ir_operation>* block_final_instruction)
{
    ir_control_flow_node* result = arena_allocator::allocate_struct<ir_control_flow_node>(list->allocator);

    result->entry_instruction = first_instruction;
    result->final_instruction = block_final_instruction;
    result->entry_id = -1;

    result->entries = intrusive_linked_list<ir_control_flow_node*>::create(list->allocator, nullptr, nullptr);
    result->exits = intrusive_linked_list<ir_control_flow_node*>::create(list->allocator, nullptr, nullptr);

    result->entry_count = 0;
    result->exit_count = 0;

    intrusive_linked_list<ir_control_flow_node*>::insert_element(list, result);

    return result;
}

static bool is_label(ir_operation* operation)
{
    switch (operation->instruction)
    {
        case ir_mark_label:
            return true;
    }

    return false;
}

static bool is_jump(ir_operation* operation)
{
    switch (operation->instruction)
    {
        case ir_jump_if:
        case ir_close_and_return:
        case ir_table_jump:
            return true;
    }

    return false;
}

static void establish_outward_connection(ir_control_flow_node* working, ir_control_flow_node* exit)
{
    working->exit_count++;
    intrusive_linked_list<ir_control_flow_node*>::insert_element(working->exits, exit);

    exit->entry_count++;
    intrusive_linked_list<ir_control_flow_node*>::insert_element(exit->entries, working);
}

static void get_linier_nodes(ir_control_flow_graph* result)
{
    ir_operation_block* source_ir = result->source_ir;

    ir_control_flow_node* working_node = create_and_insert_node(result->linier_nodes, source_ir->operations->first, source_ir->operations->last);

    std::unordered_map<int, ir_control_flow_node*> label_map;

    for (auto i = source_ir->operations->first; i != source_ir->operations->last; i = i->next)
    {
        ir_operation working_operation = i->data;

        if (is_label(&i->data))
        {
            if (working_node->entry_id != -1)
            {
                throw_error();
            }

            if (i != working_node->entry_instruction)
            {
                throw_error();
            }

            if (working_operation.sources.count == 1)
            {
                assert_is_constant(working_operation.sources[0]);

                working_node->entry_id = working_operation.sources[0].value;

                label_map[working_node->entry_id] = working_node;
            }
        }

        bool next_is_label = (i->next != nullptr && is_label(&i->next->data));

        if (is_jump(&working_operation) || next_is_label)
        {
            working_node->final_instruction = i;

            working_node = create_and_insert_node(result->linier_nodes, i->next, source_ir->operations->last);
        }
    }

    for (auto i = result->linier_nodes->first; i != nullptr; i = i->next)
    {
        if (i->data == nullptr)
            continue;

        if (i->data->final_instruction->data.instruction != ir_jump_if && i->data->final_instruction->data.instruction != ir_no_operation)
        {
            i->data->final_instruction = ir_operation_block::emits(result->source_ir, ir_no_operation, i->data->final_instruction);
        }

        auto last_operation = &i->data->final_instruction->data;

        if (last_operation->instruction == ir_jump_if)
        {
            int next_location = last_operation->sources[0].value;

            ir_operand condition = last_operation->sources[1];

            if (label_map.find(next_location) == label_map.end())
            {
                throw_error();
            }

            ir_control_flow_node* node_to_jump_to = label_map[next_location];

            if (ir_operand::is_constant(&condition))
            {
                if (condition.value)
                {
                    establish_outward_connection(i->data, node_to_jump_to);
                }
                else
                {
                    establish_outward_connection(i->data, i->next->data);
                }
            }
            else
            {
                establish_outward_connection(i->data, node_to_jump_to);
                establish_outward_connection(i->data, i->next->data);
            }
        }
        else if (i->next->data != nullptr)
        {
            establish_outward_connection(i->data, i->next->data);
        }
    }
}

ir_control_flow_graph* ir_control_flow_graph::create(ir_operation_block* source)
{
    ir_control_flow_graph* result = arena_allocator::allocate_struct<ir_control_flow_graph>(source->allocator);

    result->source_ir = source;
    result->linier_nodes = intrusive_linked_list<ir_control_flow_node*>::create(source->allocator,nullptr, nullptr);

    get_linier_nodes(result);

    return result;
}
