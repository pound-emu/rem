#include "ssa.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "ir/checks.h"

struct ssa_context;

struct ssa_cf_node
{
    int                                                             location;
    ssa_context*                                                    context;
    ir_control_flow_node*                                           raw_node;

    std::unordered_set<ssa_cf_node*>                                outlets;
    std::unordered_set<ssa_cf_node*>                                inlets;

    std::unordered_map<int, std::unordered_map<uint64_t, uint64_t>> working_remap;
    int                                                             time_end;
};

struct ssa_context
{
    ir_operation_block*                                         ir;
    ir_operation_block*                                         source;
    std::unordered_map<ir_control_flow_node*, ssa_cf_node*>     raw_node_map;
    std::unordered_map<int, ssa_cf_node*>                       labeled_node_map;
    std::unordered_set<uint64_t>                                globals;
    std::vector<intrusive_linked_list_element<ir_operation>*>   phi_nodes;

    int                                                         local_bottom;   
};

struct lrsa_register_data
{
    int first_use;
    int last_use;
};

struct lrsa_event
{
    bool        is_birth;
    uint64_t    reg;
};

struct lrsa_context
{
    std::unordered_map<uint64_t, uint64_t>*     result;
    std::unordered_set<uint64_t>                taken;
};

static uint64_t request_lrsa_register(lrsa_context* context)
{
    for (int i = 0; ; ++i)
    {
        if (context->taken.find(i) != context->taken.end())
            continue;

        context->taken.insert(i);

        return i;
    }

    throw_error();
}

static uint64_t remove_lrsa_register(lrsa_context* context, int to_remove)
{
    context->taken.erase(to_remove);
}

static bool is_global(ssa_context* context, uint64_t check)
{
    return context->globals.find(check) != context->globals.end();
}

static bool is_global(ssa_context* context, ir_operand check)
{
    return is_global(context, check.value);
}

static ssa_cf_node create_ssa_cf_node(ssa_context* context, ir_control_flow_node* node)
{
    ssa_cf_node result;

    result.context = context;
    result.raw_node = node;
    result.location = -1;

    ir_operation* first_instruction = &node->entry_instruction->data;

    if (first_instruction->instruction == ir_mark_label && first_instruction->sources.count == 1)
    {
        ir_operand label = first_instruction->sources[0];

        assert_is_constant(label);

        result.location = label.value;
    }

    return result;
}

static void create_forward_connection(ssa_cf_node* from, ssa_cf_node* to)
{
    from->outlets.insert(to);
    to->inlets.insert(from);
}

static int request_new_register(ssa_context* context)
{
    int new_register = context->local_bottom;
    context->local_bottom++;

    return new_register;
}

static uint64_t get_remap_at_time(ssa_cf_node* node, uint64_t old, int time)
{
    for (; time != -1; -- time)
    {
        if (node->working_remap.find(time) == node->working_remap.end())
            continue;

        auto working_remap_at_time = node->working_remap[time];

        if (working_remap_at_time.find(old) == working_remap_at_time.end())
        {
            continue;
        }

        return working_remap_at_time[old];
    }

    throw_error();    
}

static int create_phi(ssa_cf_node* node, std::vector<uint64_t>* sources)
{
    ssa_context* context = node->context;
    
    int result = request_new_register(context);

    ir_operand phi_sources[sources->size()];
    ir_operand destination = ir_operand::create_reg(result, int64);

    for (int i = 0; i < sources->size(); ++i)
    {
        context->globals.insert((*sources)[i]);

        phi_sources[i] = ir_operand::create_reg((*sources)[i], int64);
    }

    auto phi_instruction = ir_operation_block::emit_with(context->ir, ir_ssa_phi, &destination, 1, phi_sources, sources->size(), node->raw_node->entry_instruction->prev);

    context->phi_nodes.push_back(phi_instruction);

    context->globals.insert(result);

    return result;
}

static void remap_sources(ssa_cf_node* node, ir_operand* sources, int source_count, int time)
{
    ssa_context* context = node->context;

    for (int i = 0; i < source_count; ++i)
    {
        ir_operand* source = &sources[i];

        if (ir_operand::is_constant(source))
            continue;
        
        uint64_t old_value = source->value;

        if (context->globals.find(old_value) != context->globals.end())
        {
            continue;
        }
        
        source->value = get_remap_at_time(node, old_value, time - 1);
    }
}

