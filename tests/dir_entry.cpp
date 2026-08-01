#include <packr/utils.hpp>
#include <packr/types.hpp>
#include <packr/entry.hpp>
#include "shared_test_data.hpp"
// #include "helpers.hpp"
#include <system_error>
#include <gtest/gtest.h>
#include <string>
#include <optional>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem>

using namespace packr;

TEST_F(dirAndFileEntryConstructorData, DirectoryEntryConstructorData) {
    // dummy ec object to avoid exceptions
    std::error_code err;
    const u8 opts{0};

    // fs directory intialization
    fs::directory_entry dir_fs{full_path.value()};
    ASSERT_TRUE(fs::is_directory(dir_fs));

    // dir_entry initialization
    dir_entry dirEntry{fs::directory_entry{joined}, DEFAULT_ROOT_DIR, opts};

    struct stat ent_stat;
    // getting the dir's timestamps and such
    ASSERT_FALSE(stat(full_path.value().data(), &ent_stat) == -1);

    ASSERT_TRUE(dirEntry.m_success);
    EXPECT_STREQ(dir_fs.path().filename().c_str(), dirEntry.m_dirname);
    EXPECT_EQ(dir_fs.path().filename().string().size(), dirEntry.m_dirname_length);
    EXPECT_EQ(get_dir_size(dir_fs, opts), dirEntry.m_size);
    // NOTE: Access time won't be compared for now
    // compare_time_specs(ent_stat.st_atim, dirEntry.m_acc_time);
    // compare_time_specs(ent_stat.st_mtim, dirEntry.m_mod_time);
    // compare_time_specs(ent_stat.st_ctim, dirEntry.m_sc_time);
    EXPECT_EQ(dirEntry.m_child_entry_count, 5);
    EXPECT_EQ(dirEntry.m_child_file_count, 4);
    EXPECT_EQ(dirEntry.m_child_dir_count, 1);
    EXPECT_EQ(dirEntry.m_total_entry_count, 10);
    EXPECT_EQ(dirEntry.m_total_file_count, 7);
    EXPECT_EQ(dirEntry.m_total_dir_count, 3);
    EXPECT_EQ((fs::perms(dirEntry.m_mode)), dir_fs.status().permissions());
    EXPECT_EQ(dirEntry.m_type, dir_type::regular);
}
