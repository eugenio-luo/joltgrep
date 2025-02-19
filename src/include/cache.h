#pragma once

#include <new>

#ifdef __cpp_lib_hardware_interference_size
constexpr std::size_t DESTRUCTIVE_INTER_SIZE = std::hardware_destructive_interference_size;
#else
constexpr std::size_t DESTRUCTIVE_INTER_SIZE = 2 * sizeof(std::max_align_t);
#endif