#include <string>

#include "debugging.h"
#include "ir/checks.h"
#include "tools/bit_tools.h"
#include "x86_assembler.h"
#include "xbyak/xbyak.h"

#define ONE_MB 1ULL * 1024 * 1024

#define RAX 0
#define RCX 1
#define RDX 2
#define RSP 4

// Strings introduce memory allocations,
// and memory allocations are EVIL.

// TODO: using strings is the most elegant
// solution, but i do not want to use them
// forever.
static std::string create_label(ir_operand value) {
    assert(ir_operand::is_constant(&value));

    return "l_" + std::to_string(value.value);
}

template <typename T>
static T create_operand(ir_operand value) {
    assert(ir_operand::is_register(&value));

    int reg = value.value;

    return T(reg);
}

template <typename T>
static T convert_operand(Xbyak::Operand value) {
    return T(value.getIdx());
}

static Xbyak::Operand create_operand(ir_operand value) {
    assert(ir_operand::is_register(&value));

    switch (ir_operand::get_raw_size(&value)) {
    case int8:
        return Xbyak::Reg8(value.value);
    case int16:
        return Xbyak::Reg16(value.value);
    case int32:
        return Xbyak::Reg32(value.value);
    case int64:
        return Xbyak::Reg64(value.value);
    case int128:
        return Xbyak::Xmm(value.value);
    }

    throw_error();
}

static void assert_is_x86_register(ir_operand working_operand, int register_index) {
    assert(working_operand.value == register_index);
}

static void assert_valid_mul_div_operation(ir_operation* operation) {
    assert_operand_count(operation, 2, 2);

    assert_is_x86_register(operation->destinations[0], RDX);
    assert_is_x86_register(operation->destinations[1], RAX);

    assert_is_x86_register(operation->sources[0], RAX);
    assert_is_register(operation->sources[1]);
}

static void assert_valid_c_operation(ir_operation* operation) {
    assert_operand_count(operation, 1, 1);

    assert_is_x86_register(operation->destinations[0], RDX);
    assert_is_x86_register(operation->sources[0], RAX);
}

static void assert_valid_unary_operation(ir_operation* operation) {
    assert_operand_count(operation, 1, 1);

    assert_all_registers(operation);
    assert_same_registers(operation->destinations[0], operation->sources[0]);
}

static void assert_valid_binary_float_operation(ir_operation* operation) {
    assert_operand_count(operation, 1, 2);

    assert_same_registers(operation->destinations[0], operation->sources[0]);
    assert_same_size({operation->destinations[0], operation->sources[0], operation->sources[1]});
    assert(ir_operand::is_vector(&operation->sources[0]));
}

static void assert_valid_binary_float_comparison_operation(ir_operation* operation) {
    assert_operand_count(operation, 1, 3);

    assert_same_registers(operation->destinations[0], operation->sources[0]);
    assert_same_size({operation->destinations[0], operation->sources[0], operation->sources[1]});
    assert(ir_operand::is_vector(&operation->sources[0]));

    assert_is_constant(operation->sources[2]);
}

