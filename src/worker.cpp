#include "worker.h"

joltgrep::Worker::Worker(std::size_t queueCap)
    : m_queue(queueCap), m_thread{}, m_id{}, m_buffer(workerBufferSize)
{
}

void joltgrep::Worker::setId(int id)
{
    m_id = id;
}

int joltgrep::Worker::getId(void)
{
    return m_id;
}

std::vector<char>& joltgrep::Worker::getBuffer(void)
{
    return m_buffer;
}

void joltgrep::Worker::setSize(std::size_t size)
{
    m_bufferSize = size;
}

std::size_t joltgrep::Worker::getSize(void)
{
    return m_bufferSize;
}

void joltgrep::Worker::join(void)
{
    if (m_thread.has_value()) {
        m_thread->join();
    }
}

std::optional<joltgrep::Task> joltgrep::Worker::steal(void)
{
    return m_queue.steal();
}

void joltgrep::Worker::push(joltgrep::Task&& task)
{
    m_queue.push(std::move(task));
}

std::optional<joltgrep::Task> joltgrep::Worker::pop(void)
{
    return m_queue.pop();
}

joltgrep::WorkSystem::WorkSystem(std::string&& pattern, 
        std::size_t numWorkers)
    : m_workers(numWorkers), m_boyerMoore{},
    m_fileQueue{}, m_fileLock{}, m_dirQueue{}, m_dirLock()
{
    m_pattern = std::move(pattern);

    for (int i = 0; i < numWorkers; ++i) {
        m_workers[i].setId(i);
    }

    // TODO: Check if m_pattern is eligible for Boyer-Moore
    m_boyerMoore = BoyerMoore(m_pattern); 
}

std::string_view joltgrep::WorkSystem::getPattern(void)
{
    return m_pattern;
}

std::optional<joltgrep::Task> joltgrep::WorkSystem::steal(int id)
{
    int randomQueue;
    do {
        randomQueue = rand() % m_workers.size(); 
    } while (randomQueue == id);
    
    return m_workers[randomQueue].steal();
}

std::optional<joltgrep::Task> joltgrep::WorkSystem::readFileQueue(void)
{
    std::optional<joltgrep::Task> task{};

    if (m_fileLock.try_lock()) {
    
        if (!m_fileQueue.empty()) {
            task = m_fileQueue.front();
            m_fileQueue.pop();
        }

        m_fileLock.unlock();
    }

    return task;
}

bool joltgrep::WorkSystem::writeFileQueue(joltgrep::Task&& task)
{
    if (m_fileLock.try_lock()) {
    
        m_fileQueue.push(std::move(task));
        m_fileLock.unlock();
        return true;
    }

    return false;
}

std::optional<joltgrep::Task> joltgrep::WorkSystem::readDirQueue(void)
{
    std::optional<Task> task{};

    if (m_dirLock.try_lock()) {
    
        if (!m_dirQueue.empty()) {
            task = m_dirQueue.front();
            m_dirQueue.pop();
        }

        m_dirLock.unlock();
    }

    return task;
}

bool joltgrep::WorkSystem::writeDirQueue(joltgrep::Task&& task)
{
    if (m_dirLock.try_lock()) {
    
        m_dirQueue.push(std::move(task));
        m_dirLock.unlock();
        return true;
    }

    return false;
}

std::optional<BoyerMoore>& joltgrep::WorkSystem::getBoyerMoore(void)
{
    return m_boyerMoore;
}
