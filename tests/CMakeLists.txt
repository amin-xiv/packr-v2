add_compile_options(-fsanitize=undefined,address)
include_directories(../include/)

add_executable(main_tests
	./unpacking.cpp
	./dir_entry.cpp
	./file_entry.cpp
	./side_tests.cpp
	./fs_node.cpp

	../src/fs_node.cpp
	../src/utils.cpp
	../src/entry.cpp
)
add_executable(env_setup_tests ./env_setup.cpp)

target_link_libraries(main_tests PRIVATE
					GTest::gtest_main
					-fsanitize=undefined,address
)
target_link_libraries(env_setup_tests PRIVATE
					GTest::gtest_main
					-fsanitize=undefined,address
)

gtest_discover_tests(env_setup_tests WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	PROPERTIES FIXTURES_SETUP GlobalSetup
)
gtest_discover_tests(main_tests WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	PROPERTIES FIXTURES_REQUIRED GlobalSetup
)
