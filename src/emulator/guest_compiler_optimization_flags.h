#pragma once

enum guest_compiler_optimization_flags {
    guest_function_wide_translation = 1 << 0,
    guest_optimize_basic_ssa = 1 << 1,
    guest_optimize_group_ssa = 1 << 2,
    use_flt = 1 << 3,

    interpreted = 1 << 4,

    level_one = guest_function_wide_translation,
    level_two = guest_function_wide_translation | guest_optimize_basic_ssa,
    level_three = guest_function_wide_translation | guest_optimize_group_ssa | use_flt
};
