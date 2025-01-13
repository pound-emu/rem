#include "fast_function_table.h"
#include <string.h>
#include <iostream>

bool fast_function_table::is_open(fast_function_table* table)
{
    return table->function_store != nullptr;
}

bool fast_function_table::insert_function(fast_function_table* table, uint64_t guest_address, uint32_t jit_offset)
{
    if (!is_open(table))
    {
        init(table, guest_address);
    }

    guest_address -= table->entry_address;

    if (guest_address >= table->function_store_size)
    {
        return false;
    }
    
    *(uint32_t*)((uint64_t)table->function_store + guest_address) = jit_offset;

    return true;
}

fast_function_table::fast_function_table()
{
    function_store = nullptr;

    entry_address = -1;
    function_store_size = -1;
}

void fast_function_table::init(fast_function_table* result, uint64_t entry_address)
{
    result->entry_address = entry_address;

    result->function_store_size = 100 * 1024 * 1024;
    result->function_store = malloc(result->function_store_size);
    memset(result->function_store, -1, result->function_store_size);
}

void fast_function_table::destroy(fast_function_table* to_destroy)
{
    if (is_open(to_destroy))
    {
        free(to_destroy->function_store);
    }
}

uint32_t fast_function_table::request_address(fast_function_table* table, uint64_t guest_address, bool* is_valid)
{
    *is_valid = false;

    if (!is_open(table))
    {
        return 0;
    }

    guest_address -= table->entry_address;

    if (guest_address >= table->function_store_size)
    {
        return false;
    }

    uint32_t working_result = *(uint32_t*)((uint64_t)table->function_store + guest_address);

    *is_valid = working_result != -1;

    return working_result;
}