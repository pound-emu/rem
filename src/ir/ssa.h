#pragma once

#include "assembly/universal_flags.h"
#include "ir.h"

#include <unordered_set>
#include <vector>

void convert_to_ssa(ir_operation_block* ir, compiler_flags flags);
