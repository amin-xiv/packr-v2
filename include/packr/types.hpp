#pragma once

#include <cstdint>
#include <istream>

namespace packr {

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using pos_type = std::istream::pos_type;

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

enum class entry_type : u8 {
    directory,
    regular_file,
    special
};

enum class entry_class_t : u8 {
    CHILD_ENT,
    NESTED_ENT
};

enum class open_type : u8 {
    exists,
    fresh // well I didn't call it new since there's already the 'new' keyword
};

enum class log_type : u8 {
    error,
    warning,
    info,
    notice,
    none
};

enum class dir_entry_ret_code : u8 {
    success,
    fail,
    recursive // returned to avoid recursion
};

} // namespace packr