static void declare_destinations(ssa_cf_node* node, ir_operand* destinations, int destination_count, int time)
{
    ssa_context* context = node->context;

    for (int i = 0; i < destination_count; ++i)
    {
        ir_operand* destination = &destinations[i];

        assert_is_register(*destination);

        if (is_global(context, destination->value))
        {
            continue;
        }
        
        int new_register = request_new_register(context);

        node->working_remap[time][destination->value] = new_register;
        destination->value = new_register;
    }
}

static void construct_ssa_destinations(ssa_context* ctx, ssa_cf_node* node)
{
    int time = 0;

    for (auto i = node->raw_node->entry_instruction; i != node->raw_node->final_instruction->next; i = i->next)
    {
        declare_destinations(node, i->data.destinations.data, i->data.destinations.count, time);

        ++time;
    }

    node->time_end = time;
}

static void remap_ssa_sources(ssa_context* ctx, ssa_cf_node* node)
{
    int time = 0;

    for (auto i = node->raw_node->entry_instruction; i != node->raw_node->final_instruction->next; i = i->next)
    {
        if (i->data.instruction == ir_ssa_phi)
        {
            ir_operation_block::log(ctx->ir);

            throw_error();
        }

        remap_sources(node, i->data.sources.data, i->data.sources.count, time);

        ++time;
    }
}

void append_to_hashset(std::unordered_set<uint64_t>* result, std::vector<uint64_t>* source)
{
    for (auto i : *source)
    {
        result->insert(i);
    }
}

static void append_usage_data(std::unordered_map<uint64_t, std::unordered_set<ssa_cf_node*>>* usage, std::unordered_map<uint64_t, std::unordered_map<ssa_cf_node*, uint64_t>>* total_register_usage_count, ssa_cf_node* node, ir_operand* operands, int count)
{
    for (int i = 0; i < count; ++i)
    {
        ir_operand working = operands[i];

        if (ir_operand::is_constant(&working))
            continue;

        if (usage != nullptr)
        {
            (*usage)[working.value].insert(node);
        }

        if (total_register_usage_count != nullptr)
        {
            (*total_register_usage_count)[working.value][node]++;
        }
    }
}

static void append_usage_data_global(std::unordered_map<uint64_t, std::unordered_set<ssa_cf_node*>>* usage,std::unordered_map<uint64_t, std::unordered_map<ssa_cf_node*, uint64_t>>* total_register_usage_count, ssa_cf_node* node)
{
    for (auto i = node->raw_node->entry_instruction; i != node->raw_node->final_instruction->next; i = i->next)
    {
        auto working = i->data;

        append_usage_data(usage, total_register_usage_count, node, working.destinations.data, working.destinations.count);
        append_usage_data(usage, total_register_usage_count, node, working.sources.data, working.sources.count);
    }
}

static void set_usage_data(std::unordered_map<uint64_t, lrsa_register_data>* usage_data,ir_operand* operands, int operand_count, int time)
{
    for (int i = 0; i < operand_count; ++i)
    {
        ir_operand working = operands[i];

        if (ir_operand::is_constant(&working))
            continue;

        int raw_register = working.value;

        if (usage_data->find(raw_register) == usage_data->end())
        {
            (*usage_data)[raw_register] = {INT32_MAX, INT32_MIN};
        }

        lrsa_register_data* working_data = &(*usage_data)[raw_register];

        if (time < working_data->first_use)
        {
            working_data->first_use = time;
        }

        if (time > working_data->last_use)
        {
            working_data->last_use = time;
        }
    }
}

static void birth_register(lrsa_context* context, lrsa_event* event)
{
    int new_register = request_lrsa_register(context);

    uint64_t old_register = event->reg;

    if (context->result->find(old_register) != context->result->end())
    {
        throw_error();
    }

    (*context->result)[old_register] = new_register;
}

static void kill_register(lrsa_context* context, lrsa_event* event)
{
    uint64_t old_register = event->reg;

    if (context->result->find(old_register) == context->result->end())
    {
        throw_error();
    }

    uint64_t remapped_register = (*context->result)[old_register];

    if (context->taken.find(remapped_register) == context->taken.end())
    {
        throw_error();
    }

    context->taken.erase(remapped_register);
}

