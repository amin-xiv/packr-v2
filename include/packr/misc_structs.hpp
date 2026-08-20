#pragma once
#include <packr/types.hpp>

namespace packr {

// This is included BEFORE entry headers(to know how much memory to read)
struct special_marker {
    u8 type; // Should only be set by ENT_* and PACK_*
};

struct time_spec {
    i64 sec;
    i64 nsec;
};

struct dev_ino_t {
    u64 dev{};
    u64 ino{};
};

} // namespace packr
