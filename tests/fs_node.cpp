#include "packr/utils.hpp"
#include <gtest/gtest.h>
#include <packr/fs_node.hpp>
#include <filesystem>

using namespace packr;
namespace fs = std::filesystem;

TEST(Directory, ConstructorRegular) {
    std::error_code err;

    fs::path dir_path{"../src"};
    ASSERT_TRUE(fs::exists(dir_path, err));

    Directory dir_obj{dir_path};
    ASSERT_TRUE(dir_obj);

    EXPECT_TRUE(fs::exists(dir_obj.entry_obj(), err));
    EXPECT_TRUE(fs::exists(dir_obj.path_obj(), err));
    EXPECT_EQ(dir_obj.type(), dir_type::regular);
    EXPECT_TRUE(dir_obj.secondary_path().empty());
}

TEST(Directory, ConstructorSymlink) {
    std::error_code err;

    fs::path dir_path{"dummy_dir1"};
    ASSERT_TRUE(fs::exists(dir_path, err));

    Directory dir_obj{dir_path};
    ASSERT_TRUE(dir_obj);

    EXPECT_TRUE(fs::exists(dir_obj.entry_obj(), err));
    EXPECT_TRUE(fs::exists(dir_obj.path_obj(), err));
    EXPECT_EQ(dir_obj.type(), dir_type::symlink);
    EXPECT_STREQ(dir_obj.secondary_path().c_str(), packr::read_symlink(dir_path).c_str());
}

TEST(File, ConstructorRegular) {
    std::error_code err;

    fs::path file_path{"dummy_dir1/hallo.txt"};
    ASSERT_TRUE(fs::exists(file_path, err));

    File file_obj{file_path};
    ASSERT_TRUE(file_obj);

    EXPECT_TRUE(fs::exists(file_obj.entry_obj(), err));
    EXPECT_TRUE(fs::exists(file_obj.path_obj(), err));
    EXPECT_EQ(file_obj.type(), file_type::regular);
    EXPECT_STREQ(file_obj.secondary_path().c_str(), packr::read_symlink(file_path).c_str());
}

TEST(File, ConstructorSymlink) {
    std::error_code err;

    fs::path file_path{"dummy_dir1/sym_file"};
    ASSERT_TRUE(fs::exists(file_path, err));

    File file_obj{file_path};
    ASSERT_TRUE(file_obj);

    EXPECT_TRUE(fs::exists(file_obj.entry_obj(), err));
    EXPECT_TRUE(fs::exists(file_obj.path_obj(), err));
    EXPECT_EQ(file_obj.type(), file_type::symlink);
    EXPECT_STREQ(file_obj.secondary_path().c_str(), packr::read_symlink(file_path).c_str());
}
