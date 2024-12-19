#ifndef ARM_EMIT_CONTEXT_H
#define ARM_EMIT_CONTEXT_H

#include "ir/ir.h"

struct arm_emit_context
{
    ir_operation_block*     ir;
    uint64_t                local_top;
    uint64_t                global_bottom;
};

#endif