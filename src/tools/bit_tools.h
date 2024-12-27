#ifndef BIT_TOOLS_H
#define BIT_TOOLS_H

#include <inttypes.h>

static uint64_t get_mask_from_size(uint64_t size)
{
    if (size == 3)
        return -1;

    assert(size <= 3);

    uint64_t mask = (1ULL << (8 << size)) - 1;

    return mask;
}

static uint64_t create_int_min(uint64_t size)
{
    int bit_count = (8 << size);

    uint64_t working_result = UINT64_MAX << (bit_count - 1);

    uint64_t result = working_result & get_mask_from_size(size);

    return result;
}

static uint64_t create_int_max(uint64_t size)
{
    int bit_count = (8 << size);

    return (1ULL << (bit_count - 1)) - 1;
}


static int64_t sign_extend_from_size(uint64_t source, uint64_t size)
{
    if (size == int64)
        return source;

    assert(size != int128);

    int bit = 63 - ((8 << size) - 1);

    return ((int64_t)source << bit) >> bit;
}

static uint64_t zero_extend_from_size(uint64_t source, uint64_t size)
{
    return source & get_mask_from_size(size);
}

#endif