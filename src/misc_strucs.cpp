#include <packr/misc_structs.hpp>

#include <memory>
#include <sys/mman.h>

using namespace packr;

mmaped::mmaped(std::unique_ptr<char[]> ptr, const packr_size_t size) : m_size(size) {
    assert(size > 0);
    assert(ptr);
}
