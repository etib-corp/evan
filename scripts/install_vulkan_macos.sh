#!/usr/bin/env bash
set -euo pipefail

# Downloads and exports the Vulkan SDK for macOS CI builds.
base_url=${1:?"Missing base URL"}
version=${2:?"Missing Vulkan SDK version"}

mac/vulkansdk-macos-1.4.335.1.zip

vulkan_sdk_url="${base_url}${version}/mac/vulkansdk-macos-${version}.zip"
curl -L "$vulkan_sdk_url" -o vulkansdk-macos-${version}.zip
