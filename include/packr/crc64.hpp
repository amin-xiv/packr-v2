#pragma once

#include <packr/types.hpp>

namespace packr::hash {
class Crc64 {
  public:
    Crc64() = delete;
    Crc64(observe_ptr<char*> msg);
};

} // namespace packr::hash
