#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>

#include "search.h"

void printHelp(void)
{
    std::cout << "usage: joltgrep [-hv] [pattern] [file ...]\n";   
}

int main(int argc, char* argv[])
{
    if (getopt(argc, argv, "hv") != -1) {
        printHelp();      
        return 0;
    }

    // TODO: parse all optional flags here

    if (argc - optind < 2) {
        printHelp();
        return 1;
    }

    std::string pattern = argv[optind];
    std::vector<std::string> paths(argv + optind + 1, argv + argc);

    joltgrep::search(paths, pattern);
}
