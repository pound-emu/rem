#ifndef GUEST_FUNCTION_STORE_H
#define GUEST_FUNCTION_STORE_H

#include <unordered_map>
#include <inttypes.h>
#include <mutex>

#include "guest_function.h"

struct guest_function_store;
struct translate_request_data;

typedef guest_function (*translate_guest_function)(translate_request_data*);

struct translate_request_data
{
    void*                       process;
    uint64_t                    address;
    translate_guest_function    translate_function;
};

struct guest_function_store
{
    std::mutex                                      lock;
    std::unordered_map<uint64_t, guest_function>    functions;
    translate_guest_function                        translate_function_pointer;

    static guest_function                           get_or_translate_function(guest_function_store* context, uint64_t address, translate_request_data* process_context);
};

#endif
