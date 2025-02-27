#include "aarch64_assembler.h"
#include <string.h>

#define ONE_MB 1 * 1024 * 1024

#define X(reg)  rarma_context::X(reg)
#define SP      rarma_context::SP()
#define WSP     rarma_context::SP()

void assemble_aarch64_abi_caller_code(void* result_code, uint64_t* result_code_size, abi abi_information)
{
    rarma_context c;

    rarma_context::create(&c, ONE_MB);

    rarma_context::sub_imm12(&c, SP, SP, 8);

    rarma_context::add_imm12(&c, SP, SP, 8);

    *result_code_size = c.memory_location;
    memcpy(result_code, c.memory_block, *result_code_size);

    rarma_context::destroy(&c);
}