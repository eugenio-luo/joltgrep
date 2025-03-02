#pragma once

#include <string>
#include <cstdio>
#include <utility>

#include "task.h"
#include "worker.h"

namespace joltgrep {

void lockCoutMutex(void);
void unlockCoutMutex(void);

void printLine(joltgrep::Task& task, joltgrep::Worker& worker, 
        std::pair<size_t, size_t> pair);

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
