#ifndef SSA_H
#define SSA_H

#include "ir.h"
#include "assembly/universal_flags.h"

void ssa_construct_and_optimize(ir_operation_block* source, compiler_flags flags);

#endif