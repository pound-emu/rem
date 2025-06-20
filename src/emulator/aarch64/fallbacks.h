#pragma once

#include "emulator/uint128_t.h"

static uint64_t big_elm(uint128_t* V, int index, int size) {
    int byte_count = size >> 3;
    int max = 16 / byte_count;

    int inner_index = index % max;
    int outer_index = index / max;

    return uint128_t::extract(V[outer_index], inner_index, size);
}

static void table_lookup_fallback(uint128_t* V, int d, int n, int len, int m, int Q) {
    int datasize = 64 << Q;
    int elements = datasize / 8;
    int regs = len + 1;

    bool is_tbl = true;

    uint128_t result = 0;

    uint128_t table[regs];
    uint128_t indices = V[m];

    for (int i = 0; i < regs; ++i) {
        table[i] = V[(n + i) % 32];
    }

    for (int i = 0; i < elements; ++i) {
        uint64_t index = uint128_t::extract(indices, i, 8);

        if (index < 16 * regs) {
            uint128_t::insert(&result, i, 8, big_elm(table, index, 8));
        }
    }

    V[d] = result;
}
