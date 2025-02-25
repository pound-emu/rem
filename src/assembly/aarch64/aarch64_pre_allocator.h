#ifndef AARCH64_PRE_ALLOCATOR_H
#define AARCH64_PRE_ALLOCATOR_H

#include "ir/ir.h"
#include "abi_information.h"

struct aarch64_pre_allocator_context
{
    static void run_pass(aarch64_pre_allocator_context* pre_allocator_context, ir_operation_block* result_ir, ir_operation_block* source, cpu_information cpu_data, os_information os);
}; 

#endif