#include <gtest/gtest.h>
#include <packr/utils.hpp>
#include <filesystem>

namespace fs = std::filesystem;

class packingAndUnpackingFixture : public testing::Test {
  public:
    void SetUp() override {
        // make sure we're on a build directory
        ASSERT_TRUE(fs::exists("../CMakeLists.txt"));
        ASSERT_TRUE(fs::exists("../src"));
    }

  protected:
    // dummer error code to avoid exceptions
    inline static std::error_code err;
    // cwd
    const inline static fs::directory_entry build_dir{fs::current_path(err)};
    const inline static std::string playground_dirname{"playground"};
    const inline static std::string extension{".packr"};
    const fs::directory_entry dummy_dir1{fs::canonical("dummy_dir1")};
    const inline static std::string dummy_dir1_name{"dummy_dir1"};
    const inline static std::string dum_dirname{"dum"};
    const inline static std::string packr{"../packr"};
};
