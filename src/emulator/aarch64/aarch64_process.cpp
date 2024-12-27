#include "aarch64_process.h"
#include "aarch64_emit_context.h"
#include "jit/jit_context.h"
#include "aarch64_impl.h"

void aarch64_process::create(aarch64_process* result, guest_memory guest_memory_context, jit_context* host_jit_context, aarch64_context_offsets arm_guest_data)
{
    result->guest_memory_context = guest_memory_context;
    result->host_jit_context = host_jit_context;
    result->guest_context_offset_data = arm_guest_data;

    init_aarch64_decoder(result);
}

uint64_t aarch64_process::jit_function(aarch64_process* process, uint64_t guest_function_address, void* arm_context)
{
    translate_request_data translator_request = 
    {
        process,
        guest_function_address,
        aarch64_process::translate_function
    };
    
    guest_function function_to_execute = guest_function_store::get_or_translate_function(&process->guest_functions, guest_function_address, &translator_request);

    void* arguments[] = { arm_context };

    return jit_context::call_jitted_function(process->host_jit_context, (void*)function_to_execute.raw_function, (uint64_t*)arguments);
}

guest_function aarch64_process::translate_function(translate_request_data* data)
{
    uint64_t entry_address = data->address;
    aarch64_process* process = (aarch64_process*)data->process;

    arena_allocator allocator = arena_allocator::create(10 * 1024 * 1024);

    ir_operation_block* raw_ir;
    ir_operation_block::create(&raw_ir, &allocator);
    
    ssa_emit_context ssa_emit;
    ssa_emit_context::create(&ssa_emit, raw_ir);

    aarch64_emit_context aarch64_emit;
    aarch64_emit_context::create(process,&aarch64_emit, &ssa_emit);

    ssa_emit.context_data = &aarch64_emit;

    aarch64_emit_context::init_context(&aarch64_emit);

    aarch64_emit.basic_block_translate_que.insert(entry_address);

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

            if (aarch64_emit_context::basic_block_translated(&aarch64_emit, i))
            {
                continue;
            }

            ir_operand label = ir_operation_block::create_label(raw_ir);

            aarch64_emit.basic_block_labels[i] = label;

            ir_operation_block::mark_label(raw_ir, label);

            while (true)
            {
                void* instruction_address = process->guest_memory_context.translate_address(process->guest_memory_context.base, working_address);

                uint32_t raw_instruction = *(uint32_t*)instruction_address;

                auto instruction_table = fixed_length_decoder<uint32_t>::decode_slow(&process->decoder, raw_instruction);

                if (instruction_table == nullptr)
                {
                    std::cout << std::hex << "Undefined Instruction " << raw_instruction << std::endl;

                    throw 0;
                }

                aarch64_emit.branch_state = branch_type::no_branch;

                ssa_emit_context::reset_local(&ssa_emit);

                aarch64_emit.current_instruction_address = working_address;
                aarch64_emit.current_raw_instruction = raw_instruction;

                ((void(*)(ssa_emit_context*, uint32_t))instruction_table->emit)(&ssa_emit, raw_instruction);

                if (aarch64_emit.branch_state == no_branch)
                {
                    working_address += 4;
                }
                else if (aarch64_emit.branch_state == long_branch)
                {
                    break;
                }
                else
                {
                    throw 0;
                }
            }
        }
    }

    aarch64_emit_context::emit_context_movement(&aarch64_emit);

    void* code = jit_context::compile_code(process->host_jit_context, raw_ir, compiler_flags::check_undefined_behavior);

    guest_function result;

    result.times_executed = 0;
    result.raw_function = (void(*)(void*))code;
    
    arena_allocator::destroy(&allocator);

    return result;
}

uint64_t aarch64_process::interperate_function(aarch64_process* process, uint64_t guest_function, void* arm_context)
{
    interpreter_data interpreter;

    interpreter.process_context = process;
    interpreter.register_data = arm_context;
    interpreter.current_pc = guest_function;

    while (true)
    {
        void* physical_memory = process->guest_memory_context.translate_address(process->guest_memory_context.base,interpreter.current_pc);

        uint32_t instruction = *(uint32_t*)physical_memory;

        auto table = fixed_length_decoder<uint32_t>::decode_slow(&process->decoder, instruction);

        if (table == nullptr)
        {
            std::cout << std::hex << "undefined instruction " << instruction << std::endl;

            throw 0;
        }

        interpreter.branch_type = branch_type::no_branch;

        ((void(*)(interpreter_data*, uint32_t))table->interpret)(&interpreter, instruction);

        if (interpreter.branch_type & branch_type::long_branch)
        { 
            break;
        }
        else if (interpreter.branch_type == branch_type::no_branch)
        {
            interpreter.current_pc += 4;
        }
    }

    return interpreter.current_pc;
}