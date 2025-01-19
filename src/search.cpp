#include "search.h"

#include <iostream>

void joltgrep::search(std::vector<std::string>& paths, std::string& pattern)
{

    std::cout << "PATTERN: " << pattern << '\n';
    std::cout << "PATHS: ";
    for (auto& path : paths) {
        std::cout << path << ' '; 
    }
    std::cout << "\n";
}
