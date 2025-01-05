#ifndef DEBUGGING_H
#define DEBUGGING_H
#include <iostream>
#include <exception>

#define throw_error() do { std::cout << std::dec << "ERROR IN FILE " << __FILE__ << ":" << __LINE__ << std::endl; throw std::invalid_argument("ERROR"); } while (0)

#endif