#ifndef SSA_H
#define SSA_H

#include "ir.h"

#include <unordered_set>
#include <vector>

struct ssa_node;
struct ssa_context;

struct phi_source
{
    ssa_node*   node_source;
    int         time_of_declaration;
    int         source_register;
};

struct phi_node
{
    ssa_node*               node_context;
    int                     type;
    int                     new_register;
    std::vector<phi_source> sources;
};

struct ssa_node
{
    std::vector<phi_node>                                           phis;
    ssa_context*                                                    context;
    ir_control_flow_node*                                           raw_node;
    std::unordered_map<int, std::unordered_map<uint64_t, uint64_t>> time_remap;
    std::unordered_map<uint64_t, uint64_t>                          local_to_global_move_count;
    int                                                             count;

    std::vector<ssa_node*>                                          inlets;
    std::vector<ssa_node*>                                          outlets;
};

struct ssa_context
{
    ir_control_flow_graph*                                  raw_cfg;
    std::vector<ssa_node*>                                  ssa_nodes;
    std::unordered_map<ir_control_flow_node*, ssa_node*>    node_map;
    std::unordered_map<ssa_node*, ir_control_flow_node*>    reverse_node_map;
    std::unordered_map<int, ssa_node*>                      numbered_node_map;

    uint64_t                                                open_register;
};

void convert_to_ssa(ir_operation_block* ir, bool optimize);

#endif