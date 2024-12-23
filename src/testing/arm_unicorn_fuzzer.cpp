#include "arm_unicorn_fuzzer.h"

static void* base_plus_va(void* context, uint64_t virtual_address)
{
    return (char*)context + virtual_address;
}

static void base_plus_va_jit(void* memory, ssa_emit_context* ir, ir_operand destination, ir_operand source)
{
    throw 0;
}

static void memrand(void* result, uint64_t size)
{
    srand(size);

    for (int i = 0; i < size; ++i)
    {
        ((char*)result)[i] = rand() | (rand() >> 1);
    }
}

static int transalte_uc_reg(int reg)
{
    switch (reg)
    {
        case UC_ARM64_REG_X0 + 29: return UC_ARM64_REG_FP;
        case UC_ARM64_REG_X0 + 30: return UC_ARM64_REG_LR;
        case UC_ARM64_REG_X0 + 31: return UC_ARM64_REG_SP;
    }

    return reg;
}

static void reset_registers(arm_unicorn_fuzzer* ctx)
{
    memrand(&ctx->debug_arm_context, sizeof(arm64_context));

    for (int i = 0; i < 32; ++i)
    {
        ctx->debug_arm_context.q[i].d0 = i;
    }

    for (int i = 0; i < 32; ++i)
    {
        uc_reg_write(ctx->uc, transalte_uc_reg(UC_ARM64_REG_X0 + i), &ctx->debug_arm_context.x[i]);
        uc_reg_write(ctx->uc, UC_ARM64_REG_Q0 + i, &ctx->debug_arm_context.q[i]);
    }

    ctx->debug_arm_context.n &= 1;
    ctx->debug_arm_context.z &= 1;
    ctx->debug_arm_context.c &= 1;
    ctx->debug_arm_context.v &= 1;

    uint64_t nzcv = 
        ((ctx->debug_arm_context.n & 1) << 31) | 
        ((ctx->debug_arm_context.z & 1) << 30) | 
        ((ctx->debug_arm_context.c & 1) << 29) | 
        ((ctx->debug_arm_context.v & 1) << 28);

    assert(uc_reg_write(ctx->uc, UC_ARM64_REG_PSTATE, &nzcv) == UC_ERR_OK);
}

void arm_unicorn_fuzzer::create(arm_unicorn_fuzzer* result)
{
    uc_open(UC_ARCH_ARM64, UC_MODE_LITTLE_ENDIAN,&result->uc);

    jit_context::create(&result->my_jit_context, 1ULL * 1024 * 1024 * 1024, get_abi());
}

void arm_unicorn_fuzzer::destroy(arm_unicorn_fuzzer* to_destroy)
{
    jit_context::destroy(&to_destroy->my_jit_context);
    
    uc_close(to_destroy->uc);
}

void arm_unicorn_fuzzer::emit_guest_instruction(arm_unicorn_fuzzer* context,  uint32_t instruction)
{
    for (int i = 0; i < 4; ++i)
    {
        context->test_memory.push_back(((char*)&instruction)[i]);
    }
}

void arm_unicorn_fuzzer::validate_context(arm_unicorn_fuzzer* context)
{
    for (int i = 0; i < 32; ++i)
    {
        uint64_t    test_x;
        vec128      test_q;

        assert(uc_reg_read(context->uc, transalte_uc_reg(UC_ARM64_REG_X0 + i), &test_x) == UC_ERR_OK);
        assert(uc_reg_read(context->uc, UC_ARM64_REG_Q0 + i, &test_q) == UC_ERR_OK);

        assert(test_x == context->debug_arm_context.x[i]);
        assert(test_q == context->debug_arm_context.q[i]);
    }

    uint64_t nzcv;

    assert(uc_reg_read(context->uc, UC_ARM64_REG_PSTATE, &nzcv) == UC_ERR_OK);

    assert(((nzcv >> 31) & 1) == context->debug_arm_context.n);
    assert(((nzcv >> 30) & 1) == context->debug_arm_context.z);
    assert(((nzcv >> 29) & 1) == context->debug_arm_context.c);
    assert(((nzcv >> 28) & 1) == context->debug_arm_context.v);
}

void arm_unicorn_fuzzer::execute_code(arm_unicorn_fuzzer* context, uint64_t instruction_count)
{
    srand(0);

    reset_registers(context);

    while (context->test_memory.size() & 4095)
    {
        context->test_memory.push_back(rand());
    }

    std::vector<uint8_t> unicorn_memory = context->test_memory;
    std::vector<uint8_t> my_memory = context->test_memory;

    arm64_process my_process;
    arm64_process::create(&my_process, {my_memory.data(), base_plus_va, base_plus_va_jit},&context->my_jit_context,
        {
            offsetof(arm64_context, arm64_context::x),
            offsetof(arm64_context, arm64_context::q),

            offsetof(arm64_context, arm64_context::x),
            offsetof(arm64_context, arm64_context::z),
            offsetof(arm64_context, arm64_context::c),
            offsetof(arm64_context, arm64_context::v),

            offsetof(arm64_context, arm64_context::memory)
        }
    );

    assert(uc_mem_map_ptr(context->uc, 0, unicorn_memory.size(), UC_PROT_ALL, my_memory.data()) == UC_ERR_OK);

    if (uc_emu_start(context->uc,0, -1, -1, instruction_count) != UC_ERR_OK)
    {
        throw 0;
    }

    arm64_process::interperate_function(&my_process, 0, &context->debug_arm_context);

    validate_context(context);
}