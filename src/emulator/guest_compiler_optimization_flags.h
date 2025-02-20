#ifndef GUEST_COMPILER_OPTIMIZATION_FLAGS_H
#define GUEST_COMPILER_OPTIMIZATION_FLAGS_H

enum guest_compiler_optimization_flags
{
    function_wide_translation           = 1 << 0,
    guest_optimize_ssa                  = 1 << 1,
    guest_optimize_mathmatical_fold     =(1 << 2) | guest_optimize_ssa,
    use_flt                             = 1 << 3,

    interpreted                         = 1 << 4,

    level_zero = 0,
    level_one   = function_wide_translation,
    level_two   = function_wide_translation | guest_optimize_ssa,
    level_three = function_wide_translation | guest_optimize_mathmatical_fold | use_flt
};

#endif