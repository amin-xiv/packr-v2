#pragma once

#include <packr/types.hpp>

#include <filesystem>
#include <fstream>

namespace packr {

// Represents directories, acts like a DIR*
class Directory final {
  public:
    Directory() = delete;
    explicit Directory(const std::filesystem::path& dir_path);

    [[nodiscard]] const std::filesystem::directory_entry& entry_obj() const noexcept;
    [[nodiscard]] const std::filesystem::path& path_obj() const noexcept;
    [[nodiscard]] const dir_type& type() const noexcept;
    [[nodiscard]] const std::filesystem::path& secondary_path() const noexcept;
    operator bool() const noexcept;
    void refresh() noexcept;
    [[nodiscard]] std::string_view err() const noexcept;

  private:
    const std::filesystem::path m_dir_path;
    std::filesystem::directory_entry m_directory;
    dir_type m_type;
    std::filesystem::path m_secondary_path; // Points to target directory if it's a symlink
    bool m_is_valid{};
    std::string m_error_message;
};

// Represents files, acts kinda like a FILE*
class File {
  public:
    File() = delete;
    explicit File(const std::filesystem::path& file_path, bool symlinks_as_symlinks = true);

    [[nodiscard]] const std::filesystem::directory_entry& entry_obj() const noexcept;
    [[nodiscard]] const std::filesystem::path& path_obj() const noexcept;
    [[nodiscard]] const file_type& type() const noexcept;
    [[nodiscard]] const std::filesystem::path& secondary_path() const noexcept;
    operator bool() const noexcept;
    void refresh() noexcept;
    [[nodiscard]] std::string_view err() const noexcept;

  protected:
    const std::filesystem::path m_file_path;
    std::filesystem::directory_entry m_file;
    file_type m_type;
    std::filesystem::path m_secondary_path; // points to block device path, target path(if symlink)..etc
    bool m_is_valid{};
    std::string m_error_message;
};

// Derrived from 'File' to allow strictly reading from the file
class File_R final : public File {
  public:
    using File::File; // Inherits constructor from File class

    [[nodiscard]] bool setup_stream(const open_type type);
    [[nodiscard]] bool read(char* buffer, std::streamsize count);
    [[nodiscard]] int get_fd() const noexcept;
    [[nodiscard]] pos_type get_offset() noexcept;
    const std::istream& set_offset(const pos_type& pos, std::ios_base::seekdir = std::ios_base::cur) noexcept;

  private:
    std::ifstream m_stream;
};

// Derrived from 'File' to allow strictly writing to the file
class File_W final : public File {
  public:
    using File::File; // Inherits constructor from File class

    [[nodiscard]] bool setup_stream(const open_type type);
    [[nodiscard]] bool write(const char* buffer, std::streamsize count);
    [[nodiscard]] int get_fd() const noexcept;
    [[nodiscard]] pos_type get_offset() noexcept;
    const std::ostream& set_offset(const pos_type& pos, std::ios_base::seekdir = std::ios_base::cur) noexcept;

  private:
    std::ofstream m_stream;
};

// Used to represent symbolic links(NOT their targets)
class File_sym final {
  public:
    File_sym() = delete;
    explicit File_sym(const std::filesystem::path& file_path);

    [[nodiscard]] const std::filesystem::directory_entry& entry_obj() const noexcept;
    [[nodiscard]] const std::filesystem::path& path_obj() const noexcept;
    [[nodiscard]] const entry_type& target_type() const noexcept;
    [[nodiscard]] const std::filesystem::path& target_path() const noexcept;
    [[nodiscard]] bool has_target() const noexcept;
    operator bool() const noexcept;
    void refresh() noexcept;
    [[nodiscard]] std::string_view err() const noexcept;
    [[nodiscard]] int get_fd() const noexcept;

  private:
    std::filesystem::path m_symlink_path;
    std::filesystem::directory_entry m_symlink_ent;
    entry_type m_target_type;
    std::filesystem::path m_target_path;
    bool m_is_valid{};
    std::string m_error_message;
};

} // namespace packr
