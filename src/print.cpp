#include <mutex>
#include <iostream>

#include "print.h"

static std::mutex coutMutex{};

void joltgrep::lockCoutMutex(void)
{
    coutMutex.lock();
}

void joltgrep::unlockCoutMutex(void)
{
    coutMutex.unlock();
}

std::string_view joltgrep::getSubstrLine(std::string_view buffer, std::size_t pos)
{
    std::size_t left = pos, right = pos;

    while ((left > 0 && buffer[left] != '\n') || 
        (right < buffer.size() && buffer[right] != '\n')) {
        
        if (left > 0 && buffer[left] != '\n') {
            --left;
        }
        if (right < buffer.size() && buffer[right] != '\n') {
            ++right;
        }
    }

    if (left != 0) {
        ++left;
    }

    if (right != buffer.size()) {
        --right;
    }

    return buffer.substr(left, right - left + 1);
}

void joltgrep::printLine(joltgrep::Task& task, std::string_view buffer)
{
    lockCoutMutex();

    std::cout << task.getPath() << ":" << buffer << "\n";
    
    unlockCoutMutex();
}
