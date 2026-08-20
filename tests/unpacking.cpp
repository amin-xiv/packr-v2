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
    const int opts{};

    // As path returns to {ROOT}/build with each new test
    fs::current_path(playground_dirname, err);

    ASSERT_EQ(system(std::string{packr + " -u -l " + dummy_dir1_name + extension}.data()), 0);
    fs::directory_entry new_dummy_dir1{"dummy_dir1"};
    EXPECT_TRUE(new_dummy_dir1.is_directory(err));

    anc_map_t anc_map{};
    dir_entry dummy_dir1_data{dummy_dir1, DEFAULT_ROOT_DIR, opts, anc_map};
    anc_map.clear();
    dir_entry new_dummy_dir1_data{new_dummy_dir1, DEFAULT_ROOT_DIR, opts, anc_map};

    ASSERT_EQ(get_dir_size(dummy_dir1, opts), get_dir_size(new_dummy_dir1, opts));
    ASSERT_EQ(dummy_dir1_data.m_size, new_dummy_dir1_data.m_size);
    ASSERT_EQ(new_dummy_dir1_data.m_size, get_dir_size(new_dummy_dir1, opts));

    compare_dir_entries(dummy_dir1_data, new_dummy_dir1_data);
}

TEST_F(packingAndUnpackingTestdata, unpackFollowSymlinks) {
    std::error_code err;
    u8 opts{O_SYM};

    // As path returns to {ROOT}/build with each new test
    fs::current_path(playground_dirname, err);

    ASSERT_EQ(system(std::string{packr + " -u -l " + dum_dirname + extension}.data()), 0);
    fs::directory_entry dum{"dum"};
    EXPECT_TRUE(dum.is_directory(err));

    anc_map_t anc_map{};
    dir_entry dummy_dir1_data{dummy_dir1, DEFAULT_ROOT_DIR, opts, anc_map};
    anc_map.clear();
    dir_entry dum_data{dum, DEFAULT_ROOT_DIR, opts, anc_map};

    ASSERT_EQ(get_dir_size(dummy_dir1, opts), get_dir_size(dum, opts));
    ASSERT_EQ(dummy_dir1_data.m_size, dum_data.m_size);
    ASSERT_EQ(dum_data.m_size, get_dir_size(dum, opts));

    compare_dir_entries(dummy_dir1_data, dum_data);
}

TEST_F(packingAndUnpackingTestdata, unpackBasicDirStructure) {
    fs::current_path(playground_dirname, err);

    compare_dir_trees(dummy_dir1, fs::directory_entry(dummy_dir1_name), 0);
    compare_dir_trees(dummy_dir1, fs::directory_entry(dum_dirname), O_SYM);
}
