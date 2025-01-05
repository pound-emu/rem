#ifndef AARCH64_SOFT_FLOAT_H
#define AARCH64_SOFT_FLOAT_H

#include <tuple>
#include <inttypes.h>
#include <initializer_list>
#include <math.h>
#include <assert.h>

typedef bool            bit;
typedef bool            boolean;
typedef long double     real;
typedef int64_t         integer; 

#define bits(N) meta_number
#define FALSE   false
#define TRUE    true

#define FPFracBits  integer
#define FPBitsType  std::tuple<FPFracBits,integer>

static bool in(uint64_t check, std::initializer_list<uint64_t> cheks)
{
    for (auto v : cheks)
    {
        if (v == check)
        {
            return true;
        }
    }

    return false;
}

static void assert_in(uint64_t check, std::initializer_list<uint64_t> cheks)
{
    assert(in(check, cheks));
}

struct meta_number
{
    uint64_t    value;
    int         size;

    meta_number()
    {
        value = 0;
        size = -1;
    }

    meta_number(uint64_t value)
    {
        value = value;
        size = -1;
    }

    meta_number(uint64_t value, int size)
    {
        this->value = value;
        this->size = size;
    }

    uint64_t create_mask()
    {
        if (size >= 64)
            return UINT64_MAX;

        return (1ULL << size) - 1;
    }

    meta_number p(int bit)
    {
        return meta_number((value >> bit) & 1, 1);
    }

    meta_number p(int top, int bottom)
    {
        top++;

        int size = top - bottom;
        uint64_t mask = (1ULL << size) - 1;

        return meta_number((value >> bottom) & mask, size);
    }

    void set_p(int bit, int source)
    {
        uint64_t mask = 1ULL << bit;

        value = (value & ~mask) | ((source << bit) & mask);
    }

    operator uint64_t()
    {
        return value;
    }
};

static meta_number concat(meta_number left, meta_number right)
{
    meta_number result = meta_number(((left.value & left.create_mask()) << right.size) | (right.value & right.create_mask()), left.size + right.size);

    return result;
}

static meta_number concat(meta_number a, meta_number b, meta_number c)
{
    return concat(concat(a, b), c);
}

static meta_number concat(meta_number a, meta_number b, meta_number c, meta_number d)
{
    return concat(concat(a, b, c), d);
}

static meta_number NOT(meta_number source)
{
    source.value = ~source.value;
    source.value &= source.create_mask();

    return source;
}

enum FPType 
{   
    FPType_Zero,
    FPType_Denormal,
    FPType_Nonzero,
    FPType_Infinity,
    FPType_QNaN,
    FPType_SNaN
};

enum FPExc
{   
    FPExc_InvalidOp, 
    FPExc_DivideByZero, 
    FPExc_Overflow,
    FPExc_Underflow, 
    FPExc_Inexact, 
    FPExc_InputDenorm
};

enum FPRounding  
{
    FPRounding_TIEEVEN, 
    FPRounding_POSINF,
    FPRounding_NEGINF,  
    FPRounding_ZERO,
    FPRounding_TIEAWAY, 
    FPRounding_ODD
};

enum extensions
{
    FEAT_AFP
};

struct FPCR_Type
{
    uint64_t value;

