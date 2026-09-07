#include <packr/utils.hpp>

#include "shared_test_data.hpp"

#include <gtest/gtest.h>
#include <filesystem>
#include <memory>
#include <cstring>

using namespace packr;

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
    EXPECT_EQ(O_SYM, 0B00000001);
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

TEST(copy_file_range, main) {
    std::error_code err;
    File_R source{"dummy_dir1/hallo.txt"}; // must exist in build directory
    fs::directory_entry source_ent{source.path_obj()};
    File_W dest{"copy_file_range_test_file"}; // will be created in this test body
    fs::directory_entry dest_ent{dest.path_obj()};

    /* setup */
    ASSERT_TRUE(source) << source.err();
    ASSERT_TRUE(source.setup_stream()) << source.err();
    ASSERT_TRUE(fs::exists(source.path_obj(), err)) << err.message();
    auto source_size{source_ent.file_size(err)};
    ASSERT_GT(source_size, 0) << err.message();

    ASSERT_TRUE(dest.setup_stream(open_type::fresh)) << dest.err();
    ASSERT_TRUE(fs::exists(source.path_obj(), err)) << err.message();
    ASSERT_EQ(dest_ent.file_size(err), 0) << err.message();

    /* tests */
    ASSERT_TRUE(copy_file_range(source, 0, dest, 0, source_size));
    EXPECT_EQ(dest_ent.file_size(err), source_size);

    // make sure offsets are updated
    EXPECT_EQ(dest.get_offset(), source_size);
    EXPECT_EQ(source.get_offset(), source_size);

    // compare file contents
    std::unique_ptr<char[]> source_contents{std::make_unique<char[]>(source_size + 1)};
    source.set_offset(0, std::ios_base::beg); // reset offset
    ASSERT_TRUE(source.read(source_contents.get(), source_size));

    std::unique_ptr<char[]> new_dest_contents{std::make_unique<char[]>(source_size + 1)};
    File_R dest_r{"copy_file_range_test_file"}; // to be able to read from it
    ASSERT_TRUE(dest_r) << dest_r.err();
    ASSERT_TRUE(dest_r.setup_stream()) << dest_r.err();
    ASSERT_TRUE(dest_r.read(new_dest_contents.get(), source_size));

    // compare contents
    std::string err_msg{std::format("source: {}, dest: {}", source_contents.get(), new_dest_contents.get())};
    EXPECT_EQ(std::memcmp(source_contents.get(), new_dest_contents.get(), source_size), 0) << err_msg;
}
