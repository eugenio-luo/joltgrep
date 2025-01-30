#pragma once

#include <thread>
#include <optional>
#include <queue>
#include <filesystem>
#include <vector>
#include <fstream>

#include "clqueue.h"
#include "task.h"

namespace joltgrep {

static constexpr std::size_t workerBufferSize = 2 << 16;

class WorkSystem;

class Worker {
public:
    explicit Worker(std::size_t queueCap = 1024);

    Worker(const Worker& other) = delete;
    Worker& operator=(const Worker& other) = delete;

    void setId(int id);
    int getId(void);
    std::vector<char>& getBuffer(void);

    template <typename F, typename... Args>
    void run(F function, WorkSystem& workSystem, Args&&... args);
    void join(void);

    std::optional<Task> steal(void);
    void push(Task&& task);
    std::optional<Task> pop(void);

private:
    // buffer size is 65536 bytes (64 KB)

    CL::Queue<Task>            m_queue;
    std::optional<std::thread> m_thread;
    int                        m_id;
    std::vector<char>          m_buffer;
};

class WorkSystem {
public:
    explicit WorkSystem(std::string&& pattern, 
            std::size_t numWorkers = 4);

    WorkSystem(const WorkSystem& other) = delete;
    WorkSystem& operator=(const WorkSystem& other) = delete;

    std::string_view getPattern(void);

    template <typename F, typename... Args>
    void runWorkers(F function, Args&&... args);

    std::optional<Task> steal(int id);
    
    std::optional<Task> readFileQueue(void);
    bool writeFileQueue(Task&& task);
    
    std::optional<Task> readDirQueue(void);
    bool writeDirQueue(Task&& task);

private:
    std::vector<Worker>   m_workers;

    std::string           m_pattern;

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
        worker.run(function, *this, std::forward<Args>(args)...);
    }

    for (auto& worker : m_workers) {
        worker.join();
    }
}

} // namespace joltgrep
