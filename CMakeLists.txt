cmake_minimum_required(VERSION 4.0.0)
project(packr-v2 VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(packr ./src/entry.cpp ./src/main.cpp ./src/utils.cpp ./src/fs_node.cpp)
target_include_directories(packr PUBLIC ./include)
add_compile_options(-Wall 
					-Wextra
					$<$<CONFIG:Debug>:-O0>
					$<$<CONFIG:Release>:-O2>
					$<$<CONFIG:Debug>:-g>
				)

include(FetchContent)
FetchContent_Declare(
	googletest
	  GIT_REPOSITORY https://github.com/google/googletest.git
	  GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(main_tests ./tests/main.cpp ./src/utils.cpp ./src/entry.cpp)
include(GoogleTest)
target_link_libraries(main_tests PRIVATE GTest::gtest_main)
target_include_directories(main_tests PRIVATE ./include)
gtest_discover_tests(main_tests)
