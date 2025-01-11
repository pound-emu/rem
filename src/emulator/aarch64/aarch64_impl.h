#include <inttypes.h>
#include "emulator/ssa_emit_context.h"
#include "aarch64_context_offsets.h"
#include "emulator/guest_process.h"
#include "aarch64_soft_float.h"
#include "emulator/atomic.h"
#include "emulator/uint128_t.h"
#include "fallbacks.h"

struct interpreter_data
{
    guest_process*      process_context;
    void*               register_data;
    uint64_t            current_pc;
    int                 branch_type;
    uint32_t            current_instruction;
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
uint64_t shift_left_check_interpreter(interpreter_data* ctx, uint64_t to_shift, uint64_t shift, uint64_t size);
uint64_t shift_right_check_interpreter(interpreter_data* ctx, uint64_t to_shift, uint64_t shift, uint64_t size, uint64_t is_unsigned);
uint64_t reverse_interpreter(interpreter_data* ctx, uint128_t word, uint64_t M, uint64_t N);
void convert_to_int_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t Rd, uint64_t Rn, uint64_t round, uint64_t is_unsigned, uint64_t to_vector);
uint64_t lowest_bit_set_c_interpreter(interpreter_data* ctx, uint64_t source);
void dup_element_interpreter(interpreter_data* ctx, uint64_t index, uint64_t esize, uint64_t elements, uint64_t n, uint64_t d);
uint64_t get_flt_size_interpreter(interpreter_data* ctx, uint64_t ftype);
uint64_t vfp_expand_imm_interpreter(interpreter_data* ctx, uint64_t imm8, uint64_t N);
uint64_t expand_imm_interpreter(interpreter_data* ctx, uint64_t op, uint64_t cmode, uint64_t imm8);
void VPart_interpreter(interpreter_data* ctx, uint64_t n, uint64_t part, uint64_t width, uint64_t value);
uint64_t VPart_interpreter(interpreter_data* ctx, uint64_t n, uint64_t part, uint64_t width);
uint64_t get_from_concacted_vector_interpreter(interpreter_data* ctx, uint128_t top, uint128_t bottom, uint64_t index, uint64_t element_count, uint64_t element_size);
uint64_t call_float_binary_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t fpcr, uint64_t N, uint64_t function);
uint64_t call_float_unary_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t fpcr, uint64_t N, uint64_t function);
void convert_to_float_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd, uint64_t from_vector);
uint128_t replicate_vector_interpreter(interpreter_data* ctx, uint128_t source, uint64_t v_size, uint64_t count);
void ld1_interpreter(interpreter_data* ctx, uint64_t wback, uint64_t Q, uint64_t L, uint64_t R, uint64_t Rm, uint64_t o2, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt);
uint64_t FPAdd_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);
uint64_t FPSub_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);
uint64_t FPMul_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);
uint64_t FPDiv_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);
uint64_t FPMax_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);
uint64_t FPMin_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);
uint64_t FPMaxNum_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);
uint64_t FPMinNum_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);
uint64_t FPCompare_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);
uint64_t FPRSqrtStepFused_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);
uint64_t FPCompareEQ_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);
uint64_t FPCompareGT_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);
uint64_t FPCompareGE_interpreter(interpreter_data* ctx, uint64_t operand1, uint64_t operand2, uint64_t FPCR, uint64_t N);
uint64_t FPSqrt_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t FPCR, uint64_t N);
uint64_t FPNeg_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t FPCR, uint64_t N);
uint64_t FPAbs_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t FPCR, uint64_t N);
uint64_t FPRSqrtEstimate_interpreter(interpreter_data* ctx, uint64_t operand, uint64_t FPCR, uint64_t N);
uint64_t FixedToFP_interpreter(interpreter_data* ctx, uint64_t source, uint64_t fracbits, uint64_t is_unsigned, uint64_t to, uint64_t from);
uint64_t FPToFixed_interpreter(interpreter_data* ctx, uint64_t source, uint64_t fracbits, uint64_t is_unsigned, uint64_t round, uint64_t to, uint64_t from);
uint64_t FPConvert_interpreter(interpreter_data* ctx, uint64_t source, uint64_t to, uint64_t from);
uint64_t FPRoundInt_interpreter(interpreter_data* ctx, uint64_t source, uint64_t fpcr, uint64_t rounding, uint64_t N);
uint64_t FPMulAdd_interpreter(interpreter_data* ctx, uint64_t addend, uint64_t element1, uint64_t element2, uint64_t fpcr, uint64_t N);
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
void branch_register_interpreter(interpreter_data* ctx, uint64_t l, uint64_t Rn);
void return_register_interpreter(interpreter_data* ctx, uint64_t Rn);
void test_bit_branch_interpreter(interpreter_data* ctx, uint64_t b5, uint64_t op, uint64_t b40, uint64_t imm14, uint64_t Rt);
void compare_and_branch_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t op, uint64_t imm19, uint64_t Rt);
void b_unconditional_interpreter(interpreter_data* ctx, uint64_t op, uint64_t imm26);
void b_conditional_interpreter(interpreter_data* ctx, uint64_t imm19, uint64_t cond);
void svc_interpreter(interpreter_data* ctx, uint64_t imm16);
void msr_register_interpreter(interpreter_data* ctx, uint64_t imm15, uint64_t Rt);
void mrs_register_interpreter(interpreter_data* ctx, uint64_t imm15, uint64_t Rt);
void hints_interpreter(interpreter_data* ctx, uint64_t imm7);
void sys_interpreter(interpreter_data* ctx, uint64_t L, uint64_t imm19);
void barriers_interpreter(interpreter_data* ctx, uint64_t CRm, uint64_t op2, uint64_t Rt);
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
uint64_t _compare_and_swap_interpreter(interpreter_data* ctx, uint64_t physical_address, uint64_t expecting, uint64_t to_swap, uint64_t size);
uint64_t compare_and_swap_interpreter(interpreter_data* ctx, uint64_t address, uint64_t expecting, uint64_t to_swap, uint64_t size);
template <typename O>
void mem_interpreter(interpreter_data* ctx, uint64_t address, O value);
template <typename O>
O mem_interpreter(interpreter_data* ctx, uint64_t address);
uint64_t XSP_interpreter(interpreter_data* ctx, uint64_t reg_id);
void XSP_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value);
uint64_t X_interpreter(interpreter_data* ctx, uint64_t reg_id);
void X_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value);
void dup_general_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void dup_element_scalar_interpreter(interpreter_data* ctx, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void dup_element_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void move_to_gp_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t imm5, uint64_t U, uint64_t Rn, uint64_t Rd);
void ins_general_interpreter(interpreter_data* ctx, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void ins_element_interpreter(interpreter_data* ctx, uint64_t imm5, uint64_t imm4, uint64_t Rn, uint64_t Rd);
void movi_immediate_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t op, uint64_t immhi, uint64_t cmode, uint64_t immlo, uint64_t Rd);
void fmov_general_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t rmode, uint64_t opcode, uint64_t Rn, uint64_t Rd);
void convert_to_float_gp_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void convert_to_float_vector_scalar_interpreter(interpreter_data* ctx, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd);
void convert_to_float_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd);
void floating_point_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t opcode, uint64_t Rn, uint64_t Rd);
void conversion_between_floating_point_and_fixed_point_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t S, uint64_t ftype, uint64_t rmode, uint64_t opcode, uint64_t scale, uint64_t Rn, uint64_t Rd);
void shl_immedaite_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd);
void sshr_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd);
void shll_shll2_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd);
void rev64_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd);
void neg_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd);
void not_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rn, uint64_t Rd);
void abs_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd);
void mul_vector_index_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t L, uint64_t M, uint64_t Rm, uint64_t H, uint64_t Rn, uint64_t Rd);
void mul_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void ext_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t imm4, uint64_t Rn, uint64_t Rd);
void compare_above_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void shl_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void add_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void addlv_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rn, uint64_t Rd);
void cnt_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd);
void orr_orn_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t invert, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void bsl_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void and_bic_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t invert, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void eor_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void xnt_xnt2_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd);
void zip_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t op, uint64_t Rn, uint64_t Rd);
void tbl_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t len, uint64_t Rn, uint64_t Rd);
void ld1r_no_offset_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rt);
void ld1r_post_index_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t size, uint64_t Rn, uint64_t Rt);
void ld1_single_structure_no_offset_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt);
void ld1_single_structure_post_index_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t Rm, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt);
void floating_point_conditional_select_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t Rd);
void fcmeq_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fcmg_vector_register_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fcmeq_vector_zero_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd);
void fcmg_vector_zero_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd);
void fmov_scalar_immediate_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t imm8, uint64_t Rd);
void fcvt_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t opc, uint64_t Rn, uint64_t Rd);
void fabs_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd);
void fneg_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd);
void fsqrt_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd);
void fcmp_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t opc);
void fccmp_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t nzcv);
void fcvtz_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvtn_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvta_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvtm_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fadd_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fmul_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fsub_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fdiv_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fmul_accumulate_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t neg, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void faddp_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void frsqrte_scalar_interpreter(interpreter_data* ctx, uint64_t sz, uint64_t Rn, uint64_t Rd);
void frsqrte_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd);
void frsqrts_vector_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fmul_scalar_by_element_interpreter(interpreter_data* ctx, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t H, uint64_t Rn, uint64_t Rd);
void fmul_vector_by_element_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t H, uint64_t Rn, uint64_t Rd);
void fmul_accumulate_scalar_interpreter(interpreter_data* ctx, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t neg, uint64_t H, uint64_t Rn, uint64_t Rd);
void fmul_accumulate_element_interpreter(interpreter_data* ctx, uint64_t Q, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t neg, uint64_t H, uint64_t Rn, uint64_t Rd);
void frint_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd, uint64_t rounding);
void frintp_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd);
void frintm_scalar_interpreter(interpreter_data* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd);
void fcvtp_scalar_integer_interpreter(interpreter_data* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
uint64_t _x_interpreter(interpreter_data* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void _x_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value);//THIS FUNCTION IS USER DEFINED
uint64_t _sys_interpreter(interpreter_data* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void _sys_interpreter(interpreter_data* ctx, uint64_t reg_id, uint64_t value);//THIS FUNCTION IS USER DEFINED
uint128_t V_interpreter(interpreter_data* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void V_interpreter(interpreter_data* ctx, uint64_t reg_id, uint128_t value);//THIS FUNCTION IS USER DEFINED
void _branch_long_interpreter(interpreter_data* ctx, uint64_t location);//THIS FUNCTION IS USER DEFINED
void _branch_short_interpreter(interpreter_data* ctx, uint64_t location);//THIS FUNCTION IS USER DEFINED
void _branch_conditional_interpreter(interpreter_data* ctx, uint64_t yes, uint64_t no, uint64_t condition);//THIS FUNCTION IS USER DEFINED
uint64_t get_vector_context_interpreter(interpreter_data* ctx);//THIS FUNCTION IS USER DEFINED
void store_context_interpreter(interpreter_data* ctx);//THIS FUNCTION IS USER DEFINED
void load_context_interpreter(interpreter_data* ctx);//THIS FUNCTION IS USER DEFINED
uint64_t _get_pc_interpreter(interpreter_data* ctx);//THIS FUNCTION IS USER DEFINED
uint64_t translate_address_interpreter(interpreter_data* ctx, uint64_t address);//THIS FUNCTION IS USER DEFINED
void call_supervisor_interpreter(interpreter_data* ctx, uint64_t svc);//THIS FUNCTION IS USER DEFINED
uint64_t call_counter_interpreter(interpreter_data* ctx);//THIS FUNCTION IS USER DEFINED
void undefined_with_interpreter(interpreter_data* ctx, uint64_t value);//THIS FUNCTION IS USER DEFINED
void undefined_interpreter(interpreter_data* ctx);//THIS FUNCTION IS USER DEFINED
uint64_t call_interpreter(interpreter_data* ctx, uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t function);//THIS FUNCTION IS USER DEFINED

//JIT
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
ir_operand shift_left_check_jit(ssa_emit_context* ctx, ir_operand to_shift, ir_operand shift, uint64_t size);
ir_operand shift_right_check_jit(ssa_emit_context* ctx, ir_operand to_shift, ir_operand shift, uint64_t size, uint64_t is_unsigned);
ir_operand reverse_jit(ssa_emit_context* ctx, ir_operand word, uint64_t M, uint64_t N);
void convert_to_int_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t Rd, uint64_t Rn, uint64_t round, uint64_t is_unsigned, uint64_t to_vector);
uint64_t lowest_bit_set_c_jit(ssa_emit_context* ctx, uint64_t source);
void dup_element_jit(ssa_emit_context* ctx, uint64_t index, uint64_t esize, uint64_t elements, uint64_t n, uint64_t d);
uint64_t get_flt_size_jit(ssa_emit_context* ctx, uint64_t ftype);
uint64_t vfp_expand_imm_jit(ssa_emit_context* ctx, uint64_t imm8, uint64_t N);
uint64_t expand_imm_jit(ssa_emit_context* ctx, uint64_t op, uint64_t cmode, uint64_t imm8);
void VPart_jit(ssa_emit_context* ctx, uint64_t n, uint64_t part, uint64_t width, ir_operand value);
ir_operand VPart_jit(ssa_emit_context* ctx, uint64_t n, uint64_t part, uint64_t width);
ir_operand get_from_concacted_vector_jit(ssa_emit_context* ctx, ir_operand top, ir_operand bottom, uint64_t index, uint64_t element_count, uint64_t element_size);
ir_operand call_float_binary_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand fpcr, uint64_t N, uint64_t function);
ir_operand call_float_unary_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand fpcr, uint64_t N, uint64_t function);
void convert_to_float_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd, uint64_t from_vector);
ir_operand replicate_vector_jit(ssa_emit_context* ctx, ir_operand source, uint64_t v_size, uint64_t count);
void ld1_jit(ssa_emit_context* ctx, uint64_t wback, uint64_t Q, uint64_t L, uint64_t R, uint64_t Rm, uint64_t o2, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt);
ir_operand FPAdd_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);
ir_operand FPSub_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);
ir_operand FPMul_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);
ir_operand FPDiv_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);
ir_operand FPMax_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);
ir_operand FPMin_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);
ir_operand FPMaxNum_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);
ir_operand FPMinNum_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);
ir_operand FPCompare_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);
ir_operand FPRSqrtStepFused_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);
ir_operand FPCompareEQ_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);
ir_operand FPCompareGT_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);
ir_operand FPCompareGE_jit(ssa_emit_context* ctx, ir_operand operand1, ir_operand operand2, ir_operand FPCR, uint64_t N);
ir_operand FPSqrt_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N);
ir_operand FPNeg_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N);
ir_operand FPAbs_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N);
ir_operand FPRSqrtEstimate_jit(ssa_emit_context* ctx, ir_operand operand, ir_operand FPCR, uint64_t N);
ir_operand FixedToFP_jit(ssa_emit_context* ctx, ir_operand source, uint64_t fracbits, uint64_t is_unsigned, uint64_t to, uint64_t from);
ir_operand FPToFixed_jit(ssa_emit_context* ctx, ir_operand source, uint64_t fracbits, uint64_t is_unsigned, uint64_t round, uint64_t to, uint64_t from);
ir_operand FPConvert_jit(ssa_emit_context* ctx, ir_operand source, uint64_t to, uint64_t from);
ir_operand FPRoundInt_jit(ssa_emit_context* ctx, ir_operand source, ir_operand fpcr, uint64_t rounding, uint64_t N);
ir_operand FPMulAdd_jit(ssa_emit_context* ctx, ir_operand addend, ir_operand element1, ir_operand element2, ir_operand fpcr, uint64_t N);
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
void branch_register_jit(ssa_emit_context* ctx, uint64_t l, uint64_t Rn);
void return_register_jit(ssa_emit_context* ctx, uint64_t Rn);
void test_bit_branch_jit(ssa_emit_context* ctx, uint64_t b5, uint64_t op, uint64_t b40, uint64_t imm14, uint64_t Rt);
void compare_and_branch_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t op, uint64_t imm19, uint64_t Rt);
void b_unconditional_jit(ssa_emit_context* ctx, uint64_t op, uint64_t imm26);
void b_conditional_jit(ssa_emit_context* ctx, uint64_t imm19, uint64_t cond);
void svc_jit(ssa_emit_context* ctx, uint64_t imm16);
void msr_register_jit(ssa_emit_context* ctx, uint64_t imm15, uint64_t Rt);
void mrs_register_jit(ssa_emit_context* ctx, uint64_t imm15, uint64_t Rt);
void hints_jit(ssa_emit_context* ctx, uint64_t imm7);
void sys_jit(ssa_emit_context* ctx, uint64_t L, uint64_t imm19);
void barriers_jit(ssa_emit_context* ctx, uint64_t CRm, uint64_t op2, uint64_t Rt);
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
ir_operand _compare_and_swap_jit(ssa_emit_context* ctx, ir_operand physical_address, ir_operand expecting, ir_operand to_swap, uint64_t size);
ir_operand compare_and_swap_jit(ssa_emit_context* ctx, ir_operand address, ir_operand expecting, ir_operand to_swap, uint64_t size);
void mem_jit(ssa_emit_context* ctx,uint64_t O, ir_operand address, ir_operand value);
ir_operand mem_jit(ssa_emit_context* ctx,uint64_t O, ir_operand address);
ir_operand XSP_jit(ssa_emit_context* ctx, uint64_t reg_id);
void XSP_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);
ir_operand X_jit(ssa_emit_context* ctx, uint64_t reg_id);
void X_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);
void dup_general_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void dup_element_scalar_jit(ssa_emit_context* ctx, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void dup_element_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void move_to_gp_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t imm5, uint64_t U, uint64_t Rn, uint64_t Rd);
void ins_general_jit(ssa_emit_context* ctx, uint64_t imm5, uint64_t Rn, uint64_t Rd);
void ins_element_jit(ssa_emit_context* ctx, uint64_t imm5, uint64_t imm4, uint64_t Rn, uint64_t Rd);
void movi_immediate_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t op, uint64_t immhi, uint64_t cmode, uint64_t immlo, uint64_t Rd);
void fmov_general_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t rmode, uint64_t opcode, uint64_t Rn, uint64_t Rd);
void convert_to_float_gp_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void convert_to_float_vector_scalar_jit(ssa_emit_context* ctx, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd);
void convert_to_float_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd);
void floating_point_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t opcode, uint64_t Rn, uint64_t Rd);
void conversion_between_floating_point_and_fixed_point_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t S, uint64_t ftype, uint64_t rmode, uint64_t opcode, uint64_t scale, uint64_t Rn, uint64_t Rd);
void shl_immedaite_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd);
void sshr_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd);
void shll_shll2_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t immh, uint64_t immb, uint64_t Rn, uint64_t Rd);
void rev64_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd);
void neg_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd);
void not_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rn, uint64_t Rd);
void abs_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd);
void mul_vector_index_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t L, uint64_t M, uint64_t Rm, uint64_t H, uint64_t Rn, uint64_t Rd);
void mul_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void ext_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t imm4, uint64_t Rn, uint64_t Rd);
void compare_above_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void shl_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void add_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void addlv_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t size, uint64_t Rn, uint64_t Rd);
void cnt_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd);
void orr_orn_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t invert, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void bsl_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void and_bic_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t invert, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void eor_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void xnt_xnt2_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rd);
void zip_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rm, uint64_t op, uint64_t Rn, uint64_t Rd);
void tbl_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t len, uint64_t Rn, uint64_t Rd);
void ld1r_no_offset_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t size, uint64_t Rn, uint64_t Rt);
void ld1r_post_index_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t size, uint64_t Rn, uint64_t Rt);
void ld1_single_structure_no_offset_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt);
void ld1_single_structure_post_index_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t Rm, uint64_t opcode, uint64_t S, uint64_t size, uint64_t Rn, uint64_t Rt);
void floating_point_conditional_select_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t Rd);
void fcmeq_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fcmg_vector_register_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fcmeq_vector_zero_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd);
void fcmg_vector_zero_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t U, uint64_t sz, uint64_t Rn, uint64_t Rd);
void fmov_scalar_immediate_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t imm8, uint64_t Rd);
void fcvt_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t opc, uint64_t Rn, uint64_t Rd);
void fabs_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd);
void fneg_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd);
void fsqrt_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd);
void fcmp_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t Rn, uint64_t opc);
void fccmp_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rm, uint64_t cond, uint64_t Rn, uint64_t nzcv);
void fcvtz_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvtn_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvta_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fcvtm_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
void fadd_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fmul_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fsub_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fdiv_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fmul_accumulate_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t neg, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void faddp_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void frsqrte_scalar_jit(ssa_emit_context* ctx, uint64_t sz, uint64_t Rn, uint64_t Rd);
void frsqrte_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rn, uint64_t Rd);
void frsqrts_vector_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t Rm, uint64_t Rn, uint64_t Rd);
void fmul_scalar_by_element_jit(ssa_emit_context* ctx, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t H, uint64_t Rn, uint64_t Rd);
void fmul_vector_by_element_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t H, uint64_t Rn, uint64_t Rd);
void fmul_accumulate_scalar_jit(ssa_emit_context* ctx, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t neg, uint64_t H, uint64_t Rn, uint64_t Rd);
void fmul_accumulate_element_jit(ssa_emit_context* ctx, uint64_t Q, uint64_t sz, uint64_t L, uint64_t M, uint64_t Rm, uint64_t neg, uint64_t H, uint64_t Rn, uint64_t Rd);
void frint_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd, uint64_t rounding);
void frintp_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd);
void frintm_scalar_jit(ssa_emit_context* ctx, uint64_t ftype, uint64_t Rn, uint64_t Rd);
void fcvtp_scalar_integer_jit(ssa_emit_context* ctx, uint64_t sf, uint64_t ftype, uint64_t U, uint64_t Rn, uint64_t Rd);
ir_operand _x_jit(ssa_emit_context* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void _x_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);//THIS FUNCTION IS USER DEFINED
ir_operand _sys_jit(ssa_emit_context* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void _sys_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);//THIS FUNCTION IS USER DEFINED
ir_operand V_jit(ssa_emit_context* ctx, uint64_t reg_id);//THIS FUNCTION IS USER DEFINED
void V_jit(ssa_emit_context* ctx, uint64_t reg_id, ir_operand value);//THIS FUNCTION IS USER DEFINED
void _branch_long_jit(ssa_emit_context* ctx, ir_operand location);//THIS FUNCTION IS USER DEFINED
void _branch_short_jit(ssa_emit_context* ctx, uint64_t location);//THIS FUNCTION IS USER DEFINED
void _branch_conditional_jit(ssa_emit_context* ctx, uint64_t yes, uint64_t no, ir_operand condition);//THIS FUNCTION IS USER DEFINED
ir_operand get_vector_context_jit(ssa_emit_context* ctx);//THIS FUNCTION IS USER DEFINED
void store_context_jit(ssa_emit_context* ctx);//THIS FUNCTION IS USER DEFINED
void load_context_jit(ssa_emit_context* ctx);//THIS FUNCTION IS USER DEFINED
uint64_t _get_pc_jit(ssa_emit_context* ctx);//THIS FUNCTION IS USER DEFINED
ir_operand translate_address_jit(ssa_emit_context* ctx, ir_operand address);//THIS FUNCTION IS USER DEFINED
void call_supervisor_jit(ssa_emit_context* ctx, uint64_t svc);//THIS FUNCTION IS USER DEFINED
ir_operand call_counter_jit(ssa_emit_context* ctx);//THIS FUNCTION IS USER DEFINED
void undefined_with_jit(ssa_emit_context* ctx, uint64_t value);//THIS FUNCTION IS USER DEFINED
void undefined_jit(ssa_emit_context* ctx);//THIS FUNCTION IS USER DEFINED
ir_operand call_jit(ssa_emit_context* ctx, ir_operand a0, ir_operand a1, ir_operand a2, ir_operand a3, ir_operand a4, ir_operand a5, uint64_t function);//THIS FUNCTION IS USER DEFINED
