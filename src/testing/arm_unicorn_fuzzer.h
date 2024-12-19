#ifndef ARM_UNICORN_FUZZER_H
#define ARM_UNICORN_FUZZER_H

#include "unicorn/headers/unicorn.h"

struct arm_unicorn_fuzzer
{
    uc_engine* uc;

    static void create(arm_unicorn_fuzzer* result);
    static void destroy(arm_unicorn_fuzzer* to_destroy);
};

#endif