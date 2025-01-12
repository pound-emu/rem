#include <assert.h>
#include <string.h>

#include "growing_jit_cache.h"
#include "jit_memory.h"
#include "debugging.h"

void growing_jit_cache::create(growing_jit_cache* result, jit_memory* memory)
{
    result->top = 0;
    result->memory = memory;
}

void* growing_jit_cache::allocate(growing_jit_cache* jit_cache, uint64_t size)
{
    void* result = (char*)jit_cache->memory->raw_memory_block + jit_cache->top;

    jit_cache->top += size;

    if (jit_cache->top >= jit_cache->memory->memory_block_size)
    {
        throw_error();
    }

    return result;
}

void* growing_jit_cache::append_code(growing_jit_cache* jit_cache, void* code, uint64_t size)
{
    jit_cache->lock.lock();

    void* result = allocate(jit_cache, size);

    memcpy(result, code, size);

    jit_cache->lock.unlock();

    return result;
}