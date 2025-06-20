#pragma once

#include <exception>
#include <iostream>

#define throw_error()                                                                              \
    do {                                                                                           \
        std::cout << std::dec << "ERROR IN FILE " << __FILE__ << ":" << __LINE__ << std::endl;     \
        while (1);                                                                                 \
    } while (0)
