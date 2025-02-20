#include "ssa.h"
#include "tools/misc_tools.h"
#include "checks.h"
#include "linier_scan_register_allocator.h"

#include <unordered_set>

static phi_source create_phi_source(ssa_node* node, int source_register)
{
    phi_source result;

    result.node_source = node;
    result.source_register = source_register;

    return result;
}

static phi_node create_phi_node(ssa_node* node, uint64_t destination, std::vector<phi_source> sources)
{
    phi_node result;

    result.new_register = destination;
    result.sources = sources;
    result.node_context = node;

    return result;
}

static void create_ssa_context(ssa_context* result, ir_operation_block* ir)
{
    ir_control_flow_graph* cfg = ir_control_flow_graph::create(ir);

    for (auto i = cfg->linier_nodes->first; i != nullptr; i = i->next)
    {
        if (i->data == nullptr)
            continue;

        ssa_node* working_node = new ssa_node;

        working_node->context = result;
        working_node->raw_node = i->data;
        
        result->ssa_nodes.push_back(working_node);

        result->node_map[i->data] = working_node;
        result->reverse_node_map[working_node] = i->data;

        if (i->data->label_id != -1)
        {
            result->numbered_node_map[i->data->label_id] = working_node;
        }
    }

    for (auto i : result->ssa_nodes)
    {
        for (auto test = i->raw_node->entries->first; test != nullptr; test = test->next)
        {
            if (test->data == nullptr)
                continue;

            i->inlets.push_back(result->node_map[test->data]);
        }

        for (auto test = i->raw_node->exits->first; test != nullptr; test = test->next)
        {
            if (test->data == nullptr)
                continue;

            i->outlets.push_back(result->node_map[test->data]);
        }
    }

    result->raw_cfg = cfg;
    result->open_register = 0;
}

static uint64_t request_new_register(ssa_context* context)
{
    uint64_t result = context->open_register;
    
    context->open_register++;

    return result;
}

static void destroy_ssa_context(ssa_context* to_destroy)
{
    for (int i = 0; i < to_destroy->ssa_nodes.size(); ++i)
    {
        delete to_destroy->ssa_nodes[i];
    }
}

static void deconsturct_ssa(ssa_node* node)
{
    ir_operation_block* ir = node->context->raw_cfg->source_ir;

    for (auto phi : node->phis)
    {
        uint64_t new_destination = phi.new_register;
        
        for (auto source : phi.sources)
        {
            ssa_node* source_node = source.node_source;

            ir_operation_block::emitds(ir, ir_move, ir_operand::create_reg(phi.new_register,phi.type),ir_operand::create_reg(source.source_register,phi.type), source_node->raw_node->final_instruction->prev);

            source_node->local_to_global_move_count[source.source_register]++;
        }
    }
}

static void deconsturct_ssa(ssa_context* to_deconstruct)
{
    for (int i = 0; i < to_deconstruct->ssa_nodes.size(); ++i)
    {   
        ssa_node* working_node = to_deconstruct->ssa_nodes[i];

        deconsturct_ssa(working_node);
    }
}

static void redifine_destinations(ssa_node* node, int time, ir_operand* destinations, int destination_count)
{
    if (in_map(&node->time_remap, time))
    {
        node->time_remap[time] = std::unordered_map<uint64_t, uint64_t>();
    }

    std::unordered_map<uint64_t, uint64_t>* remap = &node->time_remap[time];

    for (int i = 0; i < destination_count; ++i)
    {
        ir_operand* working_destination = &destinations[i];
        
        assert_is_register(*working_destination);

        uint64_t new_register = request_new_register(node->context);

        (*remap)[working_destination->value] = new_register;

        working_destination->value = new_register;
    }
}

static uint64_t get_new_definition_at_time(ssa_node* node, uint64_t old_value, bool* exists,int time)
{
    *exists = false;

    for (; time != -2; time --)
    {
        std::unordered_map<uint64_t, uint64_t>* remap = &node->time_remap[time];

        if (!in_map(remap, old_value))
            continue;

        *exists = true;

        return (*remap)[old_value];
    }

    return -1;
}

static void get_phi_sources(std::vector<phi_source>* result, ssa_node* root_node, uint64_t old_value, std::unordered_set<ssa_node*>* visited)
{
    if (in_set(visited, root_node))
    {
        return;
    }
    
    visited->insert(root_node);

    bool exists;
    uint64_t possible_phi = get_new_definition_at_time(root_node, old_value, &exists, root_node->count);

    if (exists)
    {
        result->push_back(create_phi_source(root_node, possible_phi));

        return;
    }
    else
    {
        for (auto parent : root_node->inlets)
        {
            get_phi_sources(result, parent, old_value, visited);
        }
    }
}

