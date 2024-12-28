#include "ir_emulator.h"
#include "ir/checks.h"
#include "tools/numbers.h"
#include "tools/big_number.h"
#include "tools/bit_tools.h"

#define GENERIC_TEMPLATE tenplate<typename T>

static void init_ir_emulator_context(ir_emulator* ir_emulator_context, ir_operation_block* to_execute, uint64_t* arguments)
{
    ir_emulator_context->working_registers = std::unordered_map<uint64_t, uint64_t>();
    ir_emulator_context->arguments = arguments;
}

static void advance(ir_emulator* ir_emulator_context)
{
    ir_emulator_context->working_instruction_element = ir_emulator_context->working_instruction_element->next;
}

static uint64_t* get_destination(ir_emulator* ir_emulator_context, ir_operand working_operand)
{
    assert_is_register(working_operand);

    if (ir_emulator_context->working_registers.find(working_operand.value) == ir_emulator_context->working_registers.end())
    {
        ir_emulator_context->working_registers[working_operand.value] = 0;
    }

    return &ir_emulator_context->working_registers[working_operand.value];
}

static uint64_t get_source(ir_emulator* ir_emulator_context, ir_operand working_operand)
{
    uint64_t result;

    if (ir_operand::is_constant(&working_operand))
    {
        result = working_operand.value;
    }
    else
    {
        if (ir_emulator_context->working_registers.find(working_operand.value) == ir_emulator_context->working_registers.end())
        {
            ir_emulator_context->working_registers[working_operand.value] = 0;
        }

        result = ir_emulator_context->working_registers[working_operand.value];
    }

    return result & get_mask_from_size(ir_operand::get_raw_size(&working_operand));
}

static void execute_unary_operation(ir_emulator* ir_emulator_context)
{
    ir_operation working_instruction = ir_emulator_context->working_instruction_element->data;

    assert_operand_count(&working_instruction, 1, 1);
    assert_registers_same_type(&working_instruction);

    ir_operand* destinations = working_instruction.destinations.data;
    ir_operand* sources = working_instruction.sources.data;

    assert_is_register(destinations[0]);

    uint64_t working_size = ir_operand::get_raw_size(&destinations[0]);

    uint64_t x = get_source(ir_emulator_context, sources[0]);

    uint64_t result;

    switch (working_instruction.instruction)
    {
        case ir_move:           result = x;     break;
        case ir_bitwise_not:    result = ~x;    break;
        case ir_incrament:      result = x + 1; break;
        case ir_negate:         result = -x;    break;
        case ir_sign_extend:
        {
            int64_t sx = sign_extend_from_size(x, sources[0].meta_data);
            
            result = sign_extend_from_size(sx, destinations[0].meta_data);
        }; break;

        default: throw 0;
    }
    
    *get_destination(ir_emulator_context, destinations[0]) = zero_extend_from_size(result, working_size);

    advance(ir_emulator_context);
}

