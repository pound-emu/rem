#include <inttypes.h>
#include "emulator/ssa_emit_context.h"
#include "aarch64_context_offsets.h"
#include "emulator/guest_process.h"
#include "aarch64_soft_float.h"

struct interpreter_data
{
    guest_process*      process_context;
    void*               register_data;
    uint64_t            current_pc;
    int                 branch_type;
    uint32_t            current_instruction;
};

struct uint128_t
{
    uint64_t d0;
    uint64_t d1;

    uint128_t()
    {
        d0 = 0;
        d1 = 0;
    }

    operator uint64_t ()
    {
        return d0;
    }

    uint128_t (uint64_t source)
    {
        d0 = source;
        d1 = 0;
    }
    
    static void insert(uint128_t* data, int index, int size, uint64_t value)
    {
        switch (size)
        {
            case 8:     ((uint8_t*)data)[index]     = value;    break;
            case 16:    ((uint16_t*)data)[index]    = value;    break;
            case 32:    ((uint32_t*)data)[index]    = value;    break;
            case 64:    ((uint64_t*)data)[index]    = value;    break;
            default: throw 0;
        }
    }

    static uint64_t extract(uint128_t data, int index, int size)
    {
        //For some reason this breaks on o3?
        /*
        switch (size)
        {
            case 8:     return ((uint8_t*)&data)[index]  ;    break;
            case 16:    return ((uint16_t*)&data)[index] ;    break;
            case 32:    return ((uint32_t*)&data)[index] ;    break;
            case 64:    return ((uint64_t*)&data)[index] ;    break;
            default: throw 0;
        }
        */

        switch (size)
        {
            case 8: size = 1; break;
            case 16: size = 2; break;
            case 32: size = 4; break;
            case 64: size = 8; break;

            default: throw 0; break;
        }

        int byte_offset = index * size;

        uint64_t working_part = data.d0;

        if (byte_offset >= 8)
        {
            byte_offset -= 8;
            working_part = data.d1;
        }

        uint64_t mask = UINT64_MAX;

        if (size != 8)
        {
            mask = (1ULL << (8 * size)) - 1;
        }
        
        return (working_part >> (byte_offset * 8)) & mask;
    }

    bool operator == (uint128_t other)
    {
        return (d0 == other.d0) && (d1 == other.d1);
    }
};

void init_aarch64_decoder(guest_process* process);

enum sys_registers
{
    nzcv_n,
    nzcv_z,
    nzcv_c,
    nzcv_v,
    fpcr,
    fpsr,
    exclusive_value,
    exclusive_address,
    thread_local_0,
    thread_local_1
};

template <typename D, typename S>
uint64_t convert_to_float(uint64_t source, bool is_signed)
{
    int des_size = sizeof(D) * 8;
    int src_size = sizeof(S) * 8;

    double temp;

    if (is_signed)
    {
        if (src_size == 32)
        {
            temp = (int32_t)source;
        }
        else
        {
            temp = (int64_t)source;
        }
    }
    else
    {
        temp = (S)source;
    }

    if (des_size == 32)
    {
        float temp_32 = temp;

        return *(uint32_t*)&temp_32;
    }

    return *(uint64_t*)&temp;
}

