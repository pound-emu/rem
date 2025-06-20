#pragma once

enum cpu_type {
    arm,
    x86,
    power_pc
};

enum cpu_size {
    _32_bit,
    _64_bit
};

enum memory_order {
    little_endian,
    bit_endian
};
