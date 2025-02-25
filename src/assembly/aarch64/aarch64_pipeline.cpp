#include "aarch64_pipeline.h"
#include "aarch64_pre_allocator.h"

void assemble_aarch64_pipeline(void** result_code, uint64_t* result_code_size, ir_operation_block* source_ir, bool optimize, abi working_abi, compiler_flags flags)
{
    arena_allocator* allocator = source_ir->allocator;

    
}