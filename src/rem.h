#pragma once

#include "emulator/aarch64/aarch64_context_offsets.h"
#include "emulator/guest_process.h"
#include "emulator/ssa_emit_context.h"

struct external_context {
    guest_process process;
    jit_context memory;
};

extern void* base_plus_va(void* context, uint64_t virtual_address);

extern void base_plus_va_jit(void* memory, ssa_emit_context* ir, ir_operand destination, ir_operand source);

extern void* create_rem_context(void* memory, aarch64_context_offsets* context_offsets, void* svc, void* counter, void* undefined_instruction);

extern void set_log_native(external_context* context, void* log_native);

extern void destroy_rem_context(external_context* context);

extern uint64_t interperate_until_long_jump(external_context* context, uint64_t virtual_address, void* guest_context, bool* is_running);

extern uint64_t jit_until_long_jump(external_context* context, uint64_t virtual_address, void* guest_context, bool* is_running, void* log_native);

extern void invalidate_jit_region(external_context* context, uint64_t address, uint64_t size);
