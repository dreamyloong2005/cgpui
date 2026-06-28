# Linux Wayland Vulkan First Frame Design

## Purpose

This milestone brings the existing Linux target from a display-only skeleton to a real Wayland window that can hand a native `wl_display` and `wl_surface` to the shared Vulkan renderer. Running `examples/hello_window` on Linux Wayland should create a window and present the same clear plus solid-rectangle frame that already works on Windows.

## Scope

This pass targets Linux Wayland plus the cross-platform Vulkan renderer:

- implement a minimal Wayland `PlatformWindow` using `wl_compositor`, `xdg_wm_base`, `xdg_surface`, and `xdg_toplevel`.
- return `WaylandSurfaceHandle{display, surface}` from the Linux platform window.
- extend the Vulkan renderer to accept Wayland surfaces on Linux through `VK_KHR_wayland_surface`.
- keep the existing public platform, renderer, UI, and native-surface headers unchanged.
- reuse the current Vulkan render-pass path for clear color and solid rectangles.

Pointer events, keyboard events, cursor management, decorations, fractional scaling, shader pipelines, and X11 are out of scope.

## Architecture

The Linux platform target will own all Wayland protocol objects needed to create and run a toplevel window. `WaylandApplication` owns `wl_display`, `wl_registry`, and discovered globals. `WaylandWindow` owns `wl_surface`, `xdg_surface`, `xdg_toplevel`, the window state, and the callback. Configure events update pending dimensions and acknowledge the configure serial; redraw requests commit the surface and emit `WindowRedrawRequested`.

The Vulkan renderer will branch by native-surface variant at creation time. Windows keeps the existing Win32 path. Linux uses `WaylandSurfaceHandle` and creates the Vulkan instance with `VK_KHR_surface` plus `VK_KHR_wayland_surface`, then creates `VkSurfaceKHR` with `vkCreateWaylandSurfaceKHR`. Device, swapchain, render pass, rectangle clearing, synchronization, resize behavior, and presentation stay shared.

This keeps the milestone small: the platform creates a real Wayland surface, and Vulkan WSI learns how to attach to it. Rendering itself does not get another abstraction layer yet.

## Error Handling

Wayland creation returns `platform_initialization_failed` or `window_creation_failed` with messages naming the missing step, such as `wl_display_connect failed`, `wl_compositor global not available`, or `xdg_surface creation failed`.

The Vulkan renderer returns `renderer_initialization_failed` for unsupported or null surface handles. On Linux it should reject null Wayland `display` or `surface` handles before calling Vulkan. Vulkan failure messages should name the failed API step, including `vkCreateWaylandSurfaceKHR failed`.

Destructors must tolerate partial construction and destroy objects in reverse ownership order.

## Testing

Automated tests on Windows must continue to pass with `xmake test -P .`. Windows validation protects the existing Win32 Vulkan path while this branch is developed on Windows.

Linux-specific automated coverage should include surface-handle validation that can run without a live compositor when possible: null `WaylandSurfaceHandle` must fail with `renderer_initialization_failed` on Linux builds.

Manual Linux or WSLg verification is required for the visible frame:

- configure and build on Linux: `xmake f -c -m debug && xmake build hello_window`.
- run `xmake run hello_window` inside a Wayland session.
- verify the window opens and shows the blue solid rectangle over the dark clear background.
- close the window and verify the process exits.

## Acceptance Criteria

- Windows `xmake test -P .` still passes.
- Linux `hello_window` builds with `cgpui_platform_linux_wayland` and `cgpui_renderer_vulkan`.
- Linux Wayland `create_window()` returns a real `PlatformWindow` with non-null `WaylandSurfaceHandle`.
- Linux Vulkan creates a Wayland `VkSurfaceKHR` and presents through the existing render-pass path.
- public headers stay free of Wayland and Vulkan headers.
- README clearly states Windows is verified locally and Linux Wayland first-frame runtime verification requires a Linux Wayland session.

## Deferred Work

- pointer and keyboard event mapping on Wayland.
- frame callbacks and throttled redraw scheduling.
- xdg-decoration and server/client-side decorations.
- DPI/fractional-scale protocol support.
- X11 Vulkan surface support.
- a dedicated Vulkan WSI abstraction layer once macOS/Metal and Linux behavior are both richer.
