#include <iostream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <thread>

#include "search.h"
#include "worker.h"
#include "re2/re2.h"
#include "print.h"

std::optional<joltgrep::Task>
getTask(joltgrep::WorkSystem& workSystem, joltgrep::Worker& worker)
{
    std::optional<joltgrep::Task> task = worker.pop();
    if (task.has_value()) {
        joltgrep::debugPrintf("thread %d pop %s %d %d\n", worker.getId(), task->getPath(),
                task->getId(), task->getOwnerId());
        return task;   
    }

    for (int i = 0; i < 3; ++i) {
        task = workSystem.steal(worker.getId());
        if (task.has_value()) {
            joltgrep::debugPrintf("thread %d steal %s %d %d\n", 
                    worker.getId(), task->getPath(), task->getId(), task->getOwnerId());
            return task;
        }

        task = workSystem.readDirQueue();
        if (task.has_value()) {
            joltgrep::debugPrintf("thread %d read dir queue %s %d %d\n", 
                    worker.getId(), task->getPath(), task->getId(), task->getOwnerId());
            return task;
        }

        task = workSystem.readFileQueue();
        if (task.has_value()) {
            joltgrep::debugPrintf("thread %d read file queue %s %d %d\n", 
                    worker.getId(), task->getPath(), task->getId(), task->getOwnerId());
            return task;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return std::nullopt;
}

void searchDirectory(joltgrep::WorkSystem& workSystem, 
        joltgrep::Worker& worker, joltgrep::Task& task)
{
    static thread_local int id = 0;

    for (const auto& dirEntry : fs::directory_iterator{task.getPath()}) {
        
        joltgrep::Task entryTask{dirEntry.path().string(), ++id, worker.getId()};
    
        joltgrep::debugPrintf("thread %d pushed %s %d %d\n", 
            worker.getId(), entryTask.getPath(), id, worker.getId());

        switch (entryTask.getType()) {
        case joltgrep::DIRECTORY_TASK:
            while (!workSystem.writeDirQueue(std::move(entryTask)));
            break;

        case joltgrep::FILE_TASK:
            worker.push(std::move(entryTask));
            break;
        
        default:
            std::cout << "searchDirectory: Unknown Task Type\n";
            exit(1);
        }
    }
}

void searchThread(joltgrep::WorkSystem& workSystem, joltgrep::Worker& worker)
{
    std::optional<joltgrep::Task> task;
    while ((task = getTask(workSystem, worker)).has_value()) {

        switch (task->getType()) {
        case joltgrep::DIRECTORY_TASK:
            searchDirectory(workSystem, worker, *task);
            break;

        case joltgrep::FILE_TASK:
            joltgrep::searchFile(workSystem, worker, *task);
            break;
        
        default:
            std::cout << "searchThread: Unknown Task Type\n";
            exit(1);
        }
    }
    
    joltgrep::debugPrintf("thread %d statistics: %d files\n", 
            worker.getId(), worker.getFileRead());
}

void joltgrep::search(std::vector<fs::path>& paths, 
        std::string& pattern)
{
    static int id = 0;
    joltgrep::WorkSystem workSystem{std::move(pattern)};
    
    for (const auto& path : paths) {
        joltgrep::Task task{path.string(), ++id, -1};
        workSystem.writeDirQueue(std::move(task));
    }
 
    workSystem.runWorkers(&searchThread);
}
