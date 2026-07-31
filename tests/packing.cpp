#include <packr/utils.hpp>
#include <packr/types.hpp>
#include <packr/entry.hpp>
#include "shared_test_data.hpp"
// #include "helpers.hpp"
#include <system_error>
#include <gtest/gtest.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem>

using namespace packr;

TEST_F(packingAndUnpackingTestdata, packFilename) {
    std::error_code err;
    // New directory to contain the results of these tests
    // make sure that it's already fresh and deleted
    ASSERT_NE(system(std::string{"rm -rf " + playground_dirname}.data()), -1); // suppress unused variable warning
    fs::create_directory(playground_dirname, err);
    fs::current_path(playground_dirname, err);

    // testing that filenames are properly managed
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