    uint64_t FIZ() { return (value >> 0) & 1ULL; }
    uint64_t FIZ(uint64_t new_value) { value = (value & ~1ULL) | ( 1ULL & (new_value << 0)); }
    uint64_t AH() { return (value >> 1) & 1ULL; }
    uint64_t AH(uint64_t new_value) { value = (value & ~2ULL) | ( 2ULL & (new_value << 1)); }
    uint64_t NEP() { return (value >> 2) & 1ULL; }
    uint64_t NEP(uint64_t new_value) { value = (value & ~4ULL) | ( 4ULL & (new_value << 2)); }
    uint64_t RES0_0() { return (value >> 3) & 31ULL; }
    uint64_t RES0_0(uint64_t new_value) { value = (value & ~248ULL) | ( 248ULL & (new_value << 3)); }
    uint64_t IOE() { return (value >> 8) & 1ULL; }
    uint64_t IOE(uint64_t new_value) { value = (value & ~256ULL) | ( 256ULL & (new_value << 8)); }
    uint64_t DZE() { return (value >> 9) & 1ULL; }
    uint64_t DZE(uint64_t new_value) { value = (value & ~512ULL) | ( 512ULL & (new_value << 9)); }
    uint64_t OFE() { return (value >> 10) & 1ULL; }
    uint64_t OFE(uint64_t new_value) { value = (value & ~1024ULL) | ( 1024ULL & (new_value << 10)); }
    uint64_t UFE() { return (value >> 11) & 1ULL; }
    uint64_t UFE(uint64_t new_value) { value = (value & ~2048ULL) | ( 2048ULL & (new_value << 11)); }
    uint64_t IXE() { return (value >> 12) & 1ULL; }
    uint64_t IXE(uint64_t new_value) { value = (value & ~4096ULL) | ( 4096ULL & (new_value << 12)); }
    uint64_t EBF() { return (value >> 13) & 1ULL; }
    uint64_t EBF(uint64_t new_value) { value = (value & ~8192ULL) | ( 8192ULL & (new_value << 13)); }
    uint64_t RES0_1() { return (value >> 14) & 1ULL; }
    uint64_t RES0_1(uint64_t new_value) { value = (value & ~16384ULL) | ( 16384ULL & (new_value << 14)); }
    uint64_t IDE() { return (value >> 15) & 1ULL; }
    uint64_t IDE(uint64_t new_value) { value = (value & ~32768ULL) | ( 32768ULL & (new_value << 15)); }
    uint64_t Len() { return (value >> 16) & 7ULL; }
    uint64_t Len(uint64_t new_value) { value = (value & ~458752ULL) | ( 458752ULL & (new_value << 16)); }
    uint64_t FZ16() { return (value >> 19) & 1ULL; }
    uint64_t FZ16(uint64_t new_value) { value = (value & ~524288ULL) | ( 524288ULL & (new_value << 19)); }
    uint64_t Stride() { return (value >> 20) & 3ULL; }
    uint64_t Stride(uint64_t new_value) { value = (value & ~3145728ULL) | ( 3145728ULL & (new_value << 20)); }
    uint64_t RMode() { return (value >> 22) & 3ULL; }
    uint64_t RMode(uint64_t new_value) { value = (value & ~12582912ULL) | ( 12582912ULL & (new_value << 22)); }
    uint64_t FZ() { return (value >> 24) & 1ULL; }
    uint64_t FZ(uint64_t new_value) { value = (value & ~16777216ULL) | ( 16777216ULL & (new_value << 24)); }
    uint64_t DN() { return (value >> 25) & 1ULL; }
    uint64_t DN(uint64_t new_value) { value = (value & ~33554432ULL) | ( 33554432ULL & (new_value << 25)); }
    uint64_t AHP() { return (value >> 26) & 1ULL; }
    uint64_t AHP(uint64_t new_value) { value = (value & ~67108864ULL) | ( 67108864ULL & (new_value << 26)); }
};

struct FPSR_Type
{
    uint64_t value;

    uint64_t IOC() { return (value >> 0) & 1ULL; }
    uint64_t IOC(uint64_t new_value) { value = (value & ~1ULL) | ( 1ULL & (new_value << 0)); }
    uint64_t DZC() { return (value >> 1) & 1ULL; }
    uint64_t DZC(uint64_t new_value) { value = (value & ~2ULL) | ( 2ULL & (new_value << 1)); }
    uint64_t OFC() { return (value >> 2) & 1ULL; }
    uint64_t OFC(uint64_t new_value) { value = (value & ~4ULL) | ( 4ULL & (new_value << 2)); }
    uint64_t UFC() { return (value >> 3) & 1ULL; }
    uint64_t UFC(uint64_t new_value) { value = (value & ~8ULL) | ( 8ULL & (new_value << 3)); }
    uint64_t IXC() { return (value >> 4) & 1ULL; }
    uint64_t IXC(uint64_t new_value) { value = (value & ~16ULL) | ( 16ULL & (new_value << 4)); }
    uint64_t RES0_0() { return (value >> 5) & 3ULL; }
    uint64_t RES0_0(uint64_t new_value) { value = (value & ~96ULL) | ( 96ULL & (new_value << 5)); }
    uint64_t IDC() { return (value >> 7) & 1ULL; }
    uint64_t IDC(uint64_t new_value) { value = (value & ~128ULL) | ( 128ULL & (new_value << 7)); }
    uint64_t RES0_1() { return (value >> 8) & 524287ULL; }
    uint64_t RES0_1(uint64_t new_value) { value = (value & ~134217472ULL) | ( 134217472ULL & (new_value << 8)); }
    uint64_t QC() { return (value >> 27) & 1ULL; }
    uint64_t QC(uint64_t new_value) { value = (value & ~134217728ULL) | ( 134217728ULL & (new_value << 27)); }
    uint64_t V() { return (value >> 28) & 1ULL; }
    uint64_t V(uint64_t new_value) { value = (value & ~268435456ULL) | ( 268435456ULL & (new_value << 28)); }
    uint64_t C() { return (value >> 29) & 1ULL; }
    uint64_t C(uint64_t new_value) { value = (value & ~536870912ULL) | ( 536870912ULL & (new_value << 29)); }
    uint64_t Z() { return (value >> 30) & 1ULL; }
    uint64_t Z(uint64_t new_value) { value = (value & ~1073741824ULL) | ( 1073741824ULL & (new_value << 30)); }
    uint64_t N() { return (value >> 31) & 1ULL; }
    uint64_t N(uint64_t new_value) { value = (value & ~2147483648ULL) | ( 2147483648ULL & (new_value << 31)); }
};

