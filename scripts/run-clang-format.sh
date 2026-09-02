#!/bin/bash

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format is not installed. Please install it first."
    exit 1
fi

# Get the root directory (where this script is located or repository root)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

echo "Formatting all C++ files in $ROOT_DIR..."

# Find and format all .cpp and .hpp files recursively from the root directory
find "$ROOT_DIR" -type f \( -name "*.cpp" -o -name "*.hpp" \) \
    -not -path "*/\.*" \
    -not -path "*/build/*" \
    -not -path "*/cmake_build/*" \
    -not -path "*/.git/*" \
    -exec clang-format -i {} \;

echo "All C++ files have been formatted."
