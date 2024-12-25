#include "testing/arm_unicorn_fuzzer.h"

static void test_single_instruction(uint32_t instruction)
{
    arm_unicorn_fuzzer tester;

    arm_unicorn_fuzzer::create(&tester);

    arm_unicorn_fuzzer::emit_guest_instruction(&tester,instruction);
    arm_unicorn_fuzzer::emit_guest_instruction(&tester,0xd61f0000); //br x0

    arm_unicorn_fuzzer::execute_code(&tester, 1);

    arm_unicorn_fuzzer::destroy(&tester);
}

static void test_add_subtract_imm12()
{
    //ins 285212672
	//mask 528482304

    uint32_t ins = 285212672;
    ins |= (rand() & ~528482304) | ins;

    test_single_instruction(ins);
}

int main()
{
    while (1)
    {
        test_add_subtract_imm12();
    }
}