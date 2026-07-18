cmake_minimum_required(VERSION 3.31.6)
project(packr-v2 VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/build")

add_executable(packr
	./src/entry.cpp
	./src/main.cpp
	./src/utils.cpp
	./src/fs_node.cpp
)
target_include_directories(packr PUBLIC ./include)
target_link_libraries(packr PRIVATE -fsanitize=undefined)
add_compile_options(-Wall
					-Wextra
					$<$<CONFIG:Release>:-O2>
					$<$<CONFIG:Debug>:-Og>
					$<$<CONFIG:Debug>:-g>
					$<$<CONFIG:DEBUG>:-fsanitize=undefined>
				)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-fhardened)
endif()

include(FetchContent)
FetchContent_Declare(
	googletest
			GIT_REPOSITORY https://github.com/google/googletest.git
			GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(main_tests
	./tests/main.cpp
	./src/utils.cpp
	./src/entry.cpp
	./src/fs_node.cpp
)
add_executable(side_tests ./tests/side_tests.cpp ./src/utils.cpp)
include(GoogleTest)
target_link_libraries(main_tests PRIVATE GTest::gtest_main -fsanitize=undefined)
target_link_libraries(side_tests PRIVATE GTest::gtest_main -fsanitize=undefined)
target_include_directories(main_tests PRIVATE ./include)
target_include_directories(side_tests PRIVATE ./include)
gtest_discover_tests(main_tests WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
gtest_discover_tests(side_tests WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
