#pragma once

#include <cstdint>
#include <istream>
#include <memory>
#include <cassert>
#include <stdexcept>

namespace packr {

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using pos_type = std::istream::pos_type;

enum class OP_TYPE : u8 {
    PACK,
    UNPACK
};

enum class file_type : u8 {
    regular,
    symlink,
    special, // The one I'll be using for now to represent the types below
    block,
    character,
    fifo,
    socket
};

enum class dir_type : u8 {
    regular,
    symlink
};

enum class entry_type : u8 {
    directory,
    regular_file,
    special
};

enum class entry_class_t : u8 {
    CHILD_ENT,
    NESTED_ENT
};

enum class open_type : u8 {
    exists,
    fresh // well I didn't call it new since there's already the 'new' keyword
};

enum class log_type : u8 {
    error,
    warning,
    info,
    notice,
    none
};

enum class dir_entry_ret_code : u8 {
    success,
    fail,
    recursive // returned to avoid recursion
};

template <typename T>
struct [[nodiscard]] observe_ptr {
  public:
    observe_ptr() = delete;
    explicit observe_ptr(T* data) : m_data(data) {
        // nullptrs shouldn't be used to initialize an object of this type

        assert(data != nullptr && "null pointer was used to initialize observe_ptr");

        if(data == nullptr) {
            throw std::invalid_argument{"null pointer was used to initialize observe_ptr"};
        }
    }
    explicit observe_ptr(T& data) noexcept : m_data(std::addressof(data)) {
    }

    [[nodiscard]] T* get() const noexcept {
        return m_data;
    }

    [[nodiscard]] T* operator->() const noexcept {
        return m_data;
    }

    [[nodiscard]] bool valid() const noexcept {
        return m_data != nullptr;
    }

    [[nodiscard]] operator bool() const noexcept {
        return m_data != nullptr;
    }

    void reassign(T* data) {
        // reassign is meant to reassign the contained pointer with a non-null pointer

        assert(data != nullptr && "null pointer was passed to observe_ptr::assign");

        if(data == nullptr) {
            throw std::invalid_argument{"null pointer was passed to observe_ptr::assign"};
        }

        m_data = data;
    }

    void reset() noexcept {
        m_data = nullptr;
    }

  private:
    T* m_data;
};

} // namespace packr
