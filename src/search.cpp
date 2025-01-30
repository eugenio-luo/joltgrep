#include <iostream>
#include <sstream>
#include <filesystem>

#include "search.h"
#include "worker.h"
#include "re2/re2.h"

std::optional<joltgrep::Task>
getTask(joltgrep::WorkSystem& workSystem, joltgrep::Worker& worker)
{
    std::optional<joltgrep::Task> task;

    task = worker.pop();
    if (task.has_value()) {
        return task;
    }

    int i = 0;
    do {
        task = workSystem.steal(worker.getId());
        if (!task.has_value()) {
            task = workSystem.readDirQueue();
        }
        if (!task.has_value()) {
            task = workSystem.readFileQueue();
        }
        ++i;
    } while (!task.has_value() && i <= 3);

    return task;
}

void searchDirectory(joltgrep::WorkSystem& workSystem, 
        joltgrep::Worker& worker, joltgrep::Task& task)
{
    for (const auto& dirEntry : fs::directory_iterator{task.getPath()}) {
        joltgrep::Task entryTask{dirEntry.path()};
        
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
    RE2 re(workSystem.getPattern());

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
            joltgrep::searchFile(worker, *task, re);
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
    joltgrep::WorkSystem workSystem{std::move(pattern)};
    
    for (const auto& path : paths) {
        joltgrep::Task task{path};
        workSystem.writeDirQueue(std::move(task));
    }
 
    workSystem.runWorkers(&searchThread);
}
