#pragma once

#include <string>
#include <cstdio>

#include "task.h"

namespace joltgrep {

void lockCoutMutex(void);
void unlockCoutMutex(void);

std::string_view getSubstrLine(std::string_view buffer, std::size_t pos);
void printLine(joltgrep::Task& task, std::string_view buffer);

template <typename... Args>
void debugPrintf(const char *fmt, Args&&... args);

} // namespace joltgrep

template <typename... Args>
void joltgrep::debugPrintf(const char *fmt, Args&&... args)
{
#ifdef DEBUG
    lockCoutMutex();
    std::printf(fmt, std::forward<Args>(args)...);
    unlockCoutMutex();
#endif
}
