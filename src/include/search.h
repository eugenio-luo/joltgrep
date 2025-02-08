#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "re2/re2.h"
#include "task.h" 
#include "worker.h"

namespace fs = std::filesystem;

namespace joltgrep {

void search(std::vector<fs::path>& paths, std::string& pattern);
void searchFile(joltgrep::WorkSystem& workSystem, joltgrep::Worker& worker, 
    joltgrep::Task& task, const RE2& pattern);

};
