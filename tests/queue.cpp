#include <thread>
#include <vector>
#include <optional>
#include <iostream>

#include "tests.h"
#include "clqueue.h"

void testSingleThread(void)
{
    static constexpr char testName[] = "testSingleThread";
    static constexpr int  maxIterations = 2 << 10;

    tests::start(testName);

    CL::Queue<int> queue{};

    for (int i = 0; i < maxIterations; ++i) {
        
        if (i % 3 == 1) {
            
            std::size_t prevSize = queue.size();
            std::optional<int> val = queue.pop();
            tests::check(val.has_value(), "queue does not pop() correctly");
            tests::check(*val % 3 == 0, "queue does not pop() the correct value");
            tests::check(prevSize == queue.size() + 1, 
                    "queue does not have the correct size after pop()");
        } else {
        
            std::size_t prevSize = queue.size();
            queue.push(std::move(i));
            tests::check(prevSize == queue.size() - 1, 
                    "queue does not have the correct size after push()");
            tests::check(queue.capacity() >= queue.size(), 
                    "queue does not have the correct capacity");
        }
    }

    tests::end(testName);
}

void testStealThread(std::vector<CL::Queue<int>>& queues, int id)
{
    static constexpr int  maxIterations = 2 << 10;

    for (int i = 0; i < maxIterations; ++i) {
        
        switch (i % 6) {

        case 5:
        case 1: {
            std::optional<int> val = queues[id].pop();
            tests::check(val.has_value() || queues[id].empty(), 
                    "queue does not pop() correctly");
            break;
        }

        case 3: {
            int randomQueue = rand() % queues.size();
            std::optional<int> val = queues[randomQueue].steal();
            tests::check(!val.has_value() || *val % 2 == 0, 
                    "queue does not steal() correctly");
            break;
        }

        default:
            queues[id].push(std::move(i));
            tests::check(queues[id].capacity() >= queues[id].size(), 
                    "queue does not have the correct capacity");
            break;
        }
    }
}

void testSteal(int numThreads)
{
    static constexpr char testName[] = "testSingleThread";
    
    tests::start(testName, numThreads);

    std::vector<CL::Queue<int>> queues(numThreads);
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(&testStealThread, std::ref(queues), i);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    tests::end(testName, numThreads);
}

void tests::queue(void)
{
    static constexpr char testName[] = "Tests Queue";

    tests::sectionStart(testName);

    testSingleThread();
    for (int i = 3; i < 10; ++i) {
        testSteal(i);
    }

    tests::sectionEnd(testName);
}
