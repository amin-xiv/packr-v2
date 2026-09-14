#include <filesystem>
#include <packr/entry.hpp>
#include <packr/utils.hpp>

#include "./helpers/shared_test_data.hpp"
#include <gtest/gtest.h>

using namespace packr;

// This is the test environment setup that runs before all other tests

TEST_F(packingAndUnpackingFixture, packingAndUnpacking) {
    std::error_code err;
    // New directory to contain the results of these tests
    // make sure that it's already fresh and deleted
    std::ignore = system(std::string{"rm -rf " + playground_dirname}.data());
    fs::create_directory(playground_dirname, err);
    fs::current_path(playground_dirname, err);

    /* PACKING */

    // testing that filenames are properly managed
    ASSERT_EQ(system(std::string{packr + " -p -l ../" + dummy_dir1_dirname + "/"}.data()), 0);
    EXPECT_TRUE(fs::directory_entry{dummy_dir1_dirname + extension}.exists());
    ASSERT_EQ(system(std::string{"rm -rf " + dummy_dir1_dirname + extension}.data()), 0); // cleanup

    // this time without the trailing '/'
    ASSERT_EQ(system(std::string{packr + " -p -l ../" + dummy_dir1_dirname}.data()), 0);
    EXPECT_TRUE(fs::directory_entry{dummy_dir1_dirname + extension}.exists());

    // this time with a custom name + while following symlinks
    ASSERT_EQ(system(std::string{packr + " -p -l ../" + dummy_dir1_dirname + " -s -a " + dum_dirname}.data()), 0);
    EXPECT_TRUE(fs::directory_entry{dum_dirname + extension}.exists());

    // cycle_test directory: used to test symlink and recursion handling
    ASSERT_EQ(system(std::string{packr + " -p -l ../" + cycle_test_dirname + " -s"}.data()), 0);
    EXPECT_TRUE(fs::directory_entry{cycle_test_dirname + extension}.exists());

    /* UNPACKING */

    ASSERT_EQ(system(std::string{packr + " -u -l " + dummy_dir1_dirname + extension}.data()), 0);
    fs::directory_entry new_dummy_dir1{dummy_dir1_dirname};
    EXPECT_TRUE(new_dummy_dir1.is_directory(err));

    ASSERT_EQ(system(std::string{packr + " -u -l " + dum_dirname + extension}.data()), 0);
    fs::directory_entry dum{dum_dirname};
    EXPECT_TRUE(dum.is_directory(err));

    ASSERT_EQ(system(std::string{packr + " -u -l " + cycle_test_dirname + extension}.data()), 0);
    fs::directory_entry cycle_test_new{cycle_test_dirname};
    EXPECT_TRUE(cycle_test_new.is_directory(err));
}
