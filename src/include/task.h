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
    Task(TaskType type, const std::filesystem::path path);
    Task(const std::filesystem::path path, int id, int ownerId);
    Task() = default;

    TaskType getType(void);
    std::filesystem::path& getPath(void);

    // debug
    int getId(void);
    int getOwnerId(void);

private:
    TaskType m_type;
    std::filesystem::path m_path{};
    
    // debug
    int m_id = -1;
    int m_ownerId = -1;
};

} // namespace joltgrep
