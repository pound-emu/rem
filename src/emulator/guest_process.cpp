#include "guest_process.h"
#include "aarch64/aarch64_emit_context.h"
#include "jit/jit_context.h"
#include "tools/numbers.h"
#include "debugging.h"

#include "aarch64/aarch64_process.h"
#include "aarch64/aarch64_impl.h"
#include "jit/jit_memory.h"

#include <iostream>
#include <iomanip>

uint64_t guest_process::jit_function(guest_process* process, uint64_t guest_function_address, void* arm_context)
{
    translate_request_data translator_request = 
    {
        process,
        guest_function_address,
        guest_process::translate_function
    };
    
    guest_function function_to_execute = guest_function_store::get_or_translate_function(&process->guest_functions, guest_function_address, &translator_request, true);

    void* arguments[] = { arm_context };

    switch (function_to_execute.optimizations)
    {
        case interpreted:
        {
            if (function_to_execute.times_executed == 3)
            {
                guest_function_store::request_retranslate_function(&process->guest_functions,guest_function_address, level_one, translator_request);
            }

            return guest_process::interperate_function(process, guest_function_address, arm_context, nullptr, true);
        }; break;

        case level_one:
        {
            if (function_to_execute.times_executed == 50 && !process->debug_mode)
            {
                guest_function_store::request_retranslate_function(&process->guest_functions,guest_function_address, level_two, translator_request); 
            }
        } break;

        case level_two:
        {
            if (function_to_execute.times_executed == 100 && !process->debug_mode)
            {
                guest_function_store::request_retranslate_function(&process->guest_functions,guest_function_address, level_three, translator_request); 
            }
        } break;

    }

    return jit_context::call_jitted_function(process->host_jit_context, (void*)function_to_execute.raw_function, (uint64_t*)arguments);
}

static uint64_t undefined_instruction_error(guest_process* process, void* context, uint64_t address)
{
    if (process->undefined_instruction == nullptr)
    {
        void* physical_memory = process->guest_memory_context.translate_address(process->guest_memory_context.base,address);

        uint32_t instruction = *(uint32_t*)physical_memory;

        std::cout << "Undefined instruction " << std::hex << instruction << " " << std::setfill('0') << std::setw(8) << std::hex << reverse_bytes(instruction) << std::endl;

        throw_error();
    }
    else
    {
        return ((uint64_t(*)(void*, uint64_t))process->undefined_instruction)(context,address);
    }
}

