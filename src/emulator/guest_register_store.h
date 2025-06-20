#pragma once

#include <inttypes.h>
#include "ir/ir.h"

struct ssa_emit_context;

enum guest_usage {
    guest_usage_none = 0,
    guest_usage_read = 1 << 0,
    guest_usage_write = 1 << 1
    // read_write = guest_usage_read | guest_usage_write
};

struct guest_register {
    bool free_guest;
    int guest_register_offset;
    ir_operand raw_register;
    int mode;
};

struct guest_register_store {
    ssa_emit_context* ssa_emit_context_context;

    int guest_context_size;
    guest_register* guest_registers;

    static void create(guest_register_store* result, ssa_emit_context* ir, int guest_context_size);
    static void create_register(guest_register_store* result, int offset, uint64_t type);
    static ir_operand request_guest_register(guest_register_store* guest_register_store_context, int index);
    static void write_to_guest_register(guest_register_store* guest_register_store_context, int index, ir_operand new_value);
    static void emit_store_context(guest_register_store* guest_register_store_context);
    static void emit_load_context(guest_register_store* guest_register_store_context);
};
