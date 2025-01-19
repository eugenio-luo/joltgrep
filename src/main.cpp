#include <iostream>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (getopt(argc, argv, "hv") != -1) {
       
        std::cout << "usage: joltgrep [-hv] [pattern] [file ...]\n";   
        exit(0);
    }
}