/*
    WARNING: THERE IS A BUG

    loops with global state WILL break with
    this implementation. But, all current jits
    produce linier code, so for now we don't
    have to worry about this. EVENTUALLY i want
    to account for it.
*/
static void linier_scan_register_allocator_pass(ir_operation_block* source)
{
    int time = 0;

    std::unordered_map<uint64_t, lrsa_register_data> register_usage_data;

    for (auto i = source->operations->first; i != nullptr; i = i->next)
    {
        ir_operation* working_operation = &i->data;

        set_usage_data(&register_usage_data, working_operation->sources.data, working_operation->sources.count, time);
        set_usage_data(&register_usage_data, working_operation->destinations.data, working_operation->destinations.count, time);

        ++time;
    }

    std::vector<std::vector<lrsa_event>> events;

    for (int i = 0; i < time; ++i)
    {
        events.push_back(std::vector<lrsa_event>());
    }

    for (auto i : register_usage_data)
    {
        events[i.second.first_use].push_back({true, i.first});
        events[i.second.last_use].push_back({false, i.first});
    }

    std::unordered_map<uint64_t, uint64_t>  final_remap;

    lrsa_context context;
    context.result = &final_remap;

    for (int i = 0; i < events.size(); ++i)
    {
        std::vector<lrsa_event>* working_events = &events[i];

        for (auto e : *working_events)
        {
            if (e.is_birth)
            {
                birth_register(&context, &e);
            }
            else
            {
                kill_register(&context, &e);
            }
        }
    }
    
    ir_operation_block::ssa_remap(source, &final_remap);

    //ir_operation_block::log(source);

    //std::cin.get();
}

static void replace_sources_if_same(ir_operand* to_replace_buffer, int to_replace_count,int old_register, ir_operand new_operand)
{
    for (int i = 0; i < to_replace_count; ++i)
    {
        ir_operand* to_replace = &to_replace_buffer[i];

        if (ir_operand::is_constant(to_replace))
            continue;

        if (to_replace->value != old_register)
            continue;

        to_replace_buffer[i] = ir_operand::copy_new_raw_size(new_operand, to_replace->meta_data);
    }
}

static bool check_size_greater(ir_operand* check, int to_replace_count,int old_register, int size_check)
{
    for (int i = 0; i < to_replace_count; ++i)
    {
        ir_operand* to_check = &check[i];

        if (ir_operand::is_constant(to_check))
            continue;

        if (to_check->value != old_register)
            continue;

        if (to_check->meta_data > size_check)
        {
            return true;
        }
    }

    return false;
}

static bool check_if_local_used(ir_operand* check, int to_replace_count,int old_register)
{
    for (int i = 0; i < to_replace_count; ++i)
    {
        ir_operand* to_check = &check[i];

        if (ir_operand::is_constant(to_check))
            continue;

        if (to_check->value == old_register)
            return true;
    }

    return false;
}

static void set_operand(ir_operation* operation, ir_operand new_opernad)
{
    operation->instruction = ir_move;
    operation->sources[0] = ir_operand::copy_new_raw_size(new_opernad, operation->sources[0].meta_data);

    operation->sources.count = 1;    
    operation->destinations.count = 1;
}

static bool check_constant(ir_operand check, uint64_t value)
{
    if (!ir_operand::is_constant(&check))
        return false;

    return check.value == value;
}

static uint64_t get_mask(int size)
{
    switch (size)
    {
        case int8:  return UINT8_MAX;
        case int16: return UINT16_MAX;
        case int32: return UINT32_MAX;
        case int64: return UINT64_MAX;
    }

    throw_error();
}

static void empty_operation(ir_operation* operation)
{
    operation->sources.count = 0;
    operation->destinations.count = 0;
    operation->instruction = ir_no_operation;
}

static bool perform_move_propagation(ssa_cf_node* node)
{
    bool done = true;

    ssa_context* ctx = node->context;

    for (auto i = node->raw_node->entry_instruction; i != node->raw_node->final_instruction->next; i = i->next)
    {
        auto working_instruction = i->data;

        if (working_instruction.instruction != ir_move)
            continue;

        ir_operand destination = i->data.destinations[0];

        if (is_global(ctx, destination))
            continue;

        //TODO: look for zero extends.
        if (ir_operand::get_raw_size(&destination) < int64)
        {
            int working_size = destination.meta_data;

            bool is_zero_extend = false;

            for (auto s = i->next; s != nullptr; s = s->next)
            {
                is_zero_extend |= check_size_greater(s->data.sources.data, s->data.sources.count,destination.value, working_size);

                if (is_zero_extend)
                    break;
            }

            if (is_zero_extend)
            {
                continue;
            }
        }

        for (auto s = i->next; s != nullptr; s = s->next)
        {
            replace_sources_if_same(s->data.sources.data, s->data.sources.count,destination.value, i->data.sources[0]);

            if (s == node->raw_node->final_instruction)
                break;
        }

        empty_operation(&i->data);

        done = false;
    }

    return done;
}

