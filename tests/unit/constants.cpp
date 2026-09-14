#include <packr/constants.hpp>

#include <gtest/gtest.h>

using namespace packr;

/* tests that test constant values */

TEST(user_flags, main) {
    EXPECT_EQ(O_SYM, 0B00000001);
}

TEST(otherMacrosAndConstants, main) {
    EXPECT_EQ(DEFAULT_ROOT_DIR, 0);
}

TEST(specialMarkers, main) {
    EXPECT_EQ(ENT_DIR_START, 0x01);
    EXPECT_EQ(ENT_DIR_END, 0x02);
    EXPECT_EQ(ENT_FILE, 0x04);
    EXPECT_EQ(PACK_START, 0x08);
    EXPECT_EQ(PACK_END, 0x10);
}
