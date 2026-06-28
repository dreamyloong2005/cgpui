# Linux Wayland Vulkan First Frame Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the Linux Wayland target create a real Wayland window and let the Vulkan renderer present its existing clear plus solid-rectangle frame to it.

**Architecture:** Keep public APIs unchanged. Add a minimal xdg-shell Wayland window implementation in the Linux platform target, then extend the Vulkan renderer with Linux Wayland surface creation while preserving the existing shared swapchain/render-pass presentation path.

**Tech Stack:** C++23, Xmake, Wayland client, xdg-shell client protocol, Vulkan 1.x, existing CGPUI platform and renderer interfaces.

---

## File Structure

- `src/platform/linux/wayland_application.cpp`: replace the current display-only skeleton with registry binding, xdg-shell ping/configure handling, a concrete `WaylandWindow`, redraw requests, and safe teardown.
- `src/renderer/vulkan/vulkan_renderer.cpp`: branch native-surface validation and Vulkan instance/surface creation for Win32 versus Wayland; keep swapchain, render pass, and presentation shared.
- `tests/renderer/vulkan_surface_validation_test.cpp`: extend validation for Linux null Wayland handles where the target can build it.
- `xmake.lua`: add Wayland protocol generation support or source registration if needed for xdg-shell, and register Linux-only validation tests if they can run without a compositor.
- `README.md`: update platform status and Linux manual verification notes.

## Task 1: Document And Baseline The Milestone

- [ ] Confirm this work runs in `D:\Dev\Projects\cgpui\.worktrees\linux-wayland-vulkan-first-frame` on branch `linux-wayland-vulkan-first-frame`.
- [ ] Run `xmake test -P .` on Windows and record that the current baseline is 8/8 passing.
- [ ] Commit the design and this implementation plan.

## Task 2: Add Linux Wayland Window Construction

- [ ] Inspect whether xdg-shell client headers are available through the configured `wayland` package on Linux; if not, add protocol generation to `xmake.lua` using `wayland-scanner`.
- [ ] Write a Linux-only platform test or build target that expects `create_window()` to return a `PlatformWindow` when a Wayland display is available, and skips clearly when `wl_display_connect(nullptr)` fails.
- [ ] Run the new Linux target on a Linux Wayland environment and verify it fails with `Wayland surface creation is not implemented yet`.
- [ ] Implement registry binding for `wl_compositor` and `xdg_wm_base`.
- [ ] Implement `WaylandWindow` owning `wl_surface`, `xdg_surface`, and `xdg_toplevel`, with non-null `WaylandSurfaceHandle`.
- [ ] Implement configure acknowledgement, close handling, `request_redraw()`, and a simple event loop that dispatches Wayland events until `quit()`.
- [ ] Re-run the Linux platform test/build target in a Wayland environment and verify it passes.
- [ ] Commit the platform implementation.

## Task 3: Add Vulkan Wayland Surface Support

- [ ] Add a Linux-only renderer validation test for null `WaylandSurfaceHandle` that expects `renderer_initialization_failed`.
- [ ] Run the Linux validation test and verify it fails because the renderer still requires Win32 or reports the wrong platform boundary.
- [ ] Refactor `VulkanRendererState::create()` to accept a `NativeSurfaceHandle` and keep a small platform-specific `create_surface(...)` branch.
- [ ] On Windows, keep requiring non-null `Win32SurfaceHandle` and `VK_KHR_win32_surface`.
- [ ] On Linux, require non-null `WaylandSurfaceHandle`, include `<vulkan/vulkan_wayland.h>`, enable `VK_KHR_wayland_surface`, and call `vkCreateWaylandSurfaceKHR`.
- [ ] Keep device selection, swapchain creation, render pass, solid rectangles, resize, and presentation shared.
- [ ] Run Windows `xmake test -P .` and the Linux validation test.
- [ ] Commit the Vulkan Wayland WSI implementation.

## Task 4: Verify `hello_window` On Linux Wayland

- [ ] On Linux or WSLg, run `xmake f -c -m debug`.
- [ ] Run `xmake build hello_window`.
- [ ] Run `xmake run hello_window`.
- [ ] Verify a Wayland window opens and shows the blue solid rectangle over the dark clear background.
- [ ] Close the window and verify the process exits.
- [ ] Record any environment limitation if this Windows machine cannot run the Linux Wayland runtime check directly.

## Task 5: Final Docs And Regression Sweep

- [ ] Update `README.md` to describe Windows Vulkan verified status and Linux Wayland first-frame status.
- [ ] Run `xmake test -P .` on Windows.
- [ ] Run `xmake build -P . hello_window` on Windows to protect the existing demo.
- [ ] Run `git diff --check`.
- [ ] Commit the README update.
