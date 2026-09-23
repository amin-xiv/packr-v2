#include <packr/crc64.hpp>
#include <packr/types.hpp>

#include <gtest/gtest.h>
#include <cstring>
#include <print>

using namespace packr;
using namespace packr::hash;

constexpr int MSG_SIZE{17};
constexpr int MSG_PRINT_SIZE{(MSG_SIZE - 1)};
constexpr int DIVSIOR_WIDTH{8};

class crc64_fixture : public testing::Test {
  protected:
    char message[MSG_SIZE + 1]{"123456789kjkjkjkj"}; // +1 for null terminator
    constexpr static u64 s_check_value{0x6c40df5f0b497347};
};

TEST_F(crc64_fixture, compute_checksum) {
    observe_ptr ptr_obj{message};
    Crc64 crc{ptr_obj, MSG_SIZE};
    std::println(stderr, "msg len: {}", MSG_SIZE);

    for(std::size_t i{}; i < MSG_PRINT_SIZE; i++) {
        std::println(stderr, "{}: {}", i + 1, message[i]);
    }

    auto checksum{crc.compute_checksum()};

    std::println(stderr, "========================================");

    for(std::size_t i{}; i < MSG_PRINT_SIZE; i++) {
        std::println(stderr, "{}: {}", i + 1, message[i]);

        if(i > DIVSIOR_WIDTH) {
            EXPECT_EQ(message[i], 0);
        }
    }

    std::println(stderr, "message: {}, size: {}", message, MSG_PRINT_SIZE);
    std::println(stderr, "checksum: {:064b}", checksum);
    std::println(stderr, "data: {}", message[0]);
}
