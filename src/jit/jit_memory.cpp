#include <assert.h>
#include <string.h>

#include "jit_memory.h"

#define USE_MEMPROTECT defined (__linux__) || defined(__APPLE__)
#define USE_VIRTUAL_PROTECT (defined _WIN32)

#if USE_MEMPROTECT
#include "sys/mman.h"

static bool allocate_executable_memory(void** memory, uint64_t size)
{
    uint64_t map_info = MAP_PRIVATE | MAP_ANONYMOUS;
    uint64_t map_proc = PROT_READ | PROT_WRITE;

    #if defined(__APPLE__)
        map_info |= MAP_JIT;
    #else
        map_proc |= PROT_EXEC;
    #endif

    void* result = mmap(NULL, size, map_proc, map_info , -1, 0);

    *memory = result;

    return result != nullptr;
}

static void unmark_memory_executable(void* memory, uint64_t size)
{
    munmap(memory, size);
}

#elif USE_VIRTUAL_PROTECT
#include "windows.h"

static bool allocate_executable_memory(void** memory, uint64_t size)
{
    void* result = VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_READWRITE);

    DWORD dummy;
    bool success = VirtualProtect(result, size, PAGE_EXECUTE_READWRITE, &dummy) != 0;

    *memory = result;

    return success;
}

static void unmark_memory_executable(void* memory, uint64_t size)
{
    VirtualFree(memory, 0, MEM_RELEASE);
}

//TODO:

#endif

#define DEFAULT_KB_SIZE 4

static uint64_t align_page_size(uint64_t source, int kb_size = DEFAULT_KB_SIZE)
{
    uint64_t page_size = kb_size * 1024;

    uint64_t mask = page_size - 1;

    uint64_t working_result = (source & ~mask);

    if (source > working_result)
    {
        working_result += page_size;
    }

    return working_result;
}

static uint64_t align_page(uint64_t source, int kb_size = DEFAULT_KB_SIZE)
{
    uint64_t page_size = kb_size * 1024;

    uint64_t mask = page_size - 1;

    return source & ~mask;
}

bool jit_memory::create(jit_memory** result, uint64_t allocation_size, abi host_abi)
{
    jit_memory* working_result = new jit_memory();

    allocation_size = align_page_size(allocation_size, 4);

    working_result->host_abi = host_abi;
    working_result->memory_block_size = allocation_size;

    *result = working_result;

    return allocate_executable_memory(&working_result->raw_memory_block, allocation_size);
}

void jit_memory::destroy(jit_memory* to_destroy)
{
    unmark_memory_executable(to_destroy->raw_memory_block, to_destroy->memory_block_size);

    delete to_destroy;
}

static void align_page_info(jit_memory* jit_memory_context, uint64_t* offset, uint64_t* size)
{
    *offset = align_page(*offset);
    *size = align_page_size(*size);

    *offset = (uint64_t)jit_memory_context->raw_memory_block + *offset;
}

static void ready_page_for_write(jit_memory* jit_memory_context, uint64_t result_offset, uint64_t size)
{
    align_page_info(jit_memory_context, &result_offset, &size);

    mprotect((void*)result_offset, size, PROT_READ | PROT_WRITE);
}

static void ready_page_for_execution(jit_memory* jit_memory_context, uint64_t result_offset, uint64_t size)
{
    align_page_info(jit_memory_context, &result_offset, &size);

    mprotect((void*)result_offset, size, PROT_READ | PROT_EXEC);
}

void* jit_memory::coppy_over(jit_memory* jit_memory_context, uint64_t result_offset, void* source, uint64_t size)
{
    char* result_location = (char*)jit_memory_context->raw_memory_block + result_offset;

    if (get_is_apple_silicon(jit_memory_context->host_abi))
    {
        ready_page_for_write(jit_memory_context, result_offset, size);
    }
    
    memcpy(result_location, source, size);

    if (get_is_apple_silicon(jit_memory_context->host_abi))
    {
        ready_page_for_execution(jit_memory_context, result_offset, size);
    }

    return result_location;
}