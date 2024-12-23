#ifndef ARM64_CONTEXT_OFFSETS_H
#define ARM64_CONTEXT_OFFSETS_H

struct arm64_context_offsets
{
    int x_offset;
    int q_offset;

    int n_offset;
    int z_offset;
    int c_offset;
    int v_offset;

    int memory_pointer_offset;
    
    int exclusive_address_offset;
    int exclusive_value_offset;
};

#endif