#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#include "search.h"
#include "task.h"
#include "worker.h"
#include "boyermoore.h"
#include "print.h"

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
    worker.setSize(fileSize);
}

/*
std::optional<std::size_t> memchr(std::string_view buffer, std::size_t pos, char c)
{
    std::size_t size = buffer.size();

    for (std::size_t i = pos; i < size; ++i) {
        if (buffer[i] == c) {
            return i;
        }
    }

    return std::nullopt;
}
*/

void boyerMooreSearch(BoyerMoore& boyerMoore, const RE2& pattern,
    std::string_view buffer, joltgrep::Task& task)
{
    std::string_view string = boyerMoore.getPattern();
    std::size_t pos = boyerMoore.start();
    std::size_t size = buffer.size();
    
    while (pos < size) {
        auto ptr = std::memchr(buffer.data() + pos, string.back(), size - pos);
        if (ptr == nullptr) {
            return;
        }

        pos = static_cast<const char*>(ptr) - buffer.data();
        /*
        auto opt = memchr(buffer, pos, string.back());
        if (!opt.has_value()) {
            return;
        }

        pos = *opt;
        */

        int n = boyerMoore.next(buffer, pos);
        
        /*
        if (n == -1) {
            printLine(task, buffer, pos + 1);
            pos += string.size() + 1;
        } else {
            pos += n;
        }
        */
        if (n == -1 && RE2::PartialMatch(buffer.substr(pos + 1, string.size()), boyerMoore.getPattern())) {
            printLine(task, buffer, pos + 1);
            pos += string.size() + 1;
        } else {
            pos += n;
        }
    }
}

void defaultSearch(std::string_view buffer, const RE2& pattern, 
    joltgrep::Task& task)
{
    auto prev = 0;
    auto it = buffer.find('\n', 0);
    for (; it != -1; it = buffer.find('\n', it + 1)) {
        std::string_view subview = buffer.substr(prev, it - prev);
       
        if (RE2::PartialMatch(subview, pattern)) {
            std::cout << task.getPath() << ": " << subview << "\n";
        }
        prev = it + 1;
    }

    std::string_view subview = buffer.substr(prev, buffer.size() - prev);
    if (RE2::PartialMatch(subview, pattern)) {
        printLine(task, subview, prev);
    }
}

void joltgrep::searchFile(joltgrep::WorkSystem& workSystem,
    joltgrep::Worker& worker, joltgrep::Task& task, const RE2& pattern) 
{
    std::vector<char>& buffer = worker.getBuffer();

    readFile(worker, task);
    std::optional<BoyerMoore>& boyerMoore = workSystem.getBoyerMoore();
    
    if (!boyerMoore.has_value()) {
        defaultSearch({buffer.data(), worker.getSize()}, pattern, task);
    } else {
        boyerMooreSearch(*boyerMoore, pattern, {buffer.data(), worker.getSize()}, task);
    }
}