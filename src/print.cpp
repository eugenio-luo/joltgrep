#include <mutex>
#include <iostream>

#include "print.h"

static std::mutex printLock{};

void joltgrep::printLine(joltgrep::Task& task, 
    std::string_view buffer, std::size_t pos)
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

    printLock.lock();
    std::cout << task.getPath() << ":" << 
        buffer.substr(left, right - left + 1) << "\n"; 
    printLock.unlock();
}

void joltgrep::debugPrint(std::string_view buffer)
{
    printLock.lock();
    std::cout << buffer;
    printLock.unlock();
}
