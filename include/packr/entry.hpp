#pragma once

#include <filesystem>
#include <packr/types.hpp>
#include <packr/fs_node.hpp>
#include <climits>
#include <dirent.h>

#define ENT_DIR_START ((uint8_t)0x01)
#define ENT_DIR_END ((uint8_t)0x02)
#define ENT_FILE ((uint8_t)0x04)
#define PACK_START ((uint8_t)0x08)
#define PACK_END ((uint8_t)0x10) // 16
#define DEFAULT_ROOT_DIR 0
#define P_NOMETADATA 0B00000001

namespace packr {

// This struct would be written into the pack file
struct file_entry {
    // Constructors
    file_entry() = default;
    file_entry(const std::filesystem::path& file_path);

    char filename[NAME_MAX]{};
    char secondary_path[PATH_MAX]; // To store symlink target paths, block file paths..etc
    u64 size{};                    // file size
    u64 acc_time{};                // last access time
    u64 mod_time{};                // last modification time
    u64 sc_time{};                 // last status change time
    u16 filename_length{};
    u16 secondary_path_length{};
    u16 mode{};                  // permissions
    entry_class_t entry_class{}; // u8
    file_type type{};            // u8
    bool success{false};

} __attribute__((packed));

// This struct would be written into the pack file
struct dir_entry {
    // Constructors
    dir_entry() = default;
    dir_entry(const std::filesystem::directory_entry& dir, u32 nest_count);

    char dirname[NAME_MAX]{};
    char secondary_path[PATH_MAX]; // Holds the path of the target directory if this is a symlink
    u64 child_entry_count{};
    u64 child_file_count{};
    u64 child_dir_count{};
    u64 total_entry_count{};
    u64 total_dir_count{};
    u64 total_file_count{};
    u64 size{};     // directory size(obviously)
    u64 acc_time{}; // last access time
    u64 mod_time{}; // last modification time
    u64 sc_time{};  // last status change time
    u16 dirname_length{};
    u16 secondary_path_length{};
    u16 mode{};                  // permissions
    entry_class_t entry_class{}; // u8
    dir_type type{};             // u8
    bool success{false};

    // Packs a directory by writing its metadata, and children's metadata and data(for files) in a given file(the pack
    // file)
    [[nodiscard]] bool pack_dir(const std::filesystem::directory_entry& dir, File_W& pack_file, const u8 opts,
                                const u32 nest_count);

    // Unpacks a given directory, by reading data from a pack_file
    [[nodiscard]] static bool unpack_dir(File_R& pack_file, const u8 opts, const u32 nest_count);

    // Unpacks a given pack file(calls unpack_dir)
    [[nodiscard]] static bool unpack(File_R& pack_file, const u8 opts);

} __attribute__((packed));

// Almost the same as dir_entry, just offers the pack() function and just differentiates regular dirs from pack headers
struct pack_header : public dir_entry {
    // To inherit the constructors from dir_entry
    using dir_entry::dir_entry;

    // Initiates the packing process(calls pack_dir)
    [[nodiscard]] bool pack(const std::filesystem::directory_entry& dir, File_W& pack_file, const u8 opts);
};

} // namespace packr
