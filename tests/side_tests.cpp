#include <gtest/gtest.h>
#include <packr/utils.hpp>
#include "shared_test_data.hpp"

using namespace packr;

TEST_F(dirAndFileEntryConstructorData, joinToPathNormal) {
    // First test
    std::string filename1{"file.txt"};
    std::string directory{"/home/user/desktop/directory"};
    std::optional<std::string> full_path1{directory + '/' + filename1};
    std::optional<std::string> joined1{join_to_path(filename1, directory)};
    EXPECT_EQ(joined1, full_path1.value());

    // Second test
    std::string filename2{"somerandomfile"};
    std::string directory2{"/home/user/desktop/directory/"};
    std::optional<std::string> full_path2{join_to_path(filename2, directory2)};
    std::string joined2{directory2 + filename2};
    EXPECT_EQ(joined2, full_path2.value());

    // To make sure no extra '/' is added
    std::string filename3{"somerandomfile/"};
    std::string directory3{"/home/user/desktop/directory/"};
    std::optional<std::string> full_path3{join_to_path(filename3, directory3)};
    std::string joined3{directory3 + filename3};
    EXPECT_EQ(joined3, full_path3.value());

    fs::path cwd_path{fs::current_path()};
    std::string cwd_str{cwd_path.string()};

    // This time a real directory is used, this will be passed on to other tests
    std::string file_name{"hallo.txt"};
    std::optional<std::string> full_path4{join_to_path(file_name, cwd_str)};
    std::string joined4{cwd_str + '/' + file_name};
    EXPECT_EQ(joined4, full_path4.value());

    // This time a real directory is used, this will be passed on to other tests
    // This is from the text fixture passed in the first param
    EXPECT_EQ(joined, full_path.value());
}

TEST(joinToPath, nullInputs) {
    std::string filename;
    std::string path;
    std::optional<std::string> full_path{join_to_path(filename, path)};
    EXPECT_FALSE(full_path.has_value());
}

// "add_dirname" function tests
TEST(addDirname, withNamedAs) {
    dir_entry dir_ent{};
    std::string src_path{"/home/desktop/some_directory"};
    add_dirname(&dir_ent, "", src_path);
    std::string test_str{"some_directory"};
    EXPECT_EQ(test_str, std::string{dir_ent.dirname});
    EXPECT_EQ(test_str.size(), dir_ent.dirname_length);
}

TEST(addDirname, noNamedAs) {
    dir_entry dir_ent{};
    std::string src_path{"/home/desktop/some_directory"};
    std::string named_as{"bla bla bla"};
    add_dirname(&dir_ent, named_as, src_path);
    EXPECT_EQ(named_as, std::string{dir_ent.dirname});
    EXPECT_EQ(named_as.length(), dir_ent.dirname_length);
}

TEST(extractFilename, normal) {
    std::string path{"/bla/bla/home/desktop/bla/textfile.txt67"};
    std::string filename{"textfile.txt67"};
    std::optional<std::string> actual_filename{extract_filename(path)};
    EXPECT_EQ(filename, actual_filename.value());
}

TEST(extractFilename, nullInput) {
    std::string path{};
    std::optional<std::string> actual_filename{extract_filename(path)};
    EXPECT_FALSE(actual_filename.has_value());
}

TEST(specialMarkers, mainTest) {
    EXPECT_EQ(ENT_DIR_START, 0x01);
    EXPECT_EQ(ENT_DIR_END, 0x02);
    EXPECT_EQ(ENT_FILE, 0x04);
    EXPECT_EQ(PACK_START, 0x08);
    EXPECT_EQ(PACK_END, 0x10);
}

// Testing other macros and constant values
TEST(otherMacrosAndConstants, mainTests) {
    EXPECT_EQ(DEFAULT_ROOT_DIR, 0);
    EXPECT_EQ(P_NOMETADATA, 0B00000001);
}
