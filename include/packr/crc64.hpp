#pragma once

#include <packr/types.hpp>

#include <memory>
#include <bitset>

namespace packr::hash {

/* CRC-64/ECMA-182 implementation (MSB-first) */
class Crc64 final {
  public:
    Crc64() = delete;
    // this class doesn't manage msg, it simply edits it within its bounds
    // and caller must allocate at least len + sizeof(u64) in msg
    explicit Crc64(observe_ptr<char>& msg, const u64 len);

    [[nodiscard]] bool append_checksum() noexcept;

    [[nodiscard]] bool verify() const noexcept;

    [[nodiscard]] u64 compute_checksum();

    [[nodiscard]] static void* reserve_checksum_space(std::unique_ptr<char[]> ptr);

  private:
    constexpr static u64 s_divisor{0x42f0e1eba9ea3693};
    constexpr static u64 s_check_value{0x6c40df5f0b497347};
    constexpr static u8 s_divisor_width_bits{64};
    constexpr static u8 s_divisor_width_bytes{8};

    observe_ptr<char> m_data;
    packr_size_t m_byte_len{}; // total message size

    u64 m_quotient{};

    /* offset used to track current position in the data stream */
    u64 m_offset_bits{};

    /* parital remainder while doing long division */
    std::bitset<s_divisor_width_bits> m_partial_rem;

    /* divisor of the partial remainder, can either be s_divisor or zero */
    std::bitset<s_divisor_width_bits> m_partial_rem_divisor;
};

} // namespace packr::hash
