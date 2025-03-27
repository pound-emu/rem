#ifndef RARMA_H
#define RARMA_H

#include <string>
#include <inttypes.h>
#include <iostream>
#include <assert.h>
#include <vector>
#include <string.h>
#include <sstream>
#include <iomanip>

#define create_mask(bits)               ((1ULL << bits) - 1)
#define assert_in_mask(value, bits)     (assert((value & ~create_mask(bits)) == 0))
#define assert_is_register(value)       (assert_in_mask(value, 5)) 
#define assert_is_bit(value)            (assert_in_mask(value, 1))
#define assert_masked_zero(value,bits)  assert((value & create_mask(bits)) == 0)
#define outside_of_bits(value, bits)    (value & ~create_mask(bits))
#define error()                         (throw 0)

#define X(index)    rarma_context::reg(index, rarma_context::operand_size::size_int64)
#define W(index)    rarma_context::reg(index, rarma_context::operand_size::size_int32)
#define SP()        rarma_context::sp(rarma_context::operand_size::size_int64)
#define WSP()       rarma_context::sp(rarma_context::operand_size::size_int32)
#define XZR()       rarma_context::zr(rarma_context::operand_size::size_int64)
#define WZR()       rarma_context::zr(rarma_context::operand_size::size_int32)
#define LR()        X(30)

struct rarma_context
{
    void*       memory_block;

    uint64_t    memory_block_size;
    uint64_t    memory_location;

    enum operand_type
    {
        _zr,
        _sp,
        _gp,
        vec,
        immediate
    };

    enum shift_option
    {
        lsl,
        lsr,
        ror,
        asr
    };

    enum extend_type
    {
        uxtb,
        uxth,
        uxtw,
        uxtx,
        sxtb,
        sxth,
        sxtw,
        sxtx
    };

    enum operand_size
    {
        size_int8,
        size_int16,
        size_int32,
        size_int64,
        size_int128,
        size_null = -1
    };

    struct base_operand
    {
        int             data;
        operand_size    size;
        operand_type    type;

        bool            is_active;
    };

    struct operand
    {
        #define base_count  3
        base_operand        bases[base_count];

        int                 _extend_type;
        int                 _shift_type;
        
        void init()
        {   
            memset(this, 0, sizeof(operand));

            _extend_type = -1;
            _shift_type = -1;
        }

        void init_base(int index, int data, operand_size size, operand_type type)
        {
            bases[index].data = data;
            bases[index].size = size;
            bases[index].type = type;

            bases[index].is_active = true;
        }

        operand()
        {
            init();
        }

        operand(int immediate)
        {
            init();

            init_base(0, immediate, operand_size::size_null, operand_type::immediate);
        }

        bool is_deactivated_from(int index)
        {
            for (; index < base_count; ++index)
            {
                if (!bases[index].is_active)
                    continue;

                return false;
            }

            return true;
        }

        bool is_basic()
        {
            if (
                    _extend_type  != -1 ||
                    _shift_type   != -1
                )
                return false;

            if (!is_deactivated_from(1))
                return false;

            return true;
        }

        bool is_immediate(base_operand operand)
        {
            return operand.type == rarma_context::immediate;
        }

        bool is_gp_or_zr(base_operand operand)
        {
            return operand.type == rarma_context::_gp || operand.type == rarma_context::_zr;
        }

        bool is_gp_or_sp(base_operand operand)
        {
            return operand.type == rarma_context::_gp || operand.type == rarma_context::_sp;
        }

        bool is_sp(base_operand operand)
        {
            return operand.type == rarma_context::_sp;
        }

        bool is_extended_shifted_immediate()
        {
            if (_shift_type != lsl)
                return false;

            if (_extend_type == -1)
                return false;

            base_operand base = bases[0];
            base_operand shift = bases[1];

            if (!is_deactivated_from(2))
                return false;

            if (!is_gp_or_zr(base))
                return false;

            if (!is_immediate(shift))
                return false;

            return true;
        }

        bool is_shifted_immediate()
        {
            if (_shift_type == -1)
                return false;

            if (_extend_type != -1)
                return false;

            base_operand base = bases[0];
            base_operand shift = bases[1];

            if (!is_deactivated_from(2))
                return false;

            if (!is_gp_or_zr(base))
                return false;

            if (!is_immediate(shift))
                return false;

            return true;
        }

        bool is_basic_register()
        {
            if (!is_basic())
                return false;

            switch (bases[0].type)
            {
                case _zr:
                case _sp:
                case _gp:
                case vec:
                {
                    return true;
                };
            }

            return false;
        }

        bool is_base_immediate()
        {
            if (!is_basic())
                return false;

            return bases[0].type == operand_type::immediate;
        }

        static operand shifted_operand(operand base, operand shift, shift_option type)
        {
            assert(base.is_basic());
            assert(shift.is_basic());

            operand result;

            result.bases[0] = base.bases[0];
            result.bases[1] = shift.bases[0];

            result._shift_type = type;

            return result;
        }

