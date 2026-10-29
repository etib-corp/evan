#!/bin/bash
set -euo pipefail

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format is not installed. Please install it first."
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

echo "Formatting C++ files in $ROOT_DIR..."

find "$ROOT_DIR" -type f \( -name "*.cpp" -o -name "*.hpp" \) \
    -not -path "*/.git/*" \
    -not -path "*/build/*" \
    -not -path "*/cmake_build/*" \
    -not -path "*/third_party/*" \
    -print0 |
xargs -0 clang-format -i

echo "All C++ files have been formatted."
