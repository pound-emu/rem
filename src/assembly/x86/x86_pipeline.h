#pragma once

#include "abi_information.h"
#include "assembly/universal_flags.h"
#include "ir/ir.h"

void assemble_x86_64_pipeline(void** result_code, uint64_t* result_code_size, ir_operation_block* source_ir, abi working_abi, compiler_flags flags);