static void execute_operation_binary(ir_emulator* ir_emulator_context)
{
    ir_operation working_instruction = ir_emulator_context->working_instruction_element->data;

    assert_operand_count(&working_instruction, 1, 2);
    assert_registers_same_type(&working_instruction);

    ir_operand* destinations = working_instruction.destinations.data;
    ir_operand* sources = working_instruction.sources.data;

    assert_is_register(destinations[0]);

    uint64_t working_size = ir_operand::get_raw_size(&destinations[0]);
    uint64_t bit_count = 8 << working_size;

    uint64_t x = get_source(ir_emulator_context, sources[0]);
    uint64_t y = get_source(ir_emulator_context, sources[1]);

    ir_operand s1 = sources[1];

    uint64_t result;

    switch (working_instruction.instruction)
    {
        case ir_add:                            result = x + y; break;
        case ir_bitwise_and:                    result = x & y; break;
        case ir_bitwise_exclusive_or:           result = x ^ y; break;
        case ir_bitwise_or:                     result = x | y; break;
        case ir_compare_equal:                  result = x == y; break;
        case ir_compare_greater_equal_signed:   result = sign_extend_from_size(x, working_size) >= sign_extend_from_size(y, working_size); break;
        case ir_compare_greater_equal_unsigned: result = x >= y; break;
        case ir_compare_greater_signed:         result = sign_extend_from_size(x, working_size) > sign_extend_from_size(y, working_size); break;
        case ir_compare_greater_unsigned:       result = x > y; break;
        case ir_compare_less_equal_signed:      result = sign_extend_from_size(x, working_size) <= sign_extend_from_size(y, working_size); break;
        case ir_compare_less_equal_unsigned:    result = x <= y; break;
        case ir_compare_less_signed:            result = sign_extend_from_size(x, working_size) < sign_extend_from_size(y, working_size); break;
        case ir_compare_less_unsigned:          result = x < y; break;
        case ir_compare_not_equal:              result = x != y; break;
        case ir_multiply:                       result = x * y; break;
        case ir_subtract:                       result = x - y; break;

        case ir_divide_unsigned:
        case ir_divide_signed:
        {
            if (y == 0)
            {
                throw 0;
            }

            switch (working_instruction.instruction)
            {
                case ir_divide_unsigned: result = x / y; break;
                case ir_divide_signed: 
                {
                    int64_t sx = sign_extend_from_size(x, working_size);
                    int64_t sy = sign_extend_from_size(y, working_size);

                    if (sx == create_int_min(working_size) && sy == -1)
                    {
                        throw 0;
                    }

                    result = sx / sy;
                }; break;                
            }
        }; break;

        case ir_shift_left:  
        case ir_shift_right_signed:
        case ir_shift_right_unsigned:
        case ir_rotate_right:
        {
            uint64_t shift_mask = (8ULL << working_size) - 1;

            if (y > shift_mask)
            {
                throw 0;
            }

            switch (working_instruction.instruction)
            {
                case ir_shift_left:             result = x << y; break;
                case ir_shift_right_signed:     result = sign_extend_from_size(x, working_size) >> y; break;
                case ir_shift_right_unsigned:   result = x >> y; break;
                case ir_rotate_right:           result = (x >> y) | (x << (bit_count - y)); break;
                default: throw 0;
            }
        }; break;


        case ir_multiply_hi_unsigned:   
        case ir_multiply_hi_signed:
        {
            uint64_t top;
            uint64_t bottom;

            bool is_signed = working_instruction.instruction == ir_multiply_hi_signed;

            switch (working_size)
            {
            case int8:  multiply_hi<uint8_t>((uint8_t*)&top, (uint8_t*)&bottom, x, y, is_signed);       break;
            case int16: multiply_hi<uint16_t>((uint16_t*)&top, (uint16_t*)&bottom, x, y, is_signed);    break;
            case int32: multiply_hi<uint32_t>((uint32_t*)&top, (uint32_t*)&bottom, x, y, is_signed);    break;
            case int64: multiply_hi<uint64_t>((uint64_t*)&top, (uint64_t*)&bottom, x, y, is_signed);    break;
            
            default:
                break;
            }

            result = top;
        }; break;

    default:
        break;
    }

    *get_destination(ir_emulator_context, destinations[0]) = zero_extend_from_size(result, working_size);

    advance(ir_emulator_context);
}

static void execute_operation_ternary(ir_emulator* ir_emulator_context)
{
    ir_operation working_instruction = ir_emulator_context->working_instruction_element->data;

    assert_operand_count(&working_instruction, 1, 3);
    assert_registers_same_type(&working_instruction);

    ir_operand* destinations = working_instruction.destinations.data;
    ir_operand* sources = working_instruction.sources.data;

    assert_is_register(destinations[0]);

    uint64_t working_size = ir_operand::get_raw_size(&destinations[0]);
    uint64_t bit_count = 8 << working_size;

    uint64_t x = get_source(ir_emulator_context, sources[0]);
    uint64_t y = get_source(ir_emulator_context, sources[1]);
    uint64_t z = get_source(ir_emulator_context, sources[2]);

    ir_operand s1 = sources[1];

    uint64_t shift_mask = working_size <= int32 ? 31 : 63;

    uint64_t result;

    switch (working_instruction.instruction)
    {
        case ir_conditional_select:
        {
            result = x ? y : z;
        }; break;

        case ir_double_shift_right:
        {   
            int bit_count = (8 << working_size);

            if (z >= bit_count || z == 0)
            {
                throw 0;
            }
            
            result = (x >> z) | (y << (bit_count - z));

        }; break;

        default: throw 0;
    }

    *get_destination(ir_emulator_context, destinations[0]) = zero_extend_from_size(result, working_size);

    advance(ir_emulator_context);
}


