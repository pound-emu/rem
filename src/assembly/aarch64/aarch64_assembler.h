#pragma once

#include <inttypes.h>
#include "abi_information.h"

#include "rarma.h"

struct jit_context;

void create_aarch64_caller(jit_context* result, abi abi_information);
