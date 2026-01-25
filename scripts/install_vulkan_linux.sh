#!/usr/bin/env bash
set -euo pipefail

# Downloads and exports the Vulkan SDK for Linux CI builds.
base_url=${1:?"Missing base URL"}
version=${2:?"Missing Vulkan SDK version"}

vulkan_sdk_url="${base_url}${version}/linux/vulkansdk-linux-x86_64-${version}.tar.xz?Human=true"
wget "$vulkan_sdk_url" -O vulkan-sdk.tar.xz

tar -xf vulkan-sdk.tar.xz
sdk_root="${PWD}/${version}/x86_64"

export VULKAN_SDK="$sdk_root"
echo "export VULKAN_SDK=$VULKAN_SDK" >> "$GITHUB_ENV"
echo "export PATH=$VULKAN_SDK/bin:$PATH" >> "$GITHUB_ENV"
echo "export LD_LIBRARY_PATH=$VULKAN_SDK/lib:$LD_LIBRARY_PATH" >> "$GITHUB_ENV"
echo "export VK_ICD_FILENAMES=$VULKAN_SDK/etc/vulkan/icd.d/MoltenVK_icd.json" >> "$GITHUB_ENV"
echo "export VK_LAYER_PATH=$VULKAN_SDK/etc/vulkan/explicit_layer.d" >> "$GITHUB_ENV"