void assemble_x86_64_code(void** result_code, uint64_t* result_code_size, ir_operation_block* source_ir) {
    arena_allocator* allocator = source_ir->allocator;

    int buffer_size = ONE_MB * 20;

    *result_code = arena_allocator::allocate_recursive(allocator, buffer_size);
    Xbyak::CodeGenerator c(buffer_size, *result_code);

    c.setDefaultJmpNEAR(true);

    for (auto i = source_ir->operations->first; i != source_ir->operations->last; i = i->next) {
        ir_operation working_operation = i->data;
        ir_instructions instruction = (ir_instructions)working_operation.instruction;

        if (c.getSize() > buffer_size - 1024) {
            throw_error();
        }

        switch (instruction) {
        case ir_negate:
            c.neg(create_operand(working_operation.destinations[0]));
            assert_valid_unary_operation(&working_operation);
            break;
        case ir_incrament:
            c.inc(create_operand(working_operation.destinations[0]));
            assert_valid_unary_operation(&working_operation);
            break;
        case ir_decrament:
            c.dec(create_operand(working_operation.destinations[0]));
            assert_valid_unary_operation(&working_operation);
            break;
        case ir_bitwise_not:
            c.not_(create_operand(working_operation.destinations[0]));
            assert_valid_unary_operation(&working_operation);
            break;

        case x86_cwd:
            c.cwd();
            assert_valid_c_operation(&working_operation);
            break;
        case x86_cqo:
            c.cqo();
            assert_valid_c_operation(&working_operation);
            break;
        case x86_cdq:
            c.cdq();
            assert_valid_c_operation(&working_operation);
            break;

        case ir_divide_signed:
            c.idiv(create_operand(working_operation.sources[1]));
            assert_valid_mul_div_operation(&working_operation);
            break;
        case ir_divide_unsigned:
            c.div(create_operand(working_operation.sources[1]));
            assert_valid_mul_div_operation(&working_operation);
            break;
        case ir_multiply_hi_signed:
            c.imul(create_operand(working_operation.sources[1]));
            assert_valid_mul_div_operation(&working_operation);
            break;
        case ir_multiply_hi_unsigned:
            c.mul(create_operand(working_operation.sources[1]));
            assert_valid_mul_div_operation(&working_operation);
            break;

        case ir_logical_not:
            c.cmp(create_operand(working_operation.destinations[0]), 0);
            c.sete(create_operand<Xbyak::Reg8>(working_operation.destinations[0]));
            c.and_(create_operand<Xbyak::Reg64>(working_operation.destinations[0]), 1);

            assert_valid_unary_operation(&working_operation);
            break;

        case ir_external_call:
        case ir_internal_call: {
            auto function_to_call = create_operand(working_operation.sources[0]);
            int caller_size = 120;

            c.sub(c.rsp, caller_size);

            if (instruction == ir_internal_call) {
                c.sub(c.rsp, 8);
                c.push(create_operand(working_operation.sources[1]));
            }

            c.call(function_to_call);

            if (instruction == ir_internal_call) {
                c.add(c.rsp, 16);
            }

            c.add(c.rsp, caller_size);

        }; break;

        case x86_cmpxchg: {
            assert_operand_count(&working_operation, 1, 3);

            assert_is_register(working_operation.destinations[0]);

            assert_is_register(working_operation.sources[0]);
            assert_is_x86_register(working_operation.sources[1], 0);
            assert_is_register(working_operation.sources[2]);

            Xbyak::Reg64 _address = create_operand<Xbyak::Reg64>(working_operation.sources[0]);

            int size = ir_operand::get_raw_size(&working_operation.destinations[0]);

            c.lock();

            switch (size) {
            case int8:
                c.cmpxchg(c.ptr[_address], create_operand<Xbyak::Reg8>(working_operation.sources[2]));
                break;
            case int16:
                c.cmpxchg(c.ptr[_address], create_operand<Xbyak::Reg16>(working_operation.sources[2]));
                break;
            case int32:
                c.cmpxchg(c.ptr[_address], create_operand<Xbyak::Reg32>(working_operation.sources[2]));
                break;
            case int64:
                c.cmpxchg(c.ptr[_address], create_operand<Xbyak::Reg64>(working_operation.sources[2]));
                break;
            default:
                std::cout << "BAD SIZE" << std::endl;

                throw_error();
            }

            c.setz(create_operand<Xbyak::Reg8>(working_operation.destinations[0]));
            c.and_(create_operand<Xbyak::Reg64>(working_operation.destinations[0]), 1);

        }; break;

        case ir_mark_label: {
            if (working_operation.sources.count == 0) {
                break;
            }

            assert_operand_count(&working_operation, 0, 1);
            assert_is_constant(working_operation.sources[0]);

            std::string label = create_label(working_operation.sources[0]);

            c.L(label);

            c.nop();

        }; break;

        case ir_jump_if_equal:
        case ir_jump_if_not_equal:

        case ir_jump_if_greater_equal_signed:
        case ir_jump_if_greater_equal_unsigned:
        case ir_jump_if_greater_signed:
        case ir_jump_if_greater_unsigned:
        case ir_jump_if_less_equal_signed:
        case ir_jump_if_less_equal_unsigned:
        case ir_jump_if_less_signed:
        case ir_jump_if_less_unsigned: {
            ir_operand label_operand = working_operation.sources[0];
            ir_operand left = working_operation.sources[1];
            ir_operand right = working_operation.sources[2];

            std::string xbyak_label = create_label(label_operand);
            Xbyak::Operand xbyak_condition_l = create_operand(left);

            if (ir_operand::is_constant(&right)) {
                c.cmp(xbyak_condition_l, right.value);
            } else {
                Xbyak::Operand xbyak_condition_r = create_operand(right);

                c.cmp(xbyak_condition_l, xbyak_condition_r);
            }

            switch (instruction) {
            case ir_jump_if_equal:
                c.je(xbyak_label);
                break;
            case ir_jump_if_not_equal:
                c.jne(xbyak_label);
                break;

            case ir_jump_if_greater_equal_signed:
                c.jge(xbyak_label);
                break;
            case ir_jump_if_greater_equal_unsigned:
                c.jae(xbyak_label);
                break;
            case ir_jump_if_greater_signed:
                c.jg(xbyak_label);
                break;
            case ir_jump_if_greater_unsigned:
                c.ja(xbyak_label);
                break;
            case ir_jump_if_less_equal_signed:
                c.jle(xbyak_label);
                break;
            case ir_jump_if_less_equal_unsigned:
                c.jbe(xbyak_label);
                break;
            case ir_jump_if_less_signed:
                c.jl(xbyak_label);
                break;
            case ir_jump_if_less_unsigned:
                c.jb(xbyak_label);
                break;
            }

        }; break;

        case ir_jump_if: {
            assert_operand_count(&working_operation, 0, 2);

            ir_operand label_operand = working_operation.sources[0];
            ir_operand condition = working_operation.sources[1];

            assert_is_constant(label_operand);

            std::string xbyak_label = create_label(label_operand);

            if (ir_operand::is_constant(&condition)) {
                if (condition.value) {
                    c.jmp(xbyak_label);
                }
            } else {
                Xbyak::Operand xbyak_condition = create_operand(condition);

                c.cmp(xbyak_condition, 0);
                c.jne(xbyak_label);
            }

        }; break;

        case ir_shift_left:
        case ir_shift_right_signed:
        case ir_shift_right_unsigned:
        case ir_rotate_right: {
            ir_operand* destinations = working_operation.destinations.data;
            ir_operand* sources = working_operation.sources.data;

            assert_operand_count(&working_operation, 1, 2);

            assert_is_register(destinations[0]);
            assert_is_register(sources[0]);

            assert(ir_operand::are_equal(destinations[0], sources[0]));

            Xbyak::Operand dn = create_operand(destinations[0]);

            if (ir_operand::is_register(&sources[1])) {
                Xbyak::Reg8 m = create_operand<Xbyak::Reg8>(sources[1]);
                assert_is_x86_register(sources[1], RCX);

                switch (instruction) {
                case ir_shift_left:
                    c.shl(dn, m);
                    break;
                case ir_shift_right_signed:
                    c.sar(dn, m);
                    break;
                case ir_shift_right_unsigned:
                    c.shr(dn, m);
                    break;
                case ir_rotate_right:
                    c.ror(dn, m);
                    break;
                default:
                    throw_error();
                }
            } else {
                int shift = sources[1].value;

                switch (instruction) {
                case ir_shift_left:
                    c.shl(dn, shift);
                    break;
                case ir_shift_right_signed:
                    c.sar(dn, shift);
                    break;
                case ir_shift_right_unsigned:
                    c.shr(dn, shift);
                    break;
                case ir_rotate_right:
                    c.ror(dn, shift);
                    break;
                default:
                    throw_error();
                }
            }
        }; break;

        case ir_double_shift_right: {
            ir_operand* destinations = working_operation.destinations.data;
            ir_operand* sources = working_operation.sources.data;

            assert_operand_count(&working_operation, 1, 3);
            assert_same_registers(destinations[0], sources[0]);
            assert_is_register(sources[1]);
            assert_is_x86_register(sources[2], RCX);

            assert(destinations[0].value == sources[0].value);
            assert(destinations[0].meta_data == sources[0].meta_data);

            Xbyak::Operand dn = create_operand(destinations[0]);
            Xbyak::Operand m = create_operand(sources[1]);

            Xbyak::Reg8 cl = create_operand<Xbyak::Reg8>(sources[2]);

            c.shrd(dn, m.getReg(), cl);
        }; break;

        case ir_assert_true:
        case ir_assert_false: {
            ir_operand* destinations = working_operation.destinations.data;
            ir_operand* sources = working_operation.sources.data;

            assert_operand_count(&working_operation, 0, 1);

            Xbyak::Operand check = create_operand(sources[0]);

            c.cmp(check, 0);

            Xbyak::Label end;

            if (instruction == ir_assert_true) {
                c.jne(end);
            } else {
                c.je(end);
            }

            c.mov(c.rax, 0);
            c.mov(c.rax, c.ptr[c.rax]);

            c.L(end);
        }; break;

        case ir_conditional_select: {
            ir_operand* destinations = working_operation.destinations.data;
            ir_operand* sources = working_operation.sources.data;

            assert_operand_count(&working_operation, 1, 3);
            assert_is_register(destinations[0]);

            Xbyak::Operand condition = create_operand(sources[0]);

            c.cmp(condition, 0);

            switch (destinations[0].meta_data) {
            case int16:
                c.cmovne(Xbyak::Reg16(destinations[0].value), Xbyak::Reg16(sources[1].value));
                c.cmove(Xbyak::Reg16(destinations[0].value), Xbyak::Reg16(sources[2].value));
                break;
            case int32:
                c.cmovne(Xbyak::Reg32(destinations[0].value), Xbyak::Reg32(sources[1].value));
                c.cmove(Xbyak::Reg32(destinations[0].value), Xbyak::Reg32(sources[2].value));
                break;
            case int64:
                c.cmovne(Xbyak::Reg64(destinations[0].value), Xbyak::Reg64(sources[1].value));
                c.cmove(Xbyak::Reg64(destinations[0].value), Xbyak::Reg64(sources[2].value));
                break;
            default:
                throw_error();
            }
        }; break;

        case ir_compare_equal:
        case ir_compare_greater_equal_signed:
        case ir_compare_greater_equal_unsigned:
        case ir_compare_greater_signed:
        case ir_compare_greater_unsigned:
        case ir_compare_less_equal_signed:
        case ir_compare_less_equal_unsigned:
        case ir_compare_less_signed:
        case ir_compare_less_unsigned:
        case ir_compare_not_equal: {
            ir_operand* destinations = working_operation.destinations.data;
            ir_operand* sources = working_operation.sources.data;

            assert_is_register(destinations[0]);
            assert_is_register(sources[0]);

            assert_same_size({destinations[0], sources[0], sources[1]});

            if (ir_operand::is_constant(&sources[1])) {
                uint64_t constant = ir_operand::get_masked_constant(&sources[1]);

                uint64_t working_size = ir_operand::get_raw_size(&working_operation.sources[1]);

                if (working_size <= int32) {
                    assert(constant <= create_int_max(working_size));
                } else {
                    assert(constant <= create_int_max(int32));
                }

                c.cmp(create_operand(sources[0]), constant);
            } else {
                c.cmp(create_operand(sources[0]), create_operand(sources[1]));
            }

            switch (instruction) {
            case ir_compare_equal:
                c.sete(create_operand<Xbyak::Reg8>(destinations[0]));
                break;
            case ir_compare_greater_equal_signed:
                c.setge(create_operand<Xbyak::Reg8>(destinations[0]));
                break;
            case ir_compare_greater_equal_unsigned:
                c.setae(create_operand<Xbyak::Reg8>(destinations[0]));
                break;
            case ir_compare_greater_signed:
                c.setg(create_operand<Xbyak::Reg8>(destinations[0]));
                break;
            case ir_compare_greater_unsigned:
                c.seta(create_operand<Xbyak::Reg8>(destinations[0]));
                break;
            case ir_compare_less_equal_signed:
                c.setle(create_operand<Xbyak::Reg8>(destinations[0]));
                break;
            case ir_compare_less_equal_unsigned:
                c.setbe(create_operand<Xbyak::Reg8>(destinations[0]));
                break;
            case ir_compare_less_signed:
                c.setl(create_operand<Xbyak::Reg8>(destinations[0]));
                break;
            case ir_compare_less_unsigned:
                c.setb(create_operand<Xbyak::Reg8>(destinations[0]));
                break;
            case ir_compare_not_equal:
                c.setne(create_operand<Xbyak::Reg8>(destinations[0]));
                break;
            }

            ir_operand d = destinations[0];
            d.meta_data = int64;

            c.and_(create_operand(d), 1);

        }; break;

        case ir_add:
        case ir_subtract:
        case ir_bitwise_and:
        case ir_bitwise_or:
        case ir_bitwise_exclusive_or:
        case ir_multiply: {
            ir_operand* destinations = working_operation.destinations.data;
            ir_operand* sources = working_operation.sources.data;

            assert_is_register(destinations[0]);
            assert_same_registers(destinations[0], sources[0]);

            assert(ir_operand::is_register(&destinations[0]));
            assert(ir_operand::is_register(&sources[0]));

            Xbyak::Operand dn = create_operand(destinations[0]);

            if (ir_operand::is_constant(&sources[1])) {
                uint64_t imm = sources[1].value;

                if (imm >= INT32_MAX) {
                    throw_error();
                }

                switch (instruction) {
                case ir_add:
                    c.add(dn, imm);
                    break;
                case ir_subtract:
                    c.sub(dn, imm);
                    break;
                case ir_bitwise_and:
                    c.and_(dn, imm);
                    break;
                case ir_bitwise_or:
                    c.or_(dn, imm);
                    break;
                case ir_bitwise_exclusive_or:
                    c.xor_(dn, imm);
                    break;
                default:
                    throw_error();
                }
            } else {
                assert(destinations[0].value == sources[0].value);
                assert(destinations[0].meta_data == sources[0].meta_data);
                assert(destinations[0].meta_data == sources[1].meta_data);

                Xbyak::Operand m = create_operand(sources[1]);

                switch (instruction) {
                case ir_add:
                    c.add(dn, m);
                    break;
                case ir_subtract:
                    c.sub(dn, m);
                    break;
                case ir_bitwise_and:
                    c.and_(dn, m);
                    break;
                case ir_bitwise_or:
                    c.or_(dn, m);
                    break;
                case ir_bitwise_exclusive_or:
                    c.xor_(dn, m);
                    break;
                case ir_multiply:
                    c.imul(Xbyak::Reg(dn.getIdx(), dn.getKind()), m);
                    break;
                default:
                    throw_error();
                }
            }
        }; break;

        case ir_no_operation: /*NOP*/
            break;

        case ir_open_context: {
            ir_operand size = working_operation.sources[0];

            assert(ir_operand::is_constant(&size));

            uint64_t offset = size.value;

            c.sub(c.rsp, offset);

        }; break;

        case ir_close_and_return:
        case ir_table_jump: {
            ir_operand value = working_operation.sources[0];
            ir_operand context_size = working_operation.sources[1];

            uint64_t offset = context_size.value;

            assert(ir_operand::is_register(&value));
            assert(ir_operand::is_constant(&context_size));

            c.add(c.rsp, offset);

            if (value.value != c.rax.getIdx()) {
                c.mov(c.rax, create_operand<Xbyak::Reg64>(value));
            }

            if (instruction == ir_close_and_return) {
                c.ret();
            } else {
                c.jmp(c.rax);
            }

        }; break;

        case ir_load: {
            ir_operand destination = working_operation.destinations[0];
            ir_operand base_address = working_operation.sources[0];

            Xbyak::Operand result_operand = create_operand(destination);
            Xbyak::Address address_operand = Xbyak::Address(0);

            if (working_operation.sources.count == 1) {
                address_operand = c.ptr[create_operand<Xbyak::Reg64>(base_address)];
            } else {
                ir_operand address_offset = working_operation.sources[1];

                if (ir_operand::is_constant(&address_offset)) {
                    address_operand = c.ptr[create_operand<Xbyak::Reg64>(base_address) + address_offset.value];
                } else {
                    address_operand = c.ptr[create_operand<Xbyak::Reg64>(base_address) + create_operand<Xbyak::Reg64>(address_offset)];
                }
            }

            if (ir_operand::is_vector(&destination)) {
                c.movups(convert_operand<Xbyak::Xmm>(result_operand), address_operand);
            } else {
                c.mov(result_operand, address_operand);
            }
        }; break;

        case ir_store: {
            ir_operand base_address = working_operation.sources[0];
            ir_operand to_store;

            Xbyak::Address address_operand = Xbyak::Address(0);

            if (working_operation.sources.count == 3) {
                ir_operand offset_address = working_operation.sources[1];

                if (ir_operand::is_constant(&offset_address)) {
                    address_operand = c.ptr[create_operand<Xbyak::Reg64>(working_operation.sources[0]) + offset_address.value];
                } else {
                    address_operand = c.ptr[create_operand<Xbyak::Reg64>(working_operation.sources[0]) + create_operand<Xbyak::Reg64>(working_operation.sources[1])];
                }

                to_store = working_operation.sources[2];
            } else if (working_operation.sources.count == 2) {
                address_operand = c.ptr[create_operand<Xbyak::Reg64>(working_operation.sources[0])];

                to_store = working_operation.sources[1];
            } else {
                throw_error();
            }

            if (ir_operand::is_vector(&to_store)) {
                c.movups(address_operand, create_operand<Xbyak::Xmm>(to_store));
            } else {
                c.mov(address_operand, create_operand(to_store));
            }
        }; break;

        case ir_move: {
            ir_operand destination = working_operation.destinations[0];
            ir_operand source = working_operation.sources[0];

            assert(ir_operand::get_raw_size(&destination) == ir_operand::get_raw_size(&source));

            if (ir_operand::is_vector(&destination)) {
                c.movaps(create_operand<Xbyak::Xmm>(destination), create_operand<Xbyak::Xmm>(source));
            } else {
                if (ir_operand::is_constant(&source)) {
                    uint64_t working = ir_operand::get_masked_constant(&source);

                    c.mov(create_operand<Xbyak::Reg64>(destination), working);
                } else {
                    c.mov(create_operand(destination), create_operand(source));
                }
            }
        }; break;

        case ir_sign_extend: {
            ir_operand destination = working_operation.destinations[0];
            ir_operand source = working_operation.sources[0];

            if (ir_operand::get_raw_size(&source) == ir_operand_meta::int32) {
                c.movsxd(create_operand<Xbyak::Reg64>(destination), create_operand(source));
            } else {
                c.movsx(create_operand(destination).getReg(), create_operand(source));
            }
        }; break;

        case ir_vector_zero: {
            ir_operand destination = working_operation.destinations[0];

            assert_is_size(destination, int128);

            auto d = create_operand<Xbyak::Xmm>(destination);

            c.xorps(d, d);
        }; break;

        case ir_vector_one: {
            ir_operand destination = working_operation.destinations[0];

            assert_is_size(destination, int128);

            auto d = create_operand<Xbyak::Xmm>(destination);

            c.pcmpeqd(d, d);
        }; break;

        case ir_vector_extract: {
            ir_operand destination = working_operation.destinations[0];

            ir_operand source = working_operation.sources[0];
            ir_operand index = working_operation.sources[1];
            ir_operand size = working_operation.sources[2];

            assert(!ir_operand::is_vector(&destination));

            assert_is_size(source, int128);
            assert_is_constant(index);
            assert_is_constant(size);

            auto src = create_operand<Xbyak::Xmm>(source);

            switch (size.value) {
            case 8:
                c.pextrb(create_operand<Xbyak::Reg32>(destination), src, index.value);
                break;
            case 16:
                c.pextrw(create_operand<Xbyak::Reg32>(destination), src, index.value);
                break;
            case 32:
                c.pextrd(create_operand<Xbyak::Reg32>(destination), src, index.value);
                break;
            case 64:
                c.pextrq(create_operand<Xbyak::Reg64>(destination), src, index.value);
                break;
            default:
                throw_error();
            }

        }; break;

        case ir_vector_insert: {
            ir_operand destination = working_operation.destinations[0];

            ir_operand source = working_operation.sources[0];
            ir_operand value = working_operation.sources[1];
            ir_operand index = working_operation.sources[2];
            ir_operand size = working_operation.sources[3];

            assert_same_registers(destination, source);

            assert(!ir_operand::is_vector(&value));
            assert_is_constant(index);
            assert_is_constant(size);

            auto d = create_operand<Xbyak::Xmm>(destination);

            switch (size.value) {
            case 8:
                c.pinsrb(d, create_operand<Xbyak::Reg32>(value), index.value);
                break;
            case 16:
                c.pinsrw(d, create_operand<Xbyak::Reg32>(value), index.value);
                break;
            case 32:
                c.pinsrd(d, create_operand<Xbyak::Reg32>(value), index.value);
                break;
            case 64:
                c.pinsrq(d, create_operand<Xbyak::Reg64>(value), index.value);
                break;
            default:
                throw_error();
            }

        }; break;

        case x86_cvtss2si:
        case x86_cvtsd2si: {
            assert_operand_count(&working_operation, 1, 1);

            ir_operand d = working_operation.destinations[0];
            ir_operand s = working_operation.sources[0];

            assert_is_size(s, int128);
            assert_is_register(d);

            Xbyak::Reg des = (d.meta_data == int64 ? (Xbyak::Reg)create_operand<Xbyak::Reg64>(d) : (Xbyak::Reg)create_operand<Xbyak::Reg32>(d));
            auto src = create_operand<Xbyak::Xmm>(s);

            switch (instruction) {
            case x86_cvtss2si:
                c.cvtss2si(des, src);
                break;
            case x86_cvtsd2si:
                c.cvtsd2si(des, src);
                break;
            }

        }; break;

        case x86_roundss:
        case x86_roundsd: {
            assert_operand_count(&working_operation, 1, 2);

            ir_operand d = working_operation.destinations[0];
            ir_operand s = working_operation.sources[0];
            ir_operand control = working_operation.sources[1];

            assert_is_constant(control);

            assert(ir_operand::is_vector(&d));
            assert(ir_operand::is_vector(&s));

            switch (instruction) {
            case x86_roundss:
                c.roundss(create_operand<Xbyak::Xmm>(d), create_operand<Xbyak::Xmm>(s), control.value);
                break;
            case x86_roundsd:
                c.roundsd(create_operand<Xbyak::Xmm>(d), create_operand<Xbyak::Xmm>(s), control.value);
                break;
            }

        }; break;

        case x86_cvtsi2ss:
        case x86_cvtsi2sd: {
            assert_operand_count(&working_operation, 1, 1);

            ir_operand d = working_operation.destinations[0];
            ir_operand s = working_operation.sources[0];

            assert_is_size(d, int128);
            assert_is_register(s);

            auto des = create_operand<Xbyak::Xmm>(d);
            auto src = Xbyak::Reg(s.value, Xbyak::Operand::REG, 8 << s.meta_data);

            switch (instruction) {
            case x86_cvtsi2ss:
                c.cvtsi2ss(des, src);
                break;
            case x86_cvtsi2sd:
                c.cvtsi2sd(des, src);
                break;
            }

        }; break;

        case x86_movq_to_gp: {
            assert_operand_count(&working_operation, 1, 1);

            ir_operand d = working_operation.destinations[0];
            ir_operand s = working_operation.sources[0];

            assert(!ir_operand::is_vector(&d));
            assert_is_size(s, int128);

            c.movq(create_operand<Xbyak::Reg64>(d), create_operand<Xbyak::Xmm>(s));
        }; break;

        case x86_movq_to_vec: {
            assert_operand_count(&working_operation, 1, 1);

            ir_operand d = working_operation.destinations[0];
            ir_operand s = working_operation.sources[0];

            assert_is_size(d, int128);
            assert(!ir_operand::is_vector(&s));

            c.movq(create_operand<Xbyak::Xmm>(d), create_operand<Xbyak::Reg64>(s));
        }; break;

        case x86_lea: {
            assert_operand_count(&working_operation, 1, 2);

            ir_operand d = working_operation.destinations[0];
            ir_operand s0 = working_operation.sources[0];
            ir_operand s1 = working_operation.sources[1];

            if (ir_operand::is_constant(&s1)) {
                switch (d.meta_data) {
                case int32:
                    c.lea(create_operand<Xbyak::Reg32>(d), c.ptr[create_operand<Xbyak::Reg32>(s0) + s1.value]);
                    break;
                case int64:
                    c.lea(create_operand<Xbyak::Reg64>(d), c.ptr[create_operand<Xbyak::Reg64>(s0) + s1.value]);
                    break;

                default:
                    throw_error();
                }
            } else {
                switch (d.meta_data) {
                case int32:
                    c.lea(create_operand<Xbyak::Reg32>(d), c.ptr[create_operand<Xbyak::Reg32>(s0) + create_operand<Xbyak::Reg32>(s1)]);
                    break;
                case int64:
                    c.lea(create_operand<Xbyak::Reg64>(d), c.ptr[create_operand<Xbyak::Reg64>(s0) + create_operand<Xbyak::Reg64>(s1)]);
                    break;

                default:
                    throw_error();
                }
            }

        }; break;

        case x86_shufps:
        case x86_shufpd: {
            ir_operand d = working_operation.destinations[0];

            ir_operand s0 = working_operation.sources[0];
            ir_operand s1 = working_operation.sources[1];
            int index = working_operation.sources[2].value;

            assert_operand_count(&working_operation, 1, 3);

            assert(ir_operand::is_vector(&d));
            assert(ir_operand::is_vector(&s0));
            assert(ir_operand::is_vector(&s1));
            assert_is_constant(working_operation.sources[2]);

            assert_same_registers(d, s0);

            switch (instruction) {
            case x86_shufps:
                c.shufps(create_operand<Xbyak::Xmm>(d), create_operand<Xbyak::Xmm>(s1), index);
                break;
            case x86_shufpd:
                c.shufpd(create_operand<Xbyak::Xmm>(d), create_operand<Xbyak::Xmm>(s1), index);
                break;
            default:
                throw_error();
            }

        }; break;

        case x86_cmpss:
            assert_valid_binary_float_comparison_operation(&working_operation);
            c.cmpss(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]), working_operation.sources[2].value);
            break;
        case x86_cmpsd:
            assert_valid_binary_float_comparison_operation(&working_operation);
            c.cmpsd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]), working_operation.sources[2].value);
            break;
        case x86_cmpps:
            assert_valid_binary_float_comparison_operation(&working_operation);
            c.cmpps(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]), working_operation.sources[2].value);
            break;
        case x86_cmppd:
            assert_valid_binary_float_comparison_operation(&working_operation);
            c.cmppd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]), working_operation.sources[2].value);
            break;

        case x86_haddpd:
            assert_valid_binary_float_operation(&working_operation);
            c.haddpd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_haddps:
            assert_valid_binary_float_operation(&working_operation);
            c.haddps(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;

        case x86_paddb:
            assert_valid_binary_float_operation(&working_operation);
            c.paddb(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_paddw:
            assert_valid_binary_float_operation(&working_operation);
            c.paddw(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_paddd:
            assert_valid_binary_float_operation(&working_operation);
            c.paddd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_paddq:
            assert_valid_binary_float_operation(&working_operation);
            c.paddq(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;

        case x86_addpd:
            assert_valid_binary_float_operation(&working_operation);
            c.addpd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_addps:
            assert_valid_binary_float_operation(&working_operation);
            c.addps(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_addsd:
            assert_valid_binary_float_operation(&working_operation);
            c.addsd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_addss:
            assert_valid_binary_float_operation(&working_operation);
            c.addss(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_divpd:
            assert_valid_binary_float_operation(&working_operation);
            c.divpd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_divps:
            assert_valid_binary_float_operation(&working_operation);
            c.divps(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_divsd:
            assert_valid_binary_float_operation(&working_operation);
            c.divsd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_divss:
            assert_valid_binary_float_operation(&working_operation);
            c.divss(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_maxpd:
            assert_valid_binary_float_operation(&working_operation);
            c.maxpd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_maxps:
            assert_valid_binary_float_operation(&working_operation);
            c.maxps(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_maxsd:
            assert_valid_binary_float_operation(&working_operation);
            c.maxsd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_maxss:
            assert_valid_binary_float_operation(&working_operation);
            c.maxss(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_minpd:
            assert_valid_binary_float_operation(&working_operation);
            c.minpd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_minps:
            assert_valid_binary_float_operation(&working_operation);
            c.minps(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_minsd:
            assert_valid_binary_float_operation(&working_operation);
            c.minsd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_minss:
            assert_valid_binary_float_operation(&working_operation);
            c.minss(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_mulpd:
            assert_valid_binary_float_operation(&working_operation);
            c.mulpd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_mulps:
            assert_valid_binary_float_operation(&working_operation);
            c.mulps(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_mulsd:
            assert_valid_binary_float_operation(&working_operation);
            c.mulsd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_mulss:
            assert_valid_binary_float_operation(&working_operation);
            c.mulss(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_subpd:
            assert_valid_binary_float_operation(&working_operation);
            c.subpd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_subps:
            assert_valid_binary_float_operation(&working_operation);
            c.subps(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_subsd:
            assert_valid_binary_float_operation(&working_operation);
            c.subsd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_subss:
            assert_valid_binary_float_operation(&working_operation);
            c.subss(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;

        case x86_orps:
            assert_valid_binary_float_operation(&working_operation);
            c.orps(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_xorps:
            assert_valid_binary_float_operation(&working_operation);
            c.xorps(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_pand:
            assert_valid_binary_float_operation(&working_operation);
            c.pand(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;
        case x86_pandn:
            assert_valid_binary_float_operation(&working_operation);
            c.pandn(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[1]));
            break;

        case x86_cvtsd2ss:
            c.cvtsd2ss(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[0]));
            break;
        case x86_cvtss2sd:
            c.cvtss2sd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[0]));
            break;

        case x86_sqrtss:
            c.sqrtss(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[0]));
            break;
        case x86_sqrtsd:
            c.sqrtsd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[0]));
            break;
        case x86_sqrtps:
            c.sqrtps(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[0]));
            break;
        case x86_sqrtpd:
            c.sqrtpd(create_operand<Xbyak::Xmm>(working_operation.destinations[0]), create_operand<Xbyak::Xmm>(working_operation.sources[0]));
            break;

        case x86_popcnt: {
            ir_operand destination = working_operation.destinations[0];
            ir_operand source = working_operation.sources[0];

            assert_same_size({destination, source});

            switch (destination.meta_data) {
            case int16:
                c.popcnt(create_operand<Xbyak::Reg16>(destination), create_operand<Xbyak::Reg16>(source));
                break;
            case int32:
                c.popcnt(create_operand<Xbyak::Reg32>(destination), create_operand<Xbyak::Reg32>(source));
                break;
            case int64:
                c.popcnt(create_operand<Xbyak::Reg64>(destination), create_operand<Xbyak::Reg64>(source));
                break;
            default:
                throw_error();
            }
        }; break;

        case x86_lzcnt: {
            ir_operand destination = working_operation.destinations[0];
            ir_operand source = working_operation.sources[0];

            assert_same_size({destination, source});

            switch (destination.meta_data) {
            case int32:
                c.lzcnt(create_operand<Xbyak::Reg32>(destination), create_operand<Xbyak::Reg32>(source));
                break;
            case int64:
                c.lzcnt(create_operand<Xbyak::Reg64>(destination), create_operand<Xbyak::Reg64>(source));
                break;
            default:
                throw_error();
            }
        } break;

        case x86_add_flags:
        case x86_sub_flags: {
            ir_operand* destinations = working_operation.destinations.data;
            ir_operand* sources = working_operation.sources.data;

            assert_is_register(destinations[0]);
            assert_same_registers(destinations[0], sources[0]);

            assert(ir_operand::is_register(&destinations[0]));
            assert(ir_operand::is_register(&sources[0]));

            Xbyak::Operand dn = create_operand(destinations[0]);

            assert_operand_count(&working_operation, 5, 2);

            if (ir_operand::is_constant(&sources[1])) {
                uint64_t imm = sources[1].value;

                if (imm >= INT32_MAX) {
                    throw_error();
                }

                switch (instruction) {
                case x86_add_flags:
                    c.add(dn, imm);
                    break;
                case x86_sub_flags:
                    c.sub(dn, imm);
                    break;
                default:
                    throw_error();
                }
            } else {
                assert(destinations[0].value == sources[0].value);
                assert(destinations[0].meta_data == sources[0].meta_data);
                assert(destinations[0].meta_data == sources[1].meta_data);

                Xbyak::Operand m = create_operand(sources[1]);

                switch (instruction) {
                case x86_add_flags:
                    c.add(dn, m);
                    break;
                case x86_sub_flags:
                    c.sub(dn, m);
                    break;
                default:
                    throw_error();
                }
            }

            c.sets(create_operand<Xbyak::Reg8>(destinations[1]));
            c.sete(create_operand<Xbyak::Reg8>(destinations[2]));
            c.setc(create_operand<Xbyak::Reg8>(destinations[3]));
            c.seto(create_operand<Xbyak::Reg8>(destinations[4]));

            for (int i = 1; i < 5; ++i) {
                c.and_(create_operand<Xbyak::Reg64>(destinations[i]), 1);
            }
        }; break;

        case ir_floating_point_compare_equal:
        case ir_floating_point_compare_less:
        case ir_floating_point_compare_not_equal:
        case ir_floating_point_compare_greater:
        case ir_floating_point_compare_greater_equal:
        case ir_floating_point_compare_less_equal: {
            ir_operand destination = working_operation.destinations[0];
            ir_operand source_0 = working_operation.sources[0];
            ir_operand source_1 = working_operation.sources[1];

            assert(!ir_operand::is_vector(&destination));
            assert(ir_operand::is_vector(&source_0));
            assert(ir_operand::is_vector(&source_1));

            switch (ir_operand::get_raw_size(&destination)) {
            case int32: {
                c.comiss(create_operand<Xbyak::Xmm>(source_0), create_operand<Xbyak::Xmm>(source_1));
            }; break;

            case int64: {
                c.comisd(create_operand<Xbyak::Xmm>(source_0), create_operand<Xbyak::Xmm>(source_1));
            }; break;
            default:
                throw_error();
            }

            switch (instruction) {
            case ir_floating_point_compare_equal:
                c.sete(create_operand<Xbyak::Reg8>(destination));
                break;
            case ir_floating_point_compare_not_equal:
                c.setne(create_operand<Xbyak::Reg8>(destination));
                break;

            case ir_floating_point_compare_less:
                c.setb(create_operand<Xbyak::Reg8>(destination));
                break;
            case ir_floating_point_compare_less_equal:
                c.setbe(create_operand<Xbyak::Reg8>(destination));
                break;

            case ir_floating_point_compare_greater:
                c.seta(create_operand<Xbyak::Reg8>(destination));
                break;
            case ir_floating_point_compare_greater_equal:
                c.setae(create_operand<Xbyak::Reg8>(destination));
                break;
            default:
                throw_error();
            }

            c.and_(create_operand<Xbyak::Reg64>(destination), 1);
        }; break;

        default: {
            std::cout << "UNDEFINED X86 INSTRUCTION " << std::endl;

            throw_error();
        }; break;
        }
    }

    *result_code_size = c.getSize();
}

void assemble_x86_abi_caller_code(void* result_code, uint64_t* result_code_size, abi abi_information) {
    assert(abi_information.cpu == cpu_information::x86_64);

    uint64_t size = 1 * 1024;

    Xbyak::CodeGenerator c(size, result_code);

    for (int i = 0; i < 16; ++i) {
        c.push(Xbyak::Reg64(i));

        c.sub(c.rsp, 16);
        c.movups(c.ptr[c.rsp], Xbyak::Xmm(i));
    }

    switch (abi_information.os) {
    case _macos:
    case _linux: {
        c.push(c.rsi);
        c.call(c.rdi);
    }; break;

    case _windows: {
        c.push(c.rdx);
        c.call(c.rcx);
    }; break;
    }

    c.add(c.rsp, 8);

    for (int i = 15; i != -1; --i) {
        c.movups(Xbyak::Xmm(i), c.ptr[c.rsp]);
        c.add(c.rsp, 16);

        if (i == 0 || i == 4) {
            c.add(c.rsp, 8);
        } else {
            c.pop(Xbyak::Reg64(i));
        }
    }

    c.ret();

    *result_code_size = c.getSize();
}