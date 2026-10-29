#!/bin/bash
set -euo pipefail

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format is not installed. Please install it first."
    exit 1
fi

EXPECTED_CLANG_FORMAT_VERSION="21.1.8"
CLANG_FORMAT_VERSION="$(clang-format --version 2>/dev/null |
    grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -n 1 || true)"

# Formatting output changes between clang-format releases; warn when a local
# binary disagrees with the version CI pins.
if [ "$CLANG_FORMAT_VERSION" != "$EXPECTED_CLANG_FORMAT_VERSION" ]; then
    echo "Warning: found clang-format ${CLANG_FORMAT_VERSION:-unknown}," \
        "CI uses $EXPECTED_CLANG_FORMAT_VERSION. Output may differ."
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
