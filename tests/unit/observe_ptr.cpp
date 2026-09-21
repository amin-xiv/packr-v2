#include <packr/types.hpp>

#include <gtest/gtest.h>

using namespace packr;

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

    char arr[]{'a', 'b', 'c'};
    observe_ptr arr_ptr_obj{arr};

    EXPECT_EQ(arr[0], arr_ptr_obj[0]);
    EXPECT_EQ(arr[1], arr_ptr_obj[1]);
    EXPECT_EQ(arr[2], arr_ptr_obj[2]);
    arr_ptr_obj[0] = 'z';
    EXPECT_EQ(arr_ptr_obj[0], 'z');
}

TEST(observe_ptrDeathTest, must_throw) {
#ifndef NDEBUG // must run only in release as there are asserts before the throw statement
    GTEST_SKIP();
#endif

    int* null{nullptr};
    EXPECT_THROW(observe_ptr{null}, std::invalid_argument);

    int null_derference_int{1};
    observe_ptr ptr_obj1{null_derference_int};
    ptr_obj1.reset();
    EXPECT_THROW({ int dummy = *ptr_obj1; }, std::runtime_error);

    int null_reassign{1};
    observe_ptr ptr_obj2{null_reassign};
    ptr_obj2.reset();
    EXPECT_THROW(ptr_obj2.reassign(nullptr), std::invalid_argument);
}

TEST(observe_ptrDeathTest, must_die) {
#ifdef NDEBUG // must run only in debug as asserts are stripped out in release mode
    GTEST_SKIP();
#endif

    int* null{nullptr};
    EXPECT_DEATH({ observe_ptr{null}; }, ".*");

    int null_derference_int{1};
    observe_ptr ptr_obj1{null_derference_int};
    ptr_obj1.reset();
    EXPECT_DEATH({ int dummy = *ptr_obj1; }, ".*");

    int null_reassign{1};
    observe_ptr ptr_obj2{null_reassign};
    ptr_obj2.reset();
    EXPECT_DEATH(ptr_obj2.reassign(nullptr), ".*");

    int out_of_bounds_index{1};
    observe_ptr ptr_obj3{out_of_bounds_index};
    ptr_obj3.reset();
    EXPECT_DEATH({ int test_int{ptr_obj3[5]}; }, ".*");
}
