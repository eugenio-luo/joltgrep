#pragma once

#include <filesystem>

namespace joltgrep {

enum TaskType {
    NullTask,
    FileTask,
    DirectoryTask,
};

class Task {
public:
    Task(TaskType type, const std::filesystem::path& path);
    Task(const std::filesystem::path& path);
    Task() = default;

    TaskType getType(void);
    std::filesystem::path& getPath(void);

private:
    TaskType m_type;
    std::filesystem::path m_path{};
};

} // namespace joltgrep
