#ifndef AARCH64_PIPELINE_H
#define AARCH64_PIPELINE_H

#include "ir/ir.h"
#include "abi_information.h"
#include "assembly/universal_flags.h"

void assemble_aarch64_pipeline(void** result_code, uint64_t* result_code_size, ir_operation_block* source_ir, abi working_abi, compiler_flags flags);

#endif