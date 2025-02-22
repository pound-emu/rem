#include "guest_function_store.h"
#include "ir/ir.h"
#include "guest_process.h"
#include "jit/jit_memory.h"

void guest_function_store::request_retranslate_function(guest_function_store* context, uint64_t address, guest_compiler_optimization_flags flags, translate_request_data process_context)
{
    context->retranslate_lock.lock();

    retranslate_request request;

    request.address = address;
    request.flags = flags;
    request.process_context = process_context;

    context->retranslate_requests.push_back(request);

    context->retranslate_lock.unlock();

    if (!context->retranslator_is_running)
    {
        context->retranslator_is_running = true;

        std::thread(guest_function_store::retranslate_functions, context).detach();
    }
}

void guest_function_store::retranslate_functions(guest_function_store* context)
{ 
    context->retranslator_is_running = true;

    int rest = 0;

    while (1)
    {
        context->retranslate_lock.lock();

        auto to_retranslate = context->retranslate_requests;
        context->retranslate_requests = std::vector<retranslate_request>();

        context->retranslate_lock.unlock();

        if (to_retranslate.size() == 0)
        {
            break;
        }

        for (auto i : to_retranslate)
        {
            translate_request_data process_context = i.process_context;

            guest_function result = process_context.translate_function(&process_context,i.flags);

            result.optimizations = i.flags;

            if (i.flags & guest_compiler_optimization_flags::use_flt)
            {   
                fast_function_table::insert_function(&context->native_function_table, i.address, result.jit_offset);
            }

            context->main_translate_lock.lock();

            context->functions[i.address] = result;

            context->main_translate_lock.unlock();
        }
    }  

    context->retranslator_is_running = false;
}

guest_function guest_function_store::get_or_translate_function(guest_function_store* context, uint64_t address, translate_request_data* process_context, bool incrament_usage_counter)
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
    else if (context->use_flt)
    {
        fast_function_table::init(function_table, address);
    }

    guest_process* p = (guest_process*)process_context->process;

    context->main_translate_lock.lock();

    if (context->functions.find(address) == context->functions.end())
    {
        if (!p->debug_mode)
        {
            guest_function result;

            result.times_executed = 0;
            result.optimizations = guest_compiler_optimization_flags::interpreted;
            result.jit_offset = UINT32_MAX;
            result.raw_function = nullptr;

            context->functions[address] = result;

            context->main_translate_lock.unlock();

            return result;
        }

        guest_compiler_optimization_flags entry_optimization = guest_compiler_optimization_flags::level_three;

        context->main_translate_lock.unlock();

        guest_function result = process_context->translate_function(process_context, entry_optimization);

        result.optimizations = entry_optimization;

        context->main_translate_lock.lock();

        context->functions[address] = result;

        if (entry_optimization == level_three && !p->debug_mode)
        {
            fast_function_table::insert_function(&context->native_function_table, address, result.jit_offset);
        }

        context->main_translate_lock.unlock();

        return result;
    }
    else
    {
        guest_function result = context->functions[address];

        if (incrament_usage_counter)
        {
            context->functions[address].times_executed++;
        }

        context->main_translate_lock.unlock();

        return result;
    }
}

void guest_function_store::destroy(guest_function_store* to_destory)
{
    fast_function_table::destroy(&to_destory->native_function_table);
}