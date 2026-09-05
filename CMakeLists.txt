cmake_minimum_required(VERSION 3.31.6)
project(packr-v2)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/build")

add_executable(packr
	./src/entry.cpp
	./src/main.cpp
	./src/utils.cpp
	./src/fs_node.cpp
)
include_directories(./include)
target_compile_options(packr PRIVATE
					-Wall
					-Wextra
					-Werror
					-Wpedantic
					-Walloca
					-Wcast-align
					-Wcast-qual
					-Wconversion
					-Wsign-conversion
					-Wdeprecated-copy-dtor
					-Wdouble-promotion
					-Wextra-semi
					-Wformat=2
					-Wimplicit-fallthrough
					-Wmissing-braces
					-Wnon-virtual-dtor
					-Wnull-dereference
					-Wold-style-cast
					-Woverloaded-virtual
					-Wpointer-arith
					-Wrange-loop-construct
					-Wshadow
					-Wuninitialized
					-Wvla
					-Wwrite-strings
					$<$<CONFIG:Release>:-O3>
					$<$<CONFIG:Debug>:-O0>
					$<$<CONFIG:Debug>:-g>
					$<$<CONFIG:Debug>:-fsanitize=undefined,address>
)
target_link_options(packr PRIVATE
				-fsanitize=address,undefined
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(packr PRIVATE
						$<$<CONFIG:Release>:-fhardened>
						-Wno-error=hardened
						-Wduplicated-branches
						-Wduplicated-cond
						-Wformat-overflow=2
						-Wformat-signedness
						-Wlogical-op
						-Wrestrict
						-Wstrict-null-sentinel
						-Wsuggest-attribute=format
						-Wsuggest-attribute=malloc
)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(packr PRIVATE
						-Wcovered-switch-default
						-Wextra-semi-stmt
						-Wno-covered-switch-default
)
endif()

include(FetchContent)
FetchContent_Declare(
	googletest
			GIT_REPOSITORY https://github.com/google/googletest.git
			GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)
add_subdirectory(./tests/)
