#ifndef RARMA_H
#define RARMA_H

#include <string>
#include <inttypes.h>
#include <iostream>
#include <assert.h>

struct rarma_context
{
    void*       memory_block;

    uint64_t    memory_block_size;
    uint64_t    memory_location;

    enum operand_type
    {
        zr,
        sp,
        gp,
        vec
    };

    enum shift_type
    {
        lsl,
        lsr,
        ror,
        asr
    };

    struct operand
    {
        int             operand_size;
        int             operand_register_index;

        operand_type    type;
    };

    static uint64_t get_code_size(rarma_context* context)
    {
        return context->memory_location;
    }

    static operand gp_size(int index, int size, operand_type type)
    {
        operand result;

        result.operand_size = size;
        result.operand_register_index = index;
        result.type = type;

        assert_in_mask(index, 5);

        switch (type)
        {
            case sp:
            case zr:
            {
                assert(index == 31);
                
                assert(size >= 2 && size <= 3);
            }; break;
        }

        return result;
    }

    static operand X(int index)
    {
        return gp_size(index, 3, operand_type::gp);
    }

    static operand SP()
    {
        return gp_size(31, 3, operand_type::sp);
    }

    static operand W(int index)
    {
        return gp_size(index, 2, operand_type::gp);
    }

    static operand WSP()
    {
        return gp_size(31, 2, operand_type::sp);
    }

    static operand H(int index)
    {
        return gp_size(index, 1, operand_type::gp);
    }

    static operand B(int index)
    {
        return gp_size(index, 0, operand_type::gp);
    }

    static void assert_is_gp_sp(operand test)
    {
        if (test.operand_register_index == 31)
        {
            assert(test.type == sp);
        }
        else
        {
            assert(test.type == gp);
        }
    }

    static void assert_is_gp_zr(operand test)
    {
        if (test.operand_register_index == 31)
        {
            assert(test.type == zr);
        }
        else
        {
            assert(test.type == gp);
        }
    }

    static void assert_same_size(operand left, operand right)
    {
        assert(left.operand_size == right.operand_size);
    }

    static bool is_vector(operand test)
    {
        return test.operand_size == 4;
    }

    static void assert_not_vector(operand test)
    {
        assert(!is_vector(test));
    }

    static void assert_w_or_x(operand test)
    {
        assert(test.operand_size == 2 || test.operand_size == 3);
    }

    static void assert_in_mask(int imm, int bit_count)
    {
        assert((imm & ~create_mask(bit_count)) == 0);
    }

    static void create(rarma_context* result, uint64_t memory_block_size = 1024)
    {
        result->memory_block = malloc(memory_block_size);

        result->memory_location = 0;
        result->memory_block_size = memory_block_size;
    }

    static void destroy(rarma_context* to_destroy)
    {
        free(to_destroy->memory_block);
    }

    static uint32_t write_instruction(rarma_context* context,uint32_t instruction)
    {
        if (context->memory_location >= context->memory_block_size)
        {
            std::cout << "RARM OUT OF SPACE" << std::endl;

            throw 0;
        }

        *(uint32_t*)((uint64_t)context->memory_block + context->memory_location) = instruction;

        context->memory_location += 4;

        return instruction;
    }

    static int create_mask(int size)
    {
        return (1 << size) - 1;
    }

    static uint32_t add_subtract_shifted(rarma_context* context, bool is_add, bool set_flags, operand d, operand n, operand m, shift_type shift, int imm6)
    {
        int sf = d.operand_size == 3;
        int op = !is_add;
        int S = set_flags;

        assert_in_mask(imm6, 6);
        assert_in_mask(shift, 2);

        assert_is_gp_zr(d);
        assert_is_gp_zr(n);
        assert_is_gp_zr(m);

        uint32_t result = 0b01011 << 24;

        result |= sf << 31;
        result |= op << 30;
        result |= S << 28;
        result |= shift << 22;
        result |= m.operand_register_index << 16;
        result |= imm6 << 10;
        result |= n.operand_register_index << 5;
        result |= d.operand_register_index;

        write_instruction(context, result);

        return result;
    }

    static uint32_t add_subtract_imm12(rarma_context* context, bool is_add, bool set_flags, operand d, operand n, int imm12)
    {
        int sf = d.operand_size == 3;
        int op = !is_add;
        int S = set_flags;
        int sh;
        
        if (imm12 == 0)
        {
            sh = 0;
        }
        else if (imm12 & create_mask(12))
        {
            assert_in_mask(imm12, 12);

            sh = 0;
        }
        else if (imm12 >> 12)
        {
            imm12 >>= 12;

            assert((imm12 & ~create_mask(12)) == 0);

            sh = 1;
        }

        uint32_t result = 0b100010 << 23;

        assert_same_size(d, n);
        assert_not_vector(d);
        assert_w_or_x(d);

        assert_is_gp_sp(n);

        if (set_flags)
        {
            assert_is_gp_zr(d);
        }
        else
        {
            assert_is_gp_sp(d);
        }
        
        result |= (sf << 31);
        result |= (op << 30);
        result |= (S << 29);
        result |= (sh << 22);

        result |= d.operand_register_index;
        result |= n.operand_register_index << 5;
        result |= imm12 << 10;

        write_instruction(context, result);

        return result;
    }

    static uint32_t add_imm12(rarma_context* context, operand d, operand n, int imm12)
    {
        return add_subtract_imm12(context, true, false, d, n, imm12);
    }

    static uint32_t sub_imm12(rarma_context* context, operand d, operand n, int imm12)
    {
        return add_subtract_imm12(context, false, false, d, n, imm12);
    }

    static uint32_t adds_imm12(rarma_context* context, operand d, operand n, int imm12)
    {
        return add_subtract_imm12(context, true, true, d, n, imm12);
    }

    static uint32_t subs_imm12(rarma_context* context, operand d, operand n, int imm12)
    {
        return add_subtract_imm12(context, false, true, d, n, imm12);
    }

    static uint32_t add_shifted(rarma_context* context, operand d, operand n, operand m, shift_type shift = shift_type::lsl, int imm6 = 0)
    {
        return add_subtract_shifted(context, true, false, d, n, m, shift, imm6);
    }

    static uint32_t sub_shifted(rarma_context* context, operand d, operand n, operand m, shift_type shift = shift_type::lsl, int imm6 = 0)
    {
        return add_subtract_shifted(context, false, false, d, n, m, shift, imm6);
    }

    static uint32_t adds_shifted(rarma_context* context, operand d, operand n, operand m, shift_type shift = shift_type::lsl, int imm6 = 0)
    {
        return add_subtract_shifted(context, true, true, d, n, m, shift, imm6);
    }

    static uint32_t subs_shifted(rarma_context* context, operand d, operand n, operand m, shift_type shift = shift_type::lsl, int imm6 = 0)
    {
        return add_subtract_shifted(context, false, true, d, n, m, shift, imm6);
    }

    static uint32_t ret(rarma_context* context, operand n)
    {
        uint32_t result = 0b1101011001011111000000 << 10;

        assert_is_gp_zr(n);

        result |= n.operand_register_index << 5;

        write_instruction(context, result);

        return result;
    }
};

#endif