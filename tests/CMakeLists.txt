add_compile_options(-fsanitize=undefined,address)
include_directories(../include/)

add_executable(main_tests
	./unpacking.cpp
	./dir_entry.cpp
	./file_entry.cpp
	./side_tests.cpp
	./fs_node.cpp
	./dir_sym_entry.cpp

	../src/fs_node.cpp
	../src/utils.cpp
	../src/entry.cpp
)
add_executable(env_setup_tests
	./env_setup.cpp
	../src/fs_node.cpp
	../src/utils.cpp
	../src/entry.cpp
)
target_link_libraries(main_tests PRIVATE GTest::gtest_main)
target_link_options(main_tests PRIVATE -fsanitize=undefined,address)

target_link_libraries(env_setup_tests PRIVATE GTest::gtest_main)
target_link_options(env_setup_tests PRIVATE -fsanitize=undefined,address)

gtest_discover_tests(env_setup_tests PROPERTIES
	FIXTURES_SETUP GlobalSetup
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)
gtest_discover_tests(main_tests PROPERTIES
	FIXTURES_REQUIRED GlobalSetup
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)
