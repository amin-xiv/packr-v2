#include <packr/crc64.hpp>
#include <packr/misc_structs.hpp>

#include <cstddef>
#include <cstring>
#include <stdexcept>
// #include <memory>

[[maybe_unused]] constexpr static int BYTE_WIDTH{8};

namespace packr::hash {

Crc64::Crc64(observe_ptr<char>& msg, const u64 len) : m_data(msg), m_byte_len(len) {
    assert(msg); // check for null
    assert(m_byte_len > s_divisor_width_bytes);

    if(m_byte_len <= s_divisor_width_bytes) {
        throw std::invalid_argument{"tried to intialize a Crc64 objects with m_byte_len <= s_divisor_width_bytes"};
    }
}
u64 Crc64::compute_checksum() noexcept {
    // zero-out last 8 bytes of the message (divisor width in bytes)
    // byte_len -1  to index the last char
    for(std::size_t i{m_byte_len - 1}; i >= (m_byte_len - s_divisor_width_bytes); i--) {
        m_data[i] = 0;
    }

    // long division
    mmapped paged_data{m_data.get(), m_byte_len};
    // do {
    //     if(m_offset_bits == 0) {
    //         paged_data.read(observe_ptr{reinterpret_cast<char*>(&m_partial_remainder)}, s_divisor_width_bytes);
    //         m_offset_bits = s_divisor_width_bits;
    //     } else {
    //         std::byte tmp_byte{};
    //         u64 offset_bytes{m_offset_bits / BYTE_WIDTH};
    //         std::memcpy(std::addressof(tmp_byte), paged_data.get() + offset_bytes, sizeof(tmp_byte));
    //     }
    //
    //     m_offset_bits = s_divisor_width_bits;
    //
    //     m_offset_bits++;
    //
    // } while(m_offset_bits != (m_byte_len * BYTE_WIDTH) + (s_divisor_width_bits - 1));

    // std::println(stderr, "data from paged: {}", paged_data.get());
    // paged_data.read(observe_ptr{reinterpret_cast<char*>(&m_partial_remainder)}, sizeof(m_partial_remainder));
    return m_partial_remainder;
}

} // namespace packr::hash
