#include "testing/arm_unicorn_fuzzer.h"
#include "tools/numbers.h"

#include "keystone/headers/keystone/arm64.h"
#include "capstone/headers/capstone/capstone.h"

bool keystone_open = false;
bool capstone_open = false;
ks_engine* ks;
csh cs;

static uint32_t assemble_instruction(std::string instruction, bool* valid_instruction)
{
    if (!keystone_open)
    {
        keystone_open = true;
        
        ks_err error = ks_open(ks_arch::KS_ARCH_ARM64, KS_MODE_LITTLE_ENDIAN,&ks);

        if (error != KS_ERR_OK)
        {
            throw 0;
        }
    }

    unsigned char* result;
    uint64_t result_size;
    uint64_t result_count;

    ks_asm(ks, instruction.c_str(), 0, &result, &result_size, &result_count);

    *valid_instruction = result_size == 4;

    uint32_t result_ins = 0;

    if (*valid_instruction)
    {
        result_ins = *(uint32_t*)result;
    }

    ks_free(result);

    return result_ins;
}

static std::string disassemble_instruction(uint32_t instruction, bool* valid_instruction)
{
    if (!capstone_open)
    {
        capstone_open = true;

        cs_open(CS_ARCH_AARCH64, CS_MODE_LITTLE_ENDIAN, &cs);
    }

    cs_insn* insn;

    int count = cs_disasm(cs, (uint8_t*)&instruction, 4, 0, 0, &insn);

    std::string result = "";

    if (count > 0)
    {
        result = std::string(insn[0].mnemonic) + " " + insn[0].op_str;
    }
    else
    {
        result = "undefined";
    }

    cs_free(insn, count);

    return result;
}

static bool test_single_instruction(uint32_t instruction)
{
    bool result_valid;

    std::string ins = disassemble_instruction(instruction,&result_valid);

    assemble_instruction(ins, &result_valid);

    if (ins == "undefined" || !result_valid)
        return false;

    if (skip_instruction(instruction) || !is_valid_instruction(instruction))
    {
        std::cout << "Skipped " << ins << std::endl;

        return true;
    }
    else
    {
        std::cout << "Testing " << ins << std::endl;
    }

    arm_unicorn_fuzzer tester;
    arm_unicorn_fuzzer::create(&tester);

    arm_unicorn_fuzzer::emit_guest_instruction(&tester,instruction);
    arm_unicorn_fuzzer::emit_guest_instruction(&tester,0xd61f0000); //br x0

    arm_unicorn_fuzzer::execute_code(&tester, 1);

    arm_unicorn_fuzzer::destroy(&tester);

    return true;
}

static bool test_random_instruction(int seed, int instruction, int mask)
{
    srand(seed);

    uint32_t ins = instruction;
    ins |= (create_random_number() & ~mask);

    bool valid;

    return test_single_instruction(ins);
}

int main()
{
    int i = 0;
    
    aarch64_process p;
    aarch64_process::create(&p, {}, nullptr, {});

    auto table = fixed_length_decoder<uint32_t>::get_table_by_name(&p.decoder, "load_store_register_offset");

    skip_uneeded = false;

    bool valid;
    test_single_instruction(assemble_instruction("ldr x0, [x1, x2]", &valid));

    if (true)
    {
        int seed = 0;

        while (1)
        {
            for (int i = 0; i < p.decoder.entries.size(); ++i)
            {
                auto entry = p.decoder.entries[i];

                for (int k = 0; k < 100; ++k)
                {
                    while (!test_random_instruction(++seed, entry.instruction, entry.mask));
                }
            }
        }
    }
    else
    {
        while (1)
        {
            auto entry = table;

            for (int i = 0; i < 10000; ++i)
            {
                test_random_instruction(i, entry.instruction, entry.mask);
            }
        }
        

    }

    std::cout << "Every instruction tested is valid " << std::endl;

    ks_close(ks);
    cs_close(&cs);
}