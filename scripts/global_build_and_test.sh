#!/bin/bash

source .env
cd "$PROJECT_DIR" || exit

GCC_DEBUG_DIR=".gcc-debug"
GCC_RELEASE_DIR=".gcc-release"
CLANG_DEBUG_DIR=".clang-debug"
CLANG_RELEASE_DIR=".clang-release"
OUTPUT_FILE="output"

#setup

# GCC-DEBUG
if ! [ -d "$GCC_DEBUG_DIR" ]; then
	mkdir "$GCC_DEBUG_DIR"

fi

cd "$GCC_DEBUG_DIR" || exit
bash "../scripts/tests-setup.sh"

{
	cmake -S .. -B . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BINARY_DIR="." && 
	cmake --build . -j8 && 
	ctest --output-on-failure -j8
} 2>&1 | tee $OUTPUT_FILE &
cd ..

# GCC-RELEASE
if ! [ -d "$GCC_RELEASE_DIR" ]; then
	mkdir "$GCC_RELEASE_DIR"

fi

cd "$GCC_RELEASE_DIR" || exit
bash "../scripts/tests-setup.sh"

{
	cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BINARY_DIR="." && 
	cmake --build . -j8 && 
	ctest --output-on-failure -j8
} 2>&1 | tee $OUTPUT_FILE &

cd ..

# CLANG-DEBUG
if ! [ -d "$CLANG_DEBUG_DIR" ]; then
	mkdir "$CLANG_DEBUG_DIR"
fi

cd "$CLANG_DEBUG_DIR" || exit
bash "../scripts/tests-setup.sh"

{
	cmake -S .. -B . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BINARY_DIR="." && 
	cmake --build . -j8 && 
	ctest --output-on-failure -j8
} 2>&1 | tee $OUTPUT_FILE &
cd ..


# CLANG-RELEASE
if ! [ -d "$CLANG_RELEASE_DIR" ]; then
	mkdir "$CLANG_RELEASE_DIR"

fi

cd "$CLANG_RELEASE_DIR" || exit
bash "../scripts/tests-setup.sh"

{
	cmake -S .. -B . -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BINARY_DIR="." && 
	cmake --build . -j8 && 
	ctest --output-on-failure -j8
} 2>&1 | tee $OUTPUT_FILE &
cd ..

