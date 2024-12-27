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

static void test_single_instruction(uint32_t instruction)
{
    bool is_valid_instruction;

    std::string ins = disassemble_instruction(instruction,&is_valid_instruction ) ;

    if (ins == "undefined")
        return;

    std::cout << "Testing " << ins << std::endl;

    arm_unicorn_fuzzer tester;
    arm_unicorn_fuzzer::create(&tester);

    arm_unicorn_fuzzer::emit_guest_instruction(&tester,instruction);
    arm_unicorn_fuzzer::emit_guest_instruction(&tester,0xd61f0000); //br x0

    arm_unicorn_fuzzer::execute_code(&tester, 1);

    arm_unicorn_fuzzer::destroy(&tester);
}

static void test_add_subtract_imm12(int seed)
{
    //ins 285212672
	//mask 528482304

    srand(seed);

    uint32_t ins = 448792576;
    ins |= (create_random_number() & ~2145449984);

    bool valid;

    test_single_instruction(ins);
}

int main()
{
    int i = 0;

    for (int i = 0; i < 100000; ++i)
    {
        test_add_subtract_imm12(i++);
    }

    ks_close(ks);
    cs_close(&cs);
}