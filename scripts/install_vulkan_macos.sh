#!/usr/bin/env bash
set -euo pipefail

# Downloads and exports the Vulkan SDK for macOS CI builds.
base_url=${1:?"Missing base URL"}
version=${2:?"Missing Vulkan SDK version"}

vulkan_sdk_url="${base_url}${version}/mac/vulkansdk-macos-${version}.zip"
curl -L "$vulkan_sdk_url" -o vulkansdk-macos-${version}.zip

unzip vulkansdk-macos-${version}.zip -d vulkansdk-macos-${version}

""./vulkansdk-macos-${version}/Contents/MacOS/vulkansdk-macOS-${version}" install
