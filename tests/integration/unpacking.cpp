#include <packr/utils.hpp>
#include <packr/types.hpp>
#include <packr/entry.hpp>
#include <packr/constants.hpp>

#include "../helpers/shared_test_data.hpp"
#include "../helpers/helpers.hpp"

#include <system_error>
#include <gtest/gtest.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem>

namespace fs = std::filesystem;

using namespace packr;

TEST_F(packingAndUnpackingFixture, unpackBasicData) {

    std::error_code err;
    const int opts{};

    // As path returns to {ROOT}/build with each new test
    fs::current_path(playground_dirname, err);

    // files should've been already unpacked on tests env setup

    fs::directory_entry new_dummy_dir1{"dummy_dir1"};
    EXPECT_TRUE(new_dummy_dir1.is_directory(err));

    anc_map_t anc_map{};
    dir_entry dummy_dir1_data{dummy_dir1, DEFAULT_ROOT_DIR, opts, anc_map};
    anc_map.clear();
    dir_entry new_dummy_dir1_data{new_dummy_dir1, DEFAULT_ROOT_DIR, opts, anc_map};

    anc_map_t anc_table1{};
    anc_map_t anc_table2{};
    ASSERT_EQ(get_dir_size(dummy_dir1, opts, anc_table1), get_dir_size(new_dummy_dir1, opts, anc_table2));
    ASSERT_EQ(dummy_dir1_data.m_size, new_dummy_dir1_data.m_size);
    anc_table1.clear();
    anc_table2.clear();
    ASSERT_EQ(new_dummy_dir1_data.m_size, get_dir_size(new_dummy_dir1, opts, anc_table1));

    SCOPED_TRACE("-> compare_dir_entries(dummy_dir1_data, new_dummy_dir1_data)");
    compare_dir_entries(dummy_dir1_data, new_dummy_dir1_data);
}

TEST_F(packingAndUnpackingFixture, unpackFollowSymlinks) {
    std::error_code err;
    u8 opts{O_SYM};

    // As path returns to {ROOT}/build with each new test
    fs::current_path(playground_dirname, err);

    // files should've been already unpacked on tests env setup

    fs::directory_entry dum{"dum"};
    EXPECT_TRUE(dum.is_directory(err));

    anc_map_t anc_map{};
    dir_entry dummy_dir1_data{dummy_dir1, DEFAULT_ROOT_DIR, opts, anc_map};
    anc_map.clear();
    dir_entry dum_data{dum, DEFAULT_ROOT_DIR, opts, anc_map};

    anc_map_t anc_table1{};
    anc_map_t anc_table2{};

    ASSERT_EQ(get_dir_size(dummy_dir1, opts, anc_table1), get_dir_size(dum, opts, anc_table2));
    ASSERT_EQ(dummy_dir1_data.m_size, dum_data.m_size);
    anc_table1.clear();
    anc_table2.clear();
    ASSERT_EQ(dum_data.m_size, get_dir_size(dum, opts, anc_table1));

    SCOPED_TRACE("-> compare_dir_entries(dummy_dir1_data, dum_data)");
    compare_dir_entries(dummy_dir1_data, dum_data);
}

TEST_F(packingAndUnpackingFixture, unpackBasicDirStructure) {
    std::error_code err;
    fs::current_path(playground_dirname, err);

    SCOPED_TRACE("-> compare_dir_trees(dummy_dir1, fs::directory_entry(dummy_dir1_dirname), 0);)\n"
                 "-> compare_dir_trees(dummy_dir1, fs::directory_entry(dum_dirname), O_SYM);");
    compare_dir_trees(dummy_dir1, fs::directory_entry(dummy_dir1_dirname), 0);
    compare_dir_trees(dummy_dir1, fs::directory_entry(dum_dirname), O_SYM);
}

TEST_F(packingAndUnpackingFixture, cycle_tests_dereference) {
    std::error_code err;
    u8 opts{O_SYM};

    // As path returns to {ROOT}/build with each new test
    fs::current_path(playground_dirname, err);

    // files should've been already unpacked on tests env setup

    EXPECT_TRUE(cycle_test.is_directory(err));
    fs::directory_entry new_cycle_test{cycle_test_dirname};
    EXPECT_TRUE(new_cycle_test.is_directory(err));

    anc_map_t anc_map{};
    dir_entry cycle_test_data{cycle_test, DEFAULT_ROOT_DIR, opts, anc_map};
    anc_map.clear();
    dir_entry new_cycle_test_data{new_cycle_test, DEFAULT_ROOT_DIR, opts, anc_map};

    anc_map.clear();

    ASSERT_EQ(cycle_test_data.m_size, get_dir_size(cycle_test, opts, anc_map));
    ASSERT_EQ(cycle_test_data.m_size, new_cycle_test_data.m_size);
    anc_map.clear();
    ASSERT_EQ(cycle_test_data.m_size, get_dir_size(new_cycle_test, opts, anc_map));

    SCOPED_TRACE("-> compare_dir_trees(cycle_test, new_cycle_test, opts);\n"
                 "-> compare_dir_entries(cycle_test_data, new_cycle_test_data);");
    compare_dir_trees(cycle_test, new_cycle_test, opts);
    compare_dir_entries(cycle_test_data, new_cycle_test_data);
}
