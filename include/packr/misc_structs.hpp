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
struct mmapped final {
  public:
    mmapped() = delete;
    mmapped(mmapped&) = delete;
    mmapped& operator=(const mmapped&) = delete;

    explicit mmapped(std::unique_ptr<char[]> ptr, const packr_size_t size) noexcept;
    explicit mmapped(mmapped&& other) noexcept;
    mmapped& operator=(mmapped&& other) noexcept;
    ~mmapped() noexcept;

    void unmap() noexcept;
    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] void* get() const noexcept;

  private:
    void* m_data{nullptr};
    packr_size_t m_size{};
    general_status m_status{general_status::base};
};

} // namespace packr
