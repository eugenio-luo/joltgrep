#pragma once

#include <thread>
#include <optional>
#include <queue>
#include <filesystem>
#include <vector>
#include <fstream>

#include "re2/re2.h"

#include "clqueue.h"
#include "task.h"
#include "boyermoore.h"

#include "cache.h"

namespace joltgrep {

static constexpr std::size_t WORKER_BUFFER_SIZE = 2 << 16;

class WorkSystem;

enum SearchType {
    DEFAULT_SEARCH,
    BOYER_MOORE_SEARCH,
    AHO_CORASICK_SEARCH,
};

class Worker {
public:
    explicit Worker(std::size_t queueCap = 1024);

    Worker(const Worker& other) = delete;
    Worker& operator=(const Worker& other) = delete;

    std::vector<char>& getBuffer(void);

    void setSize(std::size_t size);
    std::size_t getSize(void);

    template <typename F, typename... Args>
    void run(F function, WorkSystem& workSystem, Args&&... args);
    void join(void);

    std::optional<Task> steal(void);
    void push(Task&& task);
    std::optional<Task> pop(void);

    // debug
    void setId(int id);
    int getId(void);
    void increaseFileRead(void);
    void increaseMatchFound(void);
    std::size_t getFileRead();
    std::size_t getMatchFound();

private:
    // buffer size is 65536 bytes (64 KB)
    CL::Queue<Task>            m_queue;
    std::optional<std::thread> m_thread;
    
    // debug
    int                        m_id;
    std::size_t                m_fileRead = 0;
    std::size_t                m_matchFound = 0;

    std::vector<char>          m_buffer;
    std::size_t                m_bufferSize;
};

struct alignWorker {
    alignas(DESTRUCTIVE_INTER_SIZE) Worker w;
};

class WorkSystem {
public:
    explicit WorkSystem(std::string&& pattern, 
            std::size_t numWorkers = 8);

    WorkSystem(const WorkSystem& other) = delete;
    WorkSystem& operator=(const WorkSystem& other) = delete;

    SearchType getSearchType(void);
    std::string& getPattern(void);
    re2::RE2& getPatternEngine(void);

    template <typename F, typename... Args>
    void runWorkers(F function, Args&&... args);

    std::optional<Task> steal(int id);
    
    std::optional<Task> readFileQueue(void);
    bool writeFileQueue(Task&& task);
    
    std::optional<Task> readDirQueue(void);
    bool writeDirQueue(Task&& task);

    std::optional<BoyerMoore>& getBoyerMoore(void);

private:
    std::vector<alignWorker>  m_workers;

    SearchType                m_recommended;
    std::string               m_pattern;
    re2::RE2                  m_patternEngine;
    std::optional<BoyerMoore> m_boyerMoore;

    // TODO: Remove file queue? It isn't that useful
    
    std::queue<Task>      m_fileQueue;
    std::mutex            m_fileLock;

    std::queue<Task>      m_dirQueue;
    std::mutex            m_dirLock;
};

template <typename F, typename... Args>
void joltgrep::Worker::run(F function, joltgrep::WorkSystem& workSystem, Args&&... args)
{
    m_thread = std::thread(function, std::ref(workSystem), std::ref(*this), 
        std::forward<Args>(args)...);
}

template <typename F, typename... Args>
void WorkSystem::runWorkers(F function, Args&&... args)
{
    for (auto& worker : m_workers) {
        worker.w.run(function, *this, std::forward<Args>(args)...);
    }

    for (auto& worker : m_workers) {
        worker.w.join();
    }
}

} // namespace joltgrep
