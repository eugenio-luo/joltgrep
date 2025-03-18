#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem> 
#include <limits>
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
inline size_t memchr(joltgrep::Worker& worker, size_t pos, char c)
{
    std::size_t size = worker.getSize() + joltgrep::WORKER_BUFFER_SIZE;

    for (size_t i = pos; i < size; ++i) {
        if (worker.getChar(i) == c) {
            return i;
        }
    }

    return SIZE_MAX;
}

void ahoCorasickSearch(joltgrep::WorkSystem& workSystem,
                joltgrep::Worker& worker, joltgrep::Task& task)
{
    static thread_local int v = 0;

    std::optional<AhoCorasick>& ahoCorasick = workSystem.getAhoCorasick();

    if (!ahoCorasick.has_value()) {
        joltgrep::debugPrintf("%s", "ERROR: ahoCorasickSearch but no tables?");
        throw;
    }

    std::string& pattern = workSystem.getPattern();
    std::size_t bufferSize = worker.getSize() + joltgrep::WORKER_BUFFER_SIZE;
    size_t pos = joltgrep::WORKER_BUFFER_SIZE;
    
    joltgrep::debugPrintf("START pos: %d, state: %d\n", pos, v);
    while (v != 0 || (pos = memchr(worker, pos, pattern.front())) < bufferSize) {
    
        --pos;
        do {
            ++pos;
            if (pos >= bufferSize) {
                joltgrep::debugPrintf("DEADEND pos: %d, state: %d\n", pos, v);
                return;
            }
            v = ahoCorasick->go(v, worker.getChar(pos));    
            joltgrep::debugPrintf("pos: %d, state: %d\n", pos, v);
        } while (v > 0);
        
        if (v == AhoCorasick::SUCCESS) {
            joltgrep::debugPrintf("thread %d found match in %s %d %d\n",
                worker.getId(), task.getPath(), task.getId(), task.getOwnerId());

            joltgrep::printLine(task, worker, worker.getLine(pos - 1));
            pos = memchr(worker, pos, '\n');
        }
        v = 0;
        // Reached end of file
        if (pos >= bufferSize) {
                break;
        }
    }
    joltgrep::debugPrintf("END pos: %d, state: %d\n", pos, v);
}

void boyerMooreSearch(joltgrep::WorkSystem& workSystem,
                joltgrep::Worker& worker, joltgrep::Task& task)
{
    std::optional<BoyerMoore>& boyerMoore = workSystem.getBoyerMoore();

    if (!boyerMoore.has_value()) {
        joltgrep::debugPrintf("%s", "ERROR: BoyerMooreSearch but no tables?");
        throw;
    }

    auto& charTable = boyerMoore->getBadCharTable();
    auto& suffixTable = boyerMoore->getSuffixTable();

    std::string& pattern = workSystem.getPattern();
    std::size_t bufferSize = worker.getSize() + joltgrep::WORKER_BUFFER_SIZE;
    size_t pos = joltgrep::WORKER_BUFFER_SIZE;

    while ((pos = memchr(worker, pos, pattern.back())) < bufferSize) {
        
        // there's no secondary buffer yet, no point checking behind
        if (pos <= joltgrep::WORKER_BUFFER_SIZE + pattern.size() - 1 && !worker.getUsed()) {
            ++pos;
            continue;
        }

        int j = pattern.size() - 1;
        while (j >= 0 && worker.getChar(pos) == pattern[j]) {
            --pos;
            --j;
        }

        if (j == -1) {
            joltgrep::debugPrintf("thread %d found match in %s %d %d\n",
                worker.getId(), task.getPath(), task.getId(), task.getOwnerId());

            joltgrep::printLine(task, worker, worker.getLine(pos + 1));
            pos += pattern.size() + 1;
            pos = memchr(worker, pos, '\n');
            
            // Reached end of file
            if (pos >= bufferSize) {
                return;
            }
        } else {

            pos += std::max(charTable[worker.getChar(pos)], suffixTable[j]);
        }
    }
}   

// TODO: Fix 2 buffers!!!
/*
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
*/

void joltgrep::searchFile(joltgrep::WorkSystem& workSystem,
    joltgrep::Worker& worker, joltgrep::Task& task) 
{
    int fd = open(task.getPath(), O_RDONLY);
    if (fd == -1) {
        joltgrep::debugPrintf("ERROR: %d %d %d\n", errno, task.getId(), task.getOwnerId());
        throw;
    }
    worker.increaseFileRead();
    worker.resetUsed();

    joltgrep::SearchType searchType = workSystem.getSearchType(); 
    std::size_t bytesRead;
    do {
        joltgrep::buffer_t& buffer = worker.getBuffer();
        bytesRead = read(fd, buffer.data(), joltgrep::WORKER_BUFFER_SIZE);
        if (bytesRead == SIZE_T_MAX) {
            close(fd);
            return;
        }
        worker.setSize(bytesRead);
        
        switch (searchType) {
        case joltgrep::DEFAULT_SEARCH:
            //defaultSearch(workSystem, worker, task);
            break;

        case joltgrep::BOYER_MOORE_SEARCH:
            boyerMooreSearch(workSystem, worker, task);
            break;

        case joltgrep::AHO_CORASICK_SEARCH:
            ahoCorasickSearch(workSystem, worker, task);
            break;
        
        default:
            joltgrep::debugPrintf("ERROR: search not implemented yet!\n");
            throw;
        }
        
        worker.switchPrimary();
    } while (bytesRead == joltgrep::WORKER_BUFFER_SIZE);
    
    close(fd);
}
