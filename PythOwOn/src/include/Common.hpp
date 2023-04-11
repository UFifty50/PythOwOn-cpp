#ifndef COMMON_HPP
#define COMMON_HPP

#include "fmt/core.h"


#define FMT_PRINT(formatStr, ...) fmt::print(fmt::runtime(fmt::format(fmt::runtime(formatStr), __VA_ARGS__)))

#endif
