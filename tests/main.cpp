#include "packr/types.hpp"
#include <ctime>
#include <packr/utils.hpp>
#include <packr/entry.hpp>
#include <system_error>
#include <gtest/gtest.h>
#include <string>
#include <optional>
#include <unistd.h>
#include <filesystem>

namespace fs = std::filesystem;

using namespace packr;

class dirAndFileEntryConstructorData : public testing::Test {
  protected:
    dirAndFileEntryConstructorData()
        : cwd_path(fs::current_path()), cwd_str(cwd_path.string()), dir_name("dummy_dir1"),
          full_path(join_to_path(dir_name, cwd_str)), joined(cwd_str + '/' + dir_name) {
    }

    fs::path cwd_path;
    std::string cwd_str;
    std::string dir_name;
    std::optional<std::string> full_path;
    std::string joined;
};

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
TEST_F(dirAndFileEntryConstructorData, DirectoryEntryConstructorData) {
    /*
     * First, try to use dir_name from joinToPath.normal in a fixture
     * Obtain the variable dir_name
     * Obtain the relevant directory information using std::filesystem
     * Intialize a dir_entry object
     * Compare the data, that they are equal, IN DETAIL
     */

    // dummy ec object to avoid exceptions
    std::error_code err;

    // fs directory intialization
    fs::directory_entry dir_fs{full_path.value()};
    ASSERT_TRUE(dir_fs.exists());

    // dir_entry initialization
    dir_entry dirEntry{opendir(joined.data()), joined, DEFAULT_ROOT_DIR};

    ASSERT_TRUE(dirEntry.success);
    EXPECT_STREQ(dir_fs.path().filename().c_str(), dirEntry.dirname);
    EXPECT_EQ(dir_fs.path().filename().string().size(), dirEntry.dirname_length);
    EXPECT_EQ(get_dir_size(dir_fs), dirEntry.size);
    // Let's deal with time later
    // EXPECT_EQ(dir_fs.last_write_time(err).time_since_epoch().count(), std::chrono::duration<u64>(dirEntry.mod_time).count());
    EXPECT_EQ(dirEntry.child_entry_count, 2);
    EXPECT_EQ(dirEntry.child_file_count, 1);
    EXPECT_EQ(dirEntry.child_dir_count, 1);
    EXPECT_EQ(dirEntry.total_entry_count, 7);
    EXPECT_EQ(dirEntry.total_file_count, 4);
    EXPECT_EQ(dirEntry.total_dir_count, 3);
    // Mode is also not now
    // EXPECT_EQ(std::to_underlying(fs::perms(dirEntry.mode)), std::to_underlying(dir_fs.status().permissions()));
    EXPECT_EQ(dirEntry.type, dir_type::regular);
}

TEST_F(dirAndFileEntryConstructorData, FileEntryConstructorData) {
    /*
     * First, try to use file_name from joinToPath.normal in a fixture
     * Obtain the variable file_name
     * Obtain the relevant file information using std::filesystem
     * Intialize a file_entry object
     * Compare the data, that they are equal, IN DETAIL
     */

    // dummy ec object to avoid exceptions
    std::error_code err;

    // fs directory intialization
    fs::directory_entry file_fs{full_path.value() + '/' + "hallo.txt"};
    ASSERT_TRUE(file_fs.exists());

    // dir_entry initialization
    file_entry fileEntry{file_fs.path().string(), DEFAULT_ROOT_DIR};

    ASSERT_TRUE(fileEntry.success);
    EXPECT_STREQ(file_fs.path().filename().c_str(), fileEntry.filename);
    EXPECT_EQ(file_fs.path().filename().string().size(), fileEntry.filename_length);
    EXPECT_EQ(file_fs.file_size(), fileEntry.size);

    // Let's deal with time later
    // EXPECT_EQ(dir_fs.last_write_time(err).time_since_epoch().count(), std::chrono::duration<u64>(dirEntry.mod_time).count());
    //
    // Mode is also not now
    // EXPECT_EQ(std::to_underlying(fs::perms(fileEntry.mode)), std::to_underlying(file_fs.status().permissions()));
    //
    EXPECT_EQ(fileEntry.type, file_type::regular);
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
