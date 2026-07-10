#pragma once

#include <packr/entry.hpp>
#include <gtest/gtest.h>

void compare_dir_entries(const packr::dir_entry& lhs, const packr::dir_entry& rhs) {
    ASSERT_TRUE(lhs.success);
    ASSERT_TRUE(rhs.success);

    if(std::string{lhs.dirname} == std::string{rhs.dirname}) {
        EXPECT_STREQ(lhs.dirname, rhs.dirname);
        EXPECT_EQ(lhs.dirname_length, rhs.dirname_length);
    }

    EXPECT_EQ(lhs.size, rhs.size);
    EXPECT_EQ(lhs.child_dir_count, rhs.child_dir_count);
    EXPECT_EQ(lhs.child_file_count, rhs.child_file_count);
    EXPECT_EQ(lhs.total_file_count, rhs.total_file_count);
    EXPECT_EQ(lhs.total_dir_count, rhs.total_dir_count);
    EXPECT_EQ(lhs.acc_time, rhs.acc_time);
    EXPECT_EQ(lhs.mod_time, rhs.mod_time);
    EXPECT_EQ(lhs.sc_time, rhs.sc_time);
    EXPECT_EQ(lhs.mode, rhs.mode);
    EXPECT_EQ(lhs.type, rhs.type);
}
