#include <packr/types.hpp>
#include <packr/entry.hpp>

#include <filesystem>

namespace fs = std::filesystem;

namespace packr {

extern bool inc_dir_ent_dir_count(dir_entry& dir, const fs::directory_entry& entry, const u32 nest_count, const u8 opts,
                                  anc_map_t& anc_table);
extern void inc_dir_ent_file_count(dir_entry& dir, const fs::directory_entry& entry, const u32 nest_count, bool add_size = true);
extern void populate_with_parents(fs::directory_entry dir, packr::anc_map_t& anc_table, bool second_pass = false);

} // namespace packr
