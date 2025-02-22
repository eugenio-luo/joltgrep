#include "task.h"

#include <filesystem>

joltgrep::Task::Task(std::string_view path, int id, int ownerId)
    : m_type{NULL_TASK}, m_id{id}, m_ownerId{ownerId}
{
    std::copy(path.begin(), path.end(), m_path.data());
    m_path[path.size()] = '\0';
    
    if (std::filesystem::is_directory(path)) {
        m_type = DIRECTORY_TASK;
    } else if (std::filesystem::is_regular_file(path)) {
        m_type = FILE_TASK;
    } else {
        m_type = NULL_TASK;
    }
}

joltgrep::TaskType joltgrep::Task::getType(void)
{
    return m_type;
}

const char* joltgrep::Task::getPath(void) 
{
    return m_path.data();
}

int joltgrep::Task::getId(void)
{
    return m_id;
}

int joltgrep::Task::getOwnerId(void)
{
    return m_ownerId;
}
