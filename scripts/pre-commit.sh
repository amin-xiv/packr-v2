#!/bin/bash

source .env

for entry in *; do
	if [[ "$entry" == ".git" || "$entry" == ".github" || "$entry" == build* || "$entry" == .gcc* || "$entry" == .clang* ]]; then
		continue
	fi


	if [[ $entry == *.cpp || $entry == *.hpp ]]; then
		clang-format -i --style=file:"$CLANG_FORMAT_FILE" "$entry"
		ontinue
	fi

	if [[ -d $entry ]]; then
		find "$entry" \( -name '*.cpp' -o -name '*.hpp' \) | while read -r file; do
			clang-format -i --style=file:"$CLANG_FORMAT_FILE" "$file"
		done
	fi

done

echo "done formatting"

run-clang-tidy -p "$BINARY_DIR" \
               -config-file="$CLANG_TIDY_FILE" \
               -extra-arg="-Wno-unknown-warning-option" \
               -header-filter=".*packr/.*" \
			   -source-filter=".*/packr-v2/(src|tests)/.*" \
               -j"$(nproc)" \
               -quiet
echo "done linting"