static bool perform_load_store_addition_propagation(ssa_cf_node* node, std::unordered_map<uint64_t, std::unordered_map<ssa_cf_node*, uint64_t>>* usage_count_map)
{
    bool done = true;

    ssa_context* ctx = node->context;

    for (auto i = node->raw_node->entry_instruction; i != node->raw_node->final_instruction->next; i = i->next)
    {
        auto working_instruction = i->data;

        int ins = working_instruction.instruction;

        if (ins != ir_load && ins != ir_store)
            continue; 

        if ((working_instruction.sources.count == 2 && ins == ir_load) || (working_instruction.sources.count == 3 && ins == ir_store))
            continue;

        ir_operand source = i->data.sources[0];

        if (ir_operand::is_constant(&source))
            continue;

        if (is_global(ctx, source))
            continue;

        if ((*usage_count_map)[source.value][node] > 2)
        {
            continue;
        }

        ir_operation* to_nop = nullptr;

        for (auto s = i->prev; s != nullptr; s = s->prev)
        {
            ir_operation* check_instruction = &s->data;

            if (check_instruction->destinations.count != 1)
                continue;

            ir_operand destination_check = check_instruction->destinations[0];

            if (ir_operand::are_equal(destination_check, source))
            {
                to_nop = check_instruction;

                break;
            }

            if (s == node->raw_node->entry_instruction)
                break;

            if (s->data.instruction == ir_mark_label)
            {
                throw_error();
            }
        }

        if (to_nop == nullptr)
        {
            throw_error();
        }

        if (to_nop->instruction != ir_add)
            continue;

        if (ins == ir_load)
        {
            ir_operation_block::emit_with(ctx->ir, ir_load, working_instruction.destinations.data, 1, to_nop->sources.data, 2, i);
        }
        else if (ins == ir_store)
        {
            ir_operand new_sources[3];

            new_sources[0] = to_nop->sources[0];
            new_sources[1] = to_nop->sources[1];
            new_sources[2] = working_instruction.sources[1];

            ir_operation_block::emit_with(ctx->ir, ir_store, nullptr, 0, new_sources, 3, i);
        }

        empty_operation(&i->data);
        empty_operation(to_nop);

        done = false;
    }

    return done;
}

static void swap(ir_operand* l, ir_operand* r)
{
	ir_operand tmp = *l;

	*l = *r;
	*r = tmp;
}

static bool perform_global_move_propagation(ssa_cf_node* node, std::unordered_map<uint64_t, std::unordered_map<ssa_cf_node*, uint64_t>>* usage_count_map)
{
    bool done = true;

    ssa_context* ctx = node->context;

    for (auto i = node->raw_node->entry_instruction; i != node->raw_node->final_instruction->next; i = i->next)
    {
        auto working_instruction = i->data;

        if (working_instruction.instruction != ir_move)
            continue;

        ir_operand destination = i->data.destinations[0];

        if (!is_global(ctx, destination))
            continue;

        ir_operand source = i->data.sources[0];

        if (ir_operand::is_constant(&source))
            continue;

        if (is_global(ctx, source))
            continue;

        ir_operand* to_replace = nullptr;

        for (auto s = i->prev; s != nullptr; s = s->prev)
        {
            ir_operation* check_instruction = &s->data;

            if (check_if_local_used(s->data.sources.data, s->data.sources.count, source.value))
            {
                break;
            }

            if (check_instruction->destinations.count != 1)
                continue;

            ir_operand destination_check = check_instruction->destinations[0];

            if (destination_check.value == source.value && destination_check.meta_data <= source.meta_data)
            {
                to_replace = &check_instruction->destinations[0];

                break;
            }

            if (s == node->raw_node->entry_instruction)
                break;

            if (s->data.instruction == ir_mark_label)
            {
                throw_error();
            }
        }

        if (to_replace == nullptr)
        {
            continue;
        }

        for (auto s = i->next; s != nullptr; s = s->next)
        {
            if (check_if_local_used(s->data.sources.data, s->data.sources.count, source.value))
            {
                to_replace = nullptr;

                break;
            }

            if (s == node->raw_node->final_instruction)
                break;
        }

        if (to_replace == nullptr)
        {
            continue;
        }

        to_replace->value = destination.value;

        i->data.instruction = ir_no_operation;
        i->data.destinations.count = 0;
        i->data.sources.count = 0;

        done = false;
    }

    return done;
}

