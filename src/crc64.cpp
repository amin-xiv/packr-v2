#include <packr/crc64.hpp>

namespace packr::hash {

Crc64::Crc64(observe_ptr<char>& msg, const u64 len) : m_data(msg), m_len(len) {
}

} // namespace packr::hash
