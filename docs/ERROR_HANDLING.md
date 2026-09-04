# Error handling

Evan normalizes every backend error (Vulkan, OpenXR, GLFW) into a single
taxonomy, `evan::Error`, defined in `headers/evan/Error.hpp`. Backends map their
native result codes at the boundary so consumers react uniformly.

## The taxonomy

| `evan::Error`      | Meaning                                            | Handling          |
| ------------------ | -------------------------------------------------- | ----------------- |
| `Ok`               | Success                                            | Continue          |
| `Suboptimal`       | `VK_SUBOPTIMAL_KHR` (presentable, recreate soon)   | Handle            |
| `SwapchainOutOfDate` | `VK_ERROR_OUT_OF_DATE_KHR` / `VK_ERROR_SURFACE_LOST_KHR` | Handle (engine already recreates) |
| `NotReady`         | `VK_NOT_READY` / XR session not running / invalid time | Retry or skip frame |
| `DeviceLost`       | `VK_ERROR_DEVICE_LOST`                             | Fatal             |
| `RuntimeLost`      | XR session/instance loss, `XR_SESSION_STATE_LOSS_PENDING` | Fatal     |
| `OutOfMemory`      | `VK_ERROR_OUT_OF_{HOST,DEVICE}_MEMORY`             | Fatal             |
| `RuntimeError`     | Anything else (including GLFW errors)              | Fatal             |

Helpers: `isOk`, `isFatal`, `isRecoverable`.

## Consumer contract

Frame operations return `evan::Error`:

- `Engine::update()` → `Error`
- `Engine::render()` → `Error`
- `Renderer::drawFrame()` → `Error`
- `ADeviceBackend::preprocessFrame/processFrame/postprocessFrame` → `Error`

Handle recoverable errors (`Suboptimal`, `SwapchainOutOfDate`, `NotReady`) and
keep running. On a fatal error (`DeviceLost`, `RuntimeLost`, `OutOfMemory`,
`RuntimeError`) tear down cleanly.

```cpp
while (!platform->shouldClose()) {
    if (evan::Error error = engine.update(); evan::isFatal(error)) {
        break; // stop cleanly
    }
    if (evan::Error error = engine.render(); error != evan::Error::Ok) {
        if (evan::isRecoverable(error)) {
            continue; // swapchain was recreated, retry next frame
        }
        break; // fatal
    }
    engine.pollEvents();
    if (evan::isFatal(engine.getLastError())) {
        break; // e.g. XR_SESSION_STATE_LOSS_PENDING
    }
}
```

`pollEvents()` keeps returning the event vector; the reason a platform is
closing is queryable via `Engine::getLastError()` / `IPlatform::getLastError()`.

## Backend mappings

- `mapVkResult(VkResult)` — Vulkan codes (see `sources/Error.cpp`).
- `mapXrResult(XrResult)` and `mapSessionState(XrSessionState)` — OpenXR codes
  (see `sources/openxr/XrError.cpp`).
- GLFW errors are swept in `IDesktopPlatform::pollEvents` and mapped to
  `RuntimeError`.

## Init helpers

Resource-creation helpers (`createBuffer`, `createImage`, `transitionImageLayout`,
`copyBuffer`, `copyBufferToImage`) return `evan::Error`; `createImageView`
returns `evan::Result<VkImageView>`.
