#include <inttypes.h>
#include "abi_information.h"

#ifndef AARCH64_ASSEMBLER_H
#define AARCH64_ASSEMBLER_H

#include "rarma.h"

struct jit_context;

void create_aarch64_caller(jit_context* result, abi abi_information);

#endif