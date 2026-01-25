#!/usr/bin/env bash
set -euo pipefail

# Downloads and exports the Vulkan SDK for macOS CI builds.
base_url=${1:?"Missing base URL"}
version=${2:?"Missing Vulkan SDK version"}

vulkan_sdk_url="${base_url}${version}/mac/vulkan-sdk-${version}-mac.dmg?Human=true"
curl -L "$vulkan_sdk_url" -o vulkan-sdk.dmg

hdiutil attach vulkan-sdk.dmg
sudo installer -pkg "/Volumes/vulkan-sdk-${version}-mac/vulkan-sdk-${version}-mac.pkg" -target /

export VULKAN_SDK="/usr/local/vulkan-sdk-${version}/macos"
echo "export VULKAN_SDK=$VULKAN_SDK" >> "$GITHUB_ENV"
echo "export PATH=$VULKAN_SDK/bin:$PATH" >> "$GITHUB_ENV"
echo "export DYLD_LIBRARY_PATH=$VULKAN_SDK/lib:$DYLD_LIBRARY_PATH" >> "$GITHUB_ENV"
echo "export VK_ICD_FILENAMES=$VULKAN_SDK/etc/vulkan/icd.d/MoltenVK_icd.json" >> "$GITHUB_ENV"
echo "export VK_LAYER_PATH=$VULKAN_SDK/etc/vulkan/explicit_layer.d" >> "$GITHUB_ENV"

hdiutil detach "/Volumes/vulkan-sdk-${version}-mac" || true
