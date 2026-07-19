#pragma once

#include <packr/entry.hpp>
#include <packr/types.hpp>
#include <filesystem>
#include <optional>

namespace packr {

[[nodiscard]] extern std::optional<std::string> join_to_path(const std::string& filename, const std::string& cwd);
// Params not string_view due to std::string addition/concatination being carried out on them
//
extern void add_dirname(dir_entry* dir_ent, std::string named_as, const std::string& src_path);

[[nodiscard]] extern std::optional<std::string> extract_filename(std::string_view path);

[[nodiscard]] extern u64 get_dir_size(const std::filesystem::directory_entry& dir);

[[nodiscard]] extern bool curate_src_path(std::string& src_path) noexcept;

void print_dir_data(const dir_entry& dir_data) noexcept;

std::string create_pack_filename(const dir_entry& dir_data);
} // namespace packr
