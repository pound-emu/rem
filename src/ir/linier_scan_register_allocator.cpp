#include "debugging.h"
#include "ir.h"
#include "linier_scan_register_allocator.h"
#include "tools/color_graph.h"
#include "tools/misc_tools.h"

#include <unordered_map>

struct register_lifetime;
struct lrsa_node;

struct register_lifetime {
    uint64_t source_register;

    int birth;
    int death;

    int first_use_after_birth_location;

    bool is_vector;

    ir_operation* birth_instruction;
    lrsa_node* birth_node;
    lrsa_node* death_node;

    static bool in_interval(register_lifetime* lifetime, int time) {
        return time >= lifetime->birth && time <= lifetime->death;
    }

    static bool intersects(register_lifetime* lifetime, register_lifetime* test) {
        return in_interval(lifetime, test->birth) || in_interval(lifetime, test->death);
    }

    register_lifetime() {
        birth_instruction = nullptr;
    }

    static void set_birth_instruction(register_lifetime* context, ir_operation* birth_instruction) {
        context->birth_instruction = nullptr;

        if (birth_instruction->instruction != ir_move)
            return;

        if (ir_operand::is_constant(&birth_instruction->sources[0]))
            return;

        context->birth_instruction = birth_instruction;
    }

    static register_lifetime create(uint64_t source_register, int time, ir_operation* birth_instruction, lrsa_node* birth_node) {
        register_lifetime result;

        result.source_register = source_register;
        result.birth = time;
        result.death = time;
        result.first_use_after_birth_location = result.birth;
        result.birth_node = birth_node;

        set_birth_instruction(&result, birth_instruction);

        return result;
    }
};

struct loop_data {
    int check_location;
    lrsa_node* loop_end_node;
};

struct known_global {
    uint64_t reg;
    bool is_vector;

    known_global() {
        //
    }
};

struct lrsa_node {
    ir_control_flow_node* raw_node;
    intrusive_linked_list_element<ir_control_flow_node*>* raw_node_element;

    int start_time;
    int end_time;

    std::vector<known_global> known_globals;
};

static void find_lifetimes_first_last(std::unordered_map<uint64_t, register_lifetime>* lifetimes, ir_operand* operands, ir_operation* working_operation, int operand_count, int time, int is_source, lrsa_node* working_node) {
    for (int i = 0; i < operand_count; ++i) {
        ir_operand working = operands[i];

        if (ir_operand::is_constant(&working)) {
            continue;
        }

        uint64_t working_register = working.value;

        register_lifetime* lifetime_reference;

        if (!in_map(lifetimes, working_register)) {
            (*lifetimes)[working_register] = register_lifetime::create(working_register, time, working_operation, working_node);
        }

        lifetime_reference = &(*lifetimes)[working_register];

        lifetime_reference->is_vector = ir_operand::is_vector(&working);

        lifetime_reference->death_node = working_node;
        lifetime_reference->death = time;

        if (is_source && lifetime_reference->death > lifetime_reference->birth) {
            lifetime_reference->death--;
        }

        if (lifetime_reference->death < lifetime_reference->birth) {
            throw_error();
        }

        if (lifetime_reference->first_use_after_birth_location == lifetime_reference->birth) {
            lifetime_reference->first_use_after_birth_location = lifetime_reference->death;
        }
    }
}

int find_and_use_slot(bool* slots, int max) {
    for (int i = 0;; ++i) {
        if (i >= max) {
            throw_error();
        }

        if (slots[i])
            continue;

        slots[i] = true;

        return i;
    }
}

