#pragma once

#include <array>
#include <string>

namespace joltgrep {

enum TaskType {
    NullTask,
    FileTask,
    DirectoryTask,
};

class Task {
public:
    Task(TaskType type, std::string_view path);
    Task(std::string_view path, int id, int ownerId);
    Task() = default;

    TaskType getType(void);
    const char* getPath(void);

    // debug
    int getId(void);
    int getOwnerId(void);

private:
    TaskType m_type;
    //std::filesystem::path m_path{};
    std::array<char, 1024> m_path;

    // debug
    int m_id = -1;
    int m_ownerId = -1;
};

} // namespace joltgrep
