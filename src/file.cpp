#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem> 
#include <fcntl.h>
#include <unistd.h>

#include "search.h"
#include "task.h"
#include "worker.h"
#include "boyermoore.h"
#include "print.h"
#include "memchr.h"
#include "print.h"

#include "re2/re2.h"

// TODO: write a better version of memchr
inline std::size_t memchr(std::string_view buffer, std::size_t pos, char c)
{
    std::size_t size = buffer.size();

    for (std::size_t i = pos; i < size; ++i) {
        if (buffer[i] == c) {
            return i;
        }
    }

    return SIZE_T_MAX;
}

/*
void boyerMooreSearch(BoyerMoore& boyerMoore, const std::string& pattern,
    std::string_view buffer, joltgrep::Task& task)
{
    std::string_view string = boyerMoore.getPattern();
    std::size_t pos = memchr(buffer, boyerMoore.start(), string.back());
    std::size_t size = buffer.size();
    
    while (pos < size) {
        int n = boyerMoore.next(buffer, pos);

        if (n == -1) {
            printLine(task, buffer, pos + 1);
            pos += string.size() + 1;
        } else {
            pos += n;
        }
        
        pos = memchr(buffer, pos, string.back());
    }
}
*/

void boyerMooreSearch(std::vector<std::size_t>& charTable, std::vector<std::size_t>& suffixTable,
        const std::string& pattern, std::string_view buffer, joltgrep::Task& task)
{
    std::size_t pos = memchr(buffer, pattern.size() - 1, pattern.back());
    std::size_t size = buffer.size();
    std::size_t patternSize = pattern.size();
    
    while (pos < size) {
        int j = patternSize - 1;
        while (j >= 0 && buffer[pos] == pattern[j]) {
            --pos;
            --j;
        }

        if (j == -1) {
            printLine(task, buffer, pos + 1);
            pos += patternSize + 1;
        } else {
            pos += std::max(charTable[buffer[pos]], suffixTable[j]);
        }
        
        pos = memchr(buffer, pos, pattern.back());
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
    joltgrep::Worker& worker, joltgrep::Task& task) 
{
    std::vector<char>& buffer = worker.getBuffer();

    int fd = open(task.getPath().c_str(), O_RDONLY);
    // TODO: Handle Error
    if (fd == -1) {
        std::stringstream ss;
        ss << "ERROR: " << errno << " " << task.getId() << " " << task.getOwnerId() << "\n";
        joltgrep::debugPrint(ss.str());
        throw;
    }
    worker.increaseFileRead();
    
    std::optional<BoyerMoore>& boyerMoore = workSystem.getBoyerMoore();
    std::string& pattern = workSystem.getPattern();

    if (!boyerMoore.has_value()) {
        return;
    }

    auto& badTable = boyerMoore->getBadCharTable();
    auto& suffixTable = boyerMoore->getSuffixTable();

    std::size_t bytesRead;
    do {
        bytesRead = read(fd, buffer.data(), joltgrep::WORKER_BUFFER_SIZE - 1);
        if (bytesRead == SIZE_T_MAX) {
            close(fd);
            return;
        }
        worker.setSize(bytesRead);
        boyerMooreSearch(badTable, suffixTable, pattern, {buffer.data(), worker.getSize()}, task);
    } while (bytesRead == joltgrep::WORKER_BUFFER_SIZE - 1);

    close(fd);
}
