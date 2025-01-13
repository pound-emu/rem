#include "ssa.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "ir/checks.h"

struct ssa_context;

struct ssa_cf_node
{
    int                                                             location;
    ssa_context*                                                    ir;
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

static ssa_cf_node create_ssa_cf_node(ssa_context* context, ir_control_flow_node* node)
{
    ssa_cf_node result;

    result.ir = context;
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
    ssa_context* context = node->ir;
    
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
    ssa_context* context = node->ir;

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
    ssa_context* context = node->ir;

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

static void append_usage_data(std::unordered_map<uint64_t, std::unordered_set<ssa_cf_node*>>* usage, ssa_cf_node* node, ir_operand* operands, int count)
{
    for (int i = 0; i < count; ++i)
    {
        ir_operand working = operands[i];

        if (ir_operand::is_constant(&working))
            continue;

        (*usage)[working.value].insert(node);
    }
}

static void append_usage_data_global(std::unordered_map<uint64_t, std::unordered_set<ssa_cf_node*>>* usage, ssa_cf_node* node)
{
    for (auto i = node->raw_node->entry_instruction; i != node->raw_node->final_instruction->next; i = i->next)
    {
        auto working = i->data;

        append_usage_data(usage, node, working.destinations.data, working.destinations.count);
        append_usage_data(usage, node, working.sources.data, working.sources.count);
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

void ssa_construct_and_optimize(ir_operation_block* source, compiler_flags flags)
{
    ir_control_flow_graph* raw_cfg;

    raw_cfg = ir_control_flow_graph::create(source);
    
    ssa_context ctx;

    ctx.ir = source;

    ctx.source = source;

    int count = 0;

    int size_counts[2];

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
        append_usage_data_global(&register_usage_map, &ssa_node_store[i]);
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

    linier_scan_register_allocator_pass(source);
}