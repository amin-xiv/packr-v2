#include <packr/utils.hpp>

#include "../helpers/shared_test_data.hpp"

#include <gtest/gtest.h>
#include <filesystem>
#include <memory>
#include <cstring>
#include <stdexcept>

using namespace packr;

/* Here are tests that test small utilities and helper functions */

TEST_F(packingAndUnpackingFixture, joinToPathNormal) {
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

    std::string file_name{"hallo.txt"};
    std::optional<std::string> full_path4{join_to_path(file_name, cwd_str)};
    std::string joined4{cwd_str + '/' + file_name};
    EXPECT_EQ(joined4, full_path4.value());
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
    add_dirname(std::addressof(dir_ent), "", src_path);
    std::string test_str{"some_directory"};
    EXPECT_EQ(test_str, std::string{dir_ent.m_dirname});
    EXPECT_EQ(test_str.size(), dir_ent.m_dirname_length);
}

TEST(addDirname, noNamedAs) {
    dir_entry dir_ent{};
    std::string src_path{"/home/desktop/some_directory"};
    std::string named_as{"bla bla bla"};
    add_dirname(std::addressof(dir_ent), named_as, src_path);
    EXPECT_EQ(named_as, std::string{dir_ent.m_dirname});
    EXPECT_EQ(named_as.length(), dir_ent.m_dirname_length);
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

TEST(curateSrcPath, main) {
    // First with an aboslute path
    std::string absolute_path{"/home/some_random_dir"};
    std::string absolute_path_copy{absolute_path};
    EXPECT_TRUE(curate_src_path(absolute_path));
    EXPECT_EQ(absolute_path, absolute_path_copy);

    // Now with a relative path
    std::string relative_path{"directory/file"};
    std::string correct_path{fs::current_path() / relative_path};
    EXPECT_TRUE(curate_src_path(relative_path));
    EXPECT_EQ(relative_path, correct_path);
}

TEST(createPackFilename, main) {
    std::string dir_name{"name"};
    dir_entry dir{};
    memcpy(dir.m_dirname, dir_name.c_str(), dir_name.length());
    dir.m_dirname_length = dir_name.length();
    std::string res{create_pack_filename(dir)};
    EXPECT_EQ(res, dir_name + ".packr");
}

TEST(read_symlink, main) {
    fs::path dummy_dir1_path{"dummy_dir1"};
    fs::path dummy_dir1_canonical{fs::canonical(dummy_dir1_path)};

    // Relative path
    EXPECT_EQ(packr::read_symlink(dummy_dir1_path), dummy_dir1_canonical);

    // Absolute path
    EXPECT_EQ(packr::read_symlink(dummy_dir1_path), dummy_dir1_canonical);
};

TEST(observe_ptrDeathTest, main) {
    int* int_heap{new int(1)};
    char* char_heap{new char('a')};

    ASSERT_TRUE(int_heap);
    ASSERT_TRUE(char_heap);

    observe_ptr int_obj{int_heap};
    observe_ptr<char> char_obj{*char_heap};

    ASSERT_TRUE(int_obj.valid());
    ASSERT_TRUE(char_obj.valid());

    EXPECT_EQ(int_obj.get(), int_heap);
    EXPECT_EQ(char_obj.get(), char_heap);

    EXPECT_EQ(*int_obj, *int_heap);
    EXPECT_EQ(*char_obj, *char_heap);

    int int_stack{2};
    char char_stack{'b'};

    int_obj.reassign(std::addressof(int_stack));
    char_obj.reassign(std::addressof(char_stack));

    EXPECT_EQ(int_obj.get(), std::addressof(int_stack));
    EXPECT_EQ(char_obj.get(), std::addressof(char_stack));

    EXPECT_EQ(*int_obj, int_stack);
    EXPECT_EQ(*char_obj, char_stack);

    EXPECT_TRUE(int_obj);
    EXPECT_TRUE(char_obj);

    int_obj.reset();
    char_obj.reset();

    EXPECT_FALSE(int_obj);
    EXPECT_FALSE(char_obj);

    int_obj.reassign(std::addressof(int_stack));
    char_obj.reassign(std::addressof(char_stack));

    EXPECT_EQ(int_obj.get(), std::addressof(int_stack));
    EXPECT_EQ(char_obj.get(), std::addressof(char_stack));

    std::string test_str{"123456789"};
    observe_ptr str_obj{test_str};
    str_obj->clear();

    EXPECT_TRUE(test_str.empty());

    test_str = "123456789";
    observe_ptr str_obj_copied{str_obj};
    ASSERT_TRUE(str_obj_copied);
    EXPECT_STREQ(str_obj_copied.get()->data(), "123456789");
    EXPECT_EQ(str_obj->size(), str_obj->size());

    str_obj.reset();
    str_obj_copied.reset();

    EXPECT_FALSE(str_obj.valid());
    EXPECT_FALSE(str_obj_copied.valid());

    delete int_heap;
    delete char_heap;
}

TEST(observe_ptrDeathTest, must_throw) {
#ifndef NDEBUG // must run only in release as there are asserts before the throw statement
    GTEST_SKIP();
#endif

    int* null{nullptr};
    EXPECT_THROW(observe_ptr ptr_obj{null}, std::invalid_argument);

    int null_derference_int{1};
    observe_ptr ptr_obj2{null_derference_int};
    ptr_obj2.reset();
    EXPECT_THROW({ int dummy = *ptr_obj2; }, std::runtime_error);

    int null_reassign{1};
    observe_ptr ptr_obj3{null_reassign};
    ptr_obj3.reset();
    EXPECT_THROW(ptr_obj3.reassign(nullptr), std::invalid_argument);
}

TEST(observe_ptrDeathTest, must_die) {
#ifdef NDEBUG // must run only in debug as asserts are stripped out in release mode
    GTEST_SKIP();
#endif

    int* null{nullptr};
    EXPECT_DEATH({ observe_ptr ptr_obj{null}; }, ".*");

    int null_derference_int{1};
    observe_ptr ptr_obj2{null_derference_int};
    ptr_obj2.reset();
    EXPECT_DEATH({ int dummy = *ptr_obj2; }, ".*");

    int null_reassign{1};
    observe_ptr ptr_obj3{null_reassign};
    ptr_obj3.reset();
    EXPECT_DEATH(ptr_obj3.reassign(nullptr), ".*");
}
