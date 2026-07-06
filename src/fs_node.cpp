#include <packr/fs_node.hpp>
#include <utility>
#include <system_error>
#include <filesystem>

namespace fs = std::filesystem;

namespace packr {

Directory::Directory(fs::path path) : dir_path(std::move(path)), directory(dir_path) {
    fs::file_status sym_status{this->directory.symlink_status()}; // symlink_status to NOT follow symlinks to their targets
    std::error_code err{};                                        // Just to avoid exceptions throw by fs::is_directory

    if(!fs::exists(sym_status)) {
        this->is_valid = false;
        this->error_message = std::string{"The path "} + dir_path.string() + std::string{" doesn't point to a valid directory!"};

    } else if(fs::is_directory(this->directory, err) && fs::is_symlink(sym_status)) {
        // First condition to check if the entry is a symlink, and the second to check if it was an actual directory
        this->is_valid = true;
        this->type = dir_type::symlink;

        // REVISE
        this->secondary_path = fs::read_symlink(this->dir_path).string();

    } else if(fs::is_directory(sym_status)) {
        this->is_valid = true;
        this->type = dir_type::regular;
    } else {
        this->is_valid = false;
        this->error_message = "Unknown directory type!";
    }
}

const fs::directory_entry& Directory::entry_obj() const noexcept {
    return this->directory;
}

const fs::path& Directory::path_obj() const noexcept {
    return this->dir_path;
}

Directory::operator bool() const noexcept {
    return this->is_valid;
}

File::File(const std::filesystem::path& file_path) : file_path(file_path), file(this->file_path) {
    fs::file_status sym_status{this->file.symlink_status()}; // symlink_status to NOT follow symlinks to their targets
    std::error_code err{};                                   // Just to avoid exceptions throw by fs::is_directory

    if(!fs::exists(sym_status) || this->file.is_directory(err) || fs::is_directory(sym_status)) {
        this->is_valid = false;
        this->error_message = std::string{"The path "} + file_path.string() + std::string{" doesn't point to a valid file!"};

    } else if(fs::is_symlink(sym_status)) {
        this->is_valid = true;
        this->type = file_type::symlink;

        // REVISE, both the functionality and exception safety
        this->secondary_path = fs::read_symlink(this->file_path).string();

    } else if(fs::is_regular_file(sym_status)) {
        this->is_valid = true;
        this->type = file_type::regular;

    } else {
        this->is_valid = false;
        this->type = file_type::special;
        this->error_message = std::string{"Skipping special file: "} + this->file_path.string();
    }
}

const fs::directory_entry& File::entry_obj() const noexcept {
    return this->file;
}

const fs::path& File::path_obj() const noexcept {
    return this->file_path;
}

File::operator bool() const noexcept {
    return this->is_valid;
}

} // namespace packr
