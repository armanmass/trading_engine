#pragma once
#include <iostream>

#ifndef NDEBUG
#define LOG_DEBUG(x) do { std::cout << x << std::endl; } while (0)
#else
#define LOG_DEBUG(x) do {} while (0)
#endif