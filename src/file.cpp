#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#include "search.h"
#include "task.h"
#include "worker.h"
#include "re2/re2.h"

void readFile(joltgrep::Worker& worker, joltgrep::Task& task)
{

    std::ifstream fileHandler(task.getPath(), 
        std::ios::in | std::ios::binary | std::ios::ate);
    const std::size_t fileSize = fileHandler.tellg();
    fileHandler.seekg(0, std::ios::beg);

    // TODO: Handle case where file is bigger than buffer,
    //       in that case, create another task 
    if (fileSize > joltgrep::workerBufferSize) {
        std::cout << "Error: file size too big!!!"; 
        exit(1);
    }

    fileHandler.read(worker.getBuffer().data(), fileSize);
}

void joltgrep::searchFile(joltgrep::Worker& worker, joltgrep::Task& task,
                    const RE2& pattern) 
{
    static std::mutex lock{};
    
    readFile(worker, task);
    
    if (RE2::PartialMatch(worker.getBuffer().data(), pattern)) {
        lock.lock();
        std::cout << task.getPath() << "\n";
        lock.unlock();
    }
}