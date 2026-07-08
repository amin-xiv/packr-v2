#pragma once
#include <cstdint>

namespace packr {

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

enum class OP_TYPE : u8 {
    PACK,
    UNPACK
};

enum class file_type : u8 {
    regular,
    symlink,
    special, // The one I'll be using for now to represent the types below
    block,
    character,
    fifo,
    socket
};

enum class dir_type : u8 {
    regular,
    symlink
};

enum class entry_class_t : u8 {
    CHILD_ENT = 0x40,  // 64
    NESTED_ENT = 0x80, // 128
};

enum class open_type : u8 {
    exists,
    fresh // well I didn't call it new since there's already the 'new' keyword
};

// This is included BEFORE entry headers(to know how much memory to read)
struct special_marker {
    u8 type; // Should only be set by ENT_* and PACK_* macros and binary ORed with enum entry_class
} __attribute__((packed));

} // namespace packr
