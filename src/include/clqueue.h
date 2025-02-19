#pragma once

#include <iostream>

#include <memory>
#include <atomic>
#include <cassert>
#include <vector>
#include <optional>

#include "cache.h"

/*
/ Chase-Lev work stealing queue
/ https://www.dre.vanderbilt.edu/~schmidt/PDF/work-stealing-dequeue.pdf
/ inspired by:
/ https://github.com/ConorWilliams/ConcurrentDeque/tree/main
/
*/

// TODO: Nasty bug with std::size_t underflow FIX IMMEDIATELY!

namespace CL {

static constexpr std::memory_order relaxed = std::memory_order_relaxed;
static constexpr std::memory_order consume = std::memory_order_consume;
static constexpr std::memory_order acquire = std::memory_order_acquire;
static constexpr std::memory_order release = std::memory_order_release;
static constexpr std::memory_order seq_cst = std::memory_order_seq_cst;

template <typename T>
class Buffer {
public:
    explicit Buffer(std::size_t cap)
        : m_cap{cap}, m_mask{cap - 1}, m_buffer{std::make_unique<T[]>(cap)}
    {}

    std::size_t capacity() const noexcept 
    {
        return m_cap;
    }

    void write(std::size_t idx, T&& object) noexcept 
    {
        m_buffer[idx & m_mask] = std::move(object);
    }

    T read(std::size_t idx) const noexcept 
    {
        return std::move(m_buffer[idx & m_mask]);
    }

    Buffer<T>* resize(std::size_t b, std::size_t t) const 
    {
        Buffer<T>* ptr = new Buffer{2 * m_cap};
        for (std::size_t i = t; i != b; ++i) {
            ptr->write(i, read(i));
        }

        return ptr;
    }

private:
    std::size_t m_cap;
    std::size_t m_mask;

    std::unique_ptr<T[]> m_buffer = std::make_unique<T[]>(m_cap);
};

template <typename T>
class Queue {
public:
    explicit Queue(std::size_t cap = 1024); 
    ~Queue() noexcept;

    Queue(Queue const& other) = delete;
    Queue& operator=(Queue const& other) = delete;

    std::size_t size() const noexcept;
    std::size_t capacity() const noexcept;
    bool empty() const noexcept;

    void push(T&& object);
    std::optional<T> pop(void) noexcept;
    std::optional<T> steal(void) noexcept;

private:
    alignas(DESTRUCTIVE_INTER_SIZE) std::atomic<std::size_t> m_top;
    alignas(DESTRUCTIVE_INTER_SIZE) std::atomic<std::size_t> m_bottom;
    alignas(DESTRUCTIVE_INTER_SIZE) std::atomic<Buffer<T>*> m_buffer;

    std::vector<std::unique_ptr<Buffer<T>>> m_garbage;
};

template <typename T>
Queue<T>::Queue(std::size_t cap)
    : m_top(0), m_bottom(0), m_buffer(new Buffer<T>{cap}) 
{
    m_garbage.reserve(32);
}

template <typename T>
Queue<T>::~Queue() noexcept 
{
    delete m_buffer.load();
}

template <typename T>
std::size_t Queue<T>::size() const noexcept 
{
    std::size_t b = m_bottom.load(relaxed); 
    std::size_t t = m_top.load(relaxed); 
    return static_cast<std::size_t>(b >= t ? b - t : 0);
}

template <typename T>
std::size_t Queue<T>::capacity() const noexcept 
{
    return m_buffer.load(relaxed)->capacity();
}

template <typename T>
bool Queue<T>::empty() const noexcept 
{
    return !size();
}

template <typename T>
void Queue<T>::push(T&& object) 
{
    std::size_t b = m_bottom.load(relaxed);
    std::size_t t = m_top.load(relaxed);
    auto buf = m_buffer.load(relaxed);

    if (buf->capacity() < (b - t) + 1) {
        m_garbage.emplace_back(std::exchange(buf, buf->resize(b, t)));
        m_buffer.store(buf, relaxed);
    }
    
    buf->write(b, std::move(object));
    std::atomic_thread_fence(release);
    m_bottom.store(b + 1, relaxed);
}

template <typename T>
std::optional<T> Queue<T>::pop(void) noexcept 
{
    std::size_t b = m_bottom.load(relaxed) - 1;
    auto buf = m_buffer.load(relaxed);

    m_bottom.store(b, relaxed);

    std::atomic_thread_fence(seq_cst);
    std::size_t t = m_top.load(relaxed);

    if (b != SIZE_MAX && t <= b) {
        if (t == b) {
            
            if (!m_top.compare_exchange_strong(t, t + 1, seq_cst, relaxed)) {
                m_bottom.store(b + 1, relaxed);
                return std::nullopt;
            }
            m_bottom.store(b + 1, relaxed);
        }

        return buf->read(b);
    
    } else {
        m_bottom.store(b + 1, relaxed);
        return std::nullopt;
    }
}

template <typename T>
std::optional<T> Queue<T>::steal(void) noexcept
{
    std::size_t t = m_top.load(acquire);
    std::atomic_thread_fence(seq_cst);
    std::size_t b = m_bottom.load(acquire);

    if (t < b) {
        T obj = m_buffer.load(consume)->read(t);

        if (!m_top.compare_exchange_strong(t, t + 1, seq_cst, relaxed)) {
            return std::nullopt;
        }

        return obj;
    } else {
        
        return std::nullopt;
    }
}

} // namespace CL
