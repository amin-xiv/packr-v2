#include <packr/utils.hpp>
#include <packr/types.hpp>
#include <packr/entry.hpp>
#include "shared_test_data.hpp"
#include "helpers.hpp"
#include <system_error>
#include <gtest/gtest.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem>

namespace fs = std::filesystem;

using namespace packr;

TEST_F(packingAndUnpackingTestdata, unpackBasicData) {
    std::error_code err;

    // As path returns to {ROOT}/build with each new test
    fs::current_path(playground_dirname, err);

    ASSERT_EQ(system(std::string{packr + " -u -l " + dummy_dir1_name + extension}.data()), 0);
    fs::directory_entry new_dummy_dir1{"dummy_dir1"};
    EXPECT_TRUE(new_dummy_dir1.is_directory(err)) << fs::current_path().string();

    ASSERT_EQ(system(std::string{packr + " -u -l " + dum_dirname + extension}.data()), 0);
    fs::directory_entry dum{"dum"};
    EXPECT_TRUE(dum.is_directory(err));

    // Now testing actual directory data
    ASSERT_EQ(get_dir_size(dummy_dir1, 0), get_dir_size(new_dummy_dir1, 0));
    ASSERT_EQ(get_dir_size(dummy_dir1, O_SYM), get_dir_size(dum, O_SYM)) << dummy_dir1.path().c_str();

    dir_entry dummy_dir1_data{dummy_dir1, DEFAULT_ROOT_DIR, 0};
    dir_entry dummy_dir1_data_sym{dummy_dir1, DEFAULT_ROOT_DIR, O_SYM};
    dir_entry new_dummy_dir1_data{new_dummy_dir1, DEFAULT_ROOT_DIR, 0};
    dir_entry dum_data{dum, DEFAULT_ROOT_DIR, 0};

    compare_dir_entries(dummy_dir1_data, new_dummy_dir1_data);
    compare_dir_entries(dummy_dir1_data_sym, dum_data);

    // As these won't be compared
    EXPECT_STREQ(dum_data.m_dirname, "dum");
    EXPECT_EQ(dum_data.m_dirname_length, 3);
}

// TEST_F(packingAndUnpackingTestdata, unpackFollowSymlinks) {
//     std::error_code err;
//     const u8 opts{O_SYM};
//     EXPECT_NE(system(std::string{"rm -rf " + playground_dirname}.data()), -1);
//
//     // As path returns to {ROOT}/build with each new test
//     fs::current_path(playground_dirname, err);
//
//     ASSERT_EQ(system(std::string{packr + " -u -l " + dummy_dir1_name + extension}.data()), 0);
//     fs::directory_entry new_dummy_dir1{"dummy_dir1"};
//     EXPECT_TRUE(new_dummy_dir1.is_directory(err));
//
//     ASSERT_EQ(system(std::string{packr + " -u -l " + dum_dirname + extension}.data()), 0);
//     fs::directory_entry dum{"dum"};
//     EXPECT_TRUE(dum.is_directory(err));
//
//     // Now testing actual directory data
//     ASSERT_EQ(get_dir_size(dummy_dir1, opts), get_dir_size(new_dummy_dir1, opts));
//     ASSERT_EQ(get_dir_size(dummy_dir1, opts), get_dir_size(dum, opts));
//
//     dir_entry dummy_dir1_data{dummy_dir1, DEFAULT_ROOT_DIR, 0};
//     dir_entry new_dummy_dir1_data{new_dummy_dir1, DEFAULT_ROOT_DIR, 0};
//     dir_entry dum_data{dum, DEFAULT_ROOT_DIR, 0};
//
//     compare_dir_entries(dummy_dir1_data, new_dummy_dir1_data);
//     compare_dir_entries(dummy_dir1_data, dum_data);
//
//     // As these won't be compared
//     EXPECT_STREQ(dum_data.m_dirname, "dum");
//     EXPECT_EQ(dum_data.m_dirname_length, 3);
// }

TEST_F(packingAndUnpackingTestdata, unpackBasicDirStructure) {
    // return to playground since with each test path gets reset to the build dir
    fs::current_path(playground_dirname, err);
    compare_dir_trees(dummy_dir1, fs::directory_entry(dummy_dir1_name), 0);
    compare_dir_trees(dummy_dir1, fs::directory_entry(dum_dirname), O_SYM);
}