//TODO
static FPSR_Type FPSCR;
static FPSR_Type FPSR;

static boolean IsFeatureImplemented(uint64_t feature)
{
    return true;
}

static boolean UsingAArch32()
{
    return true;
}

static boolean IsFullA64Enabled()
{
    return true;
}

static boolean InStreamingMode()
{
    return false;
}

static bool IsZero(bits(N) value)
{
    return (value & value.create_mask()) == 0;
}

static meta_number Zeros(int size)
{
    return meta_number(0, size);
}

static meta_number Ones(int size)
{
    uint64_t value;

    if (size >= 64)
    {
        size = UINT64_MAX;
    }
    else
    {
        size = (1ULL << size) - 1;
    }

    return meta_number(value, size);
}

static bool IsOnes(bits(N) value)
{
    return (value & value.create_mask()) == value.create_mask();
}

static real Real(uint64_t value)
{
    return value;
}

static uint64_t UInt(bits(N) value)
{
    return value;
}

static void FPProcessException(uint64_t value, FPCR_Type fpcr)
{

}

static FPRounding FPDecodeRounding(bits(2) rmode)
{
    switch (rmode)
    {
        case 0b00: return FPRounding_TIEEVEN; // N
        case 0b01: return FPRounding_POSINF;  // P
        case 0b10: return FPRounding_NEGINF;  // M
        case 0b11: return FPRounding_ZERO;    // Z
    }
}

static FPRounding FPRoundingMode(FPCR_Type fpcr)
{
    return FPDecodeRounding(fpcr.RMode());
}

