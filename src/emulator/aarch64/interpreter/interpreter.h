#include <inttypes.h>
template <typename O>
O add_subtract_impl(void* ctx, O n, O m, uint64_t set_flags, uint64_t is_add);
void add_subtract_imm12(void* ctx, uint64_t sf, uint64_t op, uint64_t S, uint64_t sh, uint64_t imm12, uint64_t Rn, uint64_t Rd);
uint64_t X(void* ctx, uint64_t reg_id){throw 0;}; //User Defined Function
void X(void* ctx, uint64_t reg_id, uint64_t value){throw 0;}; //User Defined Function
uint64_t SP(void* ctx){throw 0;}; //User Defined Function
void SP(void* ctx, uint64_t value){throw 0;}; //User Defined Function
uint64_t XSP(void* ctx, uint64_t reg_index);
void XSP(void* ctx, uint64_t reg_index, uint64_t value);