        static operand extended_operand(operand base, extend_type extend, operand shift)
        {
            assert_is_basic_register(base);
            assert_is_basic_immediate(shift);    

            operand result;

            result.bases[0] = base.bases[0];
            result.bases[1] = shift.bases[0];
            result._shift_type = lsl;
            result._extend_type = extend;

            return result;
        }

        operand operator << (operand other)
        {
            return shifted_operand(*this, other, shift_option::lsl);
        }

        operand operator >> (operand other)
        {
            return shifted_operand(*this, other, shift_option::lsr);
        }

        static operand asr(operand base, operand other)
        {
            return shifted_operand(base, other, shift_option::asr);
        }

        static operand ror(operand base, operand other)
        {
            return shifted_operand(base, other, shift_option::ror);
        }
    };

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

    static uint64_t get_code_size(rarma_context* context)
    {
        return context->memory_location;
    }

    static uint32_t write_instruction(rarma_context* context, uint32_t instruction)
    {
        *(uint32_t*)((uint64_t)context->memory_block + context->memory_location) = instruction;
        
        context->memory_location += 4;

        return instruction;
    }

    static operand reg(int index, operand_size size)
    {
        operand result;

        assert(index >= 0 && index < 31);

        result.init_base(0, index, size, operand_type::_gp);

        return result;
    }

    static operand sp(operand_size size)
    {
        operand result;

        result.init_base(0, 31, size, operand_type::_sp);

        return result;
    }

    static operand zr(operand_size size)
    {
        operand result;

        result.init_base(0, 31, size, operand_type::_zr);

        return result;
    }

    static bool operand_is_sp(operand test)
    {
        return test.bases[0].type == _sp;
    }

    static void assert_is_gp_or_sp(operand test)
    {
        int register_index = test.bases[0].data;
        int type = test.bases[0].type;

        if (register_index != 31)
        {
            assert(type == _gp);
        }
        else
        {
            assert(type == _sp);
        }
    }

    static void assert_is_basic_register(operand test, int size = -1)
    {
        assert(test.is_basic_register());

        if (size != -1)
        {
            assert(test.bases[0].size == size);
        }
    }

    static void assert_is_basic_immediate(operand test)
    {
        assert(test.is_base_immediate());
    }

    static void assert_is_gp_or_zr(operand test)
    {
        int register_index = test.bases[0].data;
        int type = test.bases[0].type;

        if (register_index != 31)
        {
            assert(type == _gp);
        }
        else
        {
            assert(type == _zr);
        }
    }

    static void assert_is_gp_or_zr(std::vector<operand> test)
    {
        for (int i = 0; i < test.size(); ++i)
        {
            assert_is_gp_or_zr(test[i]);
        }
    }

    static void assert_x_or_w(operand test)
    {
        base_operand base = test.bases[0];

        assert(base.size == operand_size::size_int32 || base.size == operand_size::size_int64);
    }

    static void assert_x(operand test)
    {
        base_operand base = test.bases[0];

        assert(base.size == operand_size::size_int64);
    }

    static void assert_w(operand test)
    {
        base_operand base = test.bases[0];

        assert(base.size == operand_size::size_int32);
    }

    static bool operand_same_size(operand left, operand right)
    {
        return left.bases[0].size == right.bases[0].size;
    }

    static void assert_same_size(operand left, operand right)
    {
        assert(operand_same_size(left, right));
    }

    static void assert_same_size(std::vector<operand> operands)
    {
        operand first = operands[0];

        for (int i = 1; i < operands.size(); ++i)
        {
            assert_same_size(first, operands[i]);
        }
    }

    static uint32_t add_subtract_shifted(rarma_context* context, int sf, int op, int S, int rd, int rn, int rm, shift_option shift, int imm6)
    {
        uint32_t result = 0b01011 << 24;
        
        assert_is_register(rd);
        assert_is_register(rn);
        assert_is_register(rm);
        assert_is_bit(S);
        assert_is_bit(op);
        assert_is_bit(sf);

        assert_in_mask(imm6, 6);
        assert_in_mask(shift, 2);

        result |= rd    << 0;
        result |= rn    << 5;
        result |= rm    << 16;
        result |= imm6  << 10;
        result |= shift << 22;
        result |= S     << 29;
        result |= op    << 30;
        result |= sf    << 31;

        return write_instruction(context, result);
    }

    static uint32_t add_subtract_extended(rarma_context* context, int sf, int op, int S, int rd, int rn, int rm, extend_type option, int imm3)
    {
        uint32_t result = 0b01011001 << 21;

        assert_is_register(rd);
        assert_is_register(rn);
        assert_is_register(rm);
        assert_is_bit(S);
        assert_is_bit(op);
        assert_is_bit(sf);

        assert_in_mask(option, 3);
        assert_in_mask(imm3, 3);

        result |= rd        << 0;
        result |= rn        << 5;
        result |= rm        << 16;
        result |= option    << 13;
        result |= imm3      << 10;
        result |= S         << 29;
        result |= op        << 30;
        result |= sf        << 31;

        return write_instruction(context, result);
    }

