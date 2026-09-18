#pragma once

#include <packr/types.hpp>

namespace packr::hash {

/* CRC-64/ECMA-182 implementation (MSB-first) */
class Crc64 {
  public:
    Crc64() = delete;
    // this class doesn't manage msg, it simply edits it within its bounds
    // and caller must allocate at least len + sizeof(u64) in msg
    explicit Crc64(observe_ptr<char*> msg, const u64 len);

    [[nodiscard]] bool append_checksum() noexcept;
    [[nodiscard]] bool verify() const noexcept;

  private:
    [[nodiscard]] u64 compute_checksum() const noexcept;

    const observe_ptr<char*> m_data;
    u64 m_len;

    constexpr static u64 s_divisor{0x42f0e1eba9ea3693};
    constexpr static u64 s_check_value{0x6c40df5f0b497347};
    constexpr static u8 s_divisor_width{64};
};

} // namespace packr::hash
