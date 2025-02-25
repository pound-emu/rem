#include "ssa.h"
#include "tools/misc_tools.h"
#include "checks.h"
#include "linier_scan_register_allocator.h"
#include "assembly/pre_allocator_tools.h"

#include <unordered_set>

struct ssa_context;
struct ssa_node;
struct global_usage_location;

struct global_usage_location
{   
    int                                             time;
    ir_operand*                                     operand;

    bool                                            is_global;
    intrusive_linked_list<global_usage_location*>*  connections;
    bool                                            is_remapped;
};

struct ssa_node
{
    ssa_context*                                                        context;
    ir_control_flow_node*                                               raw_node;
    std::vector<std::unordered_map<uint64_t, global_usage_location*>>   declarations_global_info;

    std::unordered_map<int, std::unordered_map<uint64_t, uint64_t>>     declaration_time_map;

    std::vector<ssa_node*>                                              inlets;
    std::vector<ssa_node*>                                              outlets;
};

struct ssa_context
{
    ir_control_flow_graph*                                  cfg;
    ir_operation_block*                                     ir;
    std::vector<ssa_node*>                                  ssa_nodes;
    std::unordered_map<ir_control_flow_node*, ssa_node*>    ssa_node_map;
    uint64_t                                                global_top;
    uint64_t                                                local_start;

    std::vector<global_usage_location*>                     global_usage_pool;
};

static void connect_global_usages(global_usage_location* a, global_usage_location* b)
{
    intrusive_linked_list<global_usage_location*>::insert_element(a->connections, b);
    intrusive_linked_list<global_usage_location*>::insert_element(b->connections, a);
}

static global_usage_location* create_global_usage_location(ssa_context* context, ir_operand* operand, int time)
{
    global_usage_location* new_usage_data = arena_allocator::allocate_struct<global_usage_location>(context->ir->allocator);

    new_usage_data->operand = operand;
    new_usage_data->time = time;

    new_usage_data->is_global = false;    
    new_usage_data->connections = intrusive_linked_list<global_usage_location*>::create(context->ir->allocator, nullptr, nullptr);
    new_usage_data->is_remapped = false;

    context->global_usage_pool.push_back(new_usage_data);

    return new_usage_data;
}

static void create_ssa_context(ssa_context* result, ir_operation_block* ir)
{
    ir_control_flow_graph* cfg = ir_control_flow_graph::create(ir);

    for (auto i = cfg->linier_nodes->first; i != nullptr; i = i->next)
    {
        if (i->data == nullptr)
            continue;

        ssa_node* new_ssa_node = new ssa_node();
        
        new_ssa_node->context = result;
        new_ssa_node->raw_node = i->data;

        result->ssa_node_map[i->data] = new_ssa_node;

        result->ssa_nodes.push_back(new_ssa_node);
    }

    for (auto i : result->ssa_nodes)
    {
        for (auto c = i->raw_node->exits->first; c != nullptr; c = c->next)
        {
            if (c->data == nullptr)
                continue;

            i->outlets.push_back(result->ssa_node_map[c->data]);
        }

        for (auto c = i->raw_node->entries->first; c != nullptr; c = c->next)
        {
            if (c->data == nullptr)
                continue;

            i->inlets.push_back(result->ssa_node_map[c->data]);
        }
    }
    
    result->ir = ir;
    result->cfg = cfg;
}

static void destroy_ssa_context(ssa_context* to_destroy)
{
    for (auto i : to_destroy->ssa_nodes)
    {
        delete i;
    }
}

static bool look_for_and_connect_global_usage(ssa_node* look_at_node,global_usage_location* to_connect, uint64_t look_for_register, int time)
{
    for (; time != -1; -- time)
    {
        if (!in_map(&look_at_node->declarations_global_info[time], look_for_register))
            continue;

        global_usage_location* working_connection = look_at_node->declarations_global_info[time][look_for_register];

        connect_global_usages(working_connection, to_connect);

        //working_connection->is_global = true;

        return true;
    }

    return false;
}

static void find_register_in_parents(ssa_node* look_at_node, uint64_t look_for_register, global_usage_location* to_connect, std::unordered_set<ssa_node*>* visited)
{
    if (in_set(visited, look_at_node))
    {
        return;
    }

    visited->insert(look_at_node);

    if (look_for_and_connect_global_usage(look_at_node,to_connect, look_for_register, look_at_node->declarations_global_info.size() - 1))
    {
        return;
    }

    for (auto i : look_at_node->inlets)
    {
        find_register_in_parents(i, look_for_register, to_connect, visited);
    }
}