static void redifine_source(ssa_node* node, int time, ir_operand* source)
{
    if (ir_operand::is_constant(source))
    {
        return;
    }

    bool exists;
    uint64_t new_value = get_new_definition_at_time(node, source->value, &exists, time);

    if (exists)
    {
        source->value = new_value;

        return;
    }

    uint64_t new_phi_register = request_new_register(node->context);

    std::vector<phi_source> phi_sources;
    std::unordered_set<ssa_node*> visited;

    for (auto parent : node->inlets)
    {
        get_phi_sources(&phi_sources, parent, source->value, &visited);
    }

    if (phi_sources.size() == 0)
    {
        return;
    }
    
    for (auto i : phi_sources)
    {
        node->time_remap[-1][source->value] = new_phi_register;
    }

    source->value = new_phi_register;

    phi_node phi = create_phi_node(node, new_phi_register, phi_sources);

    if (ir_operand::is_vector(source))
    {
        phi.type = int128;
    }
    else
    {
        phi.type = int64;
    }

    ir_operand phi_des = ir_operand::create_reg(new_phi_register, -1);
    ir_operand phi_src[phi_sources.size()];

    for (int i = 0; i < phi_sources.size(); ++i)
    {
        phi_src[i] = ir_operand::create_reg(phi_sources[i].source_register, -1);
    }

    //ir_operation_block::emit_with(node->context->raw_cfg->source_ir, ir_ssa_phi, &phi_des, 1, phi_src, phi_sources.size(), node->raw_node->entry_instruction);

    node->phis.push_back(phi);
}

static void redifine_sources(ssa_node* node, int time, ir_operand* sources, int source_count)
{
    for (int i = 0; i < source_count; ++i)
    {
        redifine_source(node, time, &sources[i]);
    }
}

static void construct_ssa_sources(ssa_node* node)
{
    int time = 0;

    for (auto i = node->raw_node->entry_instruction; i != node->raw_node->final_instruction->next; i = i->next)
    {
        ir_operation* working_operation = &i->data;

        if (working_operation->instruction == ir_ssa_phi)
            continue;

        redifine_sources(node, time - 1, working_operation->sources.data, working_operation->sources.count);

        ++time;
    }
}

static void construct_ssa_declarations(ssa_node* node)
{
    int time = 0;

    for (auto i = node->raw_node->entry_instruction; i != node->raw_node->final_instruction->next; i = i->next)
    {
        ir_operation* working_operation = &i->data;

        redifine_destinations(node, time, working_operation->destinations.data, working_operation->destinations.count);

        ++time;
    }

    node->count = time;
}

static void construct_ssa_declarations(ssa_context* context)
{
    for (int i = 0; i < context->ssa_nodes.size(); ++i)
    {
        construct_ssa_declarations(context->ssa_nodes[i]);
    }
}

static void construct_ssa_sources(ssa_context* context)
{
    for (int i = 0; i < context->ssa_nodes.size(); ++i)
    {
        construct_ssa_sources(context->ssa_nodes[i]);
    }
}

static void get_all_globals_from_phis(std::unordered_set<uint64_t>* globals, ssa_context* ssa)
{
    for (auto i : ssa->ssa_nodes)
    {
        for (auto phi : i->phis)
        {
            globals->insert(phi.new_register);
        }
    }
}

static bool is_global(std::unordered_set<uint64_t>* globals, uint64_t working_register)
{
    return in_set(globals, working_register);
}

static bool is_global(std::unordered_set<uint64_t>* globals, ir_operand working_register)
{
    if (ir_operand::is_constant(&working_register))
        return false;

    return is_global(globals, working_register.value);
}