static bool perform_dead_code_elimination(ssa_cf_node* node, std::unordered_map<uint64_t, std::unordered_map<ssa_cf_node*, uint64_t>>* usage_count_map)
{
    bool done = true;

    ssa_context* ctx = node->context;

    for (auto i = node->raw_node->entry_instruction; i != node->raw_node->final_instruction->next; i = i->next)
    {
        auto working_instruction = i->data;

        if (working_instruction.instruction == ir_external_call)
            continue;

        if (working_instruction.destinations.count != 1)
            continue;

        ir_operand destination = working_instruction.destinations[0];

        if (is_global(ctx,destination))
            continue;

        if ((*usage_count_map)[destination.value][node] >= 1)
            continue;

        i->data.instruction = ir_no_operation;
        i->data.destinations.count = 0;
        i->data.sources.count = 0;

        done = false;
    }

    return done;
}

static bool perform_math_propagation(ssa_cf_node* node)
{
    bool done = true;

    ssa_context* ctx = node->context;

    for (auto i = node->raw_node->entry_instruction; i != node->raw_node->final_instruction->next; i = i->next)
    {
        ir_operation* working_instruction = &i->data;

        ir_operand* destinations = i->data.destinations.data;
        ir_operand* sources = i->data.sources.data;

        switch (working_instruction->instruction)
        {
            case ir_shift_left:
            case ir_shift_right_unsigned:
            case ir_shift_right_signed:
            {
                if (check_constant(sources[1], 0))
                {
                    set_operand(working_instruction, sources[0]);

                    done = false;
                }

            }; break;

            case ir_load:
            {
                if (working_instruction->sources.count == 2)
                {
                    if (ir_operand::is_constant(&sources[0]) && !ir_operand::is_constant(&sources[1]))
                    {
                        swap(&sources[0], &sources[1]);

                        done = false;
                    }
                    else if (ir_operand::is_constant(&sources[0]) && ir_operand::is_constant(&sources[1]))
                    {
                        ir_operand new_operand = ir_operand::create_con(sources[0].value + sources[1].value, sources[0].meta_data);

                        sources[0] = new_operand;
                        working_instruction->sources.count = 1;

                        done = false;
                    }
                }
            }; break;

            case ir_add:
            {
                if (check_constant(sources[0], 0))
                {
                    set_operand(working_instruction, sources[1]);

                    done = false;
                }
                else if (check_constant(sources[1], 0))
                {
                    set_operand(working_instruction, sources[0]);

                    done = false;
                }
                else if (check_constant(sources[1], 1))
                {
                    working_instruction->instruction = ir_incrament;
                    working_instruction->sources.count = 1;

                    done = false;
                }

            }; break;

            case ir_subtract:
            {   
                if (check_constant(sources[0], 0))
                {
                    working_instruction->instruction = ir_negate;
                    working_instruction->sources.count = 1;
                    
                    sources[0] = sources[1];

                    done = false;
                }
                else if (check_constant(sources[1], 0))
                {
                    set_operand(working_instruction, sources[0]);

                    done = false;
                }
                else if (check_constant(sources[1], 1))
                {
                    working_instruction->instruction = ir_decrament;
                    working_instruction->sources.count = 1;

                    done = false;
                }

            }; break;

            case ir_bitwise_and:
            {
                if (check_constant(sources[0], 0) || check_constant(sources[1], 0))
                {
                    set_operand(working_instruction, ir_operand::create_con(0));

                    done = false;
                }

            }; break;

            case ir_bitwise_or:
            {
                if (check_constant(sources[0], 0))
                {
                    set_operand(working_instruction, sources[1]);

                    done = false;
                }
                else if (check_constant(sources[1], 0))
                {
                    set_operand(working_instruction, sources[0]);

                    done = false;
                }

            }; break;

            case ir_bitwise_exclusive_or:
            {
                if (check_constant(sources[0], 0))
                {
                    set_operand(working_instruction, sources[1]);

                    done = false;
                }
                else if (check_constant(sources[1], 0))
                {
                    set_operand(working_instruction, sources[0]);

                    done = false;
                }
            }; break;

            case ir_bitwise_not:
            {
                if (ir_operand::is_constant(&sources[0]))
                {
                    set_operand(working_instruction, ir_operand::create_con(~sources[0].value));

                    done = false;
                }
            }; break;

            case ir_incrament:
            {
                if (ir_operand::is_constant(&sources[0]))
                {
                    set_operand(working_instruction, ir_operand::create_con(sources[0].value + 1));

                    done = false;
                }
            }; break;

            case ir_decrament:
            {
                if (ir_operand::is_constant(&sources[0]))
                {
                    set_operand(working_instruction, ir_operand::create_con(sources[0].value - 1));

                    done = false;
                }
            }; break;

            case ir_negate:
            {
                if (ir_operand::is_constant(&sources[0]))
                {
                    set_operand(working_instruction, ir_operand::create_con(-sources[0].value));

                    done = false;
                }
            }; break;

            case ir_logical_not:
            {
                if (ir_operand::is_constant(&sources[0]))
                {
                    set_operand(working_instruction, ir_operand::create_con(!sources[0].value));

                    done = false;
                }
            }; break;

        }
    }

    return done;
}