void ir_emulator::create(ir_emulator* ir_emulator_context)
{
    ir_emulator_context->arguments = nullptr;
}

void ir_emulator::destroy(ir_emulator* to_destroy)
{
    
}

uint64_t ir_emulator::execute(ir_emulator* ir_emulator_context, ir_operation_block* to_execute, uint64_t* arguments)
{
    init_ir_emulator_context(ir_emulator_context,to_execute, arguments);

    ir_emulator_context->working_instruction_element = to_execute->operations->first;

    for (auto i = to_execute->operations->first; i != nullptr; i = i->next)
    {
        if (i->data.instruction != ir_mark_label)
            continue;

        ir_operation working_instruction = i->data;

        ir_emulator_context->labels[working_instruction.sources[0].value] = i;
    }

    while (1)
    {
        if (ir_emulator_context->working_instruction_element == nullptr)
        {
            //This should NEVER happen

            assert(false);

            throw 0;

            break;
        }

        ir_operation working_instruction = ir_emulator_context->working_instruction_element->data;

        ir_instructions instruction = (ir_instructions)working_instruction.instruction;

        switch (instruction)
        {
            case ir_add:
            case ir_bitwise_and:
            case ir_bitwise_exclusive_or:
            case ir_bitwise_or:
            case ir_compare_equal:
            case ir_compare_greater_equal_signed:
            case ir_compare_greater_equal_unsigned:
            case ir_compare_greater_signed:
            case ir_compare_greater_unsigned:
            case ir_compare_less_equal_signed:
            case ir_compare_less_equal_unsigned:
            case ir_compare_less_signed:
            case ir_compare_less_unsigned:
            case ir_compare_not_equal:
            case ir_divide_signed:
            case ir_divide_unsigned:
            case ir_multiply:
            case ir_multiply_hi_signed:
            case ir_multiply_hi_unsigned:
            case ir_subtract:
            case ir_rotate_right:
            case ir_shift_left:
            case ir_shift_right_unsigned:
            case ir_shift_right_signed:
            {
                execute_operation_binary(ir_emulator_context);
            }; break;

            case ir_move:
            case ir_negate:
            case ir_bitwise_not:
            case ir_incrament:
            case ir_sign_extend:
            {
                execute_unary_operation(ir_emulator_context);
            }; break;;

            case ir_no_operation:
            case ir_mark_label:
            {
                advance(ir_emulator_context);
            }; break;

            case ir_jump_if:
            {
                ir_operand label = working_instruction.sources[0];
                ir_operand condition = working_instruction.sources[1];

                uint64_t cond = get_source(ir_emulator_context, condition);

                if (cond)
                {
                    auto label_to_jump_to = (intrusive_linked_list_element<ir_operation>*)ir_emulator_context->labels[label.value];

                    ir_emulator_context->working_instruction_element = label_to_jump_to;
                }
                else
                {
                    advance(ir_emulator_context);
                }
            }; break;

            case ir_close_and_return:
            {
                assert_operand_count(&working_instruction, 0, 1);

                uint64_t value_to_return = get_source(ir_emulator_context, working_instruction.sources[0]);

                return value_to_return;
            }; break;

            case ir_conditional_select:
            case ir_double_shift_right:
            {
                execute_operation_ternary(ir_emulator_context);
            } break;

            default:
            {
                assert(false);

                throw 0;

                break;
            };
        }
    }
}