static std::tuple <FPType, bit, real> FPUnpackBase(bits(N) fpval, FPCR_Type fpcr_in, boolean fpexc, boolean isbfloat16)
{
    int N = fpval.size;

    assert_in(N,{16,32,64});

    FPCR_Type fpcr = fpcr_in;

    boolean altfp = IsFeatureImplemented(FEAT_AFP) && !UsingAArch32();
    boolean fiz   = altfp && fpcr.FIZ() == 1;
    boolean fz    = fpcr.FZ() == 1 && !(altfp && fpcr.AH() == 1);

    real value;
    bit sign;
    FPType fptype;

    if (N == 16 && !isbfloat16)
    {
        sign        = fpval.p(15);
        auto exp16  = fpval.p(14,10);
        auto frac16 = fpval.p(9,0);

        if (IsZero(exp16))
        {
            if (IsZero(frac16) || fpcr.FZ16() == 1)
            {
                fptype = FPType_Zero;  value = 0.0;
            }
            else
            {
                fptype = FPType_Denormal;  value = pow(2.0,-14) * (Real(UInt(frac16)) * pow(2.0,-10));
            }
        }
        else if (IsOnes(exp16) && fpcr.AHP() == 0)  // Infinity or NaN in IEEE format
        {
            if (IsZero(frac16))
            {
                fptype = FPType_Infinity;  value = pow(2.0,1000000);
            }
            else
            {
                fptype = frac16.p(9) == 1 ? FPType_QNaN : FPType_SNaN;
                value = 0.0;
            }
        }
        else
        {
            fptype = FPType_Nonzero;
            value = pow(2.0,(UInt(exp16)-15)) * (1.0 + Real(UInt(frac16)) * pow(2.0,-10));
        }
    }
    else if (N == 32 || isbfloat16)
    {
        bits(8) exp32;
        bits(23) frac32;
        if (isbfloat16)
        {
            sign   = fpval.p(15);
            exp32  = fpval.p(14,7);
            frac32 = concat(fpval.p(6,0), Zeros(16));
        }
        else
        {
            sign   = fpval.p(31);
            exp32  = fpval.p(30,23);
            frac32 = fpval.p(22,0);
        }

        if (IsZero(exp32))
        {
            if (IsZero(frac32))
            {
                // Produce zero if value is zero.
                fptype = FPType_Zero;  value = 0.0;
            }
            else if (fz || fiz)        // Flush-to-zero if FIZ==1 or AH,FZ==01
            {
                fptype = FPType_Zero;  value = 0.0;
                // Check whether to raise Input Denormal floating-point exception.
                // fpcr.FIZ==1 does not raise Input Denormal exception.
                if (fz) 
                {
                    // Denormalized input flushed to zero
                    if (fpexc) FPProcessException(FPExc_InputDenorm, fpcr);
                }
            }
            else
            {
                fptype = FPType_Denormal;  value = pow(2.0,-126) * (Real(UInt(frac32)) * pow(2.0,-23));
            }
        }
        else if (IsOnes(exp32))
        {
            if (IsZero(frac32))
            {
                fptype = FPType_Infinity;  value = pow(2.0,1000000);
            }
            else
            {
                fptype = frac32.p(22) == 1 ? FPType_QNaN : FPType_SNaN;
                value = 0.0;
            }
        }
        else
        {
            fptype = FPType_Nonzero;
            value = pow(2.0,(UInt(exp32)-127)) * (1.0 + Real(UInt(frac32)) * pow(2.0,-23));
        }
    }
    else
    {
        sign        = fpval.p(63);
        auto exp64  = fpval.p(62,52);
        auto frac64 = fpval.p(51,0);

        if (IsZero(exp64))
        {
            if (IsZero(frac64))
            {
                // Produce zero if value is zero.
                fptype = FPType_Zero;  value = 0.0;
            }
            else if (fz || fiz)        // Flush-to-zero if FIZ==1 or AH,FZ==01
            {
                fptype = FPType_Zero;  value = 0.0;
                // Check whether to raise Input Denormal floating-point exception.
                // fpcr.FIZ==1 does not raise Input Denormal exception.
                if (fz)
                {
                    // Denormalized input flushed to zero
                    if (fpexc) FPProcessException(FPExc_InputDenorm, fpcr);
                }
            }
            else
            {
                fptype = FPType_Denormal;  value = pow(2.0,-1022) * (Real(UInt(frac64)) * pow(2.0,-52));
            }
        }
        else if (IsOnes(exp64))
        {
            if (IsZero(frac64))
            {
                fptype = FPType_Infinity;  value = pow(2.0,1000000);
            }
            else
            {
                fptype = frac64.p(51) == 1 ? FPType_QNaN : FPType_SNaN;
                value = 0.0;
            }
        }
        else
        {
            fptype = FPType_Nonzero;
            value = pow(2.0,(UInt(exp64)-1023)) * (1.0 + Real(UInt(frac64)) * pow(2.0,-52));
        }
    }

    if (sign == 1)
    {
        value = -value;
    }

    return {fptype, sign, value};
}

static std::tuple<FPType, bit, real> FPUnpackBase(bits(N) fpval, FPCR_Type fpcr, boolean fpexc)
{
    boolean isbfloat16 = FALSE;
    auto [fp_type, sign, value] = FPUnpackBase(fpval, fpcr, fpexc, isbfloat16);
    return {fp_type, sign, value};
}

static std::tuple<FPType, bit, real> FPUnpack(bits(N) fpval, FPCR_Type fpcr_in, boolean fpexc)
{
    FPCR_Type fpcr = fpcr_in;
    fpcr.AHP(0);
    auto [fp_type, sign, value] = FPUnpackBase(fpval, fpcr, fpexc);
    return {fp_type, sign, value};
}

static std::tuple<FPType, bit, real> FPUnpack(bits(N) fpval, FPCR_Type fpcr_in)
{
    FPCR_Type fpcr = fpcr_in;
    fpcr.AHP(0);
    boolean fpexc = TRUE;   // Generate floating-point exceptions
    auto [fp_type, sign, value] = FPUnpackBase(fpval, fpcr, fpexc);
    return {fp_type, sign, value};
}

static bits(N) FPDefaultNaN(FPCR_Type fpcr, integer N)
{
    assert_in(N , {16,32,64});
    integer E = (N == 16 ? 5 : N == 32 ? 8 : 11);
    integer F = N - (E + 1);
    bit sign = IsFeatureImplemented(FEAT_AFP) && !UsingAArch32() ? fpcr.AH() : 0;

    bits(E) exp  = Ones(E);
    bits(F) frac = concat (meta_number(1, 1),Zeros(F-1));
    return concat(concat(sign, exp),frac);
}

