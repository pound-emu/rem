#pragma once

#include "abi_information.h"
#include "ir/ir.h"

void assemble_x86_64_code(void** result_code, uint64_t* result_code_size, ir_operation_block* source_ir);
void assemble_x86_abi_caller_code(void* result_code, uint64_t* result_code_size, abi abi_information);
