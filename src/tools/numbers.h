#pragma once

#include <random>
#include <inttypes.h>

static uint64_t create_random_number() {
    uint64_t result = 0;

    for (int i = 0; i < 8; ++i) {
        result |= ((uint64_t)rand() & 255) << (i * 8);
    }

    return result;
}

static uint32_t reverse_bytes(uint32_t source) {
    uint32_t result = 0;

    for (int i = 0; i < 4; ++i) {
        int s_bit = (3 - i) * 8;

        result |= ((source >> s_bit) & 255) << (i * 8);
    }

    return result;
}
