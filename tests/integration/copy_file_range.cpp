#include <packr/utils.hpp>

#include <gtest/gtest.h>
#include <filesystem>
#include <cstring>

using namespace packr;
namespace fs = std::filesystem;

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

TEST(copy_file_range, uninitialized) {
#ifdef NDEBUG // as asserts only run in debug mode
    GTEST_SKIP();
#endif

    File_R source{"some-random-nonexistent-file232323"};
    File_W dest{"some-random-nonexistent-file232324"};

    EXPECT_DEATH({ bool res{copy_file_range(source, 0, dest, 0, 0)}; }, ".*");
}
