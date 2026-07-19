#include <filesystem>
#include <packr/utils.hpp>
#include <packr/entry.hpp>
#include <print>
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

u64 get_dir_size(const fs::directory_entry& dir) {
    // dummy error code
    std::error_code err;

    u64 size{};

    if(!fs::is_directory(dir.status(err))) {
        return -1;
    }

    fs::path real_dir_path{fs::is_symlink(dir, err) ? fs::read_symlink(dir)
                                                    : dir.path()}; // In case the provided path refers to a sym_link

    for(const fs::directory_entry& ent : std::filesystem::recursive_directory_iterator(real_dir_path)) {
        if(fs::is_regular_file(ent.symlink_status())) {
            size += fs::file_size(ent, err);
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

    std::println("last access time: sec: {}, nsec: {}", dir_data.m_acc_time.sec, dir_data.m_acc_time.nsec);
    std::println("last modification time: sec: {}, nsec: {}", dir_data.m_mod_time.sec, dir_data.m_mod_time.nsec);
    std::println("last last status change time: sec: {}, nsec: {}", dir_data.m_sc_time.sec, dir_data.m_sc_time.nsec);
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

} // namespace packr
