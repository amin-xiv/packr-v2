#include <packr/crc64.hpp>
#include <packr/misc_structs.hpp>

#include <cstring>
#include <stdexcept>
#include <memory>
#include <cstdlib>

// NOTE: stopped at long division, in the bit substitution part, it appears that in the test file, checksum is always returned as
// 0 for some unknown reason

[[maybe_unused]] constexpr static int BYTE_WIDTH{8};

namespace packr::hash {

Crc64::Crc64(observe_ptr<char>& msg, const u64 len) : m_data(msg), m_byte_len(len) {
    assert(msg); // check for null
    assert(m_byte_len > s_divisor_width_bytes);

    if(m_byte_len <= s_divisor_width_bytes) {
        throw std::invalid_argument{"tried to intialize a Crc64 objects with m_byte_len <= s_divisor_width_bytes"};
    }
}
u64 Crc64::compute_checksum() {
    // zero-out last 8 bytes of the message (divisor width in bytes)
    // byte_len -1  to index the last char
    for(std::size_t i{m_byte_len - 1}; i >= (m_byte_len - s_divisor_width_bytes); i--) {
        m_data[i] = 0;
    }

    // long division
    mmapped paged_data{m_data.get(), m_byte_len};
    do {
        if(m_offset_bits == 0) {
            u64 temp_rem{}; // to temporarily write the read value into, then to m_partial_rem
            paged_data.read(observe_ptr{reinterpret_cast<char*>(&temp_rem)}, s_divisor_width_bytes);
            m_partial_rem = temp_rem;

            // if first bit is is 0, then the divisor is going to be zero, if it's 1, then the divisior is just s_divisor
            if(m_partial_rem.test(0)) {
                m_partial_rem_divisor = s_divisor;
            } else {
                m_partial_rem_divisor = 0;
            }

            m_partial_rem ^= m_partial_rem_divisor;

            // set offset to 64 bits
            m_offset_bits = s_divisor_width_bits;

            continue;
        }

        std::bitset<BYTE_WIDTH> tmp_byte{};
        std::lldiv_t offset_bytes{std::lldiv(static_cast<i64>(m_offset_bits), BYTE_WIDTH)};
        u8 tmp_val{}; // to temporarily write the read value into, then to tmp_byte

        std::memcpy(std::addressof(tmp_val), paged_data.get() + offset_bytes.quot, 1);
        tmp_byte = tmp_val;
        m_partial_rem <<= 1; // create room for the new bit and discard the left-most one

        // set right-most bit to the new introduced bit
        if(offset_bytes.rem > 0) {
            m_partial_rem.set(s_divisor_width_bits - 1) = tmp_byte.test(static_cast<u64>(offset_bytes.rem - 1)) ? 1 : 0;
        } else { // offset_bytes.rem == 0
            m_partial_rem.set(s_divisor_width_bits - 1) = tmp_byte.test(0) ? 1 : 0;
        }

        if(m_partial_rem.test(0)) {
            m_partial_rem_divisor = s_divisor;
        } else {
            m_partial_rem_divisor = 0;
        }

        m_partial_rem ^= m_partial_rem_divisor;

        m_offset_bits++;
    } while(m_offset_bits != (m_byte_len * BYTE_WIDTH) + (s_divisor_width_bits - 1));

    // std::println(stderr, "data from paged: {}", paged_data.get());
    // paged_data.read(observe_ptr{reinterpret_cast<char*>(&m_partial_remainder)}, sizeof(m_partial_remainder));
    return m_partial_rem.to_ullong();
}

} // namespace packr::hash
