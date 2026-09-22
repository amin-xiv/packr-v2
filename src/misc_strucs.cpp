#include <packr/misc_structs.hpp>
#include <packr/utils.hpp>

#include <memory>
#include <sys/mman.h>
#include <utility>
#include <cstring>

using namespace packr;

mmapped::mmapped(const packr_size_t size) noexcept : m_size(size) {
    assert(size > 0 && "tried to construct an mmapped object with size of 0");

    m_data = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_ANONYMOUS, -1, 0);
    if(m_data == MAP_FAILED) {
        m_status = general_status::failure;
        m_data = nullptr;
        return;
    }

    m_status = general_status::success;
}

mmapped::mmapped(std::unique_ptr<char[]> ptr, const packr_size_t size) noexcept : m_size(size) {
    assert(size > 0);
    assert(ptr);

    m_data = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_ANONYMOUS, -1, 0);
    if(m_data == MAP_FAILED) {
        m_status = general_status::failure;
        m_data = nullptr;
        return;
    }

    m_status = general_status::success;
    std::memcpy(m_data, ptr.get(), size);
}

mmapped::mmapped(const char* ptr, const packr_size_t size) noexcept : m_size(size) {
    assert(size > 0);
    assert(ptr != nullptr);

    m_data = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_ANONYMOUS, -1, 0);
    if(m_data == MAP_FAILED) {
        m_status = general_status::failure;
        m_data = nullptr;
        return;
    }

    m_status = general_status::success;
    std::memcpy(m_data, ptr, size);
}

mmapped::mmapped(mmapped&& other) noexcept
    : m_data(std::exchange(other.m_data, nullptr)), m_size(std::exchange(other.m_size, 0)),
      m_status(std::exchange(other.m_status, general_status::base)) {
}

mmapped& mmapped::operator=(mmapped&& other) noexcept {
    assert(other.valid());

    if(this == std::addressof(other)) {
        return *this;
    }

    if(m_data != nullptr) {
        [[maybe_unused]] int res{munmap(m_data, m_size)};
        assert(res == 0);

        m_data = nullptr;
        m_size = 0;
        m_status = general_status::base;
    }

    m_data = std::exchange(other.m_data, nullptr);
    m_size = std::exchange(other.m_size, 0);
    m_status = std::exchange(other.m_status, general_status::base);

    return *this;
}

mmapped::~mmapped() noexcept {
    if(m_data == nullptr) {
        return;
    }

    [[maybe_unused]] int res{munmap(m_data, m_size)};
    assert(res == 0);

    m_data = nullptr;
    m_size = 0;
    m_status = general_status::base;
}

void mmapped::unmap() noexcept {
    if(m_data == nullptr) {
        return;
    }

    [[maybe_unused]] int res{munmap(m_data, m_size)};
    assert(res == 0);

    m_data = nullptr;
    m_size = 0;
    m_status = general_status::base;
}

bool mmapped::valid() const noexcept {
    return m_data != nullptr;
}

char* mmapped::get() const noexcept {
    return static_cast<char*>(m_data);
}

void mmapped::read(observe_ptr<char> dest, const packr_size_t count) const noexcept {
    assert(m_size >= count && "tried to read an mmapped struct with a count greater than m_size");

    std::memcpy(dest.get(), m_data, count);
}

void mmapped::write(observe_ptr<char> src, const packr_size_t count) const noexcept {
    assert(m_size >= count && "tried to read an mmapped struct with a count greater than m_size");

    std::memcpy(m_data, src.get(), count);
}

void mmapped::clear() const noexcept {
    std::memset(m_data, '\0', m_size);
}

packr_size_t mmapped::size() const noexcept {
    return m_size;
}

general_status mmapped::status() const noexcept {
    return m_status;
}
