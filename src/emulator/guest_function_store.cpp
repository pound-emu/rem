#include "guest_function_store.h"
#include "ir/ir.h"
#include "guest_process.h"
#include "jit/jit_memory.h"

guest_function guest_function_store::get_or_translate_function(guest_function_store* context, uint64_t address, translate_request_data* process_context)
{
    auto function_table = &context->native_function_table;

    if (fast_function_table::is_open(function_table))
    {
        bool exists;

        uint32_t jit_offset = fast_function_table::request_address(function_table, address, &exists);

        if (exists)
        {
            guest_function result;

            result.times_executed = 0;
            result.jit_offset = jit_offset;
            
            uint64_t jit_base = (uint64_t)((guest_process*)process_context->process)->host_jit_context->jit_cache.memory->raw_memory_block;
            
            result.raw_function = (void(*)(void*))(jit_base + jit_offset);

            return result;
        }
    }

    context->lock.lock();

    if (context->functions.find(address) == context->functions.end())
    {
        context->lock.unlock();

        guest_function result = process_context->translate_function(process_context);

        context->lock.lock();

        if (context->use_flt)
        {
            fast_function_table::insert_function(function_table, address, result.jit_offset);
        }

        context->functions[address] = result;

        context->lock.unlock();

        return result;
    }
    else
    {
        guest_function result = context->functions[address];

        context->lock.unlock();

        return result;
    }
}

void guest_function_store::destroy(guest_function_store* to_destory)
{
    fast_function_table::destroy(&to_destory->native_function_table);
}