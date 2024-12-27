#ifndef AARCH64_EMIT_CONTEXT_H
#define AARCH64_EMIT_CONTEXT_H

#include "ir/ir.h"
#include "emulator/guest_register_store.h"
#include "emulator/branch_type.h"
#include <unordered_set>
#include <unordered_map>
#include <vector>

struct ssa_emit_context;
struct aarch64_process;

struct aarch64_emit_context
{
    ssa_emit_context*                                           ssa;
    ir_operation_block*                                         raw_ir;
    aarch64_process*                                            process;

    std::unordered_map<uint64_t, ir_operand>                    basic_block_labels;
    std::unordered_set<uint64_t>                                basic_block_translate_que;
    branch_type                                                 branch_state;
    uint64_t                                                    current_instruction_address;
    uint32_t                                                    current_raw_instruction;

    guest_register_store                                        registers;
    std::vector<intrusive_linked_list_element<ir_operation>*>   context_movement;    
                    
    ir_operand                                                  context_pointer;
                    
    static void                                                 create(aarch64_process* process, aarch64_emit_context* result, ssa_emit_context* ir);
    static void                                                 init_context(aarch64_emit_context* ctx);
    static void                                                 emit_load_context(aarch64_emit_context* ctx);
    static void                                                 emit_store_context(aarch64_emit_context* ctx);
    static bool                                                 basic_block_translated(aarch64_emit_context* ctx, uint64_t block);
    static void                                                 branch_long(aarch64_emit_context* ctx, ir_operand new_location);
    static void                                                 emit_context_movement(aarch64_emit_context* ctx);

    static ir_operand                                           get_x_raw(aarch64_emit_context* ctx, int index);
    static void                                                 set_x_raw(aarch64_emit_context* ctx, int index, ir_operand value);
    static void                                                 set_context_reg_raw(aarch64_emit_context* ctx, int index, ir_operand value);
};

#endif