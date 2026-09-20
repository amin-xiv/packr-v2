#pragma once

#include <packr/types.hpp>

namespace packr {

// This is included BEFORE entry headers(to know how much memory to read)
struct special_marker final {
    u8 type; // Should only be set by ENT_* and PACK_*
};

struct time_spec final {
    i64 sec;
    i64 nsec;
};

struct dev_ino_t final {
    u64 dev{};
    u64 ino{};
};

/* holds a pointer to an mmaped memory area, releases it at its destructor */
struct mmaped final {
  public:
    mmaped() = delete;
    mmaped(mmaped&) = delete;

    mmaped(std::unique_ptr<char[]> ptr, const packr_size_t size);
    mmaped(mmaped&& other) noexcept;
    ~mmaped();

  private:
    void* m_data{};
    const packr_size_t m_size{};
};

} // namespace packr
