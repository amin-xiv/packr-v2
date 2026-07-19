#pragma once
#include <packr/types.hpp>

namespace packr {

// This is included BEFORE entry headers(to know how much memory to read)
struct special_marker {
    u8 type; // Should only be set by ENT_* and PACK_*
};

struct time_spec {
    u64 sec;
    u64 nsec;
};

}; // namespace packr
