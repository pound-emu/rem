#ifndef FAST_FUNCTION_TABLE_H
#define FAST_FUNCTION_TABLE_H

#include <inttypes.h>

struct fast_function_table
{
    uint64_t    entry_address;
    uint64_t    function_store_size;

    void*       function_store;

    fast_function_table();

    static bool     is_open(fast_function_table* test);
    static bool     insert_function(fast_function_table* table, uint64_t guest_address, uint32_t jit_offset);
    static void     init(fast_function_table* result, uint64_t entry_address);
    static void     destroy(fast_function_table* to_destroy);
    static uint32_t request_address(fast_function_table* test, uint64_t guest_address, bool* is_valid);
};

#endif