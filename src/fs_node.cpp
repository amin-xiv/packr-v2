#include "packr/types.hpp"
#include <packr/fs_node.hpp>
#include <system_error>
#include <filesystem>

namespace fs = std::filesystem;

namespace packr {

// NOTE: should I read symlinks here??
Directory::Directory(fs::path path) : m_dir_path(std::move(path)), m_directory(m_dir_path) {
    std::error_code err{};                                    // Just to avoid exceptions throw by fs::is_directory
    fs::file_status sym_status{m_directory.symlink_status()}; // symlink_status to NOT follow symlinks to their targets

    if(!fs::exists(sym_status)) {
        m_is_valid = false;
        m_error_message = std::string{"The path "} + m_dir_path.string() + std::string{" doesn't point to a valid directory!"};

    } else if(fs::is_directory(m_directory, err) && fs::is_symlink(sym_status)) {
        // First condition to check if the entry is a symlink, and the second to check if it was an actual directory
        m_is_valid = true;
        m_type = dir_type::symlink;

        // REVISE
        m_secondary_path = fs::read_symlink(m_dir_path, err);

    } else if(fs::is_directory(sym_status)) {
        m_is_valid = true;
        m_type = dir_type::regular;
    } else {
        m_is_valid = false;
        // std::println("\n PATH: {}\n", m_dir_path.string());
        m_error_message = "Unknown directory type!";
    }
}

const fs::directory_entry& Directory::entry_obj() const noexcept {
    return m_directory;
}

const fs::path& Directory::path_obj() const noexcept {
    return m_dir_path;
}

const dir_type& Directory::type() const noexcept {
    return m_type;
}

const fs::path& Directory::secondary_path() const noexcept {
    return m_secondary_path;
}

Directory::operator bool() const noexcept {
    return m_is_valid;
}

File::File(const std::filesystem::path& file_path) : m_file_path(file_path), m_file(m_file_path) {
    std::error_code err{};                                  //  To avoid exceptions
    fs::file_status sym_status{m_file.symlink_status(err)}; // symlink_status to NOT follow symlinks to their targets
    fs::directory_entry file_ent{file_path, err};

    if(!fs::exists(sym_status) || m_file.is_directory(err)) {
        m_is_valid = false;
        m_error_message = std::string{"The path "} + file_path.string() + std::string{" doesn't point to a valid file!"};

    } else if(fs::is_symlink(sym_status)) {
        m_is_valid = true;
        m_type = file_type::symlink;

        // NOTE: REVISE, both the functionality and exception safety
        // NOTE: tests
        const fs::path secondary_path{fs::read_symlink(m_file_path, err)};
        const fs::directory_entry secondary_ent{secondary_path, err};
        if(!secondary_path.empty()) {
            if(secondary_path.is_absolute()) {
                m_secondary_path = fs::exists(secondary_ent.symlink_status(err)) ? secondary_path : m_secondary_path;
            } else {
                fs::path target_parent_dir{file_path.parent_path()};
                fs::directory_entry symlink_target_path{target_parent_dir / secondary_path, err};
                fs::directory_entry symlink_target_ent{symlink_target_path, err};
                // m_secondary_path won't change if it target doesn't exist
                m_secondary_path = fs::exists(symlink_target_ent.symlink_status(err)) ? symlink_target_path : m_secondary_path;
            }
        }

    } else if(fs::is_regular_file(sym_status)) {
        m_is_valid = true;
        m_type = file_type::regular;

    } else {
        m_is_valid = false;
        m_type = file_type::special;
        m_error_message = std::string{"Skipping special file: "} + m_file_path.string();
    }
}

const fs::directory_entry& File::entry_obj() const noexcept {
    return m_file;
}

const fs::path& File::path_obj() const noexcept {
    return m_file_path;
}

const file_type& File::type() const noexcept {
    return m_type;
}

const fs::path& File::secondary_path() const noexcept {
    return m_secondary_path;
}

File::operator bool() const noexcept {
    return m_is_valid;
}

void File::refresh() noexcept {
    // dummy error code
    std::error_code err;
    m_file.refresh(err);
}

bool File_R::setup_stream(const open_type type) {
    // dummy error_code

    if(type == open_type::fresh) {
        m_stream.open(this->path_obj().string(), std::ios::binary | std::ios::trunc);
        this->refresh();
        return m_stream.is_open();
    }

    // Check if the file exists
    if(!fs::exists(this->entry_obj().symlink_status())) {
        return false;
    }

    m_stream.open(this->path_obj().string(), std::ios::binary);
    return m_stream.is_open();
}

bool File_R::read(char* buffer, std::streamsize count) {
    if(!m_stream.is_open() || buffer == nullptr) {
        return false;
    }

    m_stream.read(buffer, count);
    return true;
}

bool File_W::setup_stream(const open_type type) {
    if(type == open_type::fresh) {
        m_stream.open(this->path_obj().string(), std::ios::binary | std::ios::trunc);
        this->refresh();
        return m_stream.is_open();
    }

    // Check if the file exists
    if(!fs::exists(this->entry_obj().symlink_status())) {
        return false;
    }

    m_stream.open(this->path_obj().string(), std::ios::binary);
    return m_stream.is_open();
}

bool File_W::write(const char* buffer, std::streamsize count) {
    if(!m_stream.is_open() || buffer == nullptr) {
        return false;
    }

    m_stream.write(buffer, count);
    return true;
}

} // namespace packr
