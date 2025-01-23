#include <iostream>

#include "tests.h"

void tests::main(void)
{
}

static std::size_t numTests = 0; 

void tests::sectionStart(const std::string_view output)
{
    numTests = 0;
    std::cout << "\033[1;33mSection Start: \033[0m" << output << "\n";
}

void tests::sectionEnd(const std::string_view output)
{
    std::cout << "\033[1;32mSection Success, " << numTests << " tests passed: \033[0m" << output << "\n";
    numTests = 0;
}

void tests::start(const std::string_view output, int n)
{
    std::cout << "\033[1;33mTest Start: \033[0m" << output << " " << n << "\n";
}
void tests::start(const std::string_view output)
{
    std::cout << "\033[1;33mTest Start: \033[0m" << output << "\n";

}

void tests::end(const std::string_view output, int n)
{
    ++numTests;
    std::cout << "\033[1;32mTest Success:\033[0m " << output << " " << n << "\n";
}

void tests::end(const std::string_view output)
{
    ++numTests;
    std::cout << "\033[1;32mTest Success:\033[0m " << output << "\n";
}

void tests::check(bool condition, const std::string_view output)
{
    if (!condition) {
        std::cout << "\033[1;31mTest Failed:\033[0m " << output << "\n";
        exit(1);
    }
}
