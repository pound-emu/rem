#include "arm_unicorn_fuzzer.h"

void arm_unicorn_fuzzer::create(arm_unicorn_fuzzer* result)
{
    uc_open(UC_ARCH_ARM64, UC_MODE_LITTLE_ENDIAN,&result->uc);
}

void arm_unicorn_fuzzer::destroy(arm_unicorn_fuzzer* to_destroy)
{
    uc_close(to_destroy->uc);
}