#include <gtest/gtest.h>
#include <packr/utils.hpp>
#include <filesystem>

namespace fs = std::filesystem;

class dirAndFileEntryConstructorData : public testing::Test {
  protected:
    dirAndFileEntryConstructorData()
        : cwd_path(fs::current_path()), cwd_str(cwd_path.string()), dir_name("dummy_dir1"),
          full_path(packr::join_to_path(dir_name, cwd_str)), joined(cwd_str + '/' + dir_name) {
    }

    fs::path cwd_path;
    std::string cwd_str;
    std::string dir_name;
    std::optional<std::string> full_path;
    std::string joined;
};

class packingAndUnpackingTestdata : public testing::Test {
  protected:
    // dummer error code to avoid exceptions
    inline static std::error_code err;
    // cwd
    const inline static fs::directory_entry build_dir{fs::current_path(err)};
    const inline static std::string playground_dirname{"playground"};
    const inline static std::string extension{".packr"};
    const inline static fs::directory_entry dummy_dir1{"dummy_dir1"};
    const inline static std::string dummy_dir1_name{"dummy_dir1"};
    const inline static std::string dum_dirname{"dum"};
    const inline static std::string packr{"../packr"};

  private:
};
