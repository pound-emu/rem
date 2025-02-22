#ifndef AARCH64_EMIT_CONTEXT_H
#define AARCH64_EMIT_CONTEXT_H

#include "ir/ir.h"
#include "emulator/guest_register_store.h"
#include "emulator/guest_compiler_optimization_flags.h"
#include "emulator/branch_type.h"
#include <unordered_set>
#include <unordered_map>
#include <vector>

struct ssa_emit_context;
struct guest_process;

struct aarch64_emit_context
{
    ssa_emit_context*                                           ssa;
    ir_operation_block*                                         raw_ir;
    guest_process*                                              process;
    bool                                                        translate_functions;

    std::unordered_map<uint64_t, ir_operand>                    basic_block_labels;
    std::unordered_set<uint64_t>                                basic_block_translate_que;
    branch_type                                                 branch_state;
    uint64_t                                                    current_instruction_address;
    uint32_t                                                    current_raw_instruction;

    guest_compiler_optimization_flags                           optimization_flags;

    guest_register_store                                        registers;
    std::vector<intrusive_linked_list_element<ir_operation>*>   context_movement;    
                    
    ir_operand                                                  context_pointer;
                    
    static void                                                 create(guest_process* process, aarch64_emit_context* result, ssa_emit_context* ir, guest_compiler_optimization_flags flags);
    static void                                                 init_context(aarch64_emit_context* ctx);
    static void                                                 emit_load_context(aarch64_emit_context* ctx);
    static void                                                 emit_store_context(aarch64_emit_context* ctx);
    static void                                                 branch_long(aarch64_emit_context* ctx, ir_operand new_location, bool store_context = true, bool allow_table_branch = true);
    static void                                                 branch_short(aarch64_emit_context* ctx, ir_operand new_location);
    static void                                                 emit_context_movement(aarch64_emit_context* ctx);
    static ir_operand                                           get_or_create_basic_block_label(aarch64_emit_context* ctx, uint64_t value);

    static ir_operand                                           get_x_raw(aarch64_emit_context* ctx, int index);
    static void                                                 set_x_raw(aarch64_emit_context* ctx, int index, ir_operand value);
    static ir_operand                                           get_v_raw(aarch64_emit_context* ctx, int index);
    static void                                                 set_v_raw(aarch64_emit_context* ctx, int index, ir_operand value);
    static void                                                 set_context_reg_raw(aarch64_emit_context* ctx, int index, ir_operand value);
    static ir_operand                                           get_context_reg_raw(aarch64_emit_context* ctx, int index);
};

#endif