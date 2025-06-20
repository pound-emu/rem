#pragma once

struct aarch64_context_offsets {
    int x_offset;
    int q_offset;

    int n_offset;
    int z_offset;
    int c_offset;
    int v_offset;

    int exclusive_address_offset;
    int exclusive_value_offset;

    int fpcr_offset;
    int fpsr_offset;

    int thread_local_0;
    int thread_local_1;

    int context_size;
};
