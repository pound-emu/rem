#ifndef AARCH64_SOFT_FLOAT_H
#define AARCH64_SOFT_FLOAT_H

#include <tuple>
#include <inttypes.h>
#include <initializer_list>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <exception>

typedef bool            boolean;
typedef double          real;
typedef int64_t         integer; 

#define bits(N) meta_number
#define FALSE   false
#define TRUE    true

#define FPFracBits  integer
#define FPBitsType  std::tuple<FPFracBits,integer>

template <typename T, typename S>
static T convert(S src)
{
    return *(T*)&src;
}

//Stolen from https://stackoverflow.com/questions/76799117/how-to-convert-a-float-to-a-half-type-and-the-other-way-around-in-c
static uint32_t float_as_uint32 (float a)
{
    uint32_t r;
    memcpy (&r, &a, sizeof r);
    return r;
}

static uint16_t float2half_rn (float a)
{
    uint32_t ia = float_as_uint32 (a);
    uint16_t ir;

    ir = (ia >> 16) & 0x8000;
    if ((ia & 0x7f800000) == 0x7f800000) {
        if ((ia & 0x7fffffff) == 0x7f800000) {
            ir |= 0x7c00; /* infinity */
        } else {
            ir |= 0x7e00 | ((ia >> (24 - 11)) & 0x1ff); /* NaN, quietened */
        }
    } else if ((ia & 0x7f800000) >= 0x33000000) {
        int shift = (int)((ia >> 23) & 0xff) - 127;
        if (shift > 15) {
            ir |= 0x7c00; /* infinity */
        } else {
            ia = (ia & 0x007fffff) | 0x00800000; /* extract mantissa */
            if (shift < -14) { /* denormal */  
                ir |= ia >> (-1 - shift);
                ia = ia << (32 - (-1 - shift));
            } else { /* normal */
                ir |= ia >> (24 - 11);
                ia = ia << (32 - (24 - 11));
                ir = ir + ((14 + shift) << 10);
            }
            /* IEEE-754 round to nearest of even */
            if ((ia > 0x80000000) || ((ia == 0x80000000) && (ir & 1))) {
                ir++;
            }
        }
    }
    return ir;
}

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

