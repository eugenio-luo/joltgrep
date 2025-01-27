#include "task.h"

joltgrep::Task::Task(TaskType type, const std::filesystem::path& path)
    : m_type{type}, m_path{path}
{
}

joltgrep::Task::Task(const std::filesystem::path& path)
    : m_type{NullTask}, m_path{path}
{
    if (std::filesystem::is_directory(m_path)) {
        m_type = DirectoryTask;
    } else if (std::filesystem::is_regular_file(m_path)) {
        m_type = FileTask;
    } else {
        m_type = NullTask;
    }
}

joltgrep::TaskType joltgrep::Task::getType(void)
{
    return m_type;
}

std::filesystem::path& joltgrep::Task::getPath(void) 
{
    return m_path;
}