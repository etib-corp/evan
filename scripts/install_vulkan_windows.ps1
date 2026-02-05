param(
    [Parameter(Mandatory = $true)] [string] $BaseUrl,
    [Parameter(Mandatory = $true)] [string] $Version
)

$ErrorActionPreference = "Stop"

# Downloads and exports the Vulkan SDK for Windows CI builds.
$vulkanSdkUrl = "$BaseUrl$Version/windows/vulkansdk-windows-X64-$Version.exe"
Invoke-WebRequest -Uri $vulkanSdkUrl -OutFile "vulkansdk-windows-X64-$Version.exe"

Start-Process -FilePath ".\vulkansdk-windows-X64-$Version.exe" -ArgumentList "/S" -NoNewWindow -Wait