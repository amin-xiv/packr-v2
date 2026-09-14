#include <packr/utils.hpp>

#include <format>
#include <gtest/gtest.h>
#include <packr/fs_node.hpp>
#include <filesystem>
#include <array>
#include <unistd.h>

using namespace packr;
namespace fs = std::filesystem;

TEST(Directory, ConstructorRegular) {
    std::error_code err;

    fs::path dir_path{"../src"};
    ASSERT_TRUE(fs::exists(dir_path, err));

    Directory dir_obj{dir_path};
    ASSERT_TRUE(dir_obj) << dir_obj.err();

    EXPECT_TRUE(fs::exists(dir_obj.entry_obj(), err));
    EXPECT_TRUE(fs::exists(dir_obj.path_obj(), err));
    EXPECT_EQ(dir_obj.type(), dir_type::regular);
    EXPECT_TRUE(dir_obj.secondary_path().empty());
    EXPECT_TRUE(dir_obj.err().empty()) << dir_obj.err();
}

TEST(Directory, ConstructorSymlink) {
    std::error_code err;

    fs::path dir_path{"dummy_dir1"}; // this must exist at the build directory
    ASSERT_TRUE(fs::exists(dir_path, err));

    Directory dir_obj{dir_path};
    ASSERT_TRUE(dir_obj) << dir_obj.err();

    EXPECT_TRUE(fs::exists(dir_obj.entry_obj(), err));
    EXPECT_TRUE(fs::exists(dir_obj.path_obj(), err));
    EXPECT_EQ(dir_obj.type(), dir_type::symlink);
    EXPECT_STREQ(dir_obj.secondary_path().c_str(), packr::read_symlink(dir_path).c_str());
    EXPECT_TRUE(dir_obj.err().empty()) << dir_obj.err();
}

TEST(File, ConstructorRegular) {
    std::error_code err;

    fs::path file_path{"dummy_dir1/hallo.txt"}; // this must exist at the build directory
    ASSERT_TRUE(fs::exists(file_path, err));

    File file_obj{file_path};
    ASSERT_TRUE(file_obj) << file_obj.err();

    EXPECT_TRUE(fs::exists(file_obj.entry_obj(), err));
    EXPECT_TRUE(fs::exists(file_obj.path_obj(), err));
    EXPECT_EQ(file_obj.type(), file_type::regular);
    EXPECT_STREQ(file_obj.path_obj().c_str(), fs::absolute(file_path).c_str());
    EXPECT_TRUE(file_obj.err().empty()) << file_obj.err();
}

TEST(File, ConstructorSymlink) {
    std::error_code err;

    fs::path file_path{"dummy_dir1/sym_file"}; // this must exist at the build directory
    ASSERT_TRUE(fs::exists(file_path, err));

    File file_obj{file_path};
    ASSERT_TRUE(file_obj) << file_obj.err();

    EXPECT_TRUE(fs::exists(file_obj.entry_obj(), err));
    EXPECT_TRUE(fs::exists(file_obj.path_obj(), err));
    EXPECT_EQ(file_obj.type(), file_type::symlink);
    EXPECT_STREQ(file_obj.secondary_path().c_str(), fs::read_symlink(file_path).c_str());
    EXPECT_TRUE(file_obj.err().empty()) << file_obj.err();
}

TEST(File_W, exists) {
    std::error_code err;
    fs::path file_path{"test_txt_exists"};
    constexpr int test_offset{67};
    constexpr std::array<char, 43> buf{"hi i'm writing into a text file i guess..."};

    // create file in which we're going to do the tests
    ASSERT_EQ(system(std::format("touch {}", file_path.string()).c_str()), 0);
    ASSERT_TRUE(fs::exists(file_path, err)) << err.message();

    File_W file_obj{file_path};

    // No need to test constructors as they're the same as "File"'s constructors

    ASSERT_TRUE(file_obj.setup_stream(open_type::exists)) << file_obj.err();
    EXPECT_GT(file_obj.get_fd(), 2);     // as the num 2 is the highest default open descriptor(stderr)
                                         //
    EXPECT_EQ(file_obj.get_offset(), 0); // as we didn't write anything yet
    file_obj.set_offset(test_offset);
    EXPECT_EQ(file_obj.get_offset(), test_offset);

    EXPECT_TRUE(file_obj.write(buf.data(), buf.size()));
}

