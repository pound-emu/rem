#ifndef SSA_H
#define SSA_H

#include "ir.h"
#include "assembly/universal_flags.h"

#include <unordered_set>
#include <vector>

void convert_to_ssa(ir_operation_block* ir, compiler_flags flags);

#endif