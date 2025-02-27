#include <inttypes.h>
#include "abi_information.h"

#ifndef AARCH64_ASSEMBLER_H
#define AARCH64_ASSEMBLER_H

#include "rarma.h"

void assemble_aarch64_abi_caller_code(void* result_code, uint64_t* result_code_size, abi abi_information);

#endif