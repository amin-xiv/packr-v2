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
    const std::filesystem::path m_dir_path;
    const std::filesystem::directory_entry m_directory;
    dir_type m_type;
    std::string m_secondary_path; // Points to target directory if it's a symlink
    bool m_is_valid{};
    std::string m_error_message;
};

// Represents files, acts kinda like a FILE*
class File {
  public:
    File() = delete;
    File(const std::filesystem::path& file_path);
    [[nodiscard]] const std::filesystem::directory_entry& entry_obj() const noexcept;
    [[nodiscard]] const std::filesystem::path& path_obj() const noexcept;
    operator bool() const noexcept;
    void refresh() noexcept;

  protected:
    const std::filesystem::path m_file_path;
    std::filesystem::directory_entry m_file;
    file_type m_type;
    std::string m_secondary_path; // points to block device path, target path(if symlink)..etc
    bool m_is_valid{};
    std::string m_error_message;
};

// Derrived from 'File' to allow strictly reading from the file
class File_R : public File {
  public:
    using File::File; // Inherits constructor from File class

    [[nodiscard]] bool setup_stream(const open_type type);
    [[nodiscard]] bool read(char* buffer, std::streamsize count);

  private:
    std::ifstream m_stream;
};

// Derrived from 'File' to allow strictly writing to the file
class File_W : public File {
  public:
    using File::File; // Inherits constructor from File class

    [[nodiscard]] bool setup_stream(const open_type type);
    [[nodiscard]] bool write(char* buffer, std::streamsize count);

  private:
    std::ofstream m_stream;
};

} // namespace packr
