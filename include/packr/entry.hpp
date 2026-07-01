#pragma once

#include "types.hpp"
#include <packr/types.hpp>
#include <climits>
#include <string_view>
#include <dirent.h>

#define ENT_DIR_START ((uint8_t)0x01)
#define ENT_DIR_END ((uint8_t)0x02)
#define ENT_FILE ((uint8_t)0x04)
#define PACK_START ((uint8_t)0x08)
#define PACK_END ((uint8_t)0x10) // 16
#define DEFAULT_ROOT_DIR 0
#define P_NOMETADATA 0B00000001

namespace packr {

struct file_entry {
    // Constructors
    file_entry() = default;
    file_entry(std::string_view file_path, u32 nest_count);

    char filename[NAME_MAX]{};
    u64 size{};     // file size
    u64 acc_time{}; // last access time
    u64 mod_time{}; // last modification time
    u64 sc_time{};  // last status change time
    u16 filename_length{};
    u16 mode{};       // permissions
    u8 entry_class{}; // For future features(maybe)
    bool success{false};

} __attribute__((packed));

struct dir_entry {
    // Constructors
    dir_entry() = default;
    dir_entry(DIR* dir, std::string_view dir_str, u32 nest_count);

    char dirname[NAME_MAX]{};
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
    u16 mode{};       // permissions
    u8 entry_class{}; // For future features(maybe)
    bool success{false};

    // Packs a directory by writing its metadata, and children's metadata and data(for files) in a given file(the pack
    // file)
    [[nodiscard]] bool pack_dir(std::string_view dir_path, FILE* pack_file, u8 opts, u32 nest_count);

    // Unpacks a given directory, by reading data from a pack_file
    [[nodiscard]] static bool unpack_dir(FILE* pack_file, u8 opts, u32 nest_count);

    // Unpacks a given pack file(calls unpack_dir)
    [[nodiscard]] static bool unpack(FILE* pack_file, u8 opts, u32 nest_count);

} __attribute__((packed));

// Almost the same as dir_entry, just offers the pack() function and just differentiiates regular dirs from pack headers.
struct pack_header : public dir_entry {
    // To inherit the constructors from dir_entry
    using dir_entry::dir_entry;

    // Initiates the packing process(calls pack_dir)
    [[nodiscard]] bool pack(std::string_view dir_path, DIR* dir, FILE* pack_file, u8 opts);
};

// Needed by special markers
enum class entry_class {
    CHILD_ENT = 0x40,  // 64
    NESTED_ENT = 0x80, // 128
};

// This is included BEFORE entry headers(to know how much memory to read)
struct special_marker {
    u8 type; // Should only be set by ENT_* and PACK_* macros and binary ORed with enum entry_class
} __attribute__((packed));

} // namespace packr
