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

void joltgrep::printLine(joltgrep::Task& task, joltgrep::Worker& worker,
        std::pair<size_t, size_t> pair)
{
    std::string_view primaryBuffer{worker.getBuffer().data(), worker.getSize()};
    size_t left = pair.first, right = pair.second;

    if (left >= joltgrep::WORKER_BUFFER_SIZE) {  
 
        left -= WORKER_BUFFER_SIZE;
        right -= WORKER_BUFFER_SIZE;

        lockCoutMutex();

        std::cout << task.getPath() << ":" << 
            primaryBuffer.substr(left, right - left + 1) << "\n";
    
        unlockCoutMutex();
    } else {
    
        std::string_view secondaryBuffer{
            worker.getSecondaryBuffer().data(), joltgrep::WORKER_BUFFER_SIZE};

        std::string_view line1 = secondaryBuffer.substr(
                left);
        std::string_view line2 = primaryBuffer.substr(0, right - WORKER_BUFFER_SIZE);

        lockCoutMutex();

        std::cout << task.getPath() << ":" << 
            line1 << line2 << "\n";
    
        unlockCoutMutex();
    }
}
