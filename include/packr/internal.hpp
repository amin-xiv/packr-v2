#include <packr/types.hpp>
#include <packr/entry.hpp>

#include <filesystem>

namespace fs = std::filesystem;

namespace packr {

void populate_with_parents(fs::directory_entry dir, packr::anc_map_t& anc_table, bool second_pass = false);

} // namespace packr
