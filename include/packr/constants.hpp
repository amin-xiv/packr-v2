#pragma once

#include <packr/types.hpp>

namespace packr {

// packr file markers
inline constexpr u8 ENT_DIR_START{0x01};
inline constexpr u8 ENT_DIR_END{0x02};
inline constexpr u8 ENT_FILE{0x04};
inline constexpr u8 PACK_START{0x08};
inline constexpr u8 PACK_END{0x10};    // 16
inline constexpr u8 ENT_DIR_SYM{0x20}; // 32
                                       //
// user flags
inline constexpr u8 O_SYM{1};

// other
inline constexpr u8 DEFAULT_ROOT_DIR{0};

} // namespace packr