static bits(N) FPInfinity(bit sign, integer N)
{
    assert_in(N , {16,32,64});
    integer E = (N == 16 ? 5 : N == 32 ? 8 : 11);
    integer F = N - (E + 1);
    bits(E) exp  = Ones(E);
    bits(F) frac = Zeros(F);
    return concat(sign, exp , frac);
}

static bits(N) FPZero(bit sign, integer N)
{
    assert_in(N ,{16,32,64});
    integer E = (N == 16 ? 5 : N == 32 ? 8 : 11);
    integer F = N - (E + 1);
    auto exp  = Zeros(E);
    auto frac = Zeros(F);
    auto result = concat(sign, exp, frac);
    return result;
}

static bits(N) FPProcessNaN(FPType fptype, bits(N) op, FPCR_Type fpcr, boolean fpexc)
{
    int N = op.size;

    assert_in(N, {16,32,64});
    assert_in(fptype ,{FPType_QNaN, FPType_SNaN});
    integer topfrac;

    switch (N) 
    {
        case 16: topfrac =  9; break;
        case 32: topfrac = 22; break;
        case 64: topfrac = 51; break;
    }

    auto result = op;
    if (fptype == FPType_SNaN)
    {
        result.set_p(topfrac,1);
        if (fpexc)
        {
            FPProcessException(FPExc_InvalidOp, fpcr);
        } 
    }
    if (fpcr.DN() == 1)  // DefaultNaN requested
    {
        result = FPDefaultNaN(fpcr, N);
    }
    return result;
}

static std::tuple<boolean, bits(N)> FPProcessNaNs(FPType type1, FPType type2, bits(N) op1, bits(N) op2, FPCR_Type fpcr, boolean fpexc)
{
    int N = op1.size;

    assert_in(N,{16,32,64});
    boolean done;
    bits(N) result;
    boolean altfp    = IsFeatureImplemented(FEAT_AFP) && !UsingAArch32() && fpcr.AH() == 1;
    boolean op1_nan  = in(type1,{FPType_SNaN, FPType_QNaN});
    boolean op2_nan  = in(type2,{FPType_SNaN, FPType_QNaN});
    boolean any_snan = type1 == FPType_SNaN || type2 == FPType_SNaN;
    FPType  type_nan = any_snan ? FPType_SNaN : FPType_QNaN;

    if (altfp && op1_nan && op2_nan )
    {
        // <n> register NaN selected
        done = TRUE;  result = FPProcessNaN(type_nan, op1, fpcr, fpexc);
    }
    else if (type1 == FPType_SNaN )
    {
        done = TRUE;  result = FPProcessNaN(type1, op1, fpcr, fpexc);
    }
    else if (type2 == FPType_SNaN )
    {
        done = TRUE;  result = FPProcessNaN(type2, op2, fpcr, fpexc);
    }
    else if (type1 == FPType_QNaN )
    {
        done = TRUE;  result = FPProcessNaN(type1, op1, fpcr, fpexc);
    }
    else if (type2 == FPType_QNaN )
    {
        done = TRUE;  result = FPProcessNaN(type2, op2, fpcr, fpexc);
    }
    else
    {
        done = FALSE;  result = Zeros(N);  // 'Don't care' result
    }

    return {done, result};
}

static std::tuple<boolean, bits(N)> FPProcessNaNs(FPType type1, FPType type2, bits(N) op1, bits(N) op2, FPCR_Type fpcr)
{
    boolean fpexc = TRUE;     // Generate floating-point exceptions
    return FPProcessNaNs(type1, type2, op1, op2, fpcr, fpexc);
}

static FPBitsType FPBits(integer N, boolean isbfloat16)
{
    FPFracBits F;
    integer minimum_exp;
    if (N == 16 )
    {
        minimum_exp = -14;   F = 10;
    }
    else if (N == 32 && isbfloat16 )
    {
        minimum_exp = -126;  F = 7;
    }
    else if (N == 32 )
    {
        minimum_exp = -126;  F = 23;
    }
    else  // N == 64
    {
        minimum_exp = -1022; F = 52;
    }

    return {F, minimum_exp};
}

static std::tuple<real, integer> NormalizeReal(real x)
{
    real mantissa = x;
    integer exponent = 0;
    while (mantissa < 1.0)
    {
        mantissa = mantissa * 2.0;  exponent = exponent - 1;
    }
    while (mantissa >= 2.0)
    {
        mantissa = mantissa / 2.0;  exponent = exponent + 1;
    }
    return {mantissa, exponent};
}

