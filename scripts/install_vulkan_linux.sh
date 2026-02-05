#!/usr/bin/env bash
set -euo pipefail

# Downloads and exports the Vulkan SDK for Linux CI builds.
base_url=${1:?"Missing base URL"}
version=${2:?"Missing Vulkan SDK version"}

vulkan_sdk_url="${base_url}${version}/linux/vulkansdk-linux-x86_64-${version}.tar.xz"
wget "$vulkan_sdk_url" -O vulkansdk-linux-x86_64-${version}.tar.xz

tar -xf vulkansdk-linux-x86_64-${version}.tar.xz -C vulkansdk-linux-x86_64-${version}

"./vulkansdk-linux-x86_64-${version}/vulkansdk"
