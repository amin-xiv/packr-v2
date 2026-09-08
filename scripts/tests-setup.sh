#!/bin/bash

if ! [ -h "dummy_dir1" ]; then
ln -s "$(pwd)/../tests/test-data/dummy_dir1/" dummy_dir1
fi
