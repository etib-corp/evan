param(
    [Parameter(Mandatory = $true)] [string] $BaseUrl,
    [Parameter(Mandatory = $true)] [string] $Version
)

$ErrorActionPreference = "Stop"

# Downloads and exports the Vulkan SDK for Windows CI builds.
$vulkanSdkUrl = "$BaseUrl$Version/windows/VulkanSDK-$Version-Installer.exe?Human=true"
Invoke-WebRequest -Uri $vulkanSdkUrl -OutFile "vulkan-sdk-setup.exe"
Start-Process -FilePath "vulkan-sdk-setup.exe" -ArgumentList "/S" -NoNewWindow -Wait

$env:VULKAN_SDK = "C:\VulkanSDK\$Version\"
[Environment]::SetEnvironmentVariable('VULKAN_SDK', $env:VULKAN_SDK, 'Machine')
$env:Path += ";$env:VULKAN_SDK\Bin;$env:VULKAN_SDK\Lib"

"export VULKAN_SDK=$($env:VULKAN_SDK)" >> $env:GITHUB_ENV
"export PATH=$($env:Path)" >> $env:GITHUB_ENV
