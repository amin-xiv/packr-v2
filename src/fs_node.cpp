#include <packr/types.hpp>
#include <packr/utils.hpp>
#include <packr/fs_node.hpp>
#include <string_view>
#include <system_error>
#include <filesystem>
#include <cassert>

namespace fs = std::filesystem;

namespace packr {

Directory::Directory(const fs::path& path) : m_dir_path(fs::absolute(path)), m_directory(m_dir_path) {
    std::error_code err{};                                       // Just to avoid exceptions throw by fs::is_directory
    fs::file_status sym_status{m_directory.symlink_status(err)}; // symlink_status to NOT follow symlinks to their targets

    if(!fs::exists(sym_status)) {
        m_is_valid = false;
        m_error_message = std::string{"The path "} + m_dir_path.string() + std::string{" doesn't point to a valid directory!"};

    } else if(fs::is_directory(m_directory, err) && fs::is_symlink(sym_status)) {
        // First condition to check if the entry is a symlink, and the second to check if it was an actual directory
        m_is_valid = true;
        m_type = dir_type::symlink;
        m_secondary_path = packr::read_symlink(m_dir_path);

    } else if(fs::is_directory(sym_status)) {
        m_is_valid = true;
        m_type = dir_type::regular;
    } else {
        m_is_valid = false;
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

void Directory::refresh() noexcept {
    std::error_code err;
    m_directory.refresh(err);
}

std::string_view Directory::err() const noexcept {
    return m_error_message;
}

File::File(const std::filesystem::path& file_path, const bool symlinks_as_symlinks)
    : m_file_path(fs::absolute(file_path)), m_file(m_file_path) {
    // symlinks_as_symlinks flag is used to denote that when packing the target of a symlink
    // which is a regular file, to not set is types as file_type::symlink and rather file_type::regular

    std::error_code err{};                                  //  To avoid exceptions
    fs::file_status sym_status{m_file.symlink_status(err)}; // symlink_status to NOT follow symlinks to their targets

    if(!fs::exists(sym_status)) {
        m_is_valid = false;
        m_error_message = std::string{"The path "} + file_path.string() + std::string{" doesn't point to a valid file!"};

    } else if(fs::is_symlink(sym_status)) {
        m_is_valid = true;
        m_type = (!symlinks_as_symlinks && fs::exists(m_file.status())) ? file_type::regular : file_type::symlink;
        m_secondary_path = fs::read_symlink(m_file_path);

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

std::string_view File::err() const noexcept {
    return m_error_message;
}

bool File_R::setup_stream(const open_type type) {
    std::error_code err;

    if(type == open_type::fresh) {
        m_stream.open(this->path_obj().string(), std::ios::binary | std::ios::trunc);
        this->refresh();
        return m_stream.is_open();
    }

    // Check if the file exists
    if(!fs::exists(this->entry_obj().symlink_status(err))) {
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

int File_R::get_fd() const noexcept {
    assert(m_stream.is_open() && "attempted to get a file descriptor of a non-open stream");
    return m_stream.native_handle();
}

pos_type File_R::get_offset() noexcept {
    assert(m_stream.is_open());
    return m_stream.tellg();
}

const std::istream& File_R::set_offset(const pos_type& pos, std::ios_base::seekdir seek_type) noexcept {
    assert(m_stream.is_open());
    return m_stream.seekg(pos, seek_type);
}

bool File_W::setup_stream(const open_type type) {
    std::error_code err;

    if(type == open_type::fresh) {
        m_stream.open(this->path_obj().string(), std::ios::binary | std::ios::trunc);
        this->refresh();
        return m_stream.is_open();
    }

    // Check if the file exists
    if(!fs::exists(this->entry_obj().symlink_status(err))) {
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

int File_W::get_fd() const noexcept {
    assert(m_stream.is_open() && "attempted to get a file descriptor of a non-open stream");
    return m_stream.native_handle();
}

pos_type File_W::get_offset() noexcept {
    assert(m_stream.is_open());
    return m_stream.tellp();
}

const std::ostream& File_W::set_offset(const pos_type& pos, std::ios_base::seekdir seek_type) noexcept {
    assert(m_stream.is_open());
    return m_stream.seekp(pos, seek_type);
}

File_sym::File_sym(const fs::path& file_path) {
    std::error_code err;

    const fs::file_status file_sym_stat{fs::symlink_status(file_path, err)};
    const bool symlink_exists{fs::exists(file_sym_stat)};
    assert(fs::is_symlink(file_sym_stat) && symlink_exists &&
           "Tried to construct a File_W obj with a non-symlink/existent entry");

    // As assertions won't run in release mode
    if(!symlink_exists) {
        m_error_message = "Symlink doesn't exist";
        m_is_valid = false;
        return;
    }

    m_symlink_path = file_path.is_absolute() ? file_path : fs::absolute(file_path, err);
    m_symlink_ent = fs::directory_entry{m_symlink_path, err};
    m_target_path = fs::canonical(m_symlink_path, err);

    if(fs::exists(m_symlink_path, err)) { // This will check whether the symlink target exists or not
        if(fs::is_directory(m_symlink_path, err)) {
            m_target_type = entry_type::directory;
        } else if(fs::is_regular_file(m_symlink_path, err)) {
            m_target_type = entry_type::regular_file;
        } else {
            m_target_type = entry_type::special;
        }
    }

    m_is_valid = true;
}

const fs::directory_entry& File_sym::entry_obj() const noexcept {
    return m_symlink_ent;
}

const fs::path& File_sym::path_obj() const noexcept {
    return m_symlink_path;
}

const entry_type& File_sym::target_type() const noexcept {
    return m_target_type;
}

const fs::path& File_sym::target_path() const noexcept {
    return m_target_path;
}

bool File_sym::has_target() const noexcept {
    return !m_target_path.empty();
}

File_sym::operator bool() const noexcept {
    return m_is_valid;
}

void File_sym::refresh() noexcept {
    std::error_code err;
    m_symlink_ent.refresh(err);
}

std::string_view File_sym::err() const noexcept {
    return m_error_message;
}

} // namespace packr