static void find_same_register_pools(ssa_node* node)
{
    auto raw_node = node->raw_node;

    int time = 0;

    for (auto ins_index = raw_node->entry_instruction; ins_index != raw_node->final_instruction->next; ins_index = ins_index->next)
    {   
        ir_operation* current_instruction = &ins_index->data;

        for (int i = 0; i < current_instruction->sources.count; ++i)
        {
            ir_operand* current_source = &current_instruction->sources[i];

            if (ir_operand::is_constant(current_source))
                continue;

            global_usage_location* this_global_usage = create_global_usage_location(node->context, current_source, time);

            this_global_usage->operand = current_source;
            
            if (look_for_and_connect_global_usage(node, this_global_usage, current_source->value, time - 1))
            {
                continue;
            }

            this_global_usage->is_global = true;

            for (auto inlet : node->inlets)
            {
                std::unordered_set<ssa_node*> visited;

                find_register_in_parents(inlet, current_source->value, this_global_usage, &visited);
            }
        }
        
        time++;
    }
}

static void create_time_stamped_declaration_info(ssa_context* context)
{
    for (auto ssa_node : context->ssa_nodes)
    {
        auto raw_node = ssa_node->raw_node;

        int time = 0;

        for (auto ins_index = raw_node->entry_instruction; ins_index != raw_node->final_instruction->next; ins_index = ins_index->next)
        {
            ir_operation* current_instruction = &ins_index->data;

            std::unordered_map<uint64_t, global_usage_location*> data_at_time_stamp;

            for (int i = 0; i < current_instruction->destinations.count; ++i)
            {
                ir_operand* current_destination = &current_instruction->destinations[i];
    
                data_at_time_stamp[current_destination->value] = create_global_usage_location(context, current_destination, time);
            }

            ssa_node->declarations_global_info.push_back(data_at_time_stamp);

            time++;
        }
    }
}

static void remap_global_usage(global_usage_location* to_remap, std::unordered_set<global_usage_location*>* visited, uint64_t new_register)
{
    if (in_set(visited, to_remap))
    {
        return;
    }

    to_remap->is_remapped = true;

    visited->insert(to_remap);

    to_remap->operand->value = new_register;

    for (auto i = to_remap->connections->first; i != nullptr; i = i->next)
    {
        if (i->data == nullptr)
            continue;

        remap_global_usage(i->data, visited, new_register);
    }
}

static uint64_t find_and_remap_true_globals(ssa_context* context)
{
    for (auto i : context->ssa_nodes)
    {
        find_same_register_pools(i);
    }

    std::vector<global_usage_location*> relevant_global_usages;

    for (auto i : context->global_usage_pool)
    {
        if (!i->is_global)
            continue;

        relevant_global_usages.push_back(i);
    }

    uint64_t global_remap_index = 0;

    while (true)
    {
        bool is_done = true;

        for (auto i : relevant_global_usages)
        {
            if (i->is_remapped)
                continue;

            std::unordered_set<global_usage_location*> visited;

            remap_global_usage(i, &visited, global_remap_index);

            global_remap_index++;

            is_done = false;
        }

        if (is_done)
        {
            break;
        }
    }

    return global_remap_index;
}

static bool is_global(ssa_context* context, uint64_t working_register)
{
    return working_register <= context->global_top;
}

static bool is_global(ssa_context* context, ir_operand working_register)
{
    if (ir_operand::is_constant(&working_register))
        return false;

    return is_global(context, working_register.value);
}

uint64_t look_for_local_at_time(ssa_node* node, int time, uint64_t to_look_for)
{
    for (; time != -1; time--)
    {   
        if (in_map(&node->declaration_time_map[time], to_look_for))
        {
            return node->declaration_time_map[time][to_look_for];
        }
    }

    throw_error();
}

