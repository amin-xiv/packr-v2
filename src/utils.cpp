#include "packr/types.hpp"
#include <filesystem>
#include <packr/utils.hpp>
#include <packr/entry.hpp>
#include <print>
#include <cassert>
#include <unistd.h>
#include <cstring>
#include <optional>
#include <string>

namespace fs = std::filesystem;

namespace packr {

std::optional<std::string> join_to_path(const std::string& filename, const std::string& cwd) {
    if(filename.empty() || cwd.empty()) {
        return std::nullopt;
    }

    if(cwd.back() == '/') {
        return std::string{cwd + filename};
    }

    return std::string{cwd + '/' + filename};
}

void add_dirname(dir_entry* dir_ent, std::string named_as, const std::string& src_path) {
    char* named_as_ptr{named_as.data()};

    if(!named_as.empty()) {
        memcpy(dir_ent->m_dirname, named_as_ptr, strlen(named_as_ptr) + 1); // +1 for the \0
        dir_ent->m_dirname_length = strlen(dir_ent->m_dirname);
    } else {
        size_t slash_last_instance{src_path.rfind('/')};
        std::string target_name{
            src_path.substr(slash_last_instance == std::string::npos ? 0 : slash_last_instance + 1)}; // +1 to skip the last '/'
        char* target_name_ptr{target_name.data()};
        memcpy(dir_ent->m_dirname, target_name_ptr, strlen(target_name_ptr) + 1); // +1 to include the \0
        dir_ent->m_dirname_length = strlen(dir_ent->m_dirname);
    }
}

std::optional<std::string> extract_filename(std::string_view path) {
    if(path.empty()) {
        return std::nullopt;
    }

    size_t slash_last_instance{path.rfind('/')};
    std::string filename{
        path.substr(slash_last_instance == std::string::npos ? 0 : slash_last_instance + 1)}; // +1 to skip the last '/'
    return filename;
}

u64 get_dir_size(const fs::directory_entry& dir, const u8 opts) {
    // dummy error code
    std::error_code err;
    const bool sym{(opts & O_SYM) > 0};

    u64 size{};

    if(!fs::is_directory(dir.status(err))) {
        return -1;
    }

    fs::path real_dir_path{fs::is_symlink(dir, err) ? fs::canonical(dir, err)
                                                    : dir.path()}; // In case the provided path refers to a sym_link

    for(const fs::directory_entry& ent : std::filesystem::recursive_directory_iterator(real_dir_path, err)) {
        const fs::file_status ent_sym_status(ent.symlink_status(err));

        if(fs::is_regular_file(ent_sym_status)) {
            size += fs::file_size(ent, err);

        } else if(fs::is_symlink(ent_sym_status) && sym) {
            if(fs::is_regular_file(ent)) {
                size += fs::directory_entry{fs::canonical(ent, err)}.file_size(err);

            } else if(fs::is_directory(ent)) {
                size += get_dir_size(fs::directory_entry{fs::canonical(ent, err), err}, opts);

            } // else, ignore special files or symlinks while !sym
        }
    }

    return size;
}

void print_dir_data(const dir_entry& dir_data) noexcept {
    std::println("dir name: {}", static_cast<const char*>(dir_data.m_dirname));
    std::println("dir name length: {}", static_cast<packr::u16>(dir_data.m_dirname_length));
    std::println("dir size is: {}", static_cast<packr::u64>(dir_data.m_size));

    std::println("total_dir_count: {}", static_cast<packr::u64>(dir_data.m_total_dir_count));
    std::println("total_file_count: {}", static_cast<packr::u64>(dir_data.m_total_file_count));
    std::println("total_entry_count: {}", static_cast<packr::u64>(dir_data.m_total_entry_count));

    std::println("child_dir_count: {}", static_cast<packr::u64>(dir_data.m_child_dir_count));
    std::println("child_file_count: {}", static_cast<packr::u64>(dir_data.m_child_file_count));
    std::println("child_entry_count: {}", static_cast<packr::u64>(dir_data.m_child_entry_count));

    // Need to be explicitly casted due to struct dir_entry being packed(due to misalignment)
    std::println("last access time: sec: {}, nsec: {}", static_cast<packr::i64>(dir_data.m_acc_time.sec),
                 static_cast<packr::i64>(dir_data.m_acc_time.nsec));
    std::println("last modification time: sec: {}, nsec: {}", static_cast<packr::i64>(dir_data.m_mod_time.sec),
                 static_cast<packr::i64>(dir_data.m_mod_time.nsec));
    std::println("last last status change time: sec: {}, nsec: {}", static_cast<packr::i64>(dir_data.m_sc_time.sec),
                 static_cast<packr::i64>(dir_data.m_sc_time.nsec));
    std::println("mode: {}", static_cast<packr::u64>(dir_data.m_mode));
}

bool curate_src_path(std::string& src_path) noexcept {
    if(*(src_path.data()) != '/') {
        char* cwd = getcwd(nullptr, 0);
        std::optional<std::string> src_path_temp{packr::join_to_path(src_path, cwd)};
        if(!src_path_temp) {
            return false;
        }

        src_path = src_path_temp.value();
        free(cwd);
        return true;
    }

    return true;
}

std::string create_pack_filename(const dir_entry& dir_data) {
    static constexpr std::string extension{".packr"};
    std::string pack_filename{};
    pack_filename.reserve(extension.length() + strlen(dir_data.m_dirname));

    pack_filename += dir_data.m_dirname;
    pack_filename += extension;
    return pack_filename;
}

fs::path read_symlink(const fs::path& path) {
    std::error_code err{};
    assert(!path.empty() && "Tried to read a symlink of an empty path");
    assert(fs::exists({fs::directory_entry{path}.symlink_status(err)}) && "Tried to read a symlink of a nonexistent entry");

    const fs::path secondary_path{fs::canonical(path, err)};
    // TODO: canonical CAN'T refer to a non-existent file
    const fs::directory_entry secondary_ent{secondary_path, err};
    fs::path res{}; // what's going to be returned

    if(!secondary_path.empty()) {
        res = fs::exists(secondary_ent.symlink_status(err)) ? secondary_path : "";
    }

    return res;
}

} // namespace packr
