#include <filesystem>
#include <packr/entry.hpp>
#include <packr/internal.hpp>

#include "../helpers/shared_test_data.hpp"

#include <gtest/gtest.h>

using namespace packr;

TEST_F(packingAndUnpackingFixture, inc_dir_ent_dir_count) {
    /* SETUP */
    fs::directory_entry dir{dummy_dir1.path() / "inner1"};
    ASSERT_TRUE(fs::exists(dir));
    anc_map_t anc_table{};
    dir_entry dir_data{}; // default initialized
    dir_data.m_success = dir_entry_ret_code::success;
    dir_entry dir_data_inner{dir, 0, 0, anc_table};
    anc_table.clear();

    ASSERT_EQ(dir_data_inner.m_success, dir_entry_ret_code::success);

    // root level(root + 1, as it's a direct child directory)
    ASSERT_TRUE(inc_dir_ent_dir_count(dir_data, dir, 1, 0, anc_table));

    EXPECT_EQ(dir_data.m_size, dir_data_inner.m_size);
    EXPECT_EQ(dir_data.m_total_entry_count, dir_data_inner.m_total_entry_count + 1); // +1 for the main one we created
    EXPECT_EQ(dir_data.m_total_dir_count, dir_data_inner.m_total_dir_count + 1);     // +1 for the main one we created
    EXPECT_EQ(dir_data.m_total_file_count, dir_data_inner.m_total_file_count);
    EXPECT_EQ(dir_data.m_child_dir_count, 1);
    EXPECT_EQ(dir_data.m_child_entry_count, 1);

    // nested
    dir_data = {}; // reset it
    dir_data.m_success = dir_entry_ret_code::success;
    ASSERT_TRUE(inc_dir_ent_dir_count(dir_data, dir, 0, 0, anc_table));

    EXPECT_EQ(dir_data.m_size, dir_data_inner.m_size);
    EXPECT_EQ(dir_data.m_total_entry_count, dir_data_inner.m_total_entry_count + 1); // +1 for the main one we created
    EXPECT_EQ(dir_data.m_total_dir_count, dir_data_inner.m_total_dir_count + 1);     // +1 for the main one we created
    EXPECT_EQ(dir_data.m_total_file_count, dir_data_inner.m_total_file_count);
    EXPECT_EQ(dir_data.m_child_dir_count, 0);
    EXPECT_EQ(dir_data.m_child_entry_count, 0);

    // nonexistent dir
    dir_data = {}; // reset it
    dir_data.m_success = dir_entry_ret_code::success;
    dir = fs::directory_entry{"some-nonexisten-dir"};
    ASSERT_FALSE(inc_dir_ent_dir_count(dir_data, dir, 0, 0, anc_table));
    ASSERT_EQ(dir_data.m_success, dir_entry_ret_code::fail);
}

TEST_F(packingAndUnpackingFixture, inc_dir_ent_file_count) {
    fs::directory_entry file{dummy_dir1.path() / "hallo.txt"}; // must exist at build dir
    ASSERT_TRUE(file.exists());

    const auto file_size{file.file_size()};

    // root level, with add_size = true
    dir_entry dir1{}; // all default initialized
    inc_dir_ent_file_count(dir1, file, 0);

    EXPECT_EQ(dir1.m_size, file_size);
    EXPECT_EQ(dir1.m_total_entry_count, 1);
    EXPECT_EQ(dir1.m_total_file_count, 1);
    EXPECT_EQ(dir1.m_child_entry_count, 1);
    EXPECT_EQ(dir1.m_child_file_count, 1);

    // root level, with add_size = false
    dir_entry dir2{}; // all default initialized
    inc_dir_ent_file_count(dir2, file, 0, false);

    EXPECT_EQ(dir2.m_size, 0);
    EXPECT_EQ(dir2.m_total_entry_count, 1);
    EXPECT_EQ(dir2.m_total_file_count, 1);
    EXPECT_EQ(dir2.m_child_entry_count, 1);
    EXPECT_EQ(dir2.m_child_file_count, 1);

    // nested, with add_size = true
    dir_entry dir3{}; // all default initialized
    inc_dir_ent_file_count(dir3, file, 1);

    EXPECT_EQ(dir3.m_size, file_size);
    EXPECT_EQ(dir3.m_total_entry_count, 1);
    EXPECT_EQ(dir3.m_total_file_count, 1);
    EXPECT_EQ(dir3.m_child_entry_count, 0);
    EXPECT_EQ(dir3.m_child_file_count, 0);

    // nested, with add_size = false
    dir_entry dir4{}; // all default initialized
    inc_dir_ent_file_count(dir4, file, 1, false);

    EXPECT_EQ(dir4.m_size, 0);
    EXPECT_EQ(dir4.m_total_entry_count, 1);
    EXPECT_EQ(dir4.m_total_file_count, 1);
    EXPECT_EQ(dir4.m_child_entry_count, 0);
    EXPECT_EQ(dir4.m_child_file_count, 0);
}

TEST(inc_dir_ent_file_count_DeathTest, nonexistent_File) {
#ifdef NDEBUG // as assertions only run in debug mode
    GTEST_SKIP();
#endif

    fs::directory_entry file{"some-nonexistent-file888"};
    dir_entry dir{};

    EXPECT_DEATH(inc_dir_ent_file_count(dir, file, 0), ".*");
}