static void redifine_destinations(ssa_context* context)
{
    context->local_start = context->global_top;

    for (auto node : context->ssa_nodes)
    {
        ir_control_flow_node* raw_node = node->raw_node;

        int time = 0;

        for (auto ins_index = raw_node->entry_instruction; ins_index != raw_node->final_instruction->next; ins_index = ins_index->next)
        {
            ir_operation* working_operation = &ins_index->data;

            ir_operand* destinations = working_operation->destinations.data;
            int destination_count = working_operation->destinations.count;

            for (int i = 0; i < destination_count; ++i)
            {
                ir_operand* working_destination = &destinations[i];

                if (is_global(context, *working_destination))
                {
                    continue;
                }

                uint64_t new_local_register = context->local_start ++;

                node->declaration_time_map[time][working_destination->value] = new_local_register;
                working_destination->value = new_local_register;
            }

            time++;
        }
    }
}

static void redifine_sources(ssa_context* context)
{
    for (auto node : context->ssa_nodes)
    {
        ir_control_flow_node* raw_node = node->raw_node;

        int time = 0;

        for (auto ins_index = raw_node->entry_instruction; ins_index != raw_node->final_instruction->next; ins_index = ins_index->next)
        {
            ir_operation* working_operation = &ins_index->data;

            ir_operand* sources = working_operation->sources.data;
            int source_count = working_operation->sources.count;

            for (int i = 0; i < source_count; ++i)
            {
                ir_operand* working_source = &sources[i];

                if (ir_operand::is_constant(working_source))
                {
                    continue;
                }

                if (is_global(context, *working_source))
                {
                    continue;
                }

                uint64_t new_value = look_for_local_at_time(node, time - 1, working_source->value);

                working_source->value = new_value;
            }

            time++;
        }
    }
}

static bool check_if_zero_extend(ir_operation* working_operation, ir_operand check_register)
{
    for (int i = 0; i < working_operation->sources.count; ++i)
    {
        ir_operand* working_check = &working_operation->sources[i];

        if (ir_operand::is_constant(working_check))
        {
            continue;
        }

        if (working_check->value != check_register.value)
        {
            continue;
        }
        
        if (ir_operand::get_raw_size(working_check) > ir_operand::get_raw_size(&check_register))
        {
            return true;
        }
    }

    return false;
}

static void replace_sources_if_needed(ir_operation* working_operation, uint64_t to_replace_register,ir_operand to_replace_with)
{
    for (int i = 0; i < working_operation->sources.count; ++i)
    {
        ir_operand* working_to_replace = &working_operation->sources[i];

        if (ir_operand::is_constant(working_to_replace))
        {
            continue;
        }

        if (working_to_replace->value != to_replace_register)
        {
            continue;
        }
        
        *working_to_replace = ir_operand::copy_new_raw_size(to_replace_with, working_to_replace->meta_data);
    }
}

static void nop_operation(ir_operation* to_nop)
{
    to_nop->instruction = ir_no_operation;
    to_nop->destinations.count = 0;
    to_nop->sources.count = 0;
}

static void convert_to_move(ir_operation* working_operation, ir_operand value)
{
    working_operation->instruction = ir_move;
    working_operation->sources.count = 1;

    working_operation->sources[0] = ir_operand::copy_new_raw_size(value, working_operation->sources[0].meta_data);
}

static bool check_constant(ir_operand operand, uint64_t value)
{
    return ir_operand::is_constant(&operand) && operand.value == value;
}

static bool check_constant(ir_operand operand)
{
    return ir_operand::is_constant(&operand);
}