static meta_number RoundDown(real x)
{
    return meta_number(floor(x));
}

static real Max(real l, real r)
{
    if (l >= r)
        return l;

    return r;
}

static meta_number Max_i(meta_number l, meta_number r)
{
    if (l >= r)
        return l;

    return r;
}

static bits(N) FPMaxNormal(bit sign, integer N)
{
    assert_in(N, {16,32,64});
    integer E = (N == 16 ? 5 : N == 32 ? 8 : 11);
    integer F = N - (E + 1);
    auto exp  = concat(Ones(E-1), meta_number(0, 1));
    auto frac = Ones(F);
    return concat(sign , exp , frac);
}

static bits(N) FPRoundBase(real op, FPCR_Type fpcr, FPRounding rounding, boolean isbfloat16, boolean fpexc, boolean satoflo, integer N)
{
    assert_in(N,{16,32,64});
    assert(op != 0.0);
    assert(rounding != FPRounding_TIEAWAY);
    bits(N) result;

    // Obtain format parameters - minimum exponent, numbers of exponent and fraction bits.
    auto [F, minimum_exp] = FPBits(N, isbfloat16);
    auto zeros = N == 32 && isbfloat16 ? 16 : 0;
    auto E = N - (F + 1 + zeros);
    // Split value into sign, unrounded mantissa and exponent.
    bit sign;
    integer exponent;
    real mantissa;
    if (op < 0.0)
    {
        sign = 1;  mantissa = -op;
    }
    else
    {
        sign = 0;  mantissa = op;
    }
    
    auto tmp = NormalizeReal(mantissa);

    mantissa = std::get<0>(tmp);
    exponent = std::get<1>(tmp);

    // When TRUE, detection of underflow occurs after rounding and the test for a
    // denormalized number for single and double precision values occurs after rounding.
    auto altfp = IsFeatureImplemented(FEAT_AFP) && !UsingAArch32() && fpcr.AH() == 1;

    // Deal with flush-to-zero before rounding if FPCR.AH != 1.
    if (!altfp && ((fpcr.FZ() == 1 && N != 16) || (fpcr.FZ16() == 1 && N == 16)) && exponent < minimum_exp)
    {
        // Flush-to-zero never generates a trapped exception.
        if (UsingAArch32()) 
        {
            FPSCR.UFC(1);
        }
        else
        {
            if (fpexc)
            {
                FPSR.UFC(1);
            }
        }
        return FPZero(sign, N);
    }

    auto biased_exp_unconstrained = (exponent - minimum_exp) + 1;
    auto int_mant_unconstrained = RoundDown(mantissa * pow(2.0,F));
    auto error_unconstrained = mantissa * pow(2.0,F) - Real(int_mant_unconstrained);
    // Start creating the exponent value for the result. Start by biasing the actual exponent
    // so that the minimum exponent becomes 1, lower values 0 (indicating possible underflow).
    auto biased_exp = Max_i((exponent - minimum_exp) + 1, 0);
    if (biased_exp == 0) mantissa = mantissa / pow(2.0,(minimum_exp - exponent));

    // Get the unrounded mantissa as an integer, and the "units in last place" rounding error.
    auto int_mant = RoundDown(mantissa * pow(2.0,F));  // < 2.0^F if biased_exp == 0, >= 2.0^F if not
    auto error = mantissa * pow(2.0,F) - Real(int_mant);

    // Underflow occurs if exponent is too small before rounding, and result is inexact or
    // the Underflow exception is trapped. This applies before rounding if FPCR.AH != 1.
    boolean trapped_UF = fpcr.UFE() == 1 && (!InStreamingMode() || IsFullA64Enabled());
    if (altfp && biased_exp == 0 && (error != 0.0 || trapped_UF))
    {
        if (fpexc) FPProcessException(FPExc_Underflow, fpcr);
    }

    // Round result according to rounding mode.
    boolean round_up_unconstrained;
    boolean round_up;
    boolean overflow_to_inf;

    if (altfp)
    {
        switch (rounding)
        {
            case FPRounding_TIEEVEN:
                round_up_unconstrained = (error_unconstrained > 0.5 ||
                   (error_unconstrained == 0.5 && int_mant_unconstrained.p(0) == 1));
                round_up = (error > 0.5 || (error == 0.5 && int_mant.p(0) == 1));
                overflow_to_inf = !satoflo;
            break;
            case FPRounding_POSINF:
                round_up_unconstrained = (error_unconstrained != 0.0 && sign == 0);
                round_up = (error != 0.0 && sign == 0);
                overflow_to_inf = (sign == 0);
            break;
            case FPRounding_NEGINF:
                round_up_unconstrained = (error_unconstrained != 0.0 && sign == 1);
                round_up = (error != 0.0 && sign == 1);
                overflow_to_inf = (sign == 1);
            break;
            case FPRounding_ZERO:
            case FPRounding_ODD:
                round_up_unconstrained = FALSE;
                round_up = FALSE;
                overflow_to_inf = FALSE;
            break;
        }

        if (round_up_unconstrained)
        {
            int_mant_unconstrained = int_mant_unconstrained + 1;
            if (int_mant_unconstrained == pow(2,(F+1)))    // Rounded up to next exponent
            {
                biased_exp_unconstrained = biased_exp_unconstrained + 1;
                int_mant_unconstrained   = int_mant_unconstrained / 2;
            }
        }

        // Deal with flush-to-zero and underflow after rounding if FPCR.AH == 1.
        if (biased_exp_unconstrained < 1 && int_mant_unconstrained != 0)
        {
            // the result of unconstrained rounding is less than the minimum normalized number
            if ((fpcr.FZ() == 1 && N != 16) || (fpcr.FZ16() == 1 && N == 16))   // Flush-to-zero
            {
                if (fpexc)
                {
                    FPSR.UFC(1);
                    FPProcessException(FPExc_Inexact, fpcr);
                }
                return FPZero(sign, N);
            }
            else if (error != 0.0 || trapped_UF)
            {
                if (fpexc) FPProcessException(FPExc_Underflow, fpcr);
            }
        }
    }
    else    // altfp == FALSE
    {
        switch (rounding)
        {
            case FPRounding_TIEEVEN:
                round_up = (error > 0.5 || (error == 0.5 && int_mant.p(0) == 1));
                overflow_to_inf = !satoflo;
                break;
            case FPRounding_POSINF:
                round_up = (error != 0.0 && sign == 0);
                overflow_to_inf = (sign == 0);
                break;
            case FPRounding_NEGINF:
                round_up = (error != 0.0 && sign == 1);
                overflow_to_inf = (sign == 1);
                break;
            case FPRounding_ZERO:
            case FPRounding_ODD:
                round_up = FALSE;
                overflow_to_inf = FALSE;
                break;
        }
    }

    if (round_up)
    {
        int_mant = int_mant + 1;
        if (int_mant == pow(2,F))       // Rounded up from denormalized to normalized
        {
            biased_exp = 1;
        }

        if (int_mant == pow(2,(F+1)))  // Rounded up to next exponent
        {
            biased_exp = biased_exp + 1;
            int_mant = int_mant / 2;
        }
    }

    // Handle rounding to odd
    if (error != 0.0 && rounding == FPRounding_ODD && int_mant.p(0) == 0)
    {
        int_mant = int_mant + 1;
    }

    // Deal with overflow and generate result.
    if (N != 16 || fpcr.AHP() == 0)  // Single, double or IEEE half precision
    {
        if (biased_exp >= pow(2,E) - 1)
        {
            result = overflow_to_inf ? FPInfinity(sign, N) : FPMaxNormal(sign, N);
            if (fpexc) FPProcessException(FPExc_Overflow, fpcr);
            error = 1.0;  // Ensure that an Inexact exception occurs
        }
        else
        {
            result = concat(sign, biased_exp.p(E-1,0) , int_mant.p(F-1,0) , Zeros(N-(E+F+1)));
        }
    }
    else                                    // Alternative half precision
    {
        if (biased_exp >= pow(2,E))
        {
            result = concat(sign , Ones(N-1));
            if (fpexc) FPProcessException(FPExc_InvalidOp, fpcr);
            error = 0.0;  // Ensure that an Inexact exception does not occur
        } 
        else
        {
            result = concat(sign , biased_exp.p(E-1,0) , int_mant.p(F-1,0) , Zeros(N-(E+F+1)));
        }
    }

    // Deal with Inexact exception.
    if (error != 0.0)
    {
        if (fpexc) FPProcessException(FPExc_Inexact, fpcr);
    }

    return result;
}

