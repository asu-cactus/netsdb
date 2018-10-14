#!/usr/bin/env bash
echo "Formatting the source files"
find ././../../src/ -name "*.cc" | xargs clang-format -i -style=file
echo "Formatting the header files"
find ././../../src/ -name "*.h" | xargs clang-format -i -style=file