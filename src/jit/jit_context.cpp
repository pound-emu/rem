#include <assert.h>

#include "debugging.h"
#include "jit_context.h"
#include "jit_memory.h"

#include "assembly/aarch64/aarch64_assembler.h"
#include "assembly/aarch64/aarch64_pipeline.h"
#include "assembly/x86/x86_assembler.h"
#include "assembly/x86/x86_pipeline.h"

typedef uint64_t (*abi_caller_function)(void*, uint64_t*);

static void create_x86_caller(jit_context* result) {
    uint64_t max_space = 400;

    char caller_buffer[max_space];

    uint64_t code_size;
    assemble_x86_abi_caller_code(caller_buffer, &code_size, result->jit_cache.memory->host_abi);

    growing_jit_cache::append_code(&result->jit_cache, caller_buffer, max_space);

    if (code_size > max_space) {
        throw_error();
    }
}

void jit_context::create(jit_context* result, uint64_t allocation_size, abi abi_information) {
    jit_memory* jit_memory_context;

    jit_memory::create(&jit_memory_context, allocation_size, abi_information);
    growing_jit_cache::create(&result->jit_cache, jit_memory_context);

    switch (abi_information.cpu) {
    case x86_64: {
        create_x86_caller(result);
    }; break;

    case arm_64: {
        create_aarch64_caller(result, abi_information);
    }; break;

    default: {
        throw_error();
    }; break;
    }
}

void jit_context::destroy(jit_context* to_destroy) {
    jit_memory::destroy(to_destroy->jit_cache.memory);
}

void jit_context::create_from_host(jit_context* result, uint64_t allocation_size) {
    create(result, allocation_size, get_abi());
}

uint64_t jit_context::call_jitted_function(jit_context* context, void* function, uint64_t* arguments) {
    abi_caller_function abi_function = (abi_caller_function)context->jit_cache.memory->raw_memory_block;

    return abi_function(function, arguments);
}

void* jit_context::append_to_jit_cache(jit_context* context, void* source_function, uint64_t function_size) {
    return growing_jit_cache::append_code(&context->jit_cache, source_function, function_size);
}

void* jit_context::compile_code(jit_context* context, ir_operation_block* ir_operation_block_context, compiler_flags flags, uint64_t* code_size_result) {
    void* code_buffer;
    uint64_t code_size;

    abi working_abi = context->jit_cache.memory->host_abi;

    switch (context->jit_cache.memory->host_abi.cpu) {
    case x86_64: {
        assemble_x86_64_pipeline(&code_buffer, &code_size, ir_operation_block_context, working_abi, flags);
    }; break;

    case arm_64: {
        assemble_aarch64_pipeline(&code_buffer, &code_size, ir_operation_block_context, working_abi, flags);
    }; break;

    default:
        throw_error();
    }

    if (code_size_result != nullptr) {
        *code_size_result = code_size;
    }

    return jit_context::append_to_jit_cache(context, code_buffer, code_size);
}