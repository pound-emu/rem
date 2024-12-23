#include "testing/arm_unicorn_fuzzer.h"

int main()
{
    arm_unicorn_fuzzer tester;

    arm_unicorn_fuzzer::create(&tester);

    arm_unicorn_fuzzer::emit_guest_instruction(&tester,0x91002800); //add x0, x0, 10
    arm_unicorn_fuzzer::emit_guest_instruction(&tester,0xd61f0000); //br x0

    arm_unicorn_fuzzer::execute_code(&tester, 1);
    arm_unicorn_fuzzer::validate_context(&tester);

    arm_unicorn_fuzzer::destroy(&tester);
}