static bool optimize_math(ssa_node* working_node)
{
    bool is_done = true;

    auto raw_node = working_node->raw_node;

    for (auto i = raw_node->entry_instruction; i != raw_node->final_instruction->next; i = i->next)
    {
        ir_operation* working_operation = &i->data;

        ir_operand* des = working_operation->destinations.data;
        ir_operand* src = working_operation->sources.data;

        switch (working_operation->instruction)
        {
            case ir_bitwise_and:
            {
                if (check_constant(src[0], 0) || check_constant(src[1], 0))
                {
                    convert_to_move(working_operation, ir_operand::create_con(0));

                    is_done = false;
                }
                else if (ir_operand::are_equal(src[0], src[1]))
                {
                    convert_to_move(working_operation, src[0]);

                    is_done = false;
                }
                else if (check_constant(src[0]) && check_constant(src[1]))
                {
                    uint64_t value = src[0].value & src[1].value;

                    convert_to_move(working_operation, ir_operand::create_con(value, des[0].meta_data));

                    is_done = false;
                }
            }; break;

            case ir_multiply:
            {
                if (check_constant(src[0]) && check_constant(src[1]))
                {
                    uint64_t value = src[0].value * src[1].value;

                    convert_to_move(working_operation, ir_operand::create_con(value, des[0].meta_data));

                    is_done = false;
                }
            }; break;

            case ir_bitwise_or:
            {
                if (check_constant(src[0], 0))
                {
                    convert_to_move(working_operation, src[1]);

                    is_done = false;
                }
                else if (check_constant(src[1], 0))
                {
                    convert_to_move(working_operation, src[0]);

                    is_done = false;
                }
                else if (check_constant(src[0]) && check_constant(src[1]))
                {
                    uint64_t value = src[0].value | src[1].value;

                    convert_to_move(working_operation, ir_operand::create_con(value, des[0].meta_data));

                    is_done = false;
                }
                else if (check_constant(src[0]) && check_constant(src[1]))
                {
                    uint64_t value = src[0].value | src[1].value;

                    convert_to_move(working_operation, ir_operand::create_con(value, des[0].meta_data));

                    is_done = false;
                }
            }; break;

            case ir_bitwise_exclusive_or:
            {
                if (check_constant(src[0], 0))
                {
                    convert_to_move(working_operation, src[1]);

                    is_done = false;
                }
                else if (check_constant(src[1], 0))
                {
                    convert_to_move(working_operation, src[0]);
                    
                    is_done = false;
                }
                else if (check_constant(src[0]) && check_constant(src[1]))
                {
                    uint64_t value = src[0].value ^ src[1].value;

                    convert_to_move(working_operation, ir_operand::create_con(value, des[0].meta_data));

                    is_done = false;
                }
            }; break;

            case ir_add:
            {
                if (check_constant(src[0], 0))
                {
                    convert_to_move(working_operation, src[1]);

                    is_done = false;
                } 
                else if (check_constant(src[1], 0))
                {
                    convert_to_move(working_operation, src[0]);

                    is_done = false;
                } 
                else if (check_constant(src[1], 1))
                {
                    working_operation->instruction = ir_incrament;
                    working_operation->sources.count = 1;

                    is_done = false;
                }
                else if (check_constant(src[0]) && check_constant(src[1]))
                {
                    uint64_t value = src[0].value + src[1].value;

                    convert_to_move(working_operation, ir_operand::create_con(value, des[0].meta_data));

                    is_done = false;
                }
            }; break;

            case ir_subtract:
            {
                if (check_constant(src[1], 0))
                {
                    convert_to_move(working_operation, src[0]);

                    is_done = false;
                } 
                else if (check_constant(src[0], 0))
                {
                    working_operation->instruction = ir_negate;
                    working_operation->sources.count = 1;

                    src[0] = src[1];

                    is_done = false;
                } 
                else if (check_constant(src[1], 1))
                {
                    working_operation->instruction = ir_decrament;
                    working_operation->sources.count = 1;

                    is_done = false;
                }
                else if (check_constant(src[0]) && check_constant(src[1]))
                {
                    uint64_t value = src[0].value - src[1].value;

                    convert_to_move(working_operation, ir_operand::create_con(value, des[0].meta_data));

                    is_done = false;
                }

            }; break;

            case ir_decrament:
            {
                if (check_constant(src[0]))
                {
                    uint64_t value = src[0].value;

                    convert_to_move(working_operation, ir_operand::create_con(value - 1, des[0].meta_data));
                }
            }; break;

            case ir_incrament:
            {
                if (check_constant(src[0]))
                {
                    uint64_t value = src[0].value;

                    convert_to_move(working_operation, ir_operand::create_con(value + 1, des[0].meta_data));
                }
            }; break;

            case ir_bitwise_not:
            {
                if (check_constant(src[0]))
                {
                    uint64_t value = src[0].value;

                    convert_to_move(working_operation, ir_operand::create_con(~value, des[0].meta_data));
                }
            }; break;

            case ir_negate:
            {
                if (check_constant(src[0]))
                {
                    uint64_t value = src[0].value;

                    convert_to_move(working_operation, ir_operand::create_con(~value + 1, des[0].meta_data));
                }
            }; break;

            case ir_logical_not:
            {
                if (check_constant(src[0]))
                {
                    uint64_t value = src[0].value;

                    convert_to_move(working_operation, ir_operand::create_con(value == 0, des[0].meta_data));
                }
            }; break;

            case ir_compare_equal:
            {
                if (check_constant(src[0]) && check_constant(src[1]))
                {
                    uint64_t value = src[0].value == src[1].value;

                    convert_to_move(working_operation, ir_operand::create_con(value, des[0].meta_data));

                    is_done = false;
                }
            }; break;

            case ir_compare_not_equal:
            {
                if (check_constant(src[0]) && check_constant(src[1]))
                {
                    uint64_t value = src[0].value != src[1].value;

                    convert_to_move(working_operation, ir_operand::create_con(value, des[0].meta_data));

                    is_done = false;
                }
            }; break;

            case ir_shift_left:
            case ir_shift_right_unsigned:
            case ir_shift_right_signed:
            case ir_rotate_right:
            {
                if (check_constant(src[1], 0))
                {
                    convert_to_move(working_operation, src[0]);

                    is_done = false;
                } 
                else if (check_constant(src[0], 0))
                {
                    convert_to_move(working_operation, ir_operand::create_con(0));

                    is_done = false;
                }

            }; break;
        }
    }

    return is_done;
}

