#pragma once

#include <inttypes.h>

struct uint128_t {
    uint64_t d0;
    uint64_t d1;

    uint128_t() {
        d0 = 0;
        d1 = 0;
    }

    operator uint64_t() {
        return d0;
    }

    uint128_t(uint64_t source) {
        d0 = source;
        d1 = 0;
    }

    uint128_t(uint64_t source_0, uint64_t source_1) {
        d0 = source_0;
        d1 = source_1;
    }

    static void insert(uint128_t* data, int index, int size, uint64_t value) {
        switch (size) {
        case 8:
            size = 1;
            break;
        case 16:
            size = 2;
            break;
        case 32:
            size = 4;
            break;
        case 64:
            size = 8;
            break;

        default:
            throw 0;
            break;
        }

        int byte_offset = index * size;

        uint64_t* working_part = &data->d0;

        if (byte_offset >= 8) {
            byte_offset -= 8;
            working_part = &data->d1;
        }

        uint64_t mask = UINT64_MAX;

        if (size == 8) {
            *working_part = value;
        } else {
            uint64_t mask = ((1ULL << (size * 8)) - 1) << (byte_offset * 8);
            uint64_t inverse_mask = ~mask;

            *working_part = (*working_part & inverse_mask) | ((value << (byte_offset * 8)) & mask);
        }
    }

    static uint64_t extract(uint128_t data, int index, int size) {
        switch (size) {
        case 8:
            size = 1;
            break;
        case 16:
            size = 2;
            break;
        case 32:
            size = 4;
            break;
        case 64:
            size = 8;
            break;

        default:
            throw 0;
            break;
        }

        int byte_offset = index * size;

        uint64_t working_part = data.d0;

        if (byte_offset >= 8) {
            byte_offset -= 8;
            working_part = data.d1;
        }

        uint64_t mask = UINT64_MAX;

        if (size != 8) {
            mask = (1ULL << (8 * size)) - 1;
        }

        return (working_part >> (byte_offset * 8)) & mask;
    }

    bool operator==(uint128_t other) {
        return (d0 == other.d0) && (d1 == other.d1);
    }
};
