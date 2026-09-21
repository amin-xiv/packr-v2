#include <packr/utils.hpp>

#include <gtest/gtest.h>

using namespace packr;
namespace fs = std::filesystem;

TEST(read_symlink, main) {
    fs::path dummy_dir1_path{"dummy_dir1"};
    fs::path dummy_dir1_canonical{fs::canonical(dummy_dir1_path)};

    // Relative path
    EXPECT_EQ(packr::read_symlink(dummy_dir1_path), dummy_dir1_canonical);

    // Absolute path
    EXPECT_EQ(packr::read_symlink(dummy_dir1_path), dummy_dir1_canonical);
};