void ssa_construct_and_optimize(ir_operation_block* source, compiler_flags flags)
{
    ir_control_flow_graph* raw_cfg;

    raw_cfg = ir_control_flow_graph::create(source);
    
    ssa_context ctx;

    ctx.ir = source;

    ctx.source = source;

    int count = 0;

    int size_counts[2];

    //ir_operation_block::log(source);

    ir_operation_block::clamp_operands(source, true, size_counts);

    for (auto i = raw_cfg->linier_nodes->first; i != nullptr; i = i->next)
    {
        if (i->data == nullptr)
            continue;

        count++;
    }

    ssa_cf_node ssa_node_store[count];

    count = 0;
    
    for (auto i = raw_cfg->linier_nodes->first; i != nullptr; i = i->next)
    {
        if (i->data == nullptr)
            continue;

        ssa_cf_node new_node = create_ssa_cf_node(&ctx, i->data);

        ssa_node_store[count] = new_node;

        ctx.raw_node_map[new_node.raw_node] = &ssa_node_store[count];

        if (new_node.location != -1)
        {
            ctx.labeled_node_map[new_node.location] = &ssa_node_store[count];
        }

        ++count;
    }

    for (int i = 0; i < count - 1; ++i)
    {
        create_forward_connection(&ssa_node_store[i], &ssa_node_store[i + 1]);
    }

    for (auto i : ctx.raw_node_map)
    {
        ssa_cf_node*            ssa_node = i.second;
        ir_control_flow_node*   raw_node = i.first;

        auto last_operation = raw_node->final_instruction->data;

        switch (last_operation.instruction)
        {
            case ir_jump_if:
            {
                auto new_block_label = last_operation.sources[0];

                assert_is_constant(new_block_label);

                int new_block_location = new_block_label.value;

                if (ctx.labeled_node_map.find(new_block_location) == ctx.labeled_node_map.end())
                {
                    throw_error();
                }

                create_forward_connection(ssa_node, ctx.labeled_node_map[new_block_location]);
            }; break;
        }
    }

    ctx.local_bottom = 0;

    std::unordered_map<uint64_t, std::unordered_set<ssa_cf_node*>> register_usage_map;
    for (int i = 0; i < count; ++i)
    {
        append_usage_data_global(&register_usage_map, nullptr, &ssa_node_store[i]);
    }

    for (auto i : register_usage_map)
    {
        if (i.second.size() <= 1)
            continue;

        ctx.globals.insert(i.first);
    }

    for (int i = 0; i < count; ++i)
    {
        construct_ssa_destinations(&ctx, &ssa_node_store[i]);
    }

    for (int i = 0; i < count; ++i)
    {
        remap_ssa_sources(&ctx, &ssa_node_store[i]);
    }

    //ir_operation_block::log(source);

    if (flags & compiler_flags::mathmatical_fold)
    {
        while (true)
        {
            bool done = true;

            std::unordered_map<uint64_t, std::unordered_map<ssa_cf_node*, uint64_t>> total_register_usage_count;

            for (int i = 0; i < count; ++i)
            {
                append_usage_data_global(nullptr, &total_register_usage_count, &ssa_node_store[i]);
            }

            for (int i = 0; i < count; ++i)
            {
                ssa_cf_node* working_node = &ssa_node_store[i];

                done &= perform_move_propagation(working_node);
                done &= perform_math_propagation(working_node);
                done &= perform_global_move_propagation(working_node, &total_register_usage_count);
                done &= perform_load_store_addition_propagation(working_node, &total_register_usage_count);
                done &= perform_dead_code_elimination(working_node, &total_register_usage_count);
            }

            if (done)
                break;
        }
    } 

    linier_scan_register_allocator_pass(source);
}