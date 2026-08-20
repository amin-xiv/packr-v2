#pragma once

#include <gtest/gtest.h>
#include <packr/entry.hpp>
#include <packr/utils.hpp>
#include <packr/misc_structs.hpp>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace fs = std::filesystem;
using namespace packr;

void compare_time_specs(const timespec& lhs, const packr::time_spec& rhs) {
    EXPECT_EQ(lhs.tv_sec, rhs.sec);
    EXPECT_EQ(lhs.tv_nsec, rhs.nsec);
}

void compare_time_specs(const packr::time_spec& lhs, const packr::time_spec& rhs) {
    EXPECT_EQ(lhs.sec, rhs.sec);
    EXPECT_EQ(lhs.nsec, rhs.nsec);
}

void compare_dir_entries(const packr::dir_entry& lhs, const packr::dir_entry& rhs) {
    ASSERT_EQ(lhs.m_success, dir_entry_ret_code::success);
    ASSERT_EQ(rhs.m_success, dir_entry_ret_code::success);

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

void compare_dir_trees(const fs::directory_entry& base, const std::filesystem::directory_entry& sample, const packr::u8 opts) {
    std::error_code err;
    const bool sym{(opts & packr::O_SYM) > 0};

    // verify both exist
    ASSERT_TRUE(fs::exists(base.symlink_status()));
    ASSERT_TRUE(fs::exists(sample.symlink_status()));

    // verify both have the same size before we even start
    ASSERT_EQ(packr::get_dir_size(base, opts), packr::get_dir_size(sample, opts));

    for(const fs::directory_entry& entry : std::filesystem::recursive_directory_iterator(base)) {
        std::string entry_relative_path{entry.path()};
        entry_relative_path.erase(0, base.path().string().size()); // getting relative path
        fs::directory_entry sample_copy{sample.path().string() + entry_relative_path};
        fs::file_status entry_sym_stat{entry.symlink_status(err)};

        ASSERT_TRUE(fs::exists(sample_copy.symlink_status()));
        if(fs::is_regular_file(entry_sym_stat)) {
            ASSERT_TRUE(fs::is_regular_file(sample_copy));
            ASSERT_EQ(entry.file_size(), sample_copy.file_size());

        } else if(fs::is_directory(entry_sym_stat)) {
            ASSERT_TRUE(fs::is_directory(sample_copy));
            ASSERT_EQ(get_dir_size(entry, opts), get_dir_size(sample_copy, opts));
        } else if(fs::is_symlink(entry_sym_stat)) {
            fs::path entry_canonical{packr::read_symlink(entry)};

            if(entry_canonical.empty()) {
                ASSERT_TRUE(fs::is_symlink(sample_copy));
                continue;
            }

            if(fs::is_directory(entry_canonical)) {
                if(sym) {
                    ASSERT_TRUE(fs::is_directory(sample_copy));
                    ASSERT_EQ(get_dir_size(entry, opts), get_dir_size(sample_copy, opts));

                } else {
                    ASSERT_TRUE(fs::is_symlink(sample_copy));
                }

            } else if(fs::is_regular_file(entry)) {
                if(sym) {
                    ASSERT_TRUE(fs::is_regular_file(sample_copy));
                    ASSERT_EQ(entry.file_size(), sample_copy.file_size());
                } else {
                    ASSERT_TRUE(fs::is_symlink(sample_copy));
                }
            } else {
                std::string err_msg{"Potentially found a special file: " + entry.path().string()};
                throw std::runtime_error(err_msg);
            }
        }
    }
}
