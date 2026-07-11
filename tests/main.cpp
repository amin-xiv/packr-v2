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

namespace fs = std::filesystem;

using namespace packr;

TEST_F(dirAndFileEntryConstructorData, DirectoryEntryConstructorData) {
    // dummy ec object to avoid exceptions
    std::error_code err;

    // fs directory intialization
    fs::directory_entry dir_fs{full_path.value()};
    ASSERT_TRUE(dir_fs.exists());

    // dir_entry initialization
    dir_entry dirEntry{fs::directory_entry{joined}, DEFAULT_ROOT_DIR};

    struct stat ent_stat;
    // getting the dir's timestamps and such
    ASSERT_FALSE(lstat(full_path.value().data(), &ent_stat) == -1);

    ASSERT_TRUE(dirEntry.success);
    EXPECT_STREQ(dir_fs.path().filename().c_str(), dirEntry.dirname);
    EXPECT_EQ(dir_fs.path().filename().string().size(), dirEntry.dirname_length);
    EXPECT_EQ(get_dir_size(dir_fs), dirEntry.size);
    EXPECT_EQ(ent_stat.st_mtim.tv_sec + NSEC_TO_SEC(ent_stat.st_mtim.tv_nsec), dirEntry.mod_time);
    EXPECT_EQ(ent_stat.st_atim.tv_sec + NSEC_TO_SEC(ent_stat.st_atim.tv_nsec), dirEntry.acc_time);
    EXPECT_EQ(ent_stat.st_ctim.tv_sec + NSEC_TO_SEC(ent_stat.st_ctim.tv_nsec), dirEntry.sc_time);
    EXPECT_EQ(dirEntry.child_entry_count, 2);
    EXPECT_EQ(dirEntry.child_file_count, 1);
    EXPECT_EQ(dirEntry.child_dir_count, 1);
    EXPECT_EQ(dirEntry.total_entry_count, 7);
    EXPECT_EQ(dirEntry.total_file_count, 4);
    EXPECT_EQ(dirEntry.total_dir_count, 3);
    EXPECT_EQ((fs::perms(dirEntry.mode)), dir_fs.status().permissions());
    EXPECT_EQ(dirEntry.type, dir_type::regular);
}

TEST_F(dirAndFileEntryConstructorData, FileEntryConstructorData) {
    // dummy ec object to avoid exceptions
    std::error_code err;

    // fs directory intialization
    fs::directory_entry file_fs{std::string{full_path.value() + '/' + "hallo.txt"}};
    ASSERT_TRUE(file_fs.exists()) << file_fs.path().string();

    // dir_entry initialization
    file_entry fileEntry{file_fs.path().string()};

    struct stat ent_stat;
    // getting the dir's timestamps and such
    ASSERT_FALSE(lstat(file_fs.path().c_str(), &ent_stat) == -1);

    ASSERT_TRUE(fileEntry.success);
    EXPECT_STREQ(file_fs.path().filename().c_str(), fileEntry.filename);
    EXPECT_EQ(file_fs.path().filename().string().size(), fileEntry.filename_length);
    EXPECT_EQ(file_fs.file_size(), fileEntry.size);
    EXPECT_EQ(ent_stat.st_mtim.tv_sec + NSEC_TO_SEC(ent_stat.st_mtim.tv_nsec), fileEntry.mod_time);
    EXPECT_EQ(ent_stat.st_atim.tv_sec + NSEC_TO_SEC(ent_stat.st_atim.tv_nsec), fileEntry.acc_time);
    EXPECT_EQ(ent_stat.st_ctim.tv_sec + NSEC_TO_SEC(ent_stat.st_ctim.tv_nsec), fileEntry.sc_time);
    EXPECT_EQ(fs::perms(fileEntry.mode), file_fs.status().permissions());
    EXPECT_EQ(fileEntry.type, file_type::regular);
}

TEST_F(packingAndUnpackingTestdata, packFilename) {
    std::error_code err;
    // New directory to contain the results of these tests
    // make sure that it's already fresh and deleted
    system(std::string{"rm -rf " + playground_dirname}.data());
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

// Tests that the collected data is the same for each dir
TEST_F(packingAndUnpackingTestdata, unpackBasicData) {
    std::error_code err;

    // just in case path returns to {ROOT}/build
    fs::current_path(playground_dirname, err);

    ASSERT_EQ(system(std::string{packr + " -u -l " + dummy_dir1_name + extension}.data()), 0);
    fs::directory_entry new_dummy_dir1{"dummy_dir1"};
    EXPECT_TRUE(new_dummy_dir1.is_directory(err)) << fs::current_path().string();

    ASSERT_EQ(system(std::string{packr + " -u -l " + dum_dirname + extension}.data()), 0);
    fs::directory_entry dum{"dum"};
    EXPECT_TRUE(dum.is_directory(err));

    // Now testing actual directory data
    ASSERT_EQ(get_dir_size(dummy_dir1), get_dir_size(new_dummy_dir1));
    ASSERT_EQ(get_dir_size(dummy_dir1), get_dir_size(dum));

    dir_entry dummy_dir1_data{dummy_dir1, DEFAULT_ROOT_DIR};
    dir_entry new_dummy_dir1_data{new_dummy_dir1, DEFAULT_ROOT_DIR};
    dir_entry dum_data{dum, DEFAULT_ROOT_DIR};

    compare_dir_entries(dummy_dir1_data, new_dummy_dir1_data);
    compare_dir_entries(dummy_dir1_data, dum_data);

    // As these won't be compared
    EXPECT_STREQ(dum_data.dirname, "dum");
    EXPECT_EQ(dum_data.dirname_length, 3);
}

// Tests that the dir structure is the same
TEST_F(packingAndUnpackingTestdata, unpackBasicDirStructure) {
    // return to playground since with each test path gets reset to the build dir
    fs::current_path(playground_dirname, err);
    compare_dir_trees(dummy_dir1, fs::directory_entry(dummy_dir1_name));
    compare_dir_trees(dummy_dir1, fs::directory_entry(dum_dirname));
}