void linier_scan_register_allocator_pass(ir_control_flow_graph* cfg) {
    std::unordered_map<uint64_t, register_lifetime> all_intervals;
    std::unordered_map<ir_control_flow_node*, lrsa_node*> node_map;

    int time = 0;

    int node_count = 0;

    for (auto i = cfg->linier_nodes->first; i != nullptr; i = i->next) {
        if (i->data == nullptr)
            continue;

        node_count++;
    }

    lrsa_node working_nodes[node_count];

    node_count = 0;

    for (auto i = cfg->linier_nodes->first; i != nullptr; i = i->next) {
        if (i->data == nullptr)
            continue;

        lrsa_node* working_node = &working_nodes[node_count];

        working_node->start_time = time;
        working_node->raw_node = i->data;
        working_node->raw_node_element = i;

        node_map[i->data] = working_node;

        auto raw_node = i->data;

        for (auto ins = raw_node->entry_instruction; ins != raw_node->final_instruction->next; ins = ins->next) {
            ir_operation* working_operation = &ins->data;

            find_lifetimes_first_last(&all_intervals, working_operation->destinations.data, working_operation, working_operation->destinations.count, time, false, working_node);
            find_lifetimes_first_last(&all_intervals, working_operation->sources.data, working_operation, working_operation->sources.count, time, true, working_node);

            time++;
        }

        working_node->end_time = time;

        node_count++;
    }

    std::vector<loop_data> loops;

    for (int i = 0; i < node_count; ++i) {
        lrsa_node* working_node = &working_nodes[i];

        if (!ir_operation_block::is_jump(&working_node->raw_node->final_instruction->data))
            continue;

        for (auto exit = working_node->raw_node->exits->first; exit != nullptr; exit = exit->next) {
            if (exit->data == nullptr)
                continue;

            lrsa_node* working_jump = node_map[exit->data];

            if (working_jump->start_time > working_node->start_time)
                continue;

            loop_data this_loop;

            this_loop.check_location = working_jump->start_time;

            this_loop.loop_end_node = working_node;

            loops.push_back(this_loop);
        }
    }

    for (int i = 0; i < loops.size(); ++i) {
        loop_data* loop = &loops[i];

        for (auto interval : all_intervals) {
            register_lifetime* working_lifetime = &all_intervals[interval.first];

            if (!register_lifetime::in_interval(working_lifetime, loop->check_location))
                continue;

            int loop_end_time = loop->loop_end_node->end_time;

            if (loop_end_time > working_lifetime->death) {
                working_lifetime->death = loop_end_time;
                working_lifetime->death_node = loop->loop_end_node;
            }
        }
    }

    int slot_count = all_intervals.size();

    bool slots[all_intervals.size()];

    memset(slots, 0, sizeof(slots));

    std::vector<uint64_t> births[time];
    std::vector<uint64_t> deaths[time];

    for (auto i : all_intervals) {
        register_lifetime* lifetime = &all_intervals[i.first];

        births[lifetime->birth].push_back(lifetime->source_register);
        deaths[lifetime->death].push_back(lifetime->source_register);

        if (lifetime->birth_node == lifetime->death_node)
            continue;

        lrsa_node* birth = lifetime->birth_node;
        lrsa_node* death = lifetime->death_node;

        for (auto element = birth->raw_node_element; element != death->raw_node_element->next; element = element->next) {
            lrsa_node* working_node = node_map[element->data];

            known_global data;

            data.is_vector = i.second.is_vector;
            data.reg = i.first;

            working_node->known_globals.push_back(data);
        }
    }

    for (int i = 0; i < node_count; ++i) {
        lrsa_node* working_node = &working_nodes[i];

        int source_count = working_node->known_globals.size();

        if (source_count == 0)
            continue;

        ir_operand sources[source_count];

        for (int o = 0; o < source_count; ++o) {
            known_global data = working_node->known_globals[o];

            ir_operand source = ir_operand::create_reg(data.reg, int64 + data.is_vector);

            sources[o] = source;
        }

        ir_operation_block::emit_with(cfg->source_ir, ir_register_allocator_hint_global, nullptr, 0, sources, source_count, working_node->raw_node->entry_instruction);
    }

    std::unordered_map<uint64_t, uint64_t> working_remap;

    for (int i = 0; i < time; ++i) {
        for (auto source_register : births[i]) {
            if (in_map(&working_remap, (uint64_t)source_register)) {
                throw_error();
            }

            int new_slot = find_and_use_slot(slots, slot_count);

            working_remap[source_register] = new_slot;
        }

        for (auto source_register : deaths[i]) {
            if (!in_map(&working_remap, (uint64_t)source_register)) {
                throw_error();
            }

            slots[working_remap[source_register]] = false;
        }
    }

    ir_operation_block::ssa_remap(cfg->source_ir, &working_remap);
}