static void get_used_local_registers(ssa_node* working_node, std::unordered_map<uint64_t, int>* usage_count)
{
    auto raw_node = working_node->raw_node;

    for (auto i = raw_node->entry_instruction; i != raw_node->final_instruction->next; i = i->next)
    {
        ir_operation* working_operation = &i->data;

        for (int o = 0; o < working_operation->sources.count; ++o)
        {
            ir_operand reg = working_operation->sources[o];

            if (ir_operand::is_constant(&reg))
                continue;

            (*usage_count)[reg.value]++;
        }
    }
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

static void nop_operation(ir_operation* to_nop)
{
    to_nop->instruction = ir_no_operation;
    to_nop->destinations.count = 0;
    to_nop->sources.count = 0;
}

static bool optimize_local_moves(ssa_node* working_node, std::unordered_set<uint64_t>* globals)
{
    auto raw_node = working_node->raw_node;

    for (auto i = raw_node->entry_instruction; i != raw_node->final_instruction->next; i = i->next)
    {
        ir_operation* working_operation = &i->data;

        if (working_operation->instruction != ir_move)
        {
            continue;
        }

        ir_operand destination = working_operation->destinations[0];

        if (is_global(globals, destination))
        {
            continue;
        }

        ir_operand to_replace = working_operation->sources[0];

        if (is_global(globals, to_replace))
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

        return false;
    }

    return true;
}

ir_operand* find_destination_from_source(ir_operation* working_operation, ir_operand to_find)
{
    for (int i = 0; i < working_operation->destinations.count; ++i)
    {
        if (working_operation->destinations[i].value == to_find.value)
        {
            return &working_operation->destinations[i];
        }
    }

    return nullptr;
}

static bool remove_unused_code(ssa_node* working_node, std::unordered_set<uint64_t>* globals)
{
    bool is_done = true;

    auto raw_node = working_node->raw_node;

    std::unordered_map<uint64_t, int> usage_count;

    get_used_local_registers(working_node, &usage_count);

    for (auto i = raw_node->entry_instruction; i != raw_node->final_instruction->next; i = i->next)
    {
        ir_operation* working_operation = &i->data;

        if (working_operation->instruction == ir_external_call)
            continue;

        if (working_operation->destinations.count != 1)
            continue;

        ir_operand destination = working_operation->destinations[0];

        if (is_global(globals, destination))
            continue;

        if (usage_count[destination.value] >= 1)
            continue;

        nop_operation(working_operation);
    }
    
    return is_done;
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

static void replace_registers(ir_operand* to_replace, int count, uint64_t old_register, uint64_t new_register)
{
    for (int i = 0; i < count; ++i)
    {
        ir_operand* working = &to_replace[i];

        if (ir_operand::is_constant(working))
        {
            continue;
        }

        if (working->value != old_register)
            continue;

        working->value = new_register;
    }
}

static void replace_registers(ir_operation* operation, uint64_t old_register, uint64_t new_register)
{
    replace_registers(operation->destinations.data, operation->destinations.count, old_register, new_register);
    replace_registers(operation->sources.data, operation->sources.count, old_register, new_register);
}

static bool optimize_global_moves(ssa_node* working_node, std::unordered_set<uint64_t>* globals)
{
    bool is_done = true;

    auto raw_node = working_node->raw_node;

    for (auto i = raw_node->entry_instruction; i != raw_node->final_instruction->next; i = i->next)
    {
        ir_operation* working_operation = &i->data;

        if (working_operation->instruction != ir_move)
            continue;

        ir_operand destination = working_operation->destinations[0];

        if (!is_global(globals, destination))
        {
            continue;
        }

        ir_operand source = working_operation->sources[0];

        if (ir_operand::is_constant(&source))
        {
            continue;
        }

        if (is_global(globals, source))
        {
            continue;
        }

        uint64_t dreg = destination.value;
        uint64_t sreg = source.value;

        if (dreg == sreg)
            continue;

        uint64_t times_moved = working_node->local_to_global_move_count[sreg];

        if (times_moved > 1)
        {
            continue;
        }

        for (auto tr = raw_node->entry_instruction; tr != raw_node->final_instruction->next; tr = tr->next)
        {
            replace_registers(&tr->data, sreg, dreg);
        }

        is_done = false;

        nop_operation(&i->data);
    }

    return is_done;
}

void convert_to_ssa(ir_operation_block* ir, bool optimize)
{    
    ir_operation_block::clamp_operands(ir, true);

    ssa_context ssa;

    create_ssa_context(&ssa, ir);

    construct_ssa_declarations(&ssa);

    construct_ssa_sources(&ssa);

    deconsturct_ssa(&ssa);

    std::unordered_set<uint64_t> globals;

    get_all_globals_from_phis(&globals, &ssa);

    while (true && optimize)
    {
        bool is_done = true;

        for (int i = 0; i < ssa.ssa_nodes.size(); ++i)
        {
            ssa_node* working_node = ssa.ssa_nodes[i];

            is_done &= optimize_global_moves(working_node, &globals);
        }

        if (is_done)
        {
            break;
        }
    }

    while (true && optimize)
    {
        bool is_done = true;

        for (int i = 0; i < ssa.ssa_nodes.size(); ++i)
        {
            ssa_node* working_node = ssa.ssa_nodes[i];

            is_done &= remove_unused_code(working_node, &globals);
            is_done &= optimize_local_moves(working_node, &globals);
            is_done &= optimize_math(working_node);
        }

        if (is_done)
        {
            break;
        }
    }

    linier_scan_register_allocator_pass(ssa.raw_cfg);

    destroy_ssa_context(&ssa);
}