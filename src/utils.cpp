#include "../include/packr/utils.hpp"

#include <packr/utils.hpp>
#include <packr/entry.hpp>
#include <cstddef>
#include <unistd.h>
#include <cstring>
#include <optional>
#include <string>

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
        memcpy(dir_ent->dirname, named_as_ptr, strlen(named_as_ptr) + 1); // +1 for the \0
        dir_ent->dirname_length = strlen(dir_ent->dirname);
    } else {
        size_t slash_last_instance{src_path.rfind('/')};
        std::string target_name{
            src_path.substr(slash_last_instance == std::string::npos ? 0 : slash_last_instance + 1)}; // +1 to skip the last '/'
        char* target_name_ptr{target_name.data()};
        memcpy(dir_ent->dirname, target_name_ptr, strlen(target_name_ptr) + 1); // +1 to include the \0
        dir_ent->dirname_length = strlen(dir_ent->dirname);
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

} // namespace packr
