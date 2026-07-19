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

inline constexpr u8 ENT_DIR_START{0x01};
inline constexpr u8 ENT_DIR_END{0x02};
inline constexpr u8 ENT_FILE{0x04};
inline constexpr u8 PACK_START{0x08};
inline constexpr u8 PACK_END{0x10}; // 16
inline constexpr u8 DEFAULT_ROOT_DIR{0};
inline constexpr u8 O_SYM{1};

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
    CHILD_ENT,
    NESTED_ENT
};

enum class open_type : u8 {
    exists,
    fresh // well I didn't call it new since there's already the 'new' keyword
};

} // namespace packr