guest_function guest_process::translate_function(translate_request_data* data, guest_compiler_optimization_flags flags)
{
    uint64_t entry_address = data->address;
    guest_process* process = (guest_process*)data->process;

    arena_allocator allocator = arena_allocator::create(10 * 1024 * 1024);

    ir_operation_block* raw_ir;
    ir_operation_block::create(&raw_ir, &allocator);
    
    ssa_emit_context ssa_emit;
    ssa_emit_context::create(&ssa_emit, raw_ir);

    aarch64_emit_context aarch64_emit;
    aarch64_emit_context::create(process,&aarch64_emit, &ssa_emit, flags);

    ssa_emit.context_data = &aarch64_emit;

    aarch64_emit_context::init_context(&aarch64_emit);

    aarch64_emit.basic_block_translate_que.insert(entry_address);

    std::unordered_set<uint64_t> already_translated;

    aarch64_emit.translate_functions = true;

    int instruction_limit = INT32_MAX;

    if (!(flags & guest_compiler_optimization_flags::guest_function_wide_translation))
    {
        instruction_limit = 50;
    }

    ssa_emit_context::reset_local(&ssa_emit);

    ssa_emit.memory_base = ssa_emit_context::create_global(&ssa_emit, int64);
    
    ir_operation_block::emitds(raw_ir, ir_move, ssa_emit.memory_base, ir_operand::create_con((uint64_t)process->guest_memory_context.base));

    int backend_compiler_flags = (compiler_flags)0;

    if (flags & guest_compiler_optimization_flags::guest_optimize_basic_ssa)
    {
        backend_compiler_flags = compiler_flags::optimize_basic_ssa;
    }
    
    if (flags & guest_compiler_optimization_flags::guest_optimize_group_ssa)
    {
        backend_compiler_flags = compiler_flags::optimize_group_pool_ssa;
    }

    while (true)
    {
        std::unordered_set<uint64_t> to_compile_que = aarch64_emit.basic_block_translate_que;
        aarch64_emit.basic_block_translate_que = std::unordered_set<uint64_t>();

        if (to_compile_que.size() == 0)
        {
            break;
        }

        for (auto i : to_compile_que)
        {
            uint64_t working_address = i;

            if (already_translated.find(i) != already_translated.end())
            {
                continue;
            }

            already_translated.insert(i);

            ir_operand label = aarch64_emit_context::get_or_create_basic_block_label(&aarch64_emit, working_address);

            aarch64_emit.basic_block_labels[i] = label;

            ir_operation_block::mark_label(raw_ir, label);

            while (true)
            {
                void* instruction_address = process->guest_memory_context.translate_address(process->guest_memory_context.base, working_address);

                uint32_t raw_instruction = *(uint32_t*)instruction_address;

                auto instruction_table = fixed_length_decoder<uint32_t>::decode_fast(&process->fixed_length_decoder_context, raw_instruction);

                if (!(backend_compiler_flags & guest_compiler_optimization_flags::guest_optimize_basic_ssa))
                {
                    ssa_emit_context::reset_local(&ssa_emit);
                }

                if (instruction_table == nullptr)
                {
                    ir_operand function_pointer = ir_operand::create_con((uint64_t)undefined_instruction_error, int64);
                    ir_operand new_address = ssa_emit_context::create_local(&ssa_emit, int64);

                    aarch64_emit_context::emit_store_context(&aarch64_emit);

                    ir_operation_block::emitds(raw_ir, ir_external_call, new_address, function_pointer, ir_operand::create_con((uint64_t)process), aarch64_emit.context_pointer, ir_operand::create_con(working_address));
                    
                    aarch64_emit_context::emit_load_context(&aarch64_emit);

                    aarch64_emit_context::branch_long(&aarch64_emit, new_address, false);
                }
                else
                {
                    aarch64_emit.branch_state = branch_type::no_branch;

                    aarch64_emit.current_instruction_address = working_address;
                    aarch64_emit.current_raw_instruction = raw_instruction;

                    ((void(*)(ssa_emit_context*, uint32_t))instruction_table->emit)(&ssa_emit, raw_instruction);   
                }

                if (aarch64_emit.branch_state == no_branch)
                {
                    working_address += 4;

                    instruction_limit--;

                    if (instruction_limit < 0)
                    {
                        aarch64_emit_context::branch_long(&aarch64_emit, ir_operand::create_con(working_address));

                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
    
    aarch64_emit_context::emit_context_movement(&aarch64_emit);

    uint64_t code_size;

    void* code = jit_context::compile_code(process->host_jit_context, raw_ir,(compiler_flags)backend_compiler_flags, &code_size);

    if (((guest_process*)data->process)->log_native != nullptr && flags == guest_compiler_optimization_flags::level_three)
    {
        ((void(*)(void*, int))((guest_process*)data->process)->log_native)(code, code_size);
    }

    guest_function result;

    result.times_executed = 0;
    result.raw_function = (void(*)(void*))code;
    result.jit_offset = (uint64_t)result.raw_function - (uint64_t)process->host_jit_context->jit_cache.memory->raw_memory_block;
    
    arena_allocator::destroy(&allocator);

    return result;
}

uint64_t guest_process::interperate_function(guest_process* process, uint64_t guest_function, void* arm_context, bool* is_running, bool exit_on_long_branch)
{
    interpreter_data interpreter;

    interpreter.process_context = process;
    interpreter.register_data = arm_context;
    interpreter.current_pc = guest_function;

    bool done = 1;

    if (is_running == nullptr)
    {
        is_running = &done;
    }

    while (*is_running)
    {
        void* physical_memory = process->guest_memory_context.translate_address(process->guest_memory_context.base,interpreter.current_pc);

        uint32_t instruction = *(uint32_t*)physical_memory;

        interpreter.current_instruction = instruction;

        auto table = fixed_length_decoder<uint32_t>::decode_fast(&process->fixed_length_decoder_context, instruction);

        if (table == nullptr)
        {
            interpreter.current_pc = undefined_instruction_error(process, arm_context, interpreter.current_pc);

            continue;
        }

        interpreter.branch_type = branch_type::no_branch;

        ((void(*)(interpreter_data*, uint32_t))table->interpret)(&interpreter, instruction);

        if (interpreter.process_context->debug_mode)
        {
            break;
        }

        if (interpreter.branch_type == no_branch)
        {
            interpreter.current_pc += 4;
        }

        if (interpreter.branch_type == long_branch && exit_on_long_branch)
        {
            break;
        }
    }

    return interpreter.current_pc;
}


//TODO
void guest_process::create(guest_process* result, guest_memory memory, jit_context* jit, cpu_type process_type, cpu_size process_size, memory_order process_memory_order)
{
    switch (process_type)
    {
        case arm:
        {   
            aarch64_process* process = new aarch64_process;

            process->_32_bit = process_size == _32_bit;

        }; break;
        default: throw_error();
    }
}

void guest_process::create_guest_process(guest_process* result, guest_memory guest_memory_context, jit_context* host_jit_context, void* context_data, int context_data_size, cpu_type cpu, cpu_size size, memory_order order)
{
    result->guest_memory_context = guest_memory_context;
    result->host_jit_context = host_jit_context;
    
    if (context_data != nullptr)
    {
        memcpy(result->guest_context_data, context_data, context_data_size);
    }

    result->svc_function = nullptr;
    result->undefined_instruction = nullptr;
    result->debug_mode = false;
    result->guest_functions.use_flt = true;

    memset(result->guest_functions.retranslator_workers, 0, sizeof(guest_function_store::retranslator_workers));

    result->guest_functions.retranslator_is_running = false;
    result->log_native = nullptr;

    switch (cpu)
    {
        case arm:
        {
            switch (size)
            {
                case _64_bit:
                {
                    init_aarch64_decoder(result);
                }; break;

                default:
                {
                    throw_error();
                }; break;
            } 
        }; break;

        default:
        {
            throw_error();
        }; break;
    }
}


void guest_process::destroy(guest_process* process)
{
    guest_function_store::destroy(&process->guest_functions);

    switch (process->process_type)
    {
        case arm:
        {
            delete (aarch64_process*)process->process_data;
        }; break;

        default: throw_error();
    }
}