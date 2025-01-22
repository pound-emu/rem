#ifndef ATOMIC_H
#define ATOMIC_H

#include <inttypes.h>
#include <mutex>
#include "debugging.h"

static std::mutex global_lock;

template<typename T>
static T compare_and_swap_impl(uint64_t address_src, T expecting, T to_swap)
{
    T* address = (T*)address_src;

    if (*address == expecting)
    {
        *address = to_swap;

        return true;
    }

    return false;
}

//TODO: account for unalgined pointer
static uint64_t compare_and_swap_interpreter_cpp(uint64_t physical_address, uint64_t expecting, uint64_t to_swap, uint64_t size)
{
    global_lock.lock();

    bool result;

    switch (size)
    {
        case 8:     result = compare_and_swap_impl<uint8_t>(physical_address, expecting, to_swap); break;
        case 16:    result = compare_and_swap_impl<uint16_t>(physical_address, expecting, to_swap); break;
        case 32:    result = compare_and_swap_impl<uint32_t>(physical_address, expecting, to_swap); break;
        case 64:    result = compare_and_swap_impl<uint64_t>(physical_address, expecting, to_swap); break;
        default:    throw_error();
    }

    global_lock.unlock();

    return result;
}

#endif