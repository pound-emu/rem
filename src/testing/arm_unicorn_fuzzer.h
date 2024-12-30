#ifndef ARM_UNICORN_FUZZER_H
#define ARM_UNICORN_FUZZER_H

#include "unicorn/headers/unicorn.h"
#include "capstone/headers/capstone/aarch64.h"
#include "emulator/aarch64/aarch64_process.h"


struct vec128
{
    uint64_t d0;
    uint64_t d1;

    bool operator == (vec128 other)
    {
        return (d0 == other.d0) && (d1 == other.d1);
    }

    bool operator != (vec128 other)
    {
        return !(*this == other);
    }
};

struct arm64_context
{
    uint64_t    x[32];
    vec128      q[32];

    uint64_t    memory;

    uint64_t    n, z, c, v;
};

struct arm_unicorn_fuzzer
{
    uc_engine*              uc;
    std::vector<uint8_t>    test_memory;
    
    jit_context             my_jit_context;

    arm64_context           debug_arm_interpreted_function;
    arm64_context           debug_arm_jited_function;

    static void             create(arm_unicorn_fuzzer* result);
    static void             destroy(arm_unicorn_fuzzer* to_destroy);

    static void             emit_guest_instruction(arm_unicorn_fuzzer* context,  uint32_t instruction);  
    static void             validate_context(arm_unicorn_fuzzer* context, arm64_context test);

    static void             execute_code(arm_unicorn_fuzzer* context, uint64_t instruction_count);
};

static bool skip_uneeded;

static bool skip_instruction(uint32_t instruction, bool check_uniqe = false)
{
    if (!skip_uneeded && !check_uniqe)
        return false;

    if (((instruction >> 25) & 0b101) == 0b100)
        return true;

    if (((instruction >> 26) & 0b111) == 0b101)
        return true;

    return false;
}

static bool is_valid_instruction(uint32_t instruction)
{
    //ldp stp undefined behavior
    if (((instruction >> 22) & 0b11111001) == 0b10100001)
    {
        int t2 = (instruction >> 10) & 31;
        int n = (instruction >> 5) & 31;
        int t = (instruction & 31);

        if ((instruction >> 26) & 1)
            return true;

        if (n == 31)
            return true;

        if (n == t2 || n == t || t2 == t)
            return false;
    }

    return true;
}

#endif