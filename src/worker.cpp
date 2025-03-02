#include "worker.h"
#include "print.h"

joltgrep::Worker::Worker(std::size_t queueCap)
    : m_queue{1024}, m_thread{}, m_id{}, m_buffers{},
    m_primary{0}, m_used{false}
{
    m_buffers[0].reserve(WORKER_BUFFER_SIZE);
    m_buffers[1].reserve(WORKER_BUFFER_SIZE);
}

void joltgrep::Worker::setId(int id)
{
    m_id = id;
}

int joltgrep::Worker::getId(void)
{
    return m_id;
}

void joltgrep::Worker::increaseFileRead(void)
{
    ++m_fileRead;
}

void joltgrep::Worker::increaseMatchFound(void)
{
    ++m_matchFound;
}

std::size_t joltgrep::Worker::getFileRead(void)
{
    return m_fileRead;
}

std::size_t joltgrep::Worker::getMatchFound(void)
{
    return m_matchFound;
}

std::pair<size_t, size_t> joltgrep::Worker::getLine(size_t pos)
{
    size_t left = pos, right = pos;
    size_t leftLimit = (m_used) ? 0 : WORKER_BUFFER_SIZE;
    size_t rightLimit = getSize() + WORKER_BUFFER_SIZE;

    while ((left > leftLimit && getChar(left) != '\n') || 
        (right < rightLimit && getChar(right) != '\n')) {
        
        if (left > leftLimit && getChar(left) != '\n') {
            --left;
        }
        if (right < rightLimit && getChar(right) != '\n') {
            ++right;
        }
    }

    if (left != leftLimit) {
        ++left;
    }

    if (right != rightLimit) {
        --right;
    }

    return {left, right};
}

joltgrep::buffer_t& joltgrep::Worker::getBuffer(void)
{
    return m_buffers[m_primary];
}

joltgrep::buffer_t& joltgrep::Worker::getSecondaryBuffer(void)
{
    return m_buffers[1 - m_primary];
}

void joltgrep::Worker::switchPrimary(void)
{
    m_primary = 1 - m_primary;
    m_used = true;
}

void joltgrep::Worker::resetUsed(void)
{
    m_used = false;
}

bool joltgrep::Worker::getUsed(void)
{
    return m_used;
}

char joltgrep::Worker::getChar(size_t pos)
{
    if (pos < WORKER_BUFFER_SIZE) {
        if (!m_used) {
            // TODO: handle error
            std::cout << pos << " " << WORKER_BUFFER_SIZE 
                << " pos < WORKER_BUFFER_SIZE but !m_used\n";
            throw;
        }
        return m_buffers[1 - m_primary][pos];
    }

    return m_buffers[m_primary][pos - WORKER_BUFFER_SIZE];
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

// TODO: improve this please
joltgrep::SearchType decideSearchType(std::string_view pattern)
{
    // TODO: We are too strict
    static constexpr std::string_view boyerMooreBanned = ".[]\\*+?^$()|";

    for (char c : pattern) {
        if (boyerMooreBanned.find(c) != std::string::npos) {
            return joltgrep::DEFAULT_SEARCH;
        }
    }

    return joltgrep::BOYER_MOORE_SEARCH;
}

joltgrep::WorkSystem::WorkSystem(std::string&& pattern, 
        std::size_t numWorkers)
    : m_workers(numWorkers), m_recommended{joltgrep::DEFAULT_SEARCH},
    // m_pattern{pattern}, m_patternEngine{std::nullopt}, m_boyerMoore{},
    m_pattern{pattern}, m_patternEngine{pattern}, m_boyerMoore{},
    m_fileQueue{}, m_fileLock{}, m_dirQueue{}, m_dirLock()
{
    for (int i = 0; i < numWorkers; ++i) {
        m_workers[i].w.setId(i);
    }

    m_recommended = decideSearchType(m_pattern); 

    switch (m_recommended) {
        case joltgrep::BOYER_MOORE_SEARCH:
            m_boyerMoore = BoyerMoore(std::move(pattern));
            break;

        case joltgrep::DEFAULT_SEARCH:
            // m_patternEngine{pattern};
            break;

        case joltgrep::AHO_CORASICK_SEARCH:
            debugPrintf("Not implemented yet!");
            break;
    }
}

joltgrep::SearchType joltgrep::WorkSystem::getSearchType(void)
{
    return m_recommended;
}

std::string& joltgrep::WorkSystem::getPattern(void)
{
    return m_pattern;
}

re2::RE2& joltgrep::WorkSystem::getPatternEngine(void)
{
    return m_patternEngine;
}

std::optional<joltgrep::Task> joltgrep::WorkSystem::steal(int id)
{
    int randomQueue;
    do {
        randomQueue = rand() % m_workers.size(); 
    } while (randomQueue == id);
    
    return m_workers[randomQueue].w.steal();
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
