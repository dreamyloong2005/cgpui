# Windows Vulkan Solid Rect Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make Windows Vulkan present solid rectangle paint commands.

**Architecture:** Keep the public API unchanged. Convert the Vulkan backend from transfer-only image clears to a render-pass path that clears the full frame and then uses `vkCmdClearAttachments` for each `SolidRect`.

**Tech Stack:** C++23, Xmake, Win32, Vulkan 1.x, GDI pixel sampling for the regression test.

---

## File Structure

- `src/renderer/vulkan/vulkan_renderer.cpp`: add render pass/framebuffer resources, collect frame rectangles, record render-pass commands, clamp rectangles, and destroy resources in Vulkan-safe order.
- `tests/renderer/vulkan_solid_rect_test.cpp`: create a visible Win32 window, render one blue rectangle, sample the center pixel, and fail until the rectangle is drawn.
- `xmake.lua`: register the new Windows-only Vulkan solid-rect test with `user32` and `gdi32`.
- `README.md`: update current foundation wording after verification.

## Task 1: Add The Failing Solid Rect Pixel Test

- [ ] Add `tests/renderer/vulkan_solid_rect_test.cpp` with a visible Win32 test window.
- [ ] Register `vulkan_solid_rect_test` in `xmake.lua`.
- [ ] Run `xmake build -P . vulkan_solid_rect_test` and `xmake run -P . vulkan_solid_rect_test`.
- [ ] Verify the test fails because the sampled center pixel is still the clear color, not blue-dominant.
- [ ] Commit the failing test.

## Task 2: Implement Render-Pass Rect Drawing

- [ ] Add `VkRenderPass` and framebuffer ownership to swapchain resources.
- [ ] Require `VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT` when creating the swapchain.
- [ ] Store all `draw_rect()` commands in `VulkanFrame` and pass them to presentation.
- [ ] Replace transfer clear command recording with render-pass recording.
- [ ] Clear the whole frame through render pass load clear.
- [ ] Clamp each `SolidRect` to the framebuffer and emit `vkCmdClearAttachments`.
- [ ] Run `xmake run -P . vulkan_solid_rect_test` and verify it passes.
- [ ] Run `xmake test -P .` and `git diff --check`.
- [ ] Commit the implementation.

## Task 3: Final Verification And Docs

- [ ] Update `README.md` to say Windows presents clear plus solid rectangles.
- [ ] Run `xmake f -c -m debug`, `xmake test -P .`, `xmake build -P . hello_window`, `xmake show -P . -t hello_window`, and `git diff --check`.
- [ ] Run `hello_window`, sample the center pixel, confirm it is blue-dominant, close it, and confirm no process remains.
- [ ] Commit the README update.
