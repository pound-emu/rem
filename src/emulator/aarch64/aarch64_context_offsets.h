#ifndef AARCH64_CONTEXT_OFFSETS_H
#define AARCH64_CONTEXT_OFFSETS_H

struct aarch64_context_offsets
{
    int x_offset;
    int q_offset;

    int n_offset;
    int z_offset;
    int c_offset;
    int v_offset;
    
    int exclusive_address_offset;
    int exclusive_value_offset;

    int context_size;
};

#endif