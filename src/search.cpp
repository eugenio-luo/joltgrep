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
    std::optional<joltgrep::Task> task;

    task = worker.pop();
    if (task.has_value()) {
        std::stringstream ss;
        ss << "thread # " << worker.getId() << " pop " << task->getPath() << " " << task->getId() << " " << task->getOwnerId() << "\n";
        //joltgrep::debugPrint(ss.str());

        return task;
    }

    int i = 0;
    do {
        task = workSystem.steal(worker.getId());
        if (task.has_value()) {
            std::stringstream ss;
            ss << "thread # " << worker.getId() << " steal " << task->getPath() << " " << task->getId() << " " << task->getOwnerId() << "\n";
            //joltgrep::debugPrint(ss.str());
            return task;
        }

        task = workSystem.readDirQueue();
        if (task.has_value()) {
            std::stringstream ss;
            ss << "thread # " << worker.getId() << " readDir " << task->getPath() << " " << task->getId() << " " << task->getOwnerId() << "\n";
            //joltgrep::debugPrint(ss.str());
            return task;
        }

        if (!task.has_value()) {
            task = workSystem.readFileQueue();
        }
        if (!task.has_value()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } else {
            break;
        }
        ++i;
    } while (i <= 2);

    return task;
}

void searchDirectory(joltgrep::WorkSystem& workSystem, 
        joltgrep::Worker& worker, joltgrep::Task& task)
{
    static thread_local int id = 0;

    for (const auto& dirEntry : fs::directory_iterator{task.getPath()}) {
        joltgrep::Task entryTask{dirEntry.path(), ++id, worker.getId()};
    
        std::stringstream ss;
        ss << "thread # " << worker.getId() << " pushed " << entryTask.getPath() << " " << id << " " << worker.getId() << "\n";
        //joltgrep::debugPrint(ss.str());

        switch (entryTask.getType()) {
        case joltgrep::DirectoryTask:
            while (!workSystem.writeDirQueue(std::move(entryTask)));
            break;

        case joltgrep::FileTask:
            worker.push(std::move(entryTask));
            break;
        
        default:
            std::cout << "ERROR!!!\n";
            exit(1);
        }
    }
}

void searchThread(joltgrep::WorkSystem& workSystem, joltgrep::Worker& worker)
{
    std::optional<joltgrep::Task> task;

    while (true) {
        task = getTask(workSystem, worker);
        if (!task.has_value()) {
            break;
        }
        
        switch (task->getType()) {
        case joltgrep::DirectoryTask:
            searchDirectory(workSystem, worker, *task);
            break;

        case joltgrep::FileTask:
            joltgrep::searchFile(workSystem, worker, *task);
            break;
        
        default:
            std::cout << "ERROR!!!\n";
            exit(1);
        }
    }

}

void joltgrep::search(std::vector<fs::path>& paths, 
        std::string& pattern)
{
    static int id = 0;
    joltgrep::WorkSystem workSystem{std::move(pattern)};
    
    for (const auto& path : paths) {
        joltgrep::Task task{path, ++id, -1};
        workSystem.writeDirQueue(std::move(task));
    }
 
    workSystem.runWorkers(&searchThread);
}
