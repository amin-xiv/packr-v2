#pragma once

#include <filesystem>
#include <packr/entry.hpp>
#include <packr/utils.hpp>
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

void compare_dir_trees(const std::filesystem::directory_entry& base, const std::filesystem::directory_entry& sample) {
    // verify both exist
    ASSERT_TRUE(std::filesystem::exists(base.symlink_status()));
    ASSERT_TRUE(std::filesystem::exists(sample.symlink_status()));

    // verify both have the same size before we even start
    ASSERT_EQ(packr::get_dir_size(base), packr::get_dir_size(sample));

    for(const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(base)) {
        std::string entry_relative_path{entry.path()};
        entry_relative_path.erase(0, base.path().string().size()); // getting relative path
        std::filesystem::directory_entry sample_copy{sample.path().string() + entry_relative_path};

        ASSERT_TRUE(sample_copy.exists());

        // TODO: Check for modes(after editing it of course)

        if(std::filesystem::is_regular_file(sample_copy)) {
            ASSERT_TRUE(std::filesystem::is_regular_file(entry)); // it must correspond
            ASSERT_EQ(entry.file_size(), sample_copy.file_size());

        } else if(std::filesystem::is_directory(sample_copy)) {
            ASSERT_TRUE(std::filesystem::is_directory(entry));
        }
    }
}
