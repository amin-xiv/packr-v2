#pragma once

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
    const inline static fs::directory_entry build_dir{fs::current_path()};
    const inline static std::string playground_dirname{"playground"};
    const inline static std::string extension{".packr"};
    const fs::directory_entry dummy_dir1{fs::canonical("dummy_dir1")};
    const inline static std::string dummy_dir1_dirname{"dummy_dir1"};
    const fs::directory_entry cycle_test{fs::canonical("cycle_test")};
    const inline static std::string cycle_test_dirname{"cycle_test"};
    const inline static std::string dum_dirname{"dum"};
    const inline static std::string packr{"../packr"};
};
