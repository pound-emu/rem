A (Hopefully Multi System) Jit emulator framework written in C++.

Libraries Used:
    https://github.com/herumi/xbyak
    https://github.com/unicorn-engine/unicorn
    https://github.com/keystone-engine/keystone
    https://github.com/capstone-engine/capstone

Road Plan:

    X86 Backend                         - [in progress]
    Complete arm64 frontend             - [in progress] 
                                            [X] - Complete implementation
                                            [ ] - Incomplete implementation
                                            - Data Processing immediate
                                                - PC-rel. addressing                            [X]
                                                - Add/subtract (immediate)                      [X]
                                                - Add/subtract (immediate, with tags)           [ ]
                                                - Min/max (immediate)                           [X]
                                                - Logical (immediate)                           [X]
                                                - Move wide (immediate)                         [X]
                                                - Bitfield                                      [X]
                                                - Extract                                       [X]
                                            - Data Processing -- Register
                                                - Data-processing (2 source)                    [ ]
                                                - Data-processing (1 source)                    [ ]
                                                - Logical (shifted register)                    [X]
                                                - Add/subtract (shifted register)               [X]
                                                - Add/subtract (extended register)              [X]
                                                - Add/subtract (with carry)                     [X]
                                                - Add/subtract (checked pointer)                [ ]
                                                - Rotate right into flags                       [ ]
                                                - Evaluate into flags                           [ ]
                                                - Conditional compare (register)                [X]
                                                - Conditional compare (immediate)               [X]
                                                - Conditional select                            [X]
                                                - Data-processing (3 source)                    [ ]
                                                All the base instructions have been implamented. Limited support for extensions.
                                            - Branches, Exception Generating and System instructions
                                                - Compare and branch (immediate)                [X]
                                                - Test and branch (immediate)                   [X]
                                                - Unconditional branch (immediate)              [X]
                                                - Conditional branch (immediate)                [ ] 
                                                    BC.cond not implamented
                                                - Unconditional branch (register)               [ ]
                                                    - Ret
                                                    - Br
                                                    - Blr
                                            - Loads and Stores
                                                - Load/store register pair (post-indexed)       [X]
                                                - Load/store register pair (pre-indexed)        [X]
                                                - Load/store register pair (offset)             [X]
                                                - Load/store register (unscaled immediate)      [X]
                                                - Load/store register (immediate post-indexed)  [X]
                                                - Load/store register (immediate pre-indexed)   [X]
                                                - Load/store register (unsigned immediate)      [X]
                                                - Load/store register (register offset)         [X]
                                            - Data Processing -- Scalar Floating-Point and Advanced SIMD
                                                - Not Started
    arm64 backend                       - [not started]
    Complete riscv frontend             - [not started]
    Power PC 32                         - [not started]
    Running itself in every frontend    - [not started]
    RUNNING LINUX NO MMU                - [not started]