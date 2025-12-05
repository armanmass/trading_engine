#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>

#ifndef NDEBUG
#define LOG_DEBUG(x) do { std::cout << x << std::endl; } while (0)
#else
#define LOG_DEBUG(x) do {} while (0)
#endif

#endif // DEBUG_H