//INTERPRETER
void add_subtract_imm12_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t sh, uint64_t imm12, uint64_t Rn, uint64_t Rd);
void add_subtract_shifted_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t shift, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd);
void add_subtract_extended_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t option, uint64_t imm3, uint64_t Rn, uint64_t Rd);
void add_subtract_carry_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void shift_variable_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t Rm, uint64_t op2, uint64_t Rn, uint64_t Rd);
void multiply_with_32_interpreter(interpreter_data* ctx, uint64_t U, uint64_t Rm, uint64_t o0, uint64_t Ra, uint64_t Rn, uint64_t Rd);
void multiply_hi_interpreter(interpreter_data* ctx, uint64_t U, uint64_t Rm, uint64_t o0, uint64_t Rn, uint64_t Rd);
void multiply_additive_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t Rm, uint64_t o0, uint64_t Ra, uint64_t Rn, uint64_t Rd);
void divide_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t Rm, uint64_t o1, uint64_t Rn, uint64_t Rd);
void rbit_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t Rn, uint64_t Rd);
void rev16_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t Rn, uint64_t Rd);
void reverse_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t Rn, uint64_t Rd);
void count_leading_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t s, uint64_t Rn, uint64_t Rd);
void extr_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t N, uint64_t Rm, uint64_t imms, uint64_t Rn, uint64_t Rd);
void bitfield_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t N, uint64_t immr, uint64_t imms, uint64_t Rn, uint64_t Rd);
void logical_immediate_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t N, uint64_t immr, uint64_t imms, uint64_t Rn, uint64_t Rd);
void logical_shifted_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t shift, uint64_t N, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd);
void conditional_select_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t cond, uint64_t op2, uint64_t Rn, uint64_t Rd);
void conditional_compare_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t Rm, uint64_t cond, uint64_t mode, uint64_t Rn, uint64_t nzcv);
void move_wide_immediate_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t opc, uint64_t hw, uint64_t imm16, uint64_t Rd);
void pc_rel_addressing_interpreter(interpreter_data* ctx, uint64_t op, uint64_t immlo, uint64_t immhi, uint64_t Rd);
void load_store_register_post_interpreter(interpreter_data* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t Rn, uint64_t Rt);
void load_store_register_pre_interpreter(interpreter_data* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t Rn, uint64_t Rt);
void load_store_register_unscaled_interpreter(interpreter_data* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t Rn, uint64_t Rt);
void load_store_register_pair_imm_offset_interpreter(interpreter_data* ctx, uint64_t opc, uint64_t VR, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt);
void load_store_register_pair_imm_post_interpreter(interpreter_data* ctx, uint64_t opc, uint64_t VR, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt);
void load_store_register_pair_imm_pre_interpreter(interpreter_data* ctx, uint64_t opc, uint64_t VR, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt);
void load_store_register_pair_imm_interpreter(interpreter_data* ctx, uint64_t opc, uint64_t VR, uint64_t wb, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt);
void load_store_register_imm_unsigned_interpreter(interpreter_data* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm12, uint64_t Rn, uint64_t Rt);
void load_store_register_imm_unscaled_interpreter(interpreter_data* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t wb, uint64_t Rn, uint64_t Rt);
void load_store_register_offset_interpreter(interpreter_data* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t Rm, uint64_t option, uint64_t S, uint64_t Rn, uint64_t Rt);
void load_store_exclusive_ordered_interpreter(interpreter_data* ctx, uint64_t size, uint64_t ordered, uint64_t L, uint64_t Rs, uint64_t o0, uint64_t Rn, uint64_t Rt);
void load_exclusive_interpreter(interpreter_data* ctx, uint64_t is_exclusive, uint64_t size, uint64_t Rn, uint64_t Rt);
void store_exclusive_interpreter(interpreter_data* ctx, uint64_t is_exclusive, uint64_t size, uint64_t Rn, uint64_t Rt, uint64_t Rs);
void branch_register_interpreter(interpreter_data* ctx, uint64_t l, uint64_t Rn);
void return_register_interpreter(interpreter_data* ctx, uint64_t Rn);
void test_bit_branch_interpreter(interpreter_data* ctx, uint64_t b5, uint64_t op, uint64_t b40, uint64_t imm14, uint64_t Rt);
void compare_and_branch_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t imm19, uint64_t Rt);
void b_unconditional_interpreter(interpreter_data* ctx, uint64_t op, uint64_t imm26);
void b_conditional_interpreter(interpreter_data* ctx, uint64_t imm19, uint64_t cond);
void svc_interpreter(interpreter_data* ctx, uint64_t imm16);
uint64_t sign_extend_interpreter(interpreter_data* ctx, uint64_t source, uint64_t count);
template <typename O>
O a_shift_reg_interpreter(interpreter_data* ctx, uint64_t m, uint64_t shift_type, uint64_t ammount);
template <typename O>
O a_extend_reg_interpreter(interpreter_data* ctx, uint64_t m, uint64_t extend_type, uint64_t shift);
uint64_t a_extend_reg_64_interpreter(interpreter_data* ctx, uint64_t m, uint64_t extend_type, uint64_t shift);
template <typename O>
O reverse_bytes_interpreter(interpreter_data* ctx, O source, uint64_t byte_count);
uint64_t highest_bit_set_c_interpreter(interpreter_data* ctx, uint64_t src, uint64_t size);
uint64_t ones_interpreter(interpreter_data* ctx, uint64_t size);
uint64_t replicate_c_interpreter(interpreter_data* ctx, uint64_t source, uint64_t source_size, uint64_t count);
uint64_t bits_c_interpreter(interpreter_data* ctx, uint64_t source, uint64_t top, uint64_t bottom);
uint64_t bit_c_interpreter(interpreter_data* ctx, uint64_t source, uint64_t bit);
uint64_t rotate_right_bits_interpreter(interpreter_data* ctx, uint64_t source, uint64_t ammount, uint64_t bit_count);
uint64_t decode_bitmask_tmask_interpreter(interpreter_data* ctx, uint64_t immN, uint64_t imms, uint64_t immr, uint64_t immediate, uint64_t M, uint64_t return_tmask);
uint64_t decode_add_subtract_imm_12_interpreter(interpreter_data* ctx, uint64_t source, uint64_t shift);
template <typename O>
O add_subtract_impl_interpreter(interpreter_data* ctx, O n, O m, uint64_t set_flags, uint64_t is_add);
template <typename O>
O add_subtract_carry_impl_interpreter(interpreter_data* ctx, O n, O m, uint64_t set_flags, uint64_t is_add, O carry);
uint8_t condition_holds_interpreter(interpreter_data* ctx, uint64_t cond);
void branch_long_universal_interpreter(interpreter_data* ctx, uint64_t Rn, uint64_t link);
uint64_t lowest_bit_set_c_interpreter(interpreter_data* ctx, uint64_t source);
void dup_element_interpreter(interpreter_data* ctx, uint64_t index, uint64_t esize, uint64_t elements, uint64_t n, uint64_t d);
uint64_t get_flt_size_interpreter(interpreter_data* ctx, uint64_t ftype);
uint64_t vfp_expand_imm_interpreter(interpreter_data* ctx, uint64_t imm8, uint64_t N);
uint64_t expand_imm_interpreter(interpreter_data* ctx, uint64_t op, uint64_t cmode, uint64_t imm8);
void VPart_interpreter(interpreter_data* ctx, uint64_t n, uint64_t part, uint64_t width, uint64_t value);
void dup_general_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void dup_element_scalar_interpreter(interpreter_data* ctx, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void dup_element_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void move_to_gp_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t imm5, uint64_t U, uint64_t Rn, uint64_t Rd);
void ins_general_interpreter(interpreter_data* ctx, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void ins_element_interpreter(interpreter_data* ctx, uint64_t imm5, uint64_t imm4, uint64_t Rn, uint64_t Rd);
void movi_immediate_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t op, uint64_t immhi, uint64_t cmode, uint64_t immlo, uint64_t Rd);
void fmov_general_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t rmode, uint64_t opcode, uint64_t Rn, uint64_t Rd);
void convert_to_float_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd, uint64_t from_vector);
void convert_to_float_gp_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void convert_to_float_vector_interpreter(interpreter_data* ctx, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd);
void floating_point_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t opcode, uint64_t Rn, uint64_t Rd);
template <typename O>
O vector_shift_interpreter(interpreter_data* ctx, O element, O shift, uint64_t bit_count, uint64_t is_unsigned);
void conversion_between_floating_point_and_fixed_point_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t S, uint64_t ftype, uint64_t rmode, uint64_t opcode, uint64_t scale, uint64_t Rn, uint64_t Rd);
void advanced_simd_three_same_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rm, uint64_t opcode, uint64_t Rn, uint64_t Rd);
void floating_point_conditional_select_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t Rd);
void fmov_scalar_immediate_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t imm8, uint64_t Rd);
void fcvt_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t opc, uint64_t Rn, uint64_t Rd);
void floating_point_data_processing_one_source_interpreter(interpreter_data* ctx, uint64_t M, uint64_t S, uint64_t ftype, uint64_t opcode, uint64_t Rn, uint64_t Rd);
void fcmp_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t opc);
void fccmp_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t nzcv);
void convert_to_int_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t Rd, uint64_t Rn, uint64_t round, uint64_t is_unsigned, uint64_t to_vector);
void fcvtz_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvtn_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvta_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvtm_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvtp_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void advanced_simd_across_lanes_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t opcode, uint64_t Rn, uint64_t Rd);
void advanced_simd_two_register_misc_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t opcode, uint64_t Rn, uint64_t Rd);
uint64_t compare_and_swap_interpreter(interpreter_data* ctx, uint64_t address, uint64_t expecting, uint64_t to_swap, uint64_t size);
template <typename O>
void mem_interpreter(interpreter_data* ctx, uint64_t address, O value);
template <typename O>
O mem_interpreter(interpreter_data* ctx, uint64_t address);
uint64_t XSP_interpreter(interpreter_data* ctx, uint64_t reg_id);
void XSP_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value);
uint64_t X_interpreter(interpreter_data* ctx, uint64_t reg_id);
void X_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value);
void msr_register_interpreter(interpreter_data* ctx, uint64_t imm15, uint64_t Rt);
void mrs_register_interpreter(interpreter_data* ctx, uint64_t imm15, uint64_t Rt);
void hints_interpreter(interpreter_data* ctx, uint64_t imm7);
void sys_interpreter(interpreter_data* ctx, uint64_t L, uint64_t imm19);
void barriers_interpreter(interpreter_data* ctx, uint64_t CRm, uint64_t op2, uint64_t Rt);
uint64_t _x_interpreter(interpreter_data* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void _x_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value);//THIS FUNCTION IS USER DEFINED
uint64_t _sys_interpreter(interpreter_data* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void _sys_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value);//THIS FUNCTION IS USER DEFINED
uint128_t V_interpreter(interpreter_data* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void V_interpreter(interpreter_data* ctx, uint64_t reg_id, uint128_t value);//THIS FUNCTION IS USER DEFINED
void _branch_long_interpreter(interpreter_data* ctx, uint64_t location);//THIS FUNCTION IS USER DEFINED
void _branch_short_interpreter(interpreter_data* ctx, uint64_t location);//THIS FUNCTION IS USER DEFINED
void _branch_conditional_interpreter(interpreter_data* ctx, uint64_t yes, uint64_t no, uint64_t condition);//THIS FUNCTION IS USER DEFINED
uint64_t _get_pc_interpreter(interpreter_data* ctx);//THIS FUNCTION IS USER DEFINED
uint64_t translate_address_interpreter(interpreter_data* ctx, uint64_t address);//THIS FUNCTION IS USER DEFINED
void call_supervisor_interpreter(interpreter_data* ctx, uint64_t svc);//THIS FUNCTION IS USER DEFINED
uint64_t call_counter_interpreter(interpreter_data* ctx);//THIS FUNCTION IS USER DEFINED
void undefined_with_interpreter(interpreter_data* ctx, uint64_t value);//THIS FUNCTION IS USER DEFINED
void undefined_interpreter(interpreter_data* ctx);//THIS FUNCTION IS USER DEFINED
uint64_t FPAdd_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
uint64_t FPSub_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
uint64_t FPMul_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
uint64_t FPDiv_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
uint64_t FPMax_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
uint64_t FPMin_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
uint64_t FPMaxNum_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
uint64_t FPMinNum_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
uint64_t FPCompare_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
uint64_t FPSqrt_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
uint64_t FPNeg_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
uint64_t FPAbs_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
uint64_t FixedToFP_interpreter(interpreter_data* ctx, uint64_t source, uint64_t fracbits, uint64_t is_unsigned, uint64_t to, uint64_t from);//THIS FUNCTION IS USER DEFINED
uint64_t FPToFixed_interpreter(interpreter_data* ctx, uint64_t source, uint64_t fracbits, uint64_t is_unsigned, uint64_t round, uint64_t to, uint64_t from);//THIS FUNCTION IS USER DEFINED
uint64_t FPConvert_interpreter(interpreter_data* ctx, uint64_t source, uint64_t to, uint64_t from);//THIS FUNCTION IS USER DEFINED
uint64_t _compare_and_swap_interpreter(interpreter_data* ctx, uint64_t physical_address, uint64_t expecting, uint64_t to_swap, uint64_t size);//THIS FUNCTION IS USER DEFINED

//JIT
void add_subtract_imm12_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t sh, uint64_t imm12, uint64_t Rn, uint64_t Rd);
void add_subtract_shifted_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t shift, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd);
void add_subtract_extended_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t option, uint64_t imm3, uint64_t Rn, uint64_t Rd);
void add_subtract_carry_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void shift_variable_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rm, uint64_t op2, uint64_t Rn, uint64_t Rd);
void multiply_with_32_jit(ssa_emit_context* ctx, uint64_t U, uint64_t Rm, uint64_t o0, uint64_t Ra, uint64_t Rn, uint64_t Rd);
void multiply_hi_jit(ssa_emit_context* ctx, uint64_t U, uint64_t Rm, uint64_t o0, uint64_t Rn, uint64_t Rd);
void multiply_additive_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rm, uint64_t o0, uint64_t Ra, uint64_t Rn, uint64_t Rd);
void divide_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rm, uint64_t o1, uint64_t Rn, uint64_t Rd);
void rbit_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rn, uint64_t Rd);
void rev16_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t Rn, uint64_t Rd);
void reverse_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t Rn, uint64_t Rd);
void count_leading_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t s, uint64_t Rn, uint64_t Rd);
void extr_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t N, uint64_t Rm, uint64_t imms, uint64_t Rn, uint64_t Rd);
void bitfield_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t N, uint64_t immr, uint64_t imms, uint64_t Rn, uint64_t Rd);
void logical_immediate_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t N, uint64_t immr, uint64_t imms, uint64_t Rn, uint64_t Rd);
void logical_shifted_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t shift, uint64_t N, uint64_t Rm, uint64_t imm6, uint64_t Rn, uint64_t Rd);
void conditional_select_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t Rm, uint64_t cond, uint64_t op2, uint64_t Rn, uint64_t Rd);
void conditional_compare_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t Rm, uint64_t cond, uint64_t mode, uint64_t Rn, uint64_t nzcv);
void move_wide_immediate_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t opc, uint64_t hw, uint64_t imm16, uint64_t Rd);
void pc_rel_addressing_jit(ssa_emit_context* ctx, uint64_t op, uint64_t immlo, uint64_t immhi, uint64_t Rd);
void load_store_register_post_jit(ssa_emit_context* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t Rn, uint64_t Rt);
void load_store_register_pre_jit(ssa_emit_context* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t Rn, uint64_t Rt);
void load_store_register_unscaled_jit(ssa_emit_context* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t Rn, uint64_t Rt);
void load_store_register_pair_imm_offset_jit(ssa_emit_context* ctx, uint64_t opc, uint64_t VR, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt);
void load_store_register_pair_imm_post_jit(ssa_emit_context* ctx, uint64_t opc, uint64_t VR, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt);
void load_store_register_pair_imm_pre_jit(ssa_emit_context* ctx, uint64_t opc, uint64_t VR, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt);
void load_store_register_pair_imm_jit(ssa_emit_context* ctx, uint64_t opc, uint64_t VR, uint64_t wb, uint64_t L, uint64_t imm7, uint64_t Rt2, uint64_t Rn, uint64_t Rt);
void load_store_register_imm_unsigned_jit(ssa_emit_context* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm12, uint64_t Rn, uint64_t Rt);
void load_store_register_imm_unscaled_jit(ssa_emit_context* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t imm9, uint64_t wb, uint64_t Rn, uint64_t Rt);
void load_store_register_offset_jit(ssa_emit_context* ctx, uint64_t size, uint64_t VR, uint64_t opc, uint64_t Rm, uint64_t option, uint64_t S, uint64_t Rn, uint64_t Rt);
void load_store_exclusive_ordered_jit(ssa_emit_context* ctx, uint64_t size, uint64_t ordered, uint64_t L, uint64_t Rs, uint64_t o0, uint64_t Rn, uint64_t Rt);
void load_exclusive_jit(ssa_emit_context* ctx, uint64_t is_exclusive, uint64_t size, uint64_t Rn, uint64_t Rt);
void store_exclusive_jit(ssa_emit_context* ctx, uint64_t is_exclusive, uint64_t size, uint64_t Rn, uint64_t Rt, uint64_t Rs);
void branch_register_jit(ssa_emit_context* ctx, uint64_t l, uint64_t Rn);
void return_register_jit(ssa_emit_context* ctx, uint64_t Rn);
void test_bit_branch_jit(ssa_emit_context* ctx, uint64_t b5, uint64_t op, uint64_t b40, uint64_t imm14, uint64_t Rt);
void compare_and_branch_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t imm19, uint64_t Rt);
void b_unconditional_jit(ssa_emit_context* ctx, uint64_t op, uint64_t imm26);
void b_conditional_jit(ssa_emit_context* ctx, uint64_t imm19, uint64_t cond);
void svc_jit(ssa_emit_context* ctx, uint64_t imm16);
uint64_t sign_extend_jit(ssa_emit_context* ctx, uint64_t source, uint64_t count);
ir_operand a_shift_reg_jit(ssa_emit_context* ctx,uint64_t O, uint64_t m, uint64_t shift_type, uint64_t ammount);
ir_operand a_extend_reg_jit(ssa_emit_context* ctx,uint64_t O, uint64_t m, uint64_t extend_type, uint64_t shift);
ir_operand a_extend_reg_64_jit(ssa_emit_context* ctx, uint64_t m, uint64_t extend_type, uint64_t shift);
ir_operand reverse_bytes_jit(ssa_emit_context* ctx,uint64_t O, ir_operand source, uint64_t byte_count);
uint64_t highest_bit_set_c_jit(ssa_emit_context* ctx, uint64_t src, uint64_t size);
uint64_t ones_jit(ssa_emit_context* ctx, uint64_t size);
uint64_t replicate_c_jit(ssa_emit_context* ctx, uint64_t source, uint64_t source_size, uint64_t count);
uint64_t bits_c_jit(ssa_emit_context* ctx, uint64_t source, uint64_t top, uint64_t bottom);
uint64_t bit_c_jit(ssa_emit_context* ctx, uint64_t source, uint64_t bit);
uint64_t rotate_right_bits_jit(ssa_emit_context* ctx, uint64_t source, uint64_t ammount, uint64_t bit_count);
uint64_t decode_bitmask_tmask_jit(ssa_emit_context* ctx, uint64_t immN, uint64_t imms, uint64_t immr, uint64_t immediate, uint64_t M, uint64_t return_tmask);
uint64_t decode_add_subtract_imm_12_jit(ssa_emit_context* ctx, uint64_t source, uint64_t shift);
ir_operand add_subtract_impl_jit(ssa_emit_context* ctx,uint64_t O, ir_operand n, ir_operand m, uint64_t set_flags, uint64_t is_add);
ir_operand add_subtract_carry_impl_jit(ssa_emit_context* ctx,uint64_t O, ir_operand n, ir_operand m, uint64_t set_flags, uint64_t is_add, ir_operand carry);
ir_operand condition_holds_jit(ssa_emit_context* ctx, uint64_t cond);
void branch_long_universal_jit(ssa_emit_context* ctx, uint64_t Rn, uint64_t link);
uint64_t lowest_bit_set_c_jit(ssa_emit_context* ctx, uint64_t source);
void dup_element_jit(ssa_emit_context* ctx, uint64_t index, uint64_t esize, uint64_t elements, uint64_t n, uint64_t d);
uint64_t get_flt_size_jit(ssa_emit_context* ctx, uint64_t ftype);
uint64_t vfp_expand_imm_jit(ssa_emit_context* ctx, uint64_t imm8, uint64_t N);
uint64_t expand_imm_jit(ssa_emit_context* ctx, uint64_t op, uint64_t cmode, uint64_t imm8);
void VPart_jit(ssa_emit_context* ctx, uint64_t n, uint64_t part, uint64_t width, ir_operand value);
void dup_general_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void dup_element_scalar_jit(ssa_emit_context* ctx, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void dup_element_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void move_to_gp_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t imm5, uint64_t U, uint64_t Rn, uint64_t Rd);
void ins_general_jit(ssa_emit_context* ctx, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void ins_element_jit(ssa_emit_context* ctx, uint64_t imm5, uint64_t imm4, uint64_t Rn, uint64_t Rd);
void movi_immediate_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t op, uint64_t immhi, uint64_t cmode, uint64_t immlo, uint64_t Rd);
void fmov_general_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t rmode, uint64_t opcode, uint64_t Rn, uint64_t Rd);
void convert_to_float_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd, uint64_t from_vector);
void convert_to_float_gp_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void convert_to_float_vector_jit(ssa_emit_context* ctx, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd);
void floating_point_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t opcode, uint64_t Rn, uint64_t Rd);
ir_operand vector_shift_jit(ssa_emit_context* ctx,uint64_t O, ir_operand element, ir_operand shift, uint64_t bit_count, uint64_t is_unsigned);
void conversion_between_floating_point_and_fixed_point_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t S, uint64_t ftype, uint64_t rmode, uint64_t opcode, uint64_t scale, uint64_t Rn, uint64_t Rd);
void advanced_simd_three_same_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rm, uint64_t opcode, uint64_t Rn, uint64_t Rd);
void floating_point_conditional_select_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t Rd);
void fmov_scalar_immediate_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t imm8, uint64_t Rd);
void fcvt_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t opc, uint64_t Rn, uint64_t Rd);
void floating_point_data_processing_one_source_jit(ssa_emit_context* ctx, uint64_t M, uint64_t S, uint64_t ftype, uint64_t opcode, uint64_t Rn, uint64_t Rd);
void fcmp_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t opc);
void fccmp_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t nzcv);
void convert_to_int_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t Rd, uint64_t Rn, uint64_t round, uint64_t is_unsigned, uint64_t to_vector);
void fcvtz_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvtn_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvta_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvtm_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvtp_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void advanced_simd_across_lanes_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t opcode, uint64_t Rn, uint64_t Rd);
void advanced_simd_two_register_misc_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t opcode, uint64_t Rn, uint64_t Rd);
ir_operand compare_and_swap_jit(ssa_emit_context* ctx, ir_operand address, ir_operand expecting, ir_operand to_swap, uint64_t size);
void mem_jit(ssa_emit_context* ctx,uint64_t O, ir_operand address, ir_operand value);
ir_operand mem_jit(ssa_emit_context* ctx,uint64_t O, ir_operand address);
ir_operand XSP_jit(ssa_emit_context* ctx, uint64_t reg_id);
void XSP_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);
ir_operand X_jit(ssa_emit_context* ctx, uint64_t reg_id);
void X_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);
void msr_register_jit(ssa_emit_context* ctx, uint64_t imm15, uint64_t Rt);
void mrs_register_jit(ssa_emit_context* ctx, uint64_t imm15, uint64_t Rt);
void hints_jit(ssa_emit_context* ctx, uint64_t imm7);
void sys_jit(ssa_emit_context* ctx, uint64_t L, uint64_t imm19);
void barriers_jit(ssa_emit_context* ctx, uint64_t CRm, uint64_t op2, uint64_t Rt);
ir_operand _x_jit(ssa_emit_context* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void _x_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);//THIS FUNCTION IS USER DEFINED
ir_operand _sys_jit(ssa_emit_context* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void _sys_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);//THIS FUNCTION IS USER DEFINED
ir_operand V_jit(ssa_emit_context* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void V_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);//THIS FUNCTION IS USER DEFINED
void _branch_long_jit(ssa_emit_context* ctx, ir_operand location);//THIS FUNCTION IS USER DEFINED
void _branch_short_jit(ssa_emit_context* ctx, uint64_t location);//THIS FUNCTION IS USER DEFINED
void _branch_conditional_jit(ssa_emit_context* ctx, uint64_t yes, uint64_t no, ir_operand condition);//THIS FUNCTION IS USER DEFINED
uint64_t _get_pc_jit(ssa_emit_context* ctx);//THIS FUNCTION IS USER DEFINED
ir_operand translate_address_jit(ssa_emit_context* ctx, ir_operand address);//THIS FUNCTION IS USER DEFINED
void call_supervisor_jit(ssa_emit_context* ctx, uint64_t svc);//THIS FUNCTION IS USER DEFINED
ir_operand call_counter_jit(ssa_emit_context* ctx);//THIS FUNCTION IS USER DEFINED
void undefined_with_jit(ssa_emit_context* ctx, uint64_t value);//THIS FUNCTION IS USER DEFINED
void undefined_jit(ssa_emit_context* ctx);//THIS FUNCTION IS USER DEFINED
ir_operand FPAdd_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
ir_operand FPSub_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
ir_operand FPMul_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
ir_operand FPDiv_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
ir_operand FPMax_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
ir_operand FPMin_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
ir_operand FPMaxNum_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
ir_operand FPMinNum_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
ir_operand FPCompare_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
ir_operand FPSqrt_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
ir_operand FPNeg_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
ir_operand FPAbs_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N);//THIS FUNCTION IS USER DEFINED
ir_operand FixedToFP_jit(ssa_emit_context* ctx, ir_operand source, uint64_t fracbits, uint64_t is_unsigned, uint64_t to, uint64_t from);//THIS FUNCTION IS USER DEFINED
ir_operand FPToFixed_jit(ssa_emit_context* ctx, ir_operand source, uint64_t fracbits, uint64_t is_unsigned, uint64_t round, uint64_t to, uint64_t from);//THIS FUNCTION IS USER DEFINED
ir_operand FPConvert_jit(ssa_emit_context* ctx, ir_operand source, uint64_t to, uint64_t from);//THIS FUNCTION IS USER DEFINED
ir_operand _compare_and_swap_jit(ssa_emit_context* ctx, ir_operand physical_address, ir_operand expecting, ir_operand to_swap, uint64_t size);//THIS FUNCTION IS USER DEFINED
