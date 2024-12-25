#include "guest_function_store.h"
#include "ir/ir.h"

guest_function guest_function_store::get_or_translate_function(guest_function_store* context, uint64_t address)
{
    context->lock.lock();

    if (context->functions.find(address) == context->functions.end())
    {
        context->lock.unlock();

        guest_function result = translate_function(context, address);

        context->lock.lock();

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

guest_function guest_function_store::translate_function(guest_function_store* context, uint64_t address)
{
    arena_allocator function_allocator = arena_allocator::create(1024 * 1024 * 1024);
    ir_operation_block* function_block;
    
    ir_operation_block::create(&function_block, &function_allocator);

    throw 0;

    arena_allocator::destroy(&function_allocator);
}