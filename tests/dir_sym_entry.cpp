#include <packr/utils.hpp>
#include <packr/types.hpp>
#include <packr/entry.hpp>

#include "shared_test_data.hpp"

#include <gtest/gtest.h>
#include <filesystem>
#include <utility>

using namespace packr;

TEST_F(packingAndUnpackingFixture, regularDir) {
    fs::directory_entry dir{playground_dirname};
    ASSERT_TRUE(fs::exists(dir));

    dir_sym_entry entry{dir};
    ASSERT_TRUE(entry.m_success);

    EXPECT_STREQ(entry.m_name, playground_dirname.c_str());
    EXPECT_EQ(entry.m_name_length, playground_dirname.length());
    EXPECT_STREQ(entry.m_secondary_path, "");
    EXPECT_EQ(entry.m_secondary_path_length, 0);
    EXPECT_EQ(entry.m_mode, std::to_underlying(dir.symlink_status(err).permissions()));
    EXPECT_FALSE(entry.m_is_symlink);

    // time stuff
}

TEST_F(packingAndUnpackingFixture, DirSym_Symlink) {
    fs::directory_entry dir{dummy_dir1_name};
    ASSERT_TRUE(fs::exists(dir));
    fs::path secondary_path{fs::read_symlink(dir)};

    dir_sym_entry entry{dir};
    ASSERT_TRUE(entry.m_success);

    EXPECT_STREQ(entry.m_name, dummy_dir1_name.c_str());
    EXPECT_EQ(entry.m_name_length, dummy_dir1_name.length());
    EXPECT_STREQ(entry.m_secondary_path, secondary_path.c_str());
    EXPECT_EQ(entry.m_secondary_path_length, secondary_path.string().length());
    EXPECT_EQ(entry.m_mode, std::to_underlying(dir.symlink_status(err).permissions()));
    EXPECT_TRUE(entry.m_is_symlink);

    // time stuff
}
