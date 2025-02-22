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

void boyerMooreSearch(joltgrep::WorkSystem& workSystem,
                joltgrep::Worker& worker, joltgrep::Task& task)
{
    std::optional<BoyerMoore>& boyerMoore = workSystem.getBoyerMoore();

    if (!boyerMoore.has_value()) {
        joltgrep::debugPrintf("ERROR: BoyerMooreSearch but no tables?");
        throw;
    }

    auto& charTable = boyerMoore->getBadCharTable();
    auto& suffixTable = boyerMoore->getSuffixTable();
    
    std::string_view buffer{worker.getBuffer().data(), worker.getSize()};
    std::string& pattern = workSystem.getPattern();
    std::size_t pos = pattern.size() - 1;
    
    while ((pos = memchr(buffer, pos, pattern.back())) < buffer.size()) {
        
        int j = pattern.size() - 1;
        while (j >= 0 && buffer[pos] == pattern[j]) {
            --pos;
            --j;
        }

        if (j == -1) {
            
            joltgrep::debugPrintf("thread %d found match in %s %d %d\n",
                worker.getId(), task.getPath(), task.getId(), task.getOwnerId());

            joltgrep::printLine(task, joltgrep::getSubstrLine(buffer, pos + 1));
            pos += pattern.size() + 1;
            pos = memchr(buffer, pos, '\n');
            
            // Reached end of file
            if (pos >= buffer.size()) {
                return;
            }
        } else {
        
            pos += std::max(charTable[buffer[pos]], suffixTable[j]);
        }
    }
}

void defaultSearch(joltgrep::WorkSystem& workSystem, 
        joltgrep::Worker& worker, joltgrep::Task& task)
{
    auto& patternEngine = workSystem.getPatternEngine();
    std::string_view buffer{worker.getBuffer().data(), worker.getSize()};

    auto prev = 0;
    auto it = buffer.find('\n', 0);
    for (; it != -1; it = buffer.find('\n', it + 1)) {
        std::string_view subview = buffer.substr(prev, it - prev);
       
        if (RE2::PartialMatch(subview, patternEngine)) {
            joltgrep::printLine(task, subview);
        }
        prev = it + 1;
    }

    std::string_view subview = buffer.substr(prev, buffer.size() - prev);
    if (RE2::PartialMatch(subview, patternEngine)) {
        joltgrep::printLine(task, subview);
    } 
}

void joltgrep::searchFile(joltgrep::WorkSystem& workSystem,
    joltgrep::Worker& worker, joltgrep::Task& task) 
{
    int fd = open(task.getPath(), O_RDONLY);
    if (fd == -1) {
        joltgrep::debugPrintf("ERROR: %d %d %d\n", errno, task.getId(), task.getOwnerId());
        throw;
    }
    worker.increaseFileRead();
 
    joltgrep::SearchType searchType = workSystem.getSearchType(); 
    std::size_t bytesRead;
    do {
        bytesRead = read(fd, worker.getBuffer().data(), joltgrep::WORKER_BUFFER_SIZE);
        if (bytesRead == SIZE_T_MAX) {
            close(fd);
            return;
        }
        worker.setSize(bytesRead);
        
        switch (searchType) {
        case joltgrep::DEFAULT_SEARCH:
            defaultSearch(workSystem, worker, task);
            break;

        case joltgrep::BOYER_MOORE_SEARCH:
            boyerMooreSearch(workSystem, worker, task);
            break;
        
        default:
            joltgrep::debugPrintf("ERROR: search not implemented yet!\n");
            throw;
        }

    } while (bytesRead == joltgrep::WORKER_BUFFER_SIZE);
    
    close(fd);

    /*
    std::optional<BoyerMoore>& boyerMoore = workSystem.getBoyerMoore();
    std::string& pattern = workSystem.getPattern();

    if (!boyerMoore.has_value()) {
        return;
    }

    auto& badTable = boyerMoore->getBadCharTable();
    auto& suffixTable = boyerMoore->getSuffixTable();
    std::vector<char>& buffer = worker.getBuffer();
    
    std::size_t bytesRead;
    do {
        bytesRead = read(fd, buffer.data(), joltgrep::WORKER_BUFFER_SIZE);
        if (bytesRead == SIZE_T_MAX) {
            close(fd);
            return;
        }
        worker.setSize(bytesRead);
        boyerMooreSearch(badTable, suffixTable, pattern, {buffer.data(), worker.getSize()}, task, worker.getId());
    } while (bytesRead == joltgrep::WORKER_BUFFER_SIZE);

    close(fd);
    */
}