static bool optimize_unused_code(ssa_node* working_node)
{
    bool is_done = true;

    ssa_context* context = working_node->context;
    auto raw_node = working_node->raw_node;

    std::unordered_set<uint64_t> used_locals;

    for (auto i = raw_node->entry_instruction; i != raw_node->final_instruction->next; i = i->next)
    {
        ir_operation* working_operation = &i->data;

        for (int o = 0; o < working_operation->sources.count; ++o)
        {
            ir_operand working_operand = working_operation->sources[o];

            if (ir_operand::is_constant(&working_operand))
            {
                continue;
            }

            if (is_global(context, working_operand))
            {
                continue;
            }

            used_locals.insert(working_operand.value);
        }
    }

    for (auto i = raw_node->entry_instruction; i != raw_node->final_instruction->next; i = i->next)
    {
        ir_operation* working_operation = &i->data;

        if (working_operation->destinations.count != 1)
            continue;

        if (working_operation->instruction == ir_external_call)
            continue;

        ir_operand destination = working_operation->destinations[0];

        if (is_global(context, destination))
            continue;

        if (in_set(&used_locals, destination.value))
            continue;

        nop_operation(working_operation);

        is_done = false;
    }

    return is_done;
}

static void loop_through_operands_find_usage_count(ir_operand* operands, int count, std::unordered_map<uint64_t, uint64_t>* counts)
{
    for (int i = 0; i < count; ++i)
    {
        ir_operand test = operands[i];

        if (ir_operand::is_constant(&test))
        {
            continue;
        }

        (*counts)[test.value]++;
    }
}

static bool check_for_register_in_instruction(ir_operand* operands,int count, uint64_t to_check)
{
    for (int i = 0; i < count; ++i)
    {
        if (ir_operand::is_constant(operands + i))
            continue;

        if (operands[i].value != to_check)
            continue;

        return true;
    }

    return false;
}

static bool check_for_register_in_instruction(ir_operation* operation, uint64_t to_check)
{
    return check_for_register_in_instruction(operation->destinations.data, operation->destinations.count, to_check) | check_for_register_in_instruction(operation->sources.data, operation->sources.count, to_check);
}

void copy_operands(ir_operand* destination,ir_operand* source, int count)
{  
    memcpy(destination, source, sizeof(ir_operand) * count);
}

static void replace_jump_with_condition(ir_operation_block* ir, ir_operand label, ir_operation* working_operation, ir_operation* condition_source, ir_instructions new_instruction)
{
    fast_array<ir_operand> new_sources;
    fast_array<ir_operand>::create(ir->allocator, 3, &new_sources);

    new_sources[0] = label;
    new_sources[1] = condition_source->sources[0];
    new_sources[2] = condition_source->sources[1];

    nop_operation(condition_source);
    working_operation->sources = new_sources;
    working_operation->instruction = new_instruction;
}

