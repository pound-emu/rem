#ifndef SSA_H
#define SSA_H

#include "ir.h"

#include <unordered_set>
#include <vector>

void convert_to_ssa(ir_operation_block* ir, bool optimize);

#endif