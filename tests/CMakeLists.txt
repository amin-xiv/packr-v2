add_executable(main_tests
	./packing.cpp
	./unpacking.cpp
	./dir_entry.cpp
	./file_entry.cpp
	./side_tests.cpp
	./fs_node.cpp

	../src/fs_node.cpp
	../src/utils.cpp
	../src/entry.cpp
)

add_compile_options(-fsanitize=undefined,address)

target_link_libraries(main_tests PRIVATE
					GTest::gtest_main
					-fsanitize=undefined,address
)

target_include_directories(main_tests PRIVATE ../include)

gtest_discover_tests(main_tests WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
