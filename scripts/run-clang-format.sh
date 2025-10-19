#!/bin/bash

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format is not installed. Please install it first."
    exit 1
fi

# Define your directories here
directories=(
    "sources"
	"headers"
	"tests"
	"examples"
)

# Loop through each directory
for dir in "${directories[@]}"; do
    if [ ! -d "$dir" ]; then
        echo "Warning: Directory '$dir' does not exist. Skipping."
        continue
    fi

    echo "Formatting .cpp and .hpp files in $dir..."

    # Use find to get all .cpp and .hpp files and run clang-format in-place
    find "$dir" -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i {} \;

    echo "Done formatting files in $dir."
done

echo "All specified directories processed."
