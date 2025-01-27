#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <unistd.h>

#include "search.h"

#ifdef TESTS_BUILD
#include "tests.h"
#endif

void printHelp(void)
{
    std::cout << "usage: joltgrep [-hv] [pattern] [file ...]\n";   
}

int main(int argc, char* argv[])
{
#ifdef TESTS_BUILD
    tests::main();
    return 0;
#endif

    if (getopt(argc, argv, "hv") != -1) {
        printHelp();      
        return 0;
    }

    // TODO: parse all optional flags here

    if (argc - optind < 2) {
        printHelp();
        return 1;
    }

    std::string pattern{argv[optind]};
    std::vector<fs::path> paths(argv + optind + 1, argv + argc);

    joltgrep::search(paths, pattern);
}