    static int32_t add_subtract_imm12(rarma_context* context, int sf, int op, int S, int sh, int rd, int rn, int imm12)
    {
        uint32_t result = 0b100010 << 23;

        if (imm12 == 0 && sh == 1)
        {
            sh = 0;
        }

        assert_in_mask(imm12, 12);
        assert_is_register(rd);
        assert_is_register(rn);

        assert_is_bit(sh);
        assert_is_bit(S);
        assert_is_bit(op);
        assert_is_bit(sf);

        result |= rd    << 0;
        result |= rn    << 5;
        result |= imm12 << 10;
        result |= sh    << 22;
        result |= S     << 29;
        result |= op    << 30;
        result |= sf    << 31;

        return write_instruction(context, result);
    }
    
    static uint32_t add_subtract(rarma_context* context, bool is_add, bool set_flags, operand d, operand n, operand m)
    {
        if (!(d.is_basic_register() && n.is_basic_register()))
            error();

        int sf = d.bases[0].size == size_int64;
        int op = !is_add;
        int S = set_flags;

        int rd = d.bases[0].data;
        int rn = n.bases[0].data;

        if (m.is_base_immediate())
        {
            if (set_flags)
            {
                assert_is_gp_or_zr(d);
            }
            else
            {
                assert_is_gp_or_sp(d);
            }

            assert_is_gp_or_sp(n);

            assert_x_or_w(d);
            assert_same_size(d, n);

            int imm12 = m.bases[0].data;
            int sh = 0;

            if (outside_of_bits(imm12, 12))
            {
                sh = 1;
                
                assert_masked_zero(imm12, 12);
                imm12 >>= 12;
            }

            if (outside_of_bits(imm12, 12))
            {
                error();
            }

            return add_subtract_imm12(context, sf, op, S, sh, rd, rn, imm12);
        }

        assert_is_gp_or_zr(m);

        int rm = m.bases[0].data;

        bool use_extended = false;

        if (
            m.is_extended_shifted_immediate() || 
            !operand_same_size(n, m) || 
            operand_is_sp(d) || 
            operand_is_sp(n)
        )
        {
            use_extended = true;
        }

        if (use_extended)
        {
            if (set_flags)
            {
                assert_is_gp_or_zr(d);
            }
            else
            {
                assert_is_gp_or_sp(d);
            }

            assert_is_gp_or_sp(n);

            assert_x_or_w(d);
            assert_x_or_w(m);
            assert_same_size({d, n});
            
            int option;
            int shift_ammount;

            if (m.is_basic_register())
            {
                assert_same_size(n, m);

                option = uxtw + sf;
            }
            else if (m.is_shifted_immediate())
            {
                assert(m._shift_type == lsl);
                
                shift_ammount = m.bases[1].data;
                option = uxtw + sf;
            }
            else if (m.is_extended_shifted_immediate())
            {
                assert(m._shift_type == lsl);

                shift_ammount = m.bases[1].data;
                option = m._extend_type;
                
                switch (option)
                {
                    case uxtx:
                    case sxtx:
                    {
                        assert_x(m);
                    }; break;

                    default:
                    {
                        assert_w(m);
                    }; break;
                }
            }
            else
            {
                error();
            }

            assert_in_mask(shift_ammount, 3);

            return add_subtract_extended(context, sf, op, S, rd, rn, rm, (extend_type)option, shift_ammount);
        }

        assert_same_size({d, n, m});
        assert_is_gp_or_zr({d, n, m});

        shift_option shift = (shift_option)m._shift_type;
        int imm6 = m.bases[1].data;

        if ((int)shift == -1)
        {
            shift = shift_option::lsl;
            assert(imm6 == 0);
        }

        return add_subtract_shifted(context, sf, op, S, rd, rn, rm, shift, imm6);
    }

    static uint32_t add(rarma_context* context, operand d, operand n, operand m)
    {
        return add_subtract(context, true, false, d, n, m);
    }

    static uint32_t sub(rarma_context* context, operand d, operand n, operand m)
    {
        return add_subtract(context, false, false, d, n, m);
    }

    static uint32_t adds(rarma_context* context, operand d, operand n, operand m)
    {
        return add_subtract(context, true, true, d, n, m);
    }

    static uint32_t subs(rarma_context* context, operand d, operand n, operand m)
    {
        return add_subtract(context, false, true, d, n, m);
    }

    static uint32_t ret(rarma_context* context, operand n)
    {
        assert_is_basic_register(n, size_int64);

        uint32_t result = (0b1101011001011111000000 << 10) | (n.bases[0].data << 5);

        return write_instruction(context, result);
    }

    static std::string get_debug_code(rarma_context* context)
    {
        int instruction_count = get_code_size(context) / 4;
        void* code = context->memory_block;

        std::stringstream stream;

        for (int i = 0; i < instruction_count; ++i)
        {
            int instruction = *((int*)code + i);

            for (int b = 0; b < 4; ++b)
            {
                stream << std::hex << std::setw(2) << std::setfill('0') << ((instruction >> (b * 8)) & 255) << " ";
            }

            stream << std::endl;
        }
        
        return stream.str();
    }
};

#endif