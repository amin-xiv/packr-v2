#include <packr/utils.hpp>
#include <packr/types.hpp>
#include <packr/entry.hpp>
#include "shared_test_data.hpp"
// #include "helpers.hpp"
#include <system_error>
#include <gtest/gtest.h>
#include <string>
#include <sys/stat.h>
#include <filesystem>

using namespace packr;

TEST_F(dirAndFileEntryConstructorData, FileEntryBasic) {
    // dummy ec object to avoid exceptions
    std::error_code err;
    const u8 opts{};

    // fs directory intialization
    fs::directory_entry file_fs{std::string{full_path.value() + '/' + "hallo.txt"}};
    ASSERT_TRUE(file_fs.exists()) << file_fs.path().string();

    // dir_entry initialization
    file_entry fileEntry{file_fs.path().string(), opts};

    struct stat ent_stat;
    // getting the dir's timestamps and such
    ASSERT_FALSE(lstat(file_fs.path().c_str(), &ent_stat) == -1);

    ASSERT_TRUE(fileEntry.m_success);
    EXPECT_STREQ(file_fs.path().filename().c_str(), fileEntry.m_filename);
    EXPECT_EQ(file_fs.path().filename().string().size(), fileEntry.m_filename_length);
    EXPECT_EQ(file_fs.file_size(), fileEntry.m_size);
    // compare_time_specs(ent_stat.st_atim, fileEntry.m_acc_time);
    // compare_time_specs(ent_stat.st_mtim, fileEntry.m_mod_time);
    // compare_time_specs(ent_stat.st_ctim, fileEntry.m_sc_time);

    EXPECT_EQ(fs::perms(fileEntry.m_mode), file_fs.status().permissions());
    EXPECT_EQ(fileEntry.m_type, file_type::regular);
}

TEST_F(dirAndFileEntryConstructorData, FileEntrySymlink) {
    // dummy ec object to avoid exceptions
    std::error_code err;
    const u8 opts{O_SYM};

    fs::directory_entry file_fs;

    bool valid{};
    if(fs::directory_entry{full_path.value()}.is_symlink(err)) {
        fs::path original_dir_path{fs::read_symlink(full_path.value(), err)};
        ASSERT_FALSE(original_dir_path.empty());
        file_fs = fs::directory_entry{original_dir_path.string() + '/' + "sym_file"};
        valid = true;
    }

    ASSERT_TRUE(valid);

    // fs directory intialization
    ASSERT_TRUE(file_fs.exists()) << file_fs.path().string();

    // dir_entry initialization
    file_entry fileEntry{file_fs.path().string(), opts};

    struct stat ent_stat;
    // getting the dir's timestamps and such
    ASSERT_FALSE(stat(file_fs.path().c_str(), &ent_stat) == -1);

    ASSERT_TRUE(fileEntry.m_success);
    EXPECT_STREQ(file_fs.path().filename().c_str(), fileEntry.m_filename);
    EXPECT_EQ(file_fs.path().filename().string().size(), fileEntry.m_filename_length);
    EXPECT_EQ(file_fs.file_size(), fileEntry.m_size);
    // compare_time_specs(ent_stat.st_atim, fileEntry.m_acc_time);
    // compare_time_specs(ent_stat.st_mtim, fileEntry.m_mod_time);
    // compare_time_specs(ent_stat.st_ctim, fileEntry.m_sc_time);

    EXPECT_EQ(fs::perms(fileEntry.m_mode), file_fs.status().permissions());
    EXPECT_EQ(fileEntry.m_type, file_type::symlink);
}

TEST_F(dirAndFileEntryConstructorData, FileEntryBrokenSymlink) {
    // dummy ec object to avoid exceptions
    std::error_code err;
    const u8 opts{O_SYM};

    fs::directory_entry file_fs;

    bool valid{};
    if(fs::directory_entry{full_path.value()}.is_symlink(err)) {
        fs::path original_dir_path{fs::read_symlink(full_path.value(), err)};
        ASSERT_FALSE(original_dir_path.empty());
        file_fs = fs::directory_entry{original_dir_path.string() + '/' + "broken_symlink_file"};
        valid = true;
    }

    ASSERT_TRUE(valid);

    // fs directory intialization
    ASSERT_TRUE(fs::exists(file_fs.symlink_status())) << file_fs.path().string();

    // dir_entry initialization
    file_entry fileEntry{file_fs.path().string(), opts};

    struct stat ent_stat;
    // getting the dir's timestamps and such
    ASSERT_FALSE(lstat(file_fs.path().c_str(), &ent_stat) == -1);

    ASSERT_TRUE(fileEntry.m_success);
    EXPECT_STREQ(file_fs.path().filename().c_str(), fileEntry.m_filename);
    EXPECT_EQ(file_fs.path().filename().string().size(), fileEntry.m_filename_length);
    EXPECT_EQ(0, fileEntry.m_size); // 0 as symlinks and special files must have m_size as 0
    // compare_time_specs(ent_stat.st_atim, fileEntry.m_acc_time);
    // compare_time_specs(ent_stat.st_mtim, fileEntry.m_mod_time);
    // compare_time_specs(ent_stat.st_ctim, fileEntry.m_sc_time);

    EXPECT_EQ(fs::perms(fileEntry.m_mode), file_fs.symlink_status().permissions());
    EXPECT_EQ(fileEntry.m_type, file_type::symlink);
}
