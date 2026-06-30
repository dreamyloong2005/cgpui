# Platform Mac Readiness Audit

Step 88 keeps the Windows/Linux GPUI-core track moving while reserving a clear
macOS path. This is an audit and boundary slice, not a full macOS parity port.

## Target Matrix

- Windows: Win32 + Vulkan
- Linux: Wayland + Vulkan
- macOS: Cocoa + Metal

## Current Mac Slots

- `src/platform/macos/macos_application.mm` is the Cocoa/AppKit platform slot.
  It creates an `NSWindow`, installs a `CAMetalLayer`, and returns a
  `MetalSurfaceHandle` through the existing `PlatformWindow` interface.
- `src/renderer/metal/metal_renderer.mm` is the Metal renderer slot. It accepts
  `MetalSurfaceHandle`, validates the `CAMetalLayer`, and keeps the renderer
  contract aligned with the existing `Renderer` interface.
- `xmake.lua` already keeps macOS sources behind `is_plat("macosx")` and links
  AppKit, QuartzCore, and Metal only for macOS targets.

## Public Boundary Added

- `DesktopPlatformTarget` names the supported desktop platform targets:
  Windows/Win32, Linux/Wayland, and macOS/Cocoa.
- `RendererBackendTarget` names the renderer families: Vulkan and Metal.
- `default_renderer_backend_for(...)` records the intended mapping:
  Windows/Linux use Vulkan and macOS uses Metal.

## Not in scope

- Full Cocoa event parity with Win32/Wayland input.
- A real Metal command encoder, swapchain frame lifecycle, glyph pipeline, or
  text renderer.
- Running macOS tests from the current Windows/WSL machine.

## Next Mac-Neutral Rules

- New platform-independent UI APIs should live above `PlatformWindow`,
  `PlatformApplication`, `NativeSurfaceHandle`, and `Renderer`.
- New Windows/Linux code should stay behind platform-specific xmake branches
  or platform source directories.
- New renderer behavior should be expressed through renderer-neutral command
  data first, then implemented by Vulkan and Metal backends separately.
