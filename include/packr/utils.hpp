#pragma once

#include <packr/entry.hpp>
#include <packr/types.hpp>

#include <filesystem>
#include <optional>
#include <string_view>
#include <sys/stat.h>

namespace packr {

[[nodiscard]] extern std::optional<std::string> join_to_path(const std::string& filename, const std::string& cwd);
// Params not string_view due to std::string addition/concatination being carried out on them

extern void add_dirname(dir_entry* dir_ent, std::string named_as, const std::string& src_path);

[[nodiscard]] extern std::optional<std::string> extract_filename(std::string_view path);

[[nodiscard]] extern u64 get_dir_size(const std::filesystem::directory_entry& dir, const u8 opts, anc_map_t& anc_table);

[[nodiscard]] extern bool curate_src_path(std::string& src_path) noexcept;

extern void print_dir_data(const dir_entry& dir_data);

[[nodiscard]] std::string create_pack_filename(const dir_entry& dir_data);

[[nodiscard]] extern std::filesystem::path read_symlink(const std::filesystem::path& path);

extern void debug_log(std::string_view str, const log_type type = log_type::error);

[[nodiscard]] extern bool copy_file_range(File_R& source, const off_t source_offset, File_W& dest, const off_t dest_offset,
                                          const ssize_t length);

extern bool handle_dir_ancestory(const struct ::stat& stat_obj, anc_map_t& anc_table, const std::filesystem::directory_entry& dir,
                                 bool compare_only = false);
} // namespace packr
