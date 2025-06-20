#pragma once

#include <mutex>
#include <inttypes.h>
#include "debugging.h"
#include "uint128_t.h"

static std::mutex global_lock;

template <typename T>
static T compare_and_swap_impl(uint64_t address_src, T expecting, T to_swap) {
    T* address = (T*)address_src;

    if (*address == expecting) {
        *address = to_swap;

        return true;
    }

    return false;
}

// TODO: account for unalgined pointer
static uint64_t compare_and_swap_interpreter_cpp(uint64_t physical_address, uint64_t expecting, uint64_t to_swap, uint64_t size) {
    bool result;

    global_lock.lock();

    uint64_t mask = 15;

    uint64_t offset = physical_address & mask;
    physical_address = physical_address & ~mask;

    int byte_count = size / 8;

    uint128_t new_expecting = *(uint128_t*)physical_address;
    uint128_t new_to_swap = new_expecting;

    for (int i = 0; i < byte_count; ++i) {
        uint128_t::insert(&new_expecting, offset + i, 8, (expecting >> (i * 8)) & 255);
        uint128_t::insert(&new_to_swap, offset + i, 8, (to_swap >> (i * 8)) & 255);
    }

    result = compare_and_swap_impl<uint128_t>(physical_address, new_expecting, new_to_swap);

    global_lock.unlock();

    return result;
}
