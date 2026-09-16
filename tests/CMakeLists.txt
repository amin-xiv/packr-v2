add_compile_options(-fsanitize=undefined,address)
include_directories(../include/)

add_executable(unit_tests
	# tests source files
	./unit/side_tests.cpp
	./unit/constants.cpp
	./unit/inc_ent_count.cpp

	# source files
	../src/fs_node.cpp
	../src/utils.cpp
	../src/entry.cpp
)

add_executable(integration_tests
	# tests source files
	./integration/unpacking.cpp
	./integration/dir_entry.cpp
	./integration/file_entry.cpp
	./integration/fs_node.cpp
	./integration/dir_sym_entry.cpp
	./integration/copy_file_range.cpp

	# source files
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
target_link_libraries(unit_tests PRIVATE GTest::gtest_main)
target_link_options(unit_tests PRIVATE -fsanitize=undefined,address)

target_link_libraries(integration_tests PRIVATE GTest::gtest_main)
target_link_options(integration_tests PRIVATE -fsanitize=undefined,address)

target_link_libraries(env_setup_tests PRIVATE GTest::gtest_main)
target_link_options(env_setup_tests PRIVATE -fsanitize=undefined,address)

gtest_discover_tests(env_setup_tests PROPERTIES
	FIXTURES_SETUP GlobalSetup
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

gtest_discover_tests(unit_tests PROPERTIES
	FIXTURES_REQUIRED GlobalSetup
	FIXTURES_SETUP UnitTests
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

gtest_discover_tests(integration_tests PROPERTIES
	FIXTURES_REQUIRED UnitTests
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)
