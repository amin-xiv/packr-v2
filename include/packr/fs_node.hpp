#include <packr/types.hpp>
#include <filesystem>

namespace packr {

// Represents directories, acts like a DIR*
class Directory {
  public:
    Directory() = delete;
    Directory(std::filesystem::path dir_path);
    [[nodiscard]] const std::filesystem::directory_entry& entry_obj() const noexcept;
    [[nodiscard]] const std::filesystem::path& path_obj() const noexcept;
    operator bool() const noexcept;

  private:
    const std::filesystem::path dir_path;
    const std::filesystem::directory_entry directory;
    dir_type type;
    std::string secondary_path; // Points to target directory if it's a symlink
    bool is_valid{};
    std::string error_message;
};

// Represents files, acts like a FILE*
class File {
  public:
    File() = delete;
    File(std::filesystem::path file_path);
    [[nodiscard]] const std::filesystem::directory_entry& entry_obj() const noexcept;
    [[nodiscard]] const std::filesystem::path& path_obj() const noexcept;
    operator bool() const noexcept;

  private:
    const std::filesystem::path file_path;
    const std::filesystem::directory_entry file;
    file_type type;
    std::string secondary_path; // points to block device path, target path(if symlink)..etc
    bool is_valid{};
    std::string error_message;
};

} // namespace packr
