#ifndef GUEST_MEMORY_H
#define GUEST_MEMORY_H

#include <inttypes.h>
#include "ir/ir.h"

struct ssa_emit_context;

struct guest_memory
{
    void*   base;
    void*   (*translate_address)(void*,uint64_t);
    void    (*emit_translate_address)(void*, ssa_emit_context*, ir_operand, ir_operand);
};

#endif