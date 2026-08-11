add_executable(packing_tests
	./packing.cpp
	../src/utils.cpp
	../src/entry.cpp
	../src/fs_node.cpp
)

add_executable(unpacking_tests
	./unpacking.cpp
	../src/utils.cpp
	../src/entry.cpp
	../src/fs_node.cpp
)

add_executable(dir_entry
	./dir_entry.cpp
	../src/utils.cpp
	../src/entry.cpp
	../src/fs_node.cpp
)

add_executable(file_entry
	./file_entry.cpp
	../src/utils.cpp
	../src/entry.cpp
	../src/fs_node.cpp
)

add_executable(fs_node
	./fs_node.cpp
	../src/fs_node.cpp
	../src/utils.cpp
)

add_executable(side_tests
	./side_tests.cpp
	../src/utils.cpp
)

add_compile_options(-fsanitize=undefined,address)

target_link_libraries(packing_tests PRIVATE GTest::gtest_main -fsanitize=undefined,address)
target_link_libraries(unpacking_tests PRIVATE GTest::gtest_main -fsanitize=undefined,address)
target_link_libraries(dir_entry PRIVATE GTest::gtest_main -fsanitize=undefined,address)
target_link_libraries(file_entry PRIVATE GTest::gtest_main -fsanitize=undefined,address)
target_link_libraries(fs_node PRIVATE GTest::gtest_main -fsanitize=undefined,address)
target_link_libraries(side_tests PRIVATE GTest::gtest_main -fsanitize=undefined,address)

target_include_directories(packing_tests PRIVATE ../include)
target_include_directories(unpacking_tests PRIVATE ../include)
target_include_directories(dir_entry PRIVATE ../include)
target_include_directories(file_entry PRIVATE ../include)
target_include_directories(fs_node PRIVATE ../include)
target_include_directories(side_tests PRIVATE ../include)

gtest_discover_tests(packing_tests WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
gtest_discover_tests(dir_entry WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
gtest_discover_tests(file_entry WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
gtest_discover_tests(unpacking_tests WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
gtest_discover_tests(fs_node WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
gtest_discover_tests(side_tests WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
