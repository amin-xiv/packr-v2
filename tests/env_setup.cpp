#include <packr/entry.hpp>

#include "shared_test_data.hpp"
#include <gtest/gtest.h>

TEST_F(packingAndUnpackingFixture, packFilename) {
    std::error_code err;
    // New directory to contain the results of these tests
    // make sure that it's already fresh and deleted
    std::ignore = system(std::string{"rm -rf " + playground_dirname}.data());
    fs::create_directory(playground_dirname, err);
    fs::current_path(playground_dirname, err);

    // testing that filenames are properly managed
    ASSERT_EQ(system(std::string{packr + " -p -l ../" + dummy_dir1_name + "/"}.data()), 0);
    EXPECT_TRUE(fs::directory_entry{dummy_dir1_name + extension}.exists());
    ASSERT_EQ(system(std::string{"rm -rf " + dummy_dir1_name + extension}.data()), 0); // cleanup

    // this time without the trailing '/'
    ASSERT_EQ(system(std::string{packr + " -p -l ../" + dummy_dir1_name}.data()), 0);
    EXPECT_TRUE(fs::directory_entry{dummy_dir1_name + extension}.exists());

    // this time with a custom name + while following symlinks
    ASSERT_EQ(system(std::string{packr + " -p -l ../" + dummy_dir1_name + " -s -a " + dum_dirname}.data()), 0);
    EXPECT_TRUE(fs::directory_entry{dum_dirname + extension}.exists());
}
