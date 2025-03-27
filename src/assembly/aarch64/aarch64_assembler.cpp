#include "aarch64_assembler.h"
#include "jit/jit_context.h"
#include <string.h>

#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define ONE_MB 1 * 1024 * 1024

void create_aarch64_caller(jit_context* result, abi abi_information)
{
    rarma_context c;

    rarma_context::create(&c, ONE_MB);

    //rarma_context::adds(&c, XZR(), SP(), X(1));

    growing_jit_cache::append_code(&result->jit_cache, c.memory_block, rarma_context::get_code_size(&c));

    rarma_context::destroy(&c);
}