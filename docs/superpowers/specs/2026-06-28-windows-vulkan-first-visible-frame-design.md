# Windows Vulkan First Visible Frame Design

## Purpose

This milestone turns the current Windows Vulkan renderer from a no-op boundary into a visible renderer path. Running `examples/hello_window` on Windows should open the existing Win32 window and present a real Vulkan-cleared frame.

The goal is a first visible frame, not a complete 2D renderer. Solid rectangle drawing remains deferred to the next Vulkan rendering pass.

## Scope

This pass targets Windows only:

- use the existing `Win32SurfaceHandle` from `cgpui_platform_win32`.
- implement Vulkan instance, Win32 surface, physical device selection, logical device, swapchain, image views, command recording, synchronization, clear, and present.
- keep `draw_rect()` as a no-op that records the last rectangle for now.
- keep all Win32 and Vulkan headers inside implementation files.

Linux Wayland, macOS Metal, shader pipelines, text, and rectangle rasterization are out of scope for this pass.

## Architecture

`cgpui_renderer_vulkan` remains the only target touched for Vulkan behavior. It owns the Vulkan objects needed to present to a native surface:

- `VkInstance`
- `VkSurfaceKHR`
- selected physical device
- `VkDevice`
- graphics and present queues
- `VkSwapchainKHR`
- swapchain images and image views
- command pool and command buffers
- acquire/render semaphores and an in-flight fence

The renderer validates that the native surface is a non-null `Win32SurfaceHandle` before creating a Windows Vulkan surface. Unsupported native surface variants return `renderer_initialization_failed`.

The public renderer interface does not change. `RenderFrame::clear(Color)` stores the clear color for the current frame. `RenderFrame::present()` records the image layout transitions, clears the acquired swapchain image with `vkCmdClearColorImage`, submits the command buffer, and presents the image.

## Resize And Minimize Behavior

`Renderer::resize()` updates the framebuffer size and recreates the swapchain when the size is non-empty. Empty sizes still return `renderer_initialization_failed`, matching the current validation behavior.

If `vkAcquireNextImageKHR` or `vkQueuePresentKHR` reports an out-of-date swapchain, the renderer recreates the swapchain on the next valid frame. The first implementation can keep this conservative: return a renderer error for out-of-date presentation, then rely on the resize path to recreate. It must not crash or leak handles.

## Error Handling

Every Vulkan call used for initialization and presentation returns a `Result` error with a short message when it fails. Messages should name the failed Vulkan step, for example:

- `vkCreateWin32SurfaceKHR failed`
- `No Vulkan physical device supports graphics and present queues`
- `vkCreateSwapchainKHR failed`
- `vkQueuePresentKHR reported an out-of-date swapchain`

Destructors release Vulkan resources in reverse ownership order and must tolerate partially constructed state.

## Testing

Automated tests should cover the parts that can run without opening a GUI:

- `create_renderer()` rejects non-Win32 native surfaces on the Windows Vulkan target.
- `create_renderer()` rejects null Win32 `hinstance` or `hwnd`.
- Vulkan resize validation continues to reject empty framebuffer sizes.

Manual verification covers the visible frame:

- `xmake build hello_window`
- run `xmake run hello_window`
- verify the Win32 window opens and displays a visible non-default clear color.
- close the window and verify the process exits.

Existing tests must continue to pass with `xmake test`.

## Acceptance Criteria

- `hello_window` opens on Windows and presents a Vulkan-cleared frame.
- `cgpui_renderer_vulkan` creates and destroys real Vulkan instance, Win32 surface, device, swapchain, image views, command buffers, and synchronization objects.
- public headers remain free of Win32 and Vulkan headers.
- non-Win32 and null Win32 surfaces fail with `renderer_initialization_failed`.
- `xmake test` passes.
- `xmake build hello_window` passes.
- manual `hello_window` verification leaves no GUI process running.

## Deferred Work

- drawing the solid rectangle paint command with a graphics pipeline.
- shared Vulkan WSI support for Wayland.
- multi-frame resource rings.
- validation layer configuration and debug messenger.
- robust minimized-window idle behavior.
- MSAA, color management, and present mode configuration UI.