static bits(N) FPRoundBase(real op, FPCR_Type fpcr, FPRounding rounding, boolean isbfloat16, boolean fpexc, integer N)
{
    boolean satoflo = FALSE;
    return FPRoundBase(op, fpcr, rounding, isbfloat16, fpexc, satoflo, N);
}

static bits(N) FPRound(real op, FPCR_Type fpcr_in, FPRounding rounding, boolean fpexc, integer N)
{
    FPCR_Type fpcr = fpcr_in;
    fpcr.AHP(0);
    boolean isbfloat16 = FALSE;
    return FPRoundBase(op, fpcr, rounding, isbfloat16, fpexc, N);
}

static bits(N) FPRound(real op, FPCR_Type fpcr_in, FPRounding rounding, integer N)
{
    boolean fpexc = TRUE;    // Generate floating-point exceptions
    return FPRound(op, fpcr_in, rounding, fpexc, N);
}

static bits(N) FPRound(real op, FPCR_Type fpcr, integer N)
{
    return FPRound(op, fpcr, FPRoundingMode(fpcr), N);
}

static void FPProcessDenorms(FPType type1, FPType type2, integer N, FPCR_Type fpcr)
{
    boolean altfp = IsFeatureImplemented(FEAT_AFP) && !UsingAArch32() && fpcr.AH() == 1;
    if (altfp && N != 16 && (type1 == FPType_Denormal || type2 == FPType_Denormal))
    {
        FPProcessException(FPExc_InputDenorm, fpcr);
    }
}

