#include <packr/misc_structs.hpp>
#include <packr/utils.hpp>

#include <memory>
#include <sys/mman.h>
#include <format>
#include <utility>

using namespace packr;

mmapped::mmapped([[maybe_unused]] std::unique_ptr<char[]> ptr, const packr_size_t size) noexcept : m_size(size) {
    assert(size > 0);
    assert(ptr);

    m_data = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_ANONYMOUS, -1, 0);
    if(m_data == MAP_FAILED) {
        debug_log(std::format("mmap failed with size argument of: {}", size));
        m_status = general_status::failure;
        m_data = nullptr;
        return;
    }

    m_status = general_status::success;
}

mmapped::mmapped(mmapped&& other) noexcept
    : m_data(std::exchange(other.m_data, nullptr)), m_size(other.m_size), m_status(other.m_status) {

    assert(other.m_size > 0);
    assert(other.m_data);
    assert(other.valid());
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

    if(res != 0) {
        debug_log(std::format("failed to unmap memory region, with address {} and size {}", m_data, m_size));
    }

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

void* mmapped::get() const noexcept {
    return m_data;
}
