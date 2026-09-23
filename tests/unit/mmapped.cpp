#include <packr/misc_structs.hpp>

#include <gtest/gtest.h>
#include <memory>
#include <utility>
#include <cstring>

using namespace packr;

constexpr int PLAIN_SIZE{100};
constexpr int KiB{1024};

class mmaped_fixture : public ::testing::Test {
  public:
    mmaped_fixture() {
        data_buf_ptr = std::make_unique<char[]>(test_str.size() + 1);
        assert(data_buf_ptr);
        std::memcpy(data_buf_ptr.get(), test_str.data(), test_str.size() + 1);
    }

  protected:
    std::unique_ptr<char[]> data_buf_ptr; // initialized at constructor, then moved to data_intialized
    std::string test_str{"123456789"};
    mmapped plain{PLAIN_SIZE};
};

using mmaped_DeathTest = mmaped_fixture;

TEST_F(mmaped_fixture, main) {
    // default constructor
    mmapped default_ctr{};
    ASSERT_FALSE(default_ctr.valid());
    EXPECT_TRUE(default_ctr.get() == nullptr || default_ctr.get() == NULL); // NOLINT(modernize-use-nullptr)
    EXPECT_EQ(default_ctr.size(), 0);
    EXPECT_EQ(default_ctr.status(), general_status::base);

    // construct by size
    ASSERT_TRUE(plain.valid());
    EXPECT_NE(plain.get(), nullptr);
    EXPECT_EQ(plain.size(), PLAIN_SIZE);
    EXPECT_EQ(plain.status(), general_status::success);

    // construct by buf and size
    mmapped data_initialized1{std::move(data_buf_ptr), test_str.size() + 1};
    ASSERT_TRUE(data_initialized1.valid());
    EXPECT_NE(data_initialized1.get(), nullptr);
    EXPECT_EQ(data_initialized1.size(), test_str.size() + 1);
    EXPECT_EQ(data_initialized1.status(), general_status::success);
    EXPECT_STREQ(test_str.data(), data_initialized1.get());

    // construct by raw pointer
    mmapped data_initialized2{test_str.data(), test_str.size() + 1};
    ASSERT_TRUE(data_initialized2.valid());
    EXPECT_NE(data_initialized2.get(), nullptr);
    EXPECT_EQ(data_initialized2.size(), test_str.size() + 1);
    EXPECT_EQ(data_initialized2.status(), general_status::success);
    EXPECT_STREQ(test_str.data(), data_initialized2.get());

    // move constructor
    mmapped moved1{std::move(data_initialized1)};
    ASSERT_TRUE(moved1.valid());
    EXPECT_NE(moved1.get(), nullptr);
    EXPECT_EQ(moved1.size(), test_str.size() + 1);
    EXPECT_EQ(moved1.status(), general_status::success);
    EXPECT_STREQ(test_str.data(), moved1.get());
    // make sure other object is invalidated/reset
    ASSERT_FALSE(data_initialized1.valid());                     // NOLINT(bugprone-use-after-move)
    EXPECT_EQ(data_initialized1.get(), nullptr);                 // NOLINT(bugprone-use-after-move)
    EXPECT_EQ(data_initialized1.size(), 0);                      // NOLINT(bugprone-use-after-move)
    EXPECT_EQ(data_initialized1.status(), general_status::base); // NOLINT(bugprone-use-after-move)

    // operator=
    mmapped moved2{};
    moved2 = std::move(moved1);
    ASSERT_TRUE(moved2.valid());
    EXPECT_NE(moved2.get(), nullptr);
    EXPECT_EQ(moved2.size(), test_str.size() + 1);
    EXPECT_EQ(moved2.status(), general_status::success);
    EXPECT_STREQ(test_str.data(), moved2.get());
    // make sure other object is invalidated/reset
    ASSERT_FALSE(moved1.valid());                     // NOLINT(bugprone-use-after-move)
    EXPECT_EQ(moved1.get(), nullptr);                 // NOLINT(bugprone-use-after-move)
    EXPECT_EQ(moved1.size(), 0);                      // NOLINT(bugprone-use-after-move)
    EXPECT_EQ(moved1.status(), general_status::base); // NOLINT(bugprone-use-after-move)

    // unmapping
    moved2.unmap();
    ASSERT_FALSE(moved2.valid());
    EXPECT_EQ(moved2.get(), nullptr);
    EXPECT_EQ(moved2.size(), 0);
    EXPECT_EQ(moved2.status(), general_status::base);
}

TEST_F(mmaped_fixture, io) {
    plain.write(observe_ptr{test_str.data()}, test_str.size() + 1);
    char buf1[KiB]{};
    plain.read(observe_ptr{buf1}, test_str.size() + 1);

    EXPECT_STREQ(test_str.data(), buf1);

    mmapped value_intialized{std::move(data_buf_ptr), test_str.size() + 1};

    EXPECT_STREQ(test_str.data(), value_intialized.get());
    value_intialized.clear();
    ASSERT_EQ(std::strlen(value_intialized.get()), 0);
    char buf2[KiB]{"blablabla"};
    value_intialized.write(observe_ptr{buf2}, std::strlen(buf2));
    EXPECT_STREQ(buf2, value_intialized.get());
}

TEST(mmaped_DeathTest, constructors) {
#ifdef NDEBUG
    GTEST_SKIP();
#endif
    // must die due to size = 0
    EXPECT_DEATH(mmapped{0}, ".*");

    // must die due to dummy_ptr not owning any memory
    std::unique_ptr<char[]> dummy_ptr1{};
    EXPECT_DEATH(mmapped(std::move(dummy_ptr1), 1), ".*");

    // must die due to size = 0
    std::unique_ptr<char[]> dummy_ptr2{std::make_unique<char[]>(1)};
    EXPECT_DEATH(mmapped(std::move(dummy_ptr1), 0), ".*");

    // must die due to ptr = nullptr
    char* ptr{nullptr};
    EXPECT_DEATH(mmapped(ptr, 1), ".*");
}
