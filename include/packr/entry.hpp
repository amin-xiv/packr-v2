#pragma once

#include <packr/types.hpp>
#include <packr/fs_node.hpp>
#include <packr/misc_structs.hpp>
#include <filesystem>
#include <climits>

namespace packr {

// This struct would be written into the pack file
struct [[gnu::packed]] file_entry {
    // Constructors
    file_entry() = default;
    file_entry(const std::filesystem::path& file_path);

    char m_filename[NAME_MAX]{};
    char m_secondary_path[PATH_MAX]{}; // To store symlink target paths, block file paths..etc
    u64 m_size{};                      // file size
    time_spec m_acc_time{};            // last access time
    time_spec m_mod_time{};            // last modification time
    time_spec m_sc_time{};             // last status change time
    u16 m_filename_length{};
    u16 m_secondary_path_length{};
    u16 m_mode{};                  // permissions
    entry_class_t m_entry_class{}; // u8
    file_type m_type{};            // u8
    bool m_success{false};
};

// This struct would be written into the pack file
struct [[gnu::packed]] dir_entry {
    // Constructors
    dir_entry() = default;
    dir_entry(const std::filesystem::directory_entry& dir, u32 nest_count);

    char m_dirname[NAME_MAX]{};
    char m_secondary_path[PATH_MAX]{}; // Holds the path of the target directory if this is a symlink
    u64 m_child_entry_count{};
    u64 m_child_file_count{};
    u64 m_child_dir_count{};
    u64 m_total_entry_count{};
    u64 m_total_dir_count{};
    u64 m_total_file_count{};
    u64 m_size{};           // directory size
    time_spec m_acc_time{}; // last access time
    time_spec m_mod_time{}; // last modification time
    time_spec m_sc_time{};  // last status change time
    u16 m_dirname_length{};
    u16 m_secondary_path_length{};
    u16 m_mode{};                  // permissions
    entry_class_t m_entry_class{}; // u8
    dir_type m_type{};             // u8
    bool m_success{false};

    // Packs a directory by writing its metadata, and children's metadata and data(for files) in a given file(the pack
    // file)
    [[nodiscard]] bool pack_dir(const std::filesystem::directory_entry& dir, File_W& pack_file, const u8 opts,
                                const u32 nest_count);

    // Unpacks a given directory, by reading data from a pack_file
    [[nodiscard]] static bool unpack_dir(File_R& pack_file, const u8 opts, const u32 nest_count);

    // Unpacks a given pack file(calls unpack_dir)
    [[nodiscard]] static bool unpack(File_R& pack_file, const u8 opts);
};

// Almost the same as dir_entry, just offers the pack() function and just differentiates regular dirs from pack headers
struct pack_header : public dir_entry {
    // To inherit the constructors from dir_entry
    using dir_entry::dir_entry;

    // Initiates the packing process(calls pack_dir)
    [[nodiscard]] bool pack(const std::filesystem::directory_entry& dir, File_W& pack_file, const u8 opts);
};

} // namespace packr
