#include <packr/utils.hpp>
#include <packr/types.hpp>
#include <packr/entry.hpp>
#include "shared_test_data.hpp"
#include "helpers.hpp"
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
    EXPECT_EQ(dirEntry.m_child_entry_count, 3);
    EXPECT_EQ(dirEntry.m_child_file_count, 2);
    EXPECT_EQ(dirEntry.m_child_dir_count, 1);
    EXPECT_EQ(dirEntry.m_total_entry_count, 8);
    EXPECT_EQ(dirEntry.m_total_file_count, 5);
    EXPECT_EQ(dirEntry.m_total_dir_count, 3);
    EXPECT_EQ((fs::perms(dirEntry.m_mode)), dir_fs.status().permissions());
    EXPECT_EQ(dirEntry.m_type, dir_type::regular);
}

TEST_F(dirAndFileEntryConstructorData, FileEntryConstructorData) {
    // dummy ec object to avoid exceptions
    std::error_code err;

    // fs directory intialization
    fs::directory_entry file_fs{std::string{full_path.value() + '/' + "hallo.txt"}};
    ASSERT_TRUE(file_fs.exists()) << file_fs.path().string();

    // dir_entry initialization
    file_entry fileEntry{file_fs.path().string(), 0};

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

TEST_F(packingAndUnpackingTestdata, packFilename) {
    std::error_code err;
    // New directory to contain the results of these tests
    // make sure that it's already fresh and deleted
    ASSERT_NE(system(std::string{"rm -rf " + playground_dirname}.data()), -1); // suppress unused variable warning
    fs::create_directory(playground_dirname, err);
    fs::current_path(playground_dirname, err);

    // First three EXPECTS to test that filenames are properly managed
    ASSERT_EQ(system(std::string{packr + " -p -l ../" + dummy_dir1_name + "/"}.data()), 0);
    EXPECT_TRUE(fs::directory_entry{dummy_dir1_name + extension}.exists());
    ASSERT_EQ(system(std::string{"rm -rf " + dummy_dir1_name + extension}.data()), 0); // cleanup

    // this time without the trailing '/'
    ASSERT_EQ(system(std::string{packr + " -p -l ../" + dummy_dir1_name}.data()), 0);
    EXPECT_TRUE(fs::directory_entry{dummy_dir1_name + extension}.exists());

    // this time with a custom name
    ASSERT_EQ(system(std::string{packr + " -p -l ../" + dummy_dir1_name + " -a" + dum_dirname}.data()), 0);
    EXPECT_TRUE(fs::directory_entry{dum_dirname + extension}.exists());
}
