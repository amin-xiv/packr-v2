#pragma once

#include <packr/types.hpp>
#include <filesystem>
#include <fstream>

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
    File(const std::filesystem::path& file_path);
    [[nodiscard]] const std::filesystem::directory_entry& entry_obj() const noexcept;
    [[nodiscard]] const std::filesystem::path& path_obj() const noexcept;
    operator bool() const noexcept;

  protected:
    const std::filesystem::path file_path;
    const std::filesystem::directory_entry file;
    file_type type;
    std::string secondary_path; // points to block device path, target path(if symlink)..etc
    bool is_valid{};
    std::string error_message;
};

// Derrived from 'File' to allow strictly reading from the file
class File_R : public File {
  public:
    using File::File; // Inherits constructor from File class

    [[nodiscard]] bool setup_stream() noexcept;
    [[nodiscard]] bool read(char* buffer, std::streamsize count);

  private:
    std::ifstream stream;
};

// Derrived from 'File' to allow strictly writing to the file
class File_W : public File {
  public:
    using File::File; // Inherits constructor from File class

    [[nodiscard]] bool setup_stream() noexcept;
    [[nodiscard]] bool write(char* buffer, std::streamsize count);

  private:
    std::ofstream stream;
};

} // namespace packr
