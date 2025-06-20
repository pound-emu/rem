#pragma once

#include <inttypes.h>
#include "emulator/fixed_length_decoder.h"

struct guest_process;

struct aarch64_process_data {
    void* svc_hook;
    void* counter_hook;
};

struct aarch64_process {
    guest_process* raw_process;
    bool _32_bit;
    aarch64_process_data user_data;
    fixed_length_decoder<uint32_t> decoder;
};
