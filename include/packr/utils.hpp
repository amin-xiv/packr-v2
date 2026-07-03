#pragma once
#include <string_view>
#include <packr/entry.hpp>
#include <optional>
#include <packr/types.hpp>
#define NSEC_TO_SEC(x) (static_cast<u64>((x) / 1e9))

namespace packr {

extern std::optional<std::string> join_to_path(const std::string& filename, const std::string& cwd);
// Params not string_view due to std::string addition/concatination being carried out on them
extern void add_dirname(dir_entry* dir_ent, std::string named_as, const std::string& src_path);
extern std::optional<std::string> extract_filename(std::string_view path);

} // namespace packr
