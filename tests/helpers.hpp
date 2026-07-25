#pragma once

#include <packr/entry.hpp>
#include <packr/utils.hpp>
#include <packr/misc_structs.hpp>
#include <filesystem>
#include <unistd.h>
#include <gtest/gtest.h>

void compare_time_specs(const timespec& lhs, const packr::time_spec& rhs) {
    EXPECT_EQ(lhs.tv_sec, rhs.sec);
    EXPECT_EQ(lhs.tv_nsec, rhs.nsec);
}

void compare_time_specs(const packr::time_spec& lhs, const packr::time_spec& rhs) {
    EXPECT_EQ(lhs.sec, rhs.sec);
    EXPECT_EQ(lhs.nsec, rhs.nsec);
}

void compare_dir_entries(const packr::dir_entry& lhs, const packr::dir_entry& rhs) {
    ASSERT_TRUE(lhs.m_success);
    ASSERT_TRUE(rhs.m_success);

    if(std::string{lhs.m_dirname} == std::string{rhs.m_dirname}) {
        EXPECT_STREQ(lhs.m_dirname, rhs.m_dirname);
        EXPECT_EQ(lhs.m_dirname_length, rhs.m_dirname_length);
    }

    EXPECT_EQ(lhs.m_size, rhs.m_size);
    EXPECT_EQ(lhs.m_child_dir_count, rhs.m_child_dir_count);
    EXPECT_EQ(lhs.m_child_file_count, rhs.m_child_file_count);
    EXPECT_EQ(lhs.m_total_file_count, rhs.m_total_file_count);
    EXPECT_EQ(lhs.m_total_dir_count, rhs.m_total_dir_count);
    // compare_time_specs(lhs.m_acc_time, rhs.m_acc_time);
    // compare_time_specs(lhs.m_mod_time, rhs.m_mod_time);
    // compare_time_specs(lhs.m_sc_time, rhs.m_sc_time);
    EXPECT_EQ(lhs.m_type, rhs.m_type);
}

void compare_dir_trees(const std::filesystem::directory_entry& base, const std::filesystem::directory_entry& sample,
                       const packr::u8 opts) {
    // verify both exist
    ASSERT_TRUE(std::filesystem::exists(base.symlink_status()));
    ASSERT_TRUE(std::filesystem::exists(sample.symlink_status()));

    // verify both have the same size before we even start
    ASSERT_EQ(packr::get_dir_size(base, opts), packr::get_dir_size(sample, opts));

    for(const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(base)) {
        std::string entry_relative_path{entry.path()};
        entry_relative_path.erase(0, base.path().string().size()); // getting relative path
        std::filesystem::directory_entry sample_copy{sample.path().string() + entry_relative_path};

        ASSERT_TRUE(sample_copy.exists());

        // TODO: Check for modes

        if(std::filesystem::is_regular_file(sample_copy)) {
            ASSERT_TRUE(std::filesystem::is_regular_file(entry)); // it must correspond
            ASSERT_EQ(entry.file_size(), sample_copy.file_size());

        } else if(std::filesystem::is_directory(sample_copy)) {
            ASSERT_TRUE(std::filesystem::is_directory(entry));
        }
    }
}
