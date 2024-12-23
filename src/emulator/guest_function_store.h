#ifndef GUEST_FUNCTION_STORE_H
#define GUEST_FUNCTION_STORE_H

#include <unordered_map>
#include <inttypes.h>
#include <mutex>

#include "guest_function.h"

struct guest_function_store
{
    std::mutex                                      lock;
    std::unordered_map<uint64_t, guest_function>    functions;

    static guest_function                           get_or_translate_function(guest_function_store* context, uint64_t address);
    static guest_function                           translate_function(guest_function_store* context, uint64_t address);
};

#endif
