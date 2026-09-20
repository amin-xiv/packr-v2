#include <bits/types/mbstate_t.h>
#include <packr/crc64.hpp>

#include <cstddef>
#include <stdexcept>

namespace packr::hash {

Crc64::Crc64(observe_ptr<char>& msg, const u64 len) : m_data(msg), m_byte_len(len) {
    assert(msg); // check for null
    assert(m_byte_len > s_divisor_width_bytes);

    if(m_byte_len <= s_divisor_width_bytes) {
        throw std::invalid_argument{"tried to intialize a Crc64 objects with m_byte_len <= s_divisor_width_bytes"};
    }
}
u64 Crc64::compute_checksum() const noexcept {
    // zero-out last 8 bytes of the message (divisor width in bytes)
    for(std::size_t i{m_byte_len - 1}; i >= (m_byte_len - s_divisor_width_bytes); i--) {
        m_data[i] = 0;
    }

    return 0;
}

} // namespace packr::hash