static bits(N) FPAdd(bits(N) op1, bits(N) op2, FPCR_Type fpcr, boolean fpexc)
{
    int N = op1.size;

    assert_in(N, {16,32,64});
    auto rounding = FPRoundingMode(fpcr);

    auto [type1,sign1,value1] = FPUnpack(op1, fpcr, fpexc);
    auto [type2,sign2,value2] = FPUnpack(op2, fpcr, fpexc);

    auto [done,result] = FPProcessNaNs(type1, type2, op1, op2, fpcr, fpexc);

    if (!done)
    {
        auto inf1  = (type1 == FPType_Infinity);  auto inf2  = (type2 == FPType_Infinity);
        auto zero1 = (type1 == FPType_Zero);      auto zero2 = (type2 == FPType_Zero);
        if (inf1 && inf2 && sign1 == NOT(sign2))
        {
            result = FPDefaultNaN(fpcr, N);
            if (fpexc) FPProcessException(FPExc_InvalidOp, fpcr);
        }
        else if ((inf1 && sign1 == 0) || (inf2 && sign2 == 0))
        {
            result = FPInfinity(0, N);
        }
        else if ((inf1 && sign1 == 1) || (inf2 && sign2 == 1))
        {
            result = FPInfinity(1, N);
        }
        else if (zero1 && zero2 && sign1 == sign2)
        {
            result = FPZero(sign1, N);
        }
        else
        {
            auto result_value = value1 + value2;
            if (result_value == 0.0)  // Sign of exact zero result depends on rounding mode
            {
                auto result_sign = rounding == FPRounding_NEGINF ? 1 : 0;
                result = FPZero(result_sign, N);
            }
            else
            {
                result = FPRound(result_value, fpcr, rounding, fpexc, N);
            }
        }

        if (fpexc)
        {
            FPProcessDenorms(type1, type2, N, fpcr);
        }
    }
    return result;
}

static bits(N) FPMul(bits(N) op1, bits(N) op2, FPCR_Type fpcr)
{
    int N = op1.size;

    assert_in(N, {16,32,64});
    auto [type1,sign1,value1] = FPUnpack(op1, fpcr);
    auto [type2,sign2,value2] = FPUnpack(op2, fpcr);
    auto [done,result] = FPProcessNaNs(type1, type2, op1, op2, fpcr);
    if (done)
    {
        auto inf1 = (type1 == FPType_Infinity);
        auto inf2 = (type2 == FPType_Infinity);
        auto zero1 = (type1 == FPType_Zero);
        auto zero2 = (type2 == FPType_Zero);

        if ((inf1 && zero2) || (zero1 && inf2))
        {
            result = FPDefaultNaN(fpcr, N);
            FPProcessException(FPExc_InvalidOp, fpcr);
        }
        else if (inf1 || inf2)
        {
            result = FPInfinity(sign1 ^ sign2, N);
        }
        else if (zero1 || zero2)
        {
            result = FPZero(sign1 ^ sign2, N);
        }
        else
        {
            result = FPRound(value1*value2, fpcr, N);
        }

        FPProcessDenorms(type1, type2, N, fpcr);
    }

    return result;
}

#endif