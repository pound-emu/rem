#include "ir.h"
#include "linier_scan_register_allocator.h"
#include "debugging.h"
#include "tools/color_graph.h"
#include "tools/misc_tools.h"

#include <unordered_map>

struct register_lifetime;

struct register_lifetime
{
    uint64_t        source_register;

    int             birth;
    int             death;

    int             first_use_after_birth_location;

    register_lifetime*  birth_lifetime;
    ir_operation*       birth_instruction;

    static bool in_interval(register_lifetime* lifetime, int time)
    {
        return time >= lifetime->birth && time <= lifetime->death;
    }

    static bool intersects(register_lifetime* lifetime, register_lifetime* test)
    {
        return in_interval(lifetime, test->birth) || in_interval(lifetime, test->death);
    }

    register_lifetime()
    {
        birth_instruction = nullptr;
    }

    static void set_birth_instruction(register_lifetime* context, ir_operation* birth_instruction)
    {
        context->birth_instruction = nullptr;

        if (birth_instruction->instruction != ir_move)
            return;

        if (ir_operand::is_constant(&birth_instruction->sources[0]))
            return;

        context->birth_instruction = birth_instruction;
    }

    static register_lifetime create(uint64_t source_register, int time, ir_operation* birth_instruction)
    {
        register_lifetime result;

        result.source_register = source_register;
        result.birth = time;
        result.death = time;
        result.first_use_after_birth_location = result.birth;

        set_birth_instruction(&result, birth_instruction);

        return result;
    }
};

struct loop_data
{
    int check_location;
    int loop_end;
};

struct lrsa_node
{
    ir_control_flow_node*   raw_node;
    
    int                     start_time;
    int                     end_time;
};

static void find_lifetimes_first_last(std::unordered_map<uint64_t, register_lifetime>* lifetimes,ir_operand* operands, ir_operation* working_operation, int operand_count, int time, int is_source)
{
    for (int i = 0; i < operand_count; ++i)
    {
        ir_operand working = operands[i];

        if (ir_operand::is_constant(&working))
        {
            continue;
        }

        uint64_t working_register = working.value;

        register_lifetime* lifetime_reference;

        if (!in_map(lifetimes, working_register))
        {
            (*lifetimes)[working_register] = register_lifetime::create(working_register, time, working_operation);
        }

        lifetime_reference = &(*lifetimes)[working_register];

        lifetime_reference->death = time;

        if (is_source && lifetime_reference->death > lifetime_reference->birth)
        {
            lifetime_reference->death--;
        }

        if (lifetime_reference->death < lifetime_reference->birth)
        {
            throw_error();
        }

        if (lifetime_reference->first_use_after_birth_location == lifetime_reference->birth)
        {
            lifetime_reference->first_use_after_birth_location = lifetime_reference->death;
        }
    }
}

int find_and_use_slot(bool* slots, int max)
{
    for (int i = 0; ; ++i)
    {
        if (i >= max)
        {
            throw_error();
        }

        if (slots[i])
            continue;

        slots[i] = true;

        return i;
    }
}

void linier_scan_register_allocator_pass(ir_control_flow_graph* cfg)
{
    std::unordered_map<uint64_t, register_lifetime> all_intervals;

    int time = 0;

    std::unordered_map<ir_control_flow_node*, lrsa_node*> node_map;

    int node_count = 0;

    for (auto i = cfg->linier_nodes->first; i != nullptr; i = i->next)
    {
        if (i->data == nullptr)
            continue;

        node_count++;
    }

    lrsa_node working_nodes[node_count];

    node_count = 0;

    for (auto i = cfg->linier_nodes->first; i != nullptr; i = i->next)
    {
        if (i->data == nullptr)
            continue;

        lrsa_node* temp_node = &working_nodes[node_count];

        temp_node->start_time = time;
        temp_node->raw_node = i->data;

        node_map[i->data] = temp_node;

        auto raw_node = i->data;

        for (auto ins = raw_node->entry_instruction; ins != raw_node->final_instruction->next; ins = ins->next)
        {
            ir_operation* working_operation = &ins->data;

            find_lifetimes_first_last(&all_intervals, working_operation->destinations.data, working_operation,working_operation->destinations.count, time, false);
            find_lifetimes_first_last(&all_intervals, working_operation->sources.data, working_operation,working_operation->sources.count, time, true);

            time++;
        }

        temp_node->end_time = time;

        node_count++;
    }
    
    std::vector<loop_data> loops;

    for (int i = 0; i < node_count; ++i)
    {
        lrsa_node* working_node = &working_nodes[i];

        if (working_node->raw_node->final_instruction->data.instruction != ir_jump_if)
            continue;

        for (auto exit = working_node->raw_node->exits->first; exit != nullptr; exit = exit->next)
        {
            if (exit->data == nullptr)
                continue;

            lrsa_node* working_jump = node_map[exit->data];

            if (working_jump->start_time > working_node->start_time)
                continue;
            
            loop_data this_loop;

            this_loop.check_location = working_jump->start_time;
            this_loop.loop_end = working_node->end_time;

            loops.push_back(this_loop);
        }
    }

    for (int i = 0; i < loops.size(); ++i)
    {
        loop_data* loop = &loops[i];

        for (auto interval : all_intervals)
        {
            register_lifetime* working_lifetime = &all_intervals[interval.first];

            if (!register_lifetime::in_interval(working_lifetime,loop->check_location))
                continue;

            if (loop->loop_end > working_lifetime->death)
            {
                working_lifetime->death = loop->loop_end;
            }
        }
    }

    int slot_count = all_intervals.size();

    bool slots[all_intervals.size()];

    memset(slots, 0, sizeof(slots));

    std::vector<uint64_t> births[time];
    std::vector<uint64_t> deaths[time];

    for (auto i : all_intervals)
    {
        register_lifetime lifetime = i.second;

        births[lifetime.birth].push_back(lifetime.source_register);
        deaths[lifetime.death].push_back(lifetime.source_register);
    }

    std::unordered_map<uint64_t, uint64_t> working_remap;

    for (int i = 0; i < time; ++i)
    {
        for (auto source_register : births[i])
        {
            if (in_map(&working_remap, (uint64_t)source_register))
            {
                throw_error();
            }

            int new_slot = find_and_use_slot(slots, slot_count);

            working_remap[source_register] = new_slot;
        }

        for (auto source_register : deaths[i])
        {
            if (!in_map(&working_remap, (uint64_t)source_register))
            {
                throw_error();
            }

            slots[working_remap[source_register]] = false;
        }
    }

    ir_operation_block::ssa_remap(cfg->source_ir, &working_remap);
}