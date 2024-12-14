#include "undefined_behavior_check.h"
#include "tools/bit_tools.h"

static ir_operand create_register(std::unordered_set<uint64_t>* used_operands, uint64_t* top, uint64_t new_size)
{
    while (1)
    {
        if (used_operands->find(*top) != used_operands->end())
        {
            --*top;   

            continue;
        }

        return ir_operand::create_reg(*top, new_size);
    }
}

void run_undefined_behavior_check_pass(ir_operation_block* ir, ir_operation_block* source)
{
    std::unordered_set<uint64_t> used_operands;

    ir_operation_block::get_used_registers(&used_operands, source);

    for (auto i = source->operations->first->next; i != nullptr; i = i->next)
    {
        ir_operation working_operation = i->data;

        uint64_t top = UINT64_MAX;

        ir_operand* destinations = working_operation.destinations.data;
        ir_operand* sources = working_operation.sources.data;

        switch (working_operation.instruction)
        {
            case ir_shift_left:
            case ir_shift_right_signed:
            case ir_shift_right_unsigned:
            {
                //shift d, n, m

                uint64_t working_size = ir_operand::get_raw_size(&destinations[0]);
                uint64_t bit_width = (8 << working_size);

                //When m >= bit_width, undefined.

                ir_operand m_greater_equal_bit_width = create_register(&used_operands, &top, working_size);

                ir_operation_block::emitds(ir, ir_compare_greater_equal_unsigned, m_greater_equal_bit_width, sources[1], ir_operand::create_con(bit_width, working_size));
                ir_operation_block::emits(ir, ir_assert_false, m_greater_equal_bit_width);

            }; break;

            case ir_divide_unsigned:
            case ir_divide_signed:
            {
                //div d, n, m

                uint64_t working_size = ir_operand::get_raw_size(&destinations[0]);

                //When m == 0, undefined
                ir_operand m_is_zero = create_register(&used_operands, &top, working_size);

                ir_operation_block::emitds(ir, ir_compare_equal, m_is_zero, sources[1], ir_operand::create_con(0, working_size));
                ir_operation_block::emits(ir, ir_assert_false, m_is_zero);

                if (working_operation.instruction != ir_divide_signed)
                    break;

                //When signed and n == INT_MIN and m == -1, undefined
                ir_operand n_int_min = create_register(&used_operands, &top, working_size);
                ir_operand m_neg_one = create_register(&used_operands, &top, working_size);
                ir_operand result_overflows = create_register(&used_operands, &top, working_size);

                ir_operation_block::emitds(ir, ir_compare_equal, n_int_min, sources[0], ir_operand::create_con(create_int_min(working_size), working_size));
                ir_operation_block::emitds(ir, ir_compare_equal, m_neg_one, sources[1], ir_operand::create_con(-1, working_size));

                ir_operation_block::emitds(ir, ir_bitwise_and, result_overflows, n_int_min, m_neg_one);

                ir_operation_block::emits(ir, ir_assert_false, result_overflows);

            }; break;

            case ir_double_shift_right:
            {
                //ir_double_shift_right d, n, m, a

                uint64_t working_size = ir_operand::get_raw_size(&destinations[0]);
                uint64_t bit_width = (8 << working_size);

                //When a == 0, undefined
                ir_operand a_is_zero = create_register(&used_operands, &top, working_size);
                ir_operation_block::emitds(ir, ir_compare_equal, a_is_zero, sources[2], ir_operand::create_con(0, working_size));
                ir_operation_block::emits(ir, ir_assert_false, a_is_zero);

                //When a >= bit_width, undefined
                ir_operand a_greater_equal_bit_width = create_register(&used_operands, &top, working_size);

                ir_operation_block::emitds(ir, ir_compare_greater_equal_unsigned, a_greater_equal_bit_width, sources[2], ir_operand::create_con(bit_width, working_size));
                ir_operation_block::emits(ir, ir_assert_false, a_greater_equal_bit_width);
            }; break;
        }

        ir_operation_block::emit_with(ir,
            working_operation.instruction, 
            working_operation.destinations.data, working_operation.destinations.count,
            working_operation.sources.data, working_operation.sources.count
        );
    }
}