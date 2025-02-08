#pragma once

#include <string>

#include "task.h"

namespace joltgrep {

void printLine(joltgrep::Task& task, std::string_view buffer, std::size_t pos);

} // namespace joltgrep