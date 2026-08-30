#include "packr/types.hpp"
#include <packr/utils.hpp>
#include <packr/entry.hpp>
#include <packr/fs_node.hpp>
#include <filesystem>
#include <cassert>
#include <string_view>
#include <unistd.h>
#include <cstring>
#include <optional>
#include <string>
#include <sys/stat.h>
#include <print>

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
        std::size_t slash_last_instance{src_path.rfind('/')};
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

    std::size_t slash_last_instance{path.rfind('/')};
    std::string filename{
        path.substr(slash_last_instance == std::string::npos ? 0 : slash_last_instance + 1)}; // +1 to skip the last '/'
    return filename;
}

u64 get_dir_size(const fs::directory_entry& dir, const u8 opts, anc_map_t& anc_table) {
    std::error_code err;
    const bool sym{(opts & O_SYM) > 0};
    u64 size{};

    struct ::stat stat_obj;
    int stat_res{::stat(dir.path().c_str(), &stat_obj)};

    if(stat_res == -1) {
        std::string err_msg{"stat_res returned -1 at get_dir_size  with dir.path(): " + dir.path().string()};
        debug_log(err_msg);
        return 0;
    }

    if(handle_dir_ancestory(stat_obj, anc_table, dir)) {
        std::string msg{"identified and stopped recursing dir: " + dir.path().string()};
        debug_log(msg, log_type::info);
        return 0;
    }

    assert(fs::is_directory(dir.status(err)));

    fs::path real_dir_path{fs::is_symlink(dir, err) ? fs::canonical(dir, err)
                                                    : dir.path()}; // In case the provided path refers to a sym_link

    for(const fs::directory_entry& ent : std::filesystem::recursive_directory_iterator(real_dir_path, err)) {
        const fs::file_status ent_sym_status(ent.symlink_status(err));

        if(fs::is_regular_file(ent_sym_status)) {
            size += fs::file_size(ent, err);
            std::println(stderr, "opts: {}", opts);
            std::string msg{"file_size: " + std::to_string(fs::file_size(ent)) + " name: " + ent.path().string()};
            debug_log(msg, log_type::info);

        } else if(fs::is_symlink(ent_sym_status) && sym) {
            std::println(stderr, "opts: {}", opts);
            if(fs::is_regular_file(ent)) {
                size += fs::directory_entry{fs::canonical(ent, err)}.file_size(err);
                std::string msg{"(sym)file_size: " + std::to_string(fs::directory_entry{fs::canonical(ent, err)}.file_size(err)) +
                                " name: " + ent.path().string()};
                debug_log(msg, log_type::info);

            } else if(fs::is_directory(ent, err)) {
                std::string msg{"starting (sym)dir size recursion: " + fs::canonical(ent).string() +
                                " with sym path: " + ent.path().string()};
                debug_log(msg, log_type::info);
                size += get_dir_size(fs::directory_entry{fs::canonical(ent, err), err}, opts, anc_table);

            } // else, ignore special files or symlinks while !sym
        }
    }

    return size;
}

void print_dir_data(const dir_entry& dir_data) noexcept {
    // Integers are casted to their types since the struct dir_entry is packed

    debug_log("dir name: " + std::string{static_cast<const char*>(dir_data.m_dirname)}, log_type::info);
    debug_log("dir name length: " + std::to_string(static_cast<packr::u16>(dir_data.m_dirname_length)), log_type::info);
    debug_log("dir size is: " + std::to_string(static_cast<packr::u64>(dir_data.m_size)), log_type::info);

    debug_log("total_dir_count: " + std::to_string(static_cast<packr::u64>(dir_data.m_total_dir_count)), log_type::info);
    debug_log("total_file_count: " + std::to_string(static_cast<packr::u64>(dir_data.m_total_file_count)), log_type::info);
    debug_log("total_entry_count: " + std::to_string(static_cast<packr::u64>(dir_data.m_total_entry_count)), log_type::info);

    debug_log("child_dir_count: " + std::to_string(static_cast<packr::u64>(dir_data.m_child_dir_count)), log_type::info);
    debug_log("child_file_count: " + std::to_string(static_cast<packr::u64>(dir_data.m_child_file_count)), log_type::info);
    debug_log("child_entry_count: " + std::to_string(static_cast<packr::u64>(dir_data.m_child_entry_count)), log_type::info);

    debug_log("last access time: sec: , nsec: " + std::to_string(static_cast<packr::i64>(dir_data.m_acc_time.sec)) +
                  std::to_string(static_cast<packr::i64>(dir_data.m_acc_time.nsec)),
              log_type::info);
    debug_log("last modification time: sec: , nsec: " + std::to_string(static_cast<packr::i64>(dir_data.m_mod_time.sec)) +
                  std::to_string(static_cast<packr::i64>(dir_data.m_mod_time.nsec)),
              log_type::info);
    debug_log("last last status change time: sec: , nsec: " + std::to_string(static_cast<packr::i64>(dir_data.m_sc_time.sec)) +
                  std::to_string(static_cast<packr::i64>(dir_data.m_sc_time.nsec)),
              log_type::info);
    debug_log("mode: " + std::to_string(static_cast<packr::u64>(dir_data.m_mode)), log_type::info);
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
    std::string pack_filename;
    pack_filename.reserve(extension.length() + strlen(dir_data.m_dirname));

    pack_filename += dir_data.m_dirname;
    pack_filename += extension;
    return pack_filename;
}

fs::path read_symlink(const fs::path& path) {
    std::error_code err;
    assert(!path.empty() && "Tried to read a symlink of an empty path");
    assert(fs::exists({fs::directory_entry{path}.symlink_status(err)}) && "Tried to read a symlink of a nonexistent entry");

    const fs::path secondary_path{fs::canonical(path, err)};
    // TODO: canonical CAN'T refer to a non-existent file
    const fs::directory_entry secondary_ent{secondary_path, err};
    fs::path res; // what's going to be returned

    if(!secondary_path.empty()) {
        res = fs::exists(secondary_ent.symlink_status(err)) ? secondary_path : "";
    }

    return res;
}

void debug_log([[maybe_unused]] std::string_view str, [[maybe_unused]] const log_type type) {
    // type is by default log_type::error;
#ifndef NDEBUG
    using enum log_type;
    std::string type_str{};
    if(type == error) {
        type_str = "ERROR";

    } else if(type == warning) {
        type_str = "WARNING";

    } else if(type == info) {
        type_str = "INFO";
    } else { // log_type::none
        type_str = "";
    }

    std::println(stderr, "[{}]: {}", type_str, str);
    if(type == error) {
        std::println(stderr, "errno: {}", strerror(errno));
    }
#endif
}

[[nodiscard]] bool copy_file_range(File_R& source, off_t source_offset, File_W& dest, off_t dest_offset, const ssize_t length) {
    const int dest_fd{source.get_fd()};
    const int out_fd{dest.get_fd()};

    // This copy_file_range function is coming from unistd.h
    ssize_t copy_res{::copy_file_range(dest_fd, &source_offset, out_fd, &dest_offset, static_cast<std::size_t>(length), 0)};

    // We need to advance pack_file position as copy_file_range won't advance it
    // and therefore would confuse any further reading / writing as file descriptor
    // offsets aren't related to std streams(in terms of positions)
    source.set_offset(length);
    dest.set_offset(length);

    assert(copy_res != -1);

    if(copy_res == -1) {
        std::string err_msg{"copy_file_range failed, dest_fd: " + std::to_string(dest_fd) + ", out_fd:" + std::to_string(out_fd)};
        debug_log(err_msg);
        return false;
    }

    return true;
}

} // namespace packr
