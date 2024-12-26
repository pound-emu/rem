A (Hopefully Multi System) Jit emulator framework written in C++.

Libraries Used:
    https://github.com/herumi/xbyak
    https://github.com/unicorn-engine/unicorn
    https://github.com/keystone-engine/keystone
    https://github.com/capstone-engine/capstone

Road Plan:

    X86 Backend                         - [in progress]
    Complete arm64 frontend             - [in progress] 
                                            - Working Decoder, Super slow right now, but there are plans to make it faster
                                            - Working interpreter and Jit
    arm64 backend                       - [not started]
    Complete riscv frontend             - [not started]
    Power PC 32                         - [not started]
    Running itself in every frontend    - [not started]
    RUNNING LINUX NO MMU                - [not started]