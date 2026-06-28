# Windows Vulkan Solid Rect Design

## Purpose

This milestone makes the existing Windows Vulkan renderer honor `RenderFrame::draw_rect()` for solid axis-aligned rectangles. Running `examples/hello_window` on Windows should show the blue rectangle emitted by `HelloView`, not only the gray clear color.

## Scope

This pass targets Windows Vulkan only:

- keep public renderer, UI, and platform headers unchanged.
- keep Linux Wayland and macOS Metal renderer skeleton behavior unchanged.
- draw filled, axis-aligned, opaque rectangles in framebuffer coordinates.
- use the existing `PaintList` and `SolidRect` command path.

Rounded corners, clipping stacks, text, images, batching, blending, and shader-based pipelines are out of scope.

## Architecture

The Vulkan backend will replace the transfer-only clear path with a render-pass path. Swapchain images will be created with `VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT`. Each swapchain image gets a framebuffer. Command recording begins a render pass with the frame clear color, then emits one `vkCmdClearAttachments` call per visible solid rectangle using a clamped `VkClearRect`.

This keeps the first rectangle implementation deliberately small. It proves that paint commands reach the GPU and that the presented image changes in the rectangle region, while deferring the full graphics pipeline until drawing needs vertices, shaders, blending, or more complex primitives.

## Error Handling

All Vulkan object creation continues to return `Result` errors with the existing renderer error codes. Swapchain teardown must destroy framebuffers before image views and render passes before the swapchain is released. Resize keeps the existing conservative recreation behavior.

## Testing

Automated Windows coverage adds a visible-window Vulkan test that:

- creates a small Win32 window.
- renders a gray clear plus a blue rectangle over the center.
- samples the client-area center pixel through GDI.
- fails while `draw_rect()` is ignored and passes once the rectangle is presented.

Existing Vulkan validation, resize, lifetime, header cleanliness, and UI tests must keep passing. Manual verification samples `hello_window` center pixels and confirms they are blue-dominant.