static real to_float(uint64_t source, int size)
{
    source = source & ((1ULL << (size - 1)) - 1);
    
    switch (size)
    {
        case 32: return *(float*)&source;
        case 64: return *(double*)&source;
        default: throw 0;
    }
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
        size = 64;
    }

    meta_number(uint64_t value)
    {
        this->value = value;
        this->size = 64;
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

struct bit
{
    bool value;

    operator uint64_t()
    {
        return value;
    }
    
    operator meta_number()
    {
        return meta_number(value != 0, 1);
    }

    bit()
    {
        value = 0;
    }

    bit(uint64_t value)
    {
        this->value = value;
    }

    bit(meta_number value)
    {
        assert(value.size == 1);

        this->value = value;
    }
};

static meta_number concat(meta_number left, meta_number right)
{
    meta_number result = meta_number(((left.value & left.create_mask()) << right.size) | (right.value & right.create_mask()), left.size + right.size);

    if (result.size > 64)
    {
        throw 0;
    }

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
    void FIZ(uint64_t new_value) { value = (value & ~1ULL) | ( 1ULL & (new_value << 0)); }
    uint64_t AH() { return (value >> 1) & 1ULL; }
    void AH(uint64_t new_value) { value = (value & ~2ULL) | ( 2ULL & (new_value << 1)); }
    uint64_t NEP() { return (value >> 2) & 1ULL; }
    void NEP(uint64_t new_value) { value = (value & ~4ULL) | ( 4ULL & (new_value << 2)); }
    uint64_t RES0_0() { return (value >> 3) & 31ULL; }
    void RES0_0(uint64_t new_value) { value = (value & ~248ULL) | ( 248ULL & (new_value << 3)); }
    uint64_t IOE() { return (value >> 8) & 1ULL; }
    void IOE(uint64_t new_value) { value = (value & ~256ULL) | ( 256ULL & (new_value << 8)); }
    uint64_t DZE() { return (value >> 9) & 1ULL; }
    void DZE(uint64_t new_value) { value = (value & ~512ULL) | ( 512ULL & (new_value << 9)); }
    uint64_t OFE() { return (value >> 10) & 1ULL; }
    void OFE(uint64_t new_value) { value = (value & ~1024ULL) | ( 1024ULL & (new_value << 10)); }
    uint64_t UFE() { return (value >> 11) & 1ULL; }
    void UFE(uint64_t new_value) { value = (value & ~2048ULL) | ( 2048ULL & (new_value << 11)); }
    uint64_t IXE() { return (value >> 12) & 1ULL; }
    void IXE(uint64_t new_value) { value = (value & ~4096ULL) | ( 4096ULL & (new_value << 12)); }
    uint64_t EBF() { return (value >> 13) & 1ULL; }
    void EBF(uint64_t new_value) { value = (value & ~8192ULL) | ( 8192ULL & (new_value << 13)); }
    uint64_t RES0_1() { return (value >> 14) & 1ULL; }
    void RES0_1(uint64_t new_value) { value = (value & ~16384ULL) | ( 16384ULL & (new_value << 14)); }
    uint64_t IDE() { return (value >> 15) & 1ULL; }
    void IDE(uint64_t new_value) { value = (value & ~32768ULL) | ( 32768ULL & (new_value << 15)); }
    uint64_t Len() { return (value >> 16) & 7ULL; }
    void Len(uint64_t new_value) { value = (value & ~458752ULL) | ( 458752ULL & (new_value << 16)); }
    uint64_t FZ16() { return (value >> 19) & 1ULL; }
    void FZ16(uint64_t new_value) { value = (value & ~524288ULL) | ( 524288ULL & (new_value << 19)); }
    uint64_t Stride() { return (value >> 20) & 3ULL; }
    void Stride(uint64_t new_value) { value = (value & ~3145728ULL) | ( 3145728ULL & (new_value << 20)); }
    uint64_t RMode() { return (value >> 22) & 3ULL; }
    void RMode(uint64_t new_value) { value = (value & ~12582912ULL) | ( 12582912ULL & (new_value << 22)); }
    uint64_t FZ() { return (value >> 24) & 1ULL; }
    void FZ(uint64_t new_value) { value = (value & ~16777216ULL) | ( 16777216ULL & (new_value << 24)); }
    uint64_t DN() { return (value >> 25) & 1ULL; }
    void DN(uint64_t new_value) { value = (value & ~33554432ULL) | ( 33554432ULL & (new_value << 25)); }
    uint64_t AHP() { return (value >> 26) & 1ULL; }
    void AHP(uint64_t new_value) { value = (value & ~67108864ULL) | ( 67108864ULL & (new_value << 26)); }
};

struct FPSR_Type
{
    uint64_t value;

    uint64_t IOC() { return (value >> 0) & 1ULL; }
    void IOC(uint64_t new_value) { value = (value & ~1ULL) | ( 1ULL & (new_value << 0)); }
    uint64_t DZC() { return (value >> 1) & 1ULL; }
    void DZC(uint64_t new_value) { value = (value & ~2ULL) | ( 2ULL & (new_value << 1)); }
    uint64_t OFC() { return (value >> 2) & 1ULL; }
    void OFC(uint64_t new_value) { value = (value & ~4ULL) | ( 4ULL & (new_value << 2)); }
    uint64_t UFC() { return (value >> 3) & 1ULL; }
    void UFC(uint64_t new_value) { value = (value & ~8ULL) | ( 8ULL & (new_value << 3)); }
    uint64_t IXC() { return (value >> 4) & 1ULL; }
    void IXC(uint64_t new_value) { value = (value & ~16ULL) | ( 16ULL & (new_value << 4)); }
    uint64_t RES0_0() { return (value >> 5) & 3ULL; }
    void RES0_0(uint64_t new_value) { value = (value & ~96ULL) | ( 96ULL & (new_value << 5)); }
    uint64_t IDC() { return (value >> 7) & 1ULL; }
    void IDC(uint64_t new_value) { value = (value & ~128ULL) | ( 128ULL & (new_value << 7)); }
    uint64_t RES0_1() { return (value >> 8) & 524287ULL; }
    void RES0_1(uint64_t new_value) { value = (value & ~134217472ULL) | ( 134217472ULL & (new_value << 8)); }
    uint64_t QC() { return (value >> 27) & 1ULL; }
    void QC(uint64_t new_value) { value = (value & ~134217728ULL) | ( 134217728ULL & (new_value << 27)); }
    uint64_t V() { return (value >> 28) & 1ULL; }
    void V(uint64_t new_value) { value = (value & ~268435456ULL) | ( 268435456ULL & (new_value << 28)); }
    uint64_t C() { return (value >> 29) & 1ULL; }
    void C(uint64_t new_value) { value = (value & ~536870912ULL) | ( 536870912ULL & (new_value << 29)); }
    uint64_t Z() { return (value >> 30) & 1ULL; }
    void Z(uint64_t new_value) { value = (value & ~1073741824ULL) | ( 1073741824ULL & (new_value << 30)); }
    uint64_t N() { return (value >> 31) & 1ULL; }
    void N(uint64_t new_value) { value = (value & ~2147483648ULL) | ( 2147483648ULL & (new_value << 31)); }
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
        value = UINT64_MAX;
    }
    else
    {
        value = (1ULL << size) - 1;
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
            value = pow(2.0, (int64_t)(UInt(exp16)-15)) * (1.0 + Real(UInt(frac16)) * pow(2.0,-10));
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
                fptype = FPType_Denormal;  
                
                //value = pow(2.0,-126) * (Real(UInt(frac32)) * pow(2.0,-23));
                value = to_float(fpval, N);
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

            //value = pow(2.0,(UInt(exp32)-127)) * (1.0 + Real(UInt(frac32)) * pow(2.0,-23));
            value = to_float(fpval, N);
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
                fptype = FPType_Denormal;  

                //value = pow(2.0,-1022) * (Real(UInt(frac64)) * pow(2.0,-52));
                value = to_float(fpval, N);
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

            //value = pow(2.0,(UInt(exp64)-1023)) * (1.0 + Real(UInt(frac64)) * pow(2.0,-52));
            value = to_float(fpval, N);
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

static std::tuple<FPType, bit, real> FPUnpackCV(bits(N) fpval, FPCR_Type fpcr_in)
{
    FPCR_Type fpcr = fpcr_in;
    fpcr.FZ16(0);
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

static bits(N) FPProcessNaN(FPType fptype, bits(N) op, FPCR_Type fpcr)
{
    boolean fpexc = TRUE;   // Generate floating-point exceptions
    return FPProcessNaN(fptype, op, fpcr, fpexc);
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
        done = FALSE;  result = Zeros(N);  // Dont care result
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
    uint64_t result;

    switch (N)
    {
        case 16: { result = float2half_rn(op); } break;
        case 32: { float w = op; result = *(uint32_t*)&w; } break;
        case 64: { double w = op; result = *(uint64_t*)&w;} break;
    }

    if (N != 64)
    {
        result = result & ((1ULL << N) - 1);
    }

    return meta_number(result, N);
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

static void FPProcessDenorm(FPType fptype, integer N, FPCR_Type fpcr)
{
    boolean altfp = IsFeatureImplemented(FEAT_AFP) && !UsingAArch32() && fpcr.AH() == 1;
    if (altfp && N != 16 && fptype == FPType_Denormal)
        FPProcessException(FPExc_InputDenorm, fpcr);
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

    if (!done)
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

static bits(N) FPDiv(bits(N) op1, bits(N) op2, FPCR_Type fpcr)
{
    int N = op1.size;

    assert_in(N, {16,32,64});
    auto [type1,sign1,value1] = FPUnpack(op1, fpcr);
    auto [type2,sign2,value2] = FPUnpack(op2, fpcr);
    auto [done,result] = FPProcessNaNs(type1, type2, op1, op2, fpcr);

    if (!done)
    {
        auto inf1  = type1 == FPType_Infinity;
        auto inf2  = type2 == FPType_Infinity;
        auto zero1 = type1 == FPType_Zero;
        auto zero2 = type2 == FPType_Zero;

        if ((inf1 && inf2) || (zero1 && zero2))
        {
            result = FPDefaultNaN(fpcr, N);
            FPProcessException(FPExc_InvalidOp, fpcr);
        }
        else if (inf1 || zero2)
        {
            result = FPInfinity(sign1 ^ sign2, N);
            if (!inf1) FPProcessException(FPExc_DivideByZero, fpcr);
        }
        else if (zero1 || inf2)
        {
            result = FPZero(sign1 ^ sign2, N);
        }
        else
        {
            result = FPRound(value1/value2, fpcr, N);
        }

        if (!zero2)
        {
            FPProcessDenorms(type1, type2, N, fpcr);
        }
    }
    return result;
}

static bits(N) FPSub(bits(N) op1, bits(N) op2, FPCR_Type fpcr, boolean fpexc)
{
    int N = op1.size;

    assert_in(N, {16,32,64});
    auto rounding = FPRoundingMode(fpcr);

    auto [type1,sign1,value1] = FPUnpack(op1, fpcr, fpexc);
    auto [type2,sign2,value2] = FPUnpack(op2, fpcr, fpexc);

    auto [done,result] = FPProcessNaNs(type1, type2, op1, op2, fpcr, fpexc);
    if (!done)
    {
        auto inf1 = (type1 == FPType_Infinity);
        auto inf2 = (type2 == FPType_Infinity);
        auto zero1 = (type1 == FPType_Zero);
        auto zero2 = (type2 == FPType_Zero);

        if (inf1 && inf2 && sign1 == sign2)
        {
            result = FPDefaultNaN(fpcr, N);
            if (fpexc) FPProcessException(FPExc_InvalidOp, fpcr);
        }
        else if ((inf1 && sign1 == 0) || (inf2 && sign2 == 1))
        {
            result = FPInfinity(0, N);
        }
        else if ((inf1 && sign1 == 1) || (inf2 && sign2 == 0))
        {
            result = FPInfinity(1, N);
        }
        else if (zero1 && zero2 && sign1 == NOT(sign2))
        {
            result = FPZero(sign1, N);
        }
        else
        {
            auto result_value = value1 - value2;
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

static bits(N) FPMin(bits(N) op1, bits(N) op2, FPCR_Type fpcr_in, boolean altfp, boolean fpexc)
{
    int N = op1.size;
    assert_in(N, {16,32,64});
    FPCR_Type fpcr = fpcr_in;
    auto [type1,sign1,value1] = FPUnpack(op1, fpcr, fpexc);
    auto [type2,sign2,value2] = FPUnpack(op2, fpcr, fpexc);

    if ( altfp && type1 == FPType_Zero && type2 == FPType_Zero && sign1 != sign2 )
    {
        // Alternate handling of zeros with dif fering sign
        return FPZero(sign2, N);
    }
    else if ( altfp && (in(type1, {FPType_SNaN, FPType_QNaN}) || in(type2 , {FPType_SNaN, FPType_QNaN})) )
    {
        // Alternate handling of NaN inputs
        if ( fpexc ) FPProcessException(FPExc_InvalidOp, fpcr);
        return (( type2 == FPType_Zero ) ? FPZero(sign2, N) : op2);
    }

    auto [done,result] = FPProcessNaNs(type1, type2, op1, op2, fpcr, fpexc);

    if ( !done )
    {
        FPType fptype;
        bit sign;
        real value;
        FPRounding rounding;
        if ( value1 < value2 )
        {
            (fptype,sign,value) = (type1,sign1,value1);
        }
        else
        {
            (fptype,sign,value) = (type2,sign2,value2);
        }
        if ( fptype == FPType_Infinity )
        {
            result = FPInfinity(sign, N);
        }
        else if ( fptype == FPType_Zero )
        {
            sign = sign1 | sign2;              // Use most negative sign
            result = FPZero(sign, N);
        }
        else
        {
            // The use of FPRound() covers the case where there is a trapped underflow exception
            // for a denormalized number even though the result is exact.
            rounding = FPRoundingMode(fpcr);
            if ( altfp )    // Denormal output is not flushed to zero
            {
                fpcr.FZ (0);
                fpcr.FZ16 (0);
            }

            result = FPRound(value, fpcr, rounding, fpexc, N);
        }
    }

        if ( fpexc ) FPProcessDenorms(type1, type2, N, fpcr);
    return result;
}

static bits(N) FPMax(bits(N) op1, bits(N) op2, FPCR_Type fpcr_in, boolean altfp, boolean fpexc)
{
    int N = op1.size;
    assert_in(N, {16,32,64});
    FPCR_Type fpcr = fpcr_in;
    auto [type1,sign1,value1] = FPUnpack(op1, fpcr, fpexc);
    auto [type2,sign2,value2] = FPUnpack(op2, fpcr, fpexc);

    if (altfp && type1 == FPType_Zero && type2 == FPType_Zero && sign1 != sign2)
    {
        // Alternate handling of zeros with differing sign
        return FPZero(sign2, N);
    }
    else if (altfp && (in(type1, {FPType_SNaN, FPType_QNaN}) || in(type2,{FPType_SNaN, FPType_QNaN})))
    {
        // Alternate handling of NaN inputs
        if (fpexc) FPProcessException(FPExc_InvalidOp, fpcr);
        return (type2 == FPType_Zero ? FPZero(sign2, N) : op2);
    }

    auto [done,result] = FPProcessNaNs(type1, type2, op1, op2, fpcr, fpexc);
    
    if (!done)
    {
        FPType fptype;
        bit sign;
        real value;
        if (value1 > value2)
        {
            (fptype,sign,value) = (type1,sign1,value1);
        }
        else
        {
            (fptype,sign,value) = (type2,sign2,value2);
        }
        if (fptype == FPType_Infinity)
        {
            result = FPInfinity(sign, N);
        }
        else if (fptype == FPType_Zero)
        {
            sign = sign1 & sign2;         // Use most positive sign
            result = FPZero(sign, N);
        }
        else
        {
            // The use of FPRound() covers the case where there is a trapped underflow exception
            // for a denormalized number even though the result is exact.
            auto rounding = FPRoundingMode(fpcr);
            if (altfp)     // Denormal output is not flushed to zero
            {
                fpcr.FZ(0);
                fpcr.FZ16(0);
            }

            result = FPRound(value, fpcr, rounding, fpexc, N);
        }
        if (fpexc) FPProcessDenorms(type1, type2, N, fpcr);
    }

    return result;
}

static bits(N) FPMaxNum(bits(N) op1_in, bits(N) op2_in, FPCR_Type fpcr, boolean fpexc)
{
    int N = op1_in.size;

    assert_in(N, {16,32,64});
    bits(N) op1 = op1_in;
    bits(N) op2 = op2_in;
    auto [type1,_0,_1] = FPUnpack(op1, fpcr, fpexc);
    auto [type2,_2,_3] = FPUnpack(op2, fpcr, fpexc);

    boolean type1_nan = in(type1 , {FPType_QNaN, FPType_SNaN});
    boolean type2_nan = in(type2 , {FPType_QNaN, FPType_SNaN});
    boolean altfp = IsFeatureImplemented(FEAT_AFP) && !UsingAArch32() && fpcr.AH() == 1;

    if (!(altfp && type1_nan && type2_nan))
    {
        // Treat a single quiet-NaN as -Infinity.
        if (type1 == FPType_QNaN && type2 != FPType_QNaN)
        {
            op1 = FPInfinity(1, N);
        }
        else if (type1 != FPType_QNaN && type2 == FPType_QNaN)
        {
            op2 = FPInfinity(1, N);
        }
    }

    auto altfmaxfmin = FALSE;    // Restrict use of FMAX/FMIN NaN propagation rules
    auto result = FPMax(op1, op2, fpcr, altfmaxfmin, fpexc);

    return result;
}

static bits(N) FPMinNum(bits(N) op1_in, bits(N) op2_in, FPCR_Type fpcr, boolean fpexc)
{
    int N = op1_in.size;

    assert_in(N, {16,32,64});
    bits(N) op1 = op1_in;
    bits(N) op2 = op2_in;
    auto [type1,_0,_1] = FPUnpack(op1, fpcr, fpexc);
    auto [type2,_2,_3] = FPUnpack(op2, fpcr, fpexc);

    boolean type1_nan = in(type1, {FPType_QNaN, FPType_SNaN});
    boolean type2_nan = in(type2 , {FPType_QNaN, FPType_SNaN});
    boolean altfp = IsFeatureImplemented(FEAT_AFP) && !UsingAArch32() && fpcr.AH() == 1;

    if (!(altfp && type1_nan && type2_nan))
    {
        // Treat a single quiet-NaN as +Infinity.
        if (type1 == FPType_QNaN && type2 != FPType_QNaN)
            op1 = FPInfinity(0, N);
        else if (type1 != FPType_QNaN && type2 == FPType_QNaN)
            op2 = FPInfinity(0, N);
    }

    auto altfmaxfmin = FALSE;    // Restrict use of FMAX/FMIN NaN propagation rules
    auto result = FPMin(op1, op2, fpcr, altfmaxfmin, fpexc);

    return result;
}

static bits(N) FPSqrt(bits(N) op, FPCR_Type fpcr)
{
    int N = op.size;
    
    assert_in(N,{16,32,64});
    auto [fptype,sign,value] = FPUnpack(op, fpcr);

    bits(N) result;
    if (fptype == FPType_SNaN || fptype == FPType_QNaN)
    {
        result = FPProcessNaN(fptype, op, fpcr);
    }
    else if (fptype == FPType_Zero)
    {
        result = FPZero(sign, N);
    }
    else if (fptype == FPType_Infinity && sign == 0)
    {
        result = FPInfinity(sign, N);
    }
    else if (sign == 1)
    {
        result = FPDefaultNaN(fpcr, N);
        FPProcessException(FPExc_InvalidOp, fpcr);
    }
    else
    {
        integer prec;
        boolean inexact;
        if (N == 16)
        {
            prec = 12; // 10 fraction bit + 2
        }
        else if (N == 32)
        {
            prec = 25; // 23 fraction bits + 2
        }
        else // N == 64
        {
            prec = 54; // 52 fraction bits + 2
        }
        value = sqrt(value);
        result = FPRound(value, fpcr, N);
        if (inexact)
        {
            FPProcessException(FPExc_Inexact, fpcr);
        }
        FPProcessDenorm(fptype, N, fpcr);
    }

    return result;
}

static bits(N) FPNeg(bits(N) op, FPCR_Type fpcr)
{
    int N = op.size;

    assert_in(N, {16,32,64});
    if (!UsingAArch32() && IsFeatureImplemented(FEAT_AFP))
    {
        if (fpcr.AH() == 1)
        {
            auto [fptype, _0, _1] = FPUnpack(op, fpcr, FALSE);
            if (in(fptype, {FPType_SNaN, FPType_QNaN})) 
            {
                return op;        // When fpcr.AH=1, sign of NaN has no consequence
            }
        }
    }
    return concat(NOT(op.p(N-1)) , op.p(N-2,0));
}

static bits(N) FPAbs(bits(N) op, FPCR_Type fpcr)
{
    int N = op.size;

    assert_in(N, {16,32,64});
    if (!UsingAArch32() && IsFeatureImplemented(FEAT_AFP))
    {
        if (fpcr.AH() == 1)
        {
            auto [fptype, _0, _1] = FPUnpack(op, fpcr, FALSE);
            if (in(fptype, {FPType_SNaN, FPType_QNaN}))
            {
                return op;        // When fpcr.AH=1, sign of NaN has no consequence
            }
        }
    }
    return concat(meta_number(0, 1) , op.p(N-2,0));
}


static bool is_even(real source)
{
    real mod = fmod(source, 2);

    return mod == 0;
}

static bits(N) FixedToFP(bits(M) op, integer fbits, boolean is_unsigned, FPCR_Type fpcr, integer N)
{
    real result;

    if (is_unsigned)
    {
        uint64_t working = op.value & op.create_mask();

        result = working;
    }
    else
    {
        int64_t working = op.value;

        switch (op.size)
        {
            case 8: throw 0;
            case 16: working = (int16_t)working; break;
            case 32: working = (int32_t)working; break;
        }

        result = working;
    }

    result = result / pow(2.0, fbits);

    return FPRound(result, fpcr, N);
}

static bits(M) FPToFixed(bits(N) op, integer fbits, boolean is_unsigned, FPCR_Type fpcr, FPRounding rounding, integer M)
{
    auto [fptype,sign,value] = FPUnpack(op, fpcr);

    if (fptype == FPType_SNaN || fptype == FPType_QNaN)
    {
        FPProcessException(FPExc_InvalidOp, fpcr);
    }

    value = value * pow(2.0,fbits);
    auto int_result = floor(value);
    auto error = value - int_result;

    bool round_up;

    switch (rounding)
    {
        case FPRounding_TIEEVEN:
            round_up = (error > 0.5 || (error == 0.5 && !is_even(int_result)));
        break;
        case FPRounding_POSINF:
            round_up = (error != 0.0);
        break;
        case FPRounding_NEGINF:
            round_up = FALSE;
        break;
        case FPRounding_ZERO:
            round_up = (error != 0.0 && int_result < 0);
        break;
        case FPRounding_TIEAWAY:
            round_up = (error > 0.5 || (error == 0.5 && int_result >= 0));
        break;
    }

    if (round_up)
    {
        int_result += 1;
    }

    uint64_t result;

    if (is_unsigned)
    {
        uint64_t max = M == 16 ? UINT16_MAX : M == 32 ? UINT32_MAX : UINT64_MAX;
        uint64_t min = 0;

        if (int_result >= max)
        {
            result = max;
        }
        else if (int_result <= min)
        {
            result = min;
        }
        else
        {
            result = int_result;
        }
    }
    else
    {
        int64_t max = M == 16 ? INT16_MAX : M == 32 ? INT32_MAX : INT64_MAX;
        int64_t min = M == 16 ? INT16_MIN : M == 32 ? INT32_MIN : INT64_MIN;

        if (int_result >= max)
        {
            result = max;
        }
        else if (int_result <= min)
        {
            result = min;
        }
        else
        {
            result = int_result;
        }
    }

    if (M != 64)
    {
        result = result & ((1ULL << M) - 1);
    }

    return {result, M};
}

static bits(N) FPRoundBF(real op, FPCR_Type fpcr, FPRounding rounding, boolean fpexc, integer N)
{
    assert(N == 32);
    boolean isbfloat16 = TRUE;
    return FPRoundBase(op, fpcr, rounding, isbfloat16, fpexc, N);
}

static bits(M) FPConvertNaN(bits(N) op, integer M)
{
    int N = op.size;

    assert_in(N , {16,32,64});
    assert_in(M , {16,32,64});
    bits(M) result;
    bits(51) frac;

    auto sign = op.p(N-1);

    // Unpack payload from input NaN
    switch (N) 
    {
        case 64: frac = op.p(50,0);                     break;
        case 32: frac = concat (op.p(21,0),Zeros(29));  break;
        case 16: frac = concat (op.p(8,0),Zeros(42));   break;
    }

    // Repack payload into output NaN, while
    // converting an SNaN to a QNaN.
    switch (M) 
    {
        case 64: result = concat(sign,Ones(M-52),frac);             break;
        case 32: result = concat(sign,Ones(M-23),frac.p(50,29));    break;
        case 16: result = concat(sign,Ones(M-10),frac.p(50,42));    break;
    }

    return result;
}

static bits(N) FPRoundCV(real op, FPCR_Type fpcr_in, FPRounding rounding, integer N)
{
    FPCR_Type fpcr = fpcr_in;
    fpcr.FZ16(0);
    boolean fpexc = TRUE;    // Generate floating-point exceptions
    boolean isbfloat16 = FALSE;
    return FPRoundBase(op, fpcr, rounding, isbfloat16, fpexc, N);
}

static bits(M) FPConvert(bits(N) op, FPCR_Type fpcr, FPRounding rounding, integer M)
{
    int N = op.size;

    assert_in(M , {16,32,64});
    assert_in(N , {16,32,64});
    bits(M) result;

    // Unpack floating-point operand optionally with flush-to-zero.
    auto [fptype,sign,value] = FPUnpackCV(op, fpcr);

    auto alt_hp = (M == 16) && (fpcr.AHP() == 1);

    if (fptype == FPType_SNaN || fptype == FPType_QNaN)
    {
        if (alt_hp) 
        {
            result = FPZero(sign, M);
        }
        else if (fpcr.DN() == 1)
        {
            result = FPDefaultNaN(fpcr, M);
        }
        else
        {
            result = FPConvertNaN(op, M);
        }
        if (fptype == FPType_SNaN || alt_hp)
        {
            FPProcessException(FPExc_InvalidOp,fpcr);
        }
    }
    else if (fptype == FPType_Infinity)
    {
        if (alt_hp)
        {
            result = concat(sign,Ones(M-1));
            FPProcessException(FPExc_InvalidOp, fpcr);
        }
        else
        {
            result = FPInfinity(sign, M);
        }
    }
    else if (fptype == FPType_Zero)
    {
        result = FPZero(sign, M);
    }
    else
    {
        result = FPRoundCV(value, fpcr, rounding, M);
        FPProcessDenorm(fptype, N, fpcr);
    }

    return result;
}

static bits(4) FPCompare(bits(N) op1, bits(N) op2, boolean signal_nans, FPCR_Type fpcr)
{
    int N = op1.size;

    assert_in(N , {16,32,64});
    auto [type1,sign1,value1] = FPUnpack(op1, fpcr);
    auto [type2,sign2,value2] = FPUnpack(op2, fpcr);

    bits(4) result;
    if (in(type1, {FPType_SNaN, FPType_QNaN}) || in(type2, {FPType_SNaN, FPType_QNaN}))
    {
        result = 0b0011;
        if (type1 == FPType_SNaN || type2 == FPType_SNaN || signal_nans)
        {
            FPProcessException(FPExc_InvalidOp, fpcr);
        }
    }
    else
    {
        // All non-NaN cases can be evaluated on the values produced by FPUnpack()
        if (value1 == value2)
        {
            result = 0b0110;
        }
        else if (value1 < value2)
        {
            result = 0b1000;
        }
        else  // value1 > value2
        {
            result = 0b0010;
        }

        FPProcessDenorms(type1, type2, N, fpcr);
    }

    return result;
}

static uint64_t FPCompare_I(uint64_t op1, uint64_t op2, boolean signal_nans, uint64_t fpcr, uint64_t N)
{
    return FPCompare({op1, N}, {op2, N}, signal_nans, {fpcr});
}

static uint64_t FPAbs_I(uint64_t op, uint64_t fpcr, uint64_t N)
{
    return FPAbs({op, N}, {fpcr});
}

static uint64_t FPNeg_I(uint64_t op, uint64_t fpcr, uint64_t N)
{
    return FPNeg({op, N}, {fpcr});
}

static uint64_t FPSqrt_I(uint64_t op, uint64_t fpcr, uint64_t N)
{
    return FPSqrt({op, N}, {fpcr});
}

static uint64_t FPMinNum_I(uint64_t op1_in, uint64_t op2_in, uint64_t fpcr, uint64_t N)
{
    return FPMinNum({op1_in, N}, {op2_in, N}, {fpcr}, 0);
}

static uint64_t FPMaxNum_I(uint64_t op1_in, uint64_t op2_in, uint64_t fpcr, uint64_t N)
{
    return FPMaxNum({op1_in, N}, {op2_in, N}, {fpcr}, 0);
}

static uint64_t FPMax_I(uint64_t op1, uint64_t op2, uint64_t fpcr_in, uint64_t N)
{
    return FPMax({op1, N}, {op2, N}, {fpcr_in}, 0, 0);
}

static uint64_t FPMin_I(uint64_t op1, uint64_t op2, uint64_t fpcr_in, uint64_t N)
{
    return FPMin({op1, N}, {op2, N}, {fpcr_in}, 0, 0);
}

static uint64_t FPSub_I(uint64_t op1, uint64_t op2, uint64_t fpcr, uint64_t N)
{
    return FPSub({op1, N}, {op2, N}, {fpcr}, 0);
}

static uint64_t FPAdd_I(uint64_t op1, uint64_t op2, uint64_t fpcr, uint64_t N)
{
    return FPAdd({op1, N}, {op2, N}, {fpcr}, 0);
}

static uint64_t FPMul_I(uint64_t op1, uint64_t op2, uint64_t fpcr, uint64_t N)
{
    return FPMul({op1, N}, {op2, N}, {fpcr});
}

static uint64_t FPDiv_I(uint64_t op1, uint64_t op2, uint64_t fpcr, uint64_t N)
{
    return FPDiv({op1, N}, {op2, N}, {fpcr});
}

static uint64_t FixedToFP_I(uint64_t op, integer fbits, boolean is_unsigned, integer to, int from)
{
    return FixedToFP({op, from}, fbits, is_unsigned, {0}, to);
}

static uint64_t FPToFixed_I(uint64_t op, integer fbits, boolean is_unsigned, FPRounding rounding, integer to, int from)
{
    return FPToFixed({op, from}, fbits, is_unsigned, {0}, rounding, to);
}

static uint64_t FPConvert_I(uint64_t op, uint64_t fpcr, int rounding, int M, int N)
{
    return FPConvert({op, N}, {fpcr}, (FPRounding)rounding, M);
}

#endif