TEST(File_W, fresh) {
    std::error_code err;
    fs::path file_path{"test_txt_fresh"};
    constexpr int test_offset{67};
    constexpr std::array<char, 43> buf{"hi i'm writing into a text file i guess..."};

    File_W file_obj{file_path};

    // No need to test constructors as they're the same as "File"'s constructors

    ASSERT_TRUE(file_obj.setup_stream(open_type::fresh)) << file_obj.err(); // setup_stream must create the file
    ASSERT_TRUE(fs::exists(file_path));
    EXPECT_GT(file_obj.get_fd(), 2);     // as the num 2 is the highest default open descriptor(stderr)
                                         //
    EXPECT_EQ(file_obj.get_offset(), 0); // as we didn't write anything yet
    file_obj.set_offset(test_offset);
    EXPECT_EQ(file_obj.get_offset(), test_offset);

    EXPECT_TRUE(file_obj.write(buf.data(), buf.size()));
}

TEST(File_R, main) {
    std::error_code err;
    fs::path file_path{"dummy_dir1/hallo.txt"}; // this must exist at the build directory
    constexpr int test_offset{67};
    constexpr int one_kib{1024}; // to avoid 'magic number' warnings
    std::array<char, one_kib> buf{'\0'};

    ASSERT_TRUE(fs::exists(file_path, err)) << err.message();

    File_R file_obj{file_path};

    // No need to test constructors as they're the same as "File"'s constructors

    ASSERT_TRUE(file_obj.setup_stream()) << file_obj.err();
    EXPECT_GT(file_obj.get_fd(), STDERR_FILENO); // as the num 2(stderr) is the highest default open descriptor(stderr)

    EXPECT_EQ(file_obj.get_offset(), 0); // as we didn't read anything yet
    file_obj.set_offset(test_offset);
    EXPECT_EQ(file_obj.get_offset(), test_offset);
    file_obj.set_offset(0, std::ios_base::beg); // reset offset
    EXPECT_EQ(file_obj.get_offset(), 0);

    EXPECT_TRUE(file_obj.read(buf.data(), one_kib));
    EXPECT_GT(std::size(buf), 0) << buf.data(); // check that actual data has been read
}

TEST(File_sym, target_regular_file) {
    std::error_code err;
    fs::path file_path{"dummy_dir1/sym_file"}; // this must exist at the build directory

    ASSERT_TRUE(fs::exists(file_path, err)) << err.message();

    File_sym file_obj{file_path};
    ASSERT_TRUE(file_obj) << file_obj.err();

    EXPECT_TRUE(file_obj.entry_obj().exists(err)) << err.message();
    EXPECT_EQ(file_obj.entry_obj().path(), file_obj.path_obj());
    EXPECT_EQ(file_obj.path_obj().filename(), file_path.filename());
    EXPECT_TRUE(fs::exists(file_obj.target_path()));
    EXPECT_TRUE(file_obj.has_target());
    EXPECT_EQ(file_obj.target_type(), entry_type::regular_file);
}

TEST(File_sym, target_directory) {
    std::error_code err;
    // this is a symlink in dummy_dir1 that refers to a directory
    fs::path file_path{"dummy_dir1/parent_loop"}; // this must exist at the build directory

    ASSERT_TRUE(fs::exists(file_path, err)) << err.message();

    File_sym file_obj{file_path};
    ASSERT_TRUE(file_obj) << file_obj.err();

    EXPECT_TRUE(file_obj.entry_obj().exists(err)) << err.message();
    EXPECT_EQ(fs::absolute(file_obj.entry_obj().path()), fs::absolute(file_obj.path_obj()));
    EXPECT_EQ(file_obj.path_obj().filename(), file_path.filename());
    EXPECT_TRUE(fs::exists(file_obj.target_path()));
    EXPECT_TRUE(file_obj.has_target());
    EXPECT_EQ(file_obj.target_type(), entry_type::directory);
}

TEST(File_sym, broken_symlink) {
    std::error_code err;
    fs::path file_path{"dummy_dir1/broken_symlink_file"}; // this must exist at the build directory
    fs::directory_entry file_ent{file_path, err};

    ASSERT_TRUE(fs::exists(file_ent.symlink_status(err))) << err.message();

    File_sym file_obj{file_path};
    ASSERT_TRUE(file_obj) << file_obj.err();

    EXPECT_TRUE(fs::exists(file_obj.entry_obj().symlink_status())) << err.message();
    EXPECT_EQ(fs::absolute(file_obj.entry_obj().path()), fs::absolute(file_obj.path_obj()));
    EXPECT_EQ(file_obj.path_obj().filename(), file_path.filename());
    EXPECT_FALSE(fs::exists(file_obj.target_path()));
    EXPECT_FALSE(file_obj.has_target());
}

TEST(File_sym, nonexistent) {
#ifndef NDEBUG
    GTEST_SKIP(); // skip this test as File_sym ctr will terminate(due to the assert) as file doesn't exist
#endif

    fs::path file_path{"some-random-nonexistent-file"};

    File_sym file_obj{file_path};
    EXPECT_FALSE(file_obj);
    EXPECT_FALSE(file_obj.err().empty());
}
