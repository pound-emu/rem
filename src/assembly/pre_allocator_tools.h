#pragma once

#include "ir/ir.h"

static bool instruction_is_commutative(uint64_t instruction) {
    switch (instruction) {
    case ir_add:
    case ir_multiply:
    case ir_bitwise_and:
    case ir_bitwise_or:
    case ir_bitwise_exclusive_or:
    case x86_addps:
    case x86_addpd:
    case x86_addss:
    case x86_addsd:
    case x86_xorps:
    case x86_pand:
    case x86_orps:
    case x86_mulps:
    case x86_mulpd:
    case x86_mulss:
    case x86_mulsd:
    case x86_add_flags:
    case ir_compare_equal:
    case ir_compare_not_equal:
    case ir_floating_point_add:
    case ir_floating_point_multiply: {
        return true;
    };
    }

    return false;
}

static void swap_operands(ir_operand* l, ir_operand* r) {
    ir_operand tmp = *l;

    *l = *r;
    *r = tmp;
}