static bool optimize_multiple_instructions(ssa_node* working_node)
{
    auto raw_node = working_node->raw_node;

    ssa_context* context = working_node->context;
    ir_operation_block* ir = context->ir;

    bool is_done = true;

    std::unordered_map<uint64_t, uint64_t>                      usage_count;
    std::unordered_map<uint64_t, ir_operation*>                 declaration_location;

    int time = 0;

    for (auto i = raw_node->entry_instruction; i != raw_node->final_instruction->next; i = i->next)
    {
        time++;

        ir_operation* working_operation = &i->data;

        loop_through_operands_find_usage_count(working_operation->sources.data, working_operation->sources.count, &usage_count);

        if (working_operation->destinations.count != 1)
            continue;

        declaration_location[working_operation->destinations[0].value] = working_operation;
    }

    time = 0;

    for (auto i = raw_node->entry_instruction; i != raw_node->final_instruction->next; i = i->next)
    {
        time++;

        ir_operation* working_operation = &i->data;

        ir_operand* src = working_operation->sources.data;
        ir_operand* des = working_operation->destinations.data;

        if (instruction_is_commutative(working_operation->instruction) && working_operation->sources.count == 2 && ir_operand::is_constant(&src[0]) && ir_operand::is_register(&src[1]))
        {
            swap_operands(&working_operation->sources[0], &working_operation->sources[1]);
        }

        switch (working_operation->instruction)
        {
            case ir_zero_extend:
            {
                ir_operand destination = des[0];
                ir_operand source = src[0];

                if (is_global(context, source))
                {
                    continue;
                }

                if (is_global(context, destination))
                {
                    continue;
                }

                if (ir_operand::is_constant(&source))
                {
                    continue;
                }

                ir_operation* source_operation = declaration_location[source.value];

                if (source_operation->destinations.count != 1)
                {
                    continue;
                }

                bool is_compare = ir_operation_block::is_compare(source_operation);

                if (!is_compare && ir_operand::get_raw_size(&source_operation->destinations[0]) != ir_operand::get_raw_size(&source))
                    continue;

                working_operation->instruction = ir_move;

                des[0].meta_data = int64;
                src[0].meta_data = int64;

                is_done = false;
            }; break;

            case ir_bitwise_exclusive_or:
            {
                if (ir_operand::is_register(&src[0]) && check_constant(src[1], 1))
                {
                    ir_operand flipped_operand = src[0];

                    if (is_global(context,flipped_operand))
                    {
                        continue;
                    }

                    if (usage_count[flipped_operand.value] > 1)
                    {
                        continue;
                    }

                    ir_operation* source_operation = declaration_location[flipped_operand.value];

                    if (source_operation == nullptr)
                        continue;

                    switch (source_operation->instruction)
                    {
                        case ir_compare_equal:
                        {
                            source_operation->instruction = ir_compare_not_equal;

                            convert_to_move(working_operation, src[0]);

                            is_done = false;
                        }; break;

                        case ir_compare_not_equal:
                        {
                            source_operation->instruction = ir_compare_equal;

                            convert_to_move(working_operation, src[0]);

                            is_done = false;
                        }; break;

                    }
                }
            }; break;

            case ir_load:
            case ir_store:
            {
                switch (working_operation->instruction)
                {
                    case ir_load:   if (working_operation->sources.count != 1) continue; break;
                    case ir_store:  if (working_operation->sources.count != 2) continue; break;
                }

                ir_operand address_operand = working_operation->sources[0];

                if (is_global(context, address_operand))
                {
                    continue;
                }

                if (usage_count[address_operand.value] > 1)
                {
                    continue;
                }

                ir_operation* source_operation = declaration_location[address_operand.value];

                if (source_operation == nullptr)
                    continue;

                if (source_operation->instruction != ir_add)
                {
                    continue;
                }

                ir_operand new_sources[3];

                switch (working_operation->instruction)
                {
                    case ir_load:
                    {
                        copy_operands(new_sources, source_operation->sources.data, 2);
        
                        ir_operation_block::emit_with(ir, working_operation->instruction, working_operation->destinations.data, 1, new_sources, 2, i);
                    }; break;

                    case ir_store:
                    {
                        copy_operands(new_sources, source_operation->sources.data, 2);
        
                        new_sources[2] = working_operation->sources[1];
        
                        ir_operation_block::emit_with(ir, working_operation->instruction, nullptr, 0, new_sources, 3, i);
                    }; break;
                }

                nop_operation(working_operation);
                nop_operation(source_operation);

                is_done = false;

            }; break;

            case ir_jump_if:
            {
                ir_operand condition = src[1];
                ir_operand label = src[0];

                if (ir_operand::is_constant(&condition))
                {
                    continue;
                }

                if (is_global(context, condition))
                {
                    continue;
                }

                if (usage_count[condition.value] > 1)
                {
                    continue;
                }

                ir_operation* condition_source = declaration_location[condition.value];

                switch (condition_source->instruction)
                {
                    case ir_compare_not_equal:
                    {                        
                        replace_jump_with_condition(ir, label, working_operation, condition_source, ir_jump_if_not_equal);

                        is_done = false;
                    }; break;

                    case ir_compare_equal:
                    {
                        replace_jump_with_condition(ir, label, working_operation, condition_source, ir_jump_if_equal);

                        is_done = false;
                    }; break;
                
                default:
                    break;
                }

            }; break;

            case ir_move:
            {
                if (!is_global(context, des[0]))
                {
                    continue;
                }

                if (is_global(context, src[0]))
                {
                    continue;
                }

                ir_operand destination_operand = des[0];
                ir_operand source_operand = src[0];

                if (ir_operand::is_constant(&source_operand))
                {
                    continue;
                }

                ir_operation* local_declared_operation = declaration_location[source_operand.value];

                if (local_declared_operation == nullptr)
                {
                    continue;
                }

                if (usage_count[source_operand.value] > 1)
                {
                    continue;
                }

                if (local_declared_operation->destinations.count == 0)
                {
                    continue;
                }

                bool is_valid = true;

                for (auto b = i->prev; b != raw_node->entry_instruction->prev; b = b->prev)
                {
                    ir_operation* check_operation = &b->data;

                    if (check_operation == local_declared_operation)
                    {
                        break;
                    }

                    if (check_for_register_in_instruction(check_operation, destination_operand.value))
                    {
                        is_valid = false;

                        break;
                    }
                }

                if (!is_valid)
                {
                    continue;
                }

                for (int o = 0; o < local_declared_operation->destinations.count; ++o)
                {
                    ir_operand* replace_candidate = &local_declared_operation->destinations.data[o];

                    if (replace_candidate->value == source_operand.value)
                    {
                        if (replace_candidate->meta_data != source_operand.meta_data)
                        {
                            //is_valid = false;

                            //IN A LOT OF CACES, THIS CAN BE IGNORED
                            //TODO, FIND THOSE CASES

                            //break;
                        }

                        replace_candidate->value = destination_operand.value;
                    }
                }

                if (!is_valid)
                {
                    continue;
                }

                nop_operation(working_operation);

                is_done = false;

            }; break;
        
        default:
            break;
        }
    }

    return is_done;
}

static bool optimize_local_moves(ssa_node* working_node)
{
    auto raw_node = working_node->raw_node;

    ssa_context* context = working_node->context;

    bool is_done = true;

    for (auto i = raw_node->entry_instruction; i != raw_node->final_instruction->next; i = i->next)
    {
        ir_operation* working_operation = &i->data;

        if (working_operation->instruction != ir_move)
        {
            continue;
        }

        ir_operand destination = working_operation->destinations[0];

        if (is_global(context, destination))
        {
            continue;
        }

        ir_operand to_replace = working_operation->sources[0];

        if (is_global(context, to_replace))
        {
            continue;
        }

        bool is_zero_extend_check = false;

        for (auto r = i->next; r != raw_node->final_instruction->next; r = r->next)
        {
            if (check_if_zero_extend(&r->data, destination))
            {
                is_zero_extend_check = true;

                break;
            }
        }

        if (is_zero_extend_check)
        {
            working_operation->instruction = ir_zero_extend;

            continue;
        }

        for (auto r = i->next; r != raw_node->final_instruction->next; r = r->next)
        {
            replace_sources_if_needed(&r->data, destination.value,to_replace);
        }

        nop_operation(working_operation);

        is_done = false;
    }

    return is_done;
}

void convert_to_ssa(ir_operation_block* ir, bool optimize)
{    
    ir_operation_block::clamp_operands(ir, true);

    ssa_context ssa;

    create_ssa_context(&ssa,ir);

    create_time_stamped_declaration_info(&ssa);

    ssa.global_top = find_and_remap_true_globals(&ssa);

    redifine_destinations(&ssa);

    redifine_sources(&ssa);

    while (optimize)
    {
        bool is_done = true;

        for (int i = 0; i < ssa.ssa_nodes.size(); ++i)
        {
            ssa_node* working_node = ssa.ssa_nodes[i];

            is_done &= optimize_local_moves(working_node);
            is_done &= optimize_math(working_node);
            is_done &= optimize_unused_code(working_node);
            is_done &= optimize_multiple_instructions(working_node);
        }

        if (is_done)
        {
            break;
        }
    }

    linier_scan_register_allocator_pass(ssa.cfg);

    destroy_ssa_context(&ssa);
}