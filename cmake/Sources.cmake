# Explicit list of Evan source files.
#
# This list is intentionally explicit (no file(GLOB)) so that the installed
# target is reproducible across machines and packaging is deterministic.
# When adding or removing a source file, update this list.

set(EVAN_SOURCES_COMMON
    sources/ADeviceBackend.cpp
    sources/ASwapchainContext.cpp
    sources/ASwapchainImage.cpp
    sources/DeviceContext.cpp
    sources/Engine.cpp
    sources/Frame.cpp
    sources/GPUMaterial.cpp
    sources/GPUMesh.cpp
    sources/GPUShader.cpp
    sources/GPUTexture.cpp
    sources/RenderObject.cpp
    sources/Renderer.cpp
    sources/RessourceManager.cpp
    sources/Scene.cpp
    sources/Version.cpp
    sources/ViewSet.cpp
)

# OpenXR sources. The Android platform file is only compiled for Android.
set(EVAN_SOURCES_OPENXR
    sources/openxr/IXrPlatform.cpp
    sources/openxr/InteractionProfile.cpp
    sources/openxr/XrDeviceBackend.cpp
    sources/openxr/XrManageActions.cpp
    sources/openxr/XrSwapchainContext.cpp
    sources/openxr/XrSwapchainImage.cpp
    sources/openxr/actions/AXrAction.cpp
    sources/openxr/actions/XrHandsMotionActions.cpp
    sources/openxr/actions/XrManageButtonsActions.cpp
    sources/openxr/actions/XrManageThumbStickActions.cpp
    sources/openxr/platform/LinuxXrPlatform.cpp
)

set(EVAN_SOURCES_OPENXR_ANDROID
    sources/openxr/platform/AndroidXrPlatform.cpp
)

# GLFW sources.
set(EVAN_SOURCES_GLFW
    sources/glfw/DesktopBackend.cpp
    sources/glfw/DesktopSwapchainContext.cpp
    sources/glfw/DesktopSwapchainImage.cpp
    sources/glfw/IDesktopPlatform.cpp
    sources/glfw/platform/LinuxDesktopPlatform.cpp
    sources/glfw/platform/MacOsDesktopPlatform.cpp
    sources/glfw/platform/WindowsPlatoform.cpp
)
