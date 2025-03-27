#ifndef GUEST_FUNCTION_STORE_H
#define GUEST_FUNCTION_STORE_H

#include <unordered_map>
#include <inttypes.h>
#include <mutex>
#include <vector>
#include <thread>
#include <atomic>

#include "translate_request_data.h"
#include "fast_function_table.h"
#include "guest_compiler_optimization_flags.h"
#include "translate_request_data.h"

#define THREAD_COUNT 10

struct guest_function_store;
struct translate_request_data;

struct retranslate_request
{
    uint64_t                            address;
    guest_compiler_optimization_flags   flags;
    translate_request_data              process_context;
};

struct guest_function_store
{
    std::mutex                                      main_translate_lock;
    std::mutex                                      retranslate_lock;
    std::unordered_map<uint64_t, guest_function>    functions;
    translate_guest_function                        translate_function_pointer;
    fast_function_table                             native_function_table;
    bool                                            use_flt;

    std::vector<retranslate_request>                retranslate_requests;
    std::thread                                     retranslator_thread;

    bool                                            retranslator_is_running;
    bool                                            retranslator_workers[THREAD_COUNT];

    static void                                     request_retranslate_function(guest_function_store* context, uint64_t address, guest_compiler_optimization_flags flags, translate_request_data process_context);
    
    static guest_function                           get_or_translate_function(guest_function_store* context, uint64_t address, translate_request_data* process_context, bool incrament_usgae_counter = false);
    static void                                     destroy(guest_function_store* to_destory);
};

#endif
