#ifndef UNIVERSAL_FLAGS_H
#define UNIVERSAL_FLAGS_H

enum compiler_flags
{
    check_undefined_behavior    = 1ULL << 0,
    optimize_ssa                = 1ULL << 1,
    mathmatical_fold            = optimize_ssa | 1ULL << 2,

    compiler_flags_all          = check_undefined_behavior | mathmatical_fold
};

#endif