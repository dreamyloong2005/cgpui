# Platform Mac Readiness

Phase H completes the macOS Cocoa + Metal production target without changing
the public authoring model used by Windows and Linux.

## Target Matrix

- Windows: Win32 + Vulkan, completed before Phase H.
- Linux: Wayland + Vulkan, completed before Phase H.
- macOS: Cocoa + Metal, completed in Phase H.
- X11: XCB + Vulkan, completed in Phase I.

## Production Coverage

- Cocoa application and `NSWindow` lifecycle, close policy, activation,
  scaling, resize, cursor, and run-loop wakeups.
- `CAMetalLayer`, Metal device/queue/drawable lifecycle, frame pacing,
  primitive pipelines, stable command ordering, diagnostics, and RGBA capture.
- Pointer, scroll, keyboard, modifiers, capture, text input, CoreText font
  discovery, IME ranges, and candidate placement.
- NSPasteboard clipboard, drag source/target negotiation and cancellation,
  native menus, accelerators, dialogs, URL opening, reopen/quit, and chrome.
- Generation-keyed NSAccessibility providers, actions, notifications, stale
  rejection, multi-window isolation, and teardown.
- All public examples build from the same sources and run first-frame, resize,
  close, and interaction smoke flows on macOS.

## Module Boundary

`src/platform/macos/macos_application.mm` and
`src/renderer/metal/metal_renderer.mm` are composition roots. Focused private
Objective-C++ modules own native behavior, and public headers contain no
AppKit, CoreText, QuartzCore, or Metal implementation details.

## Verification

Phase H macOS full debug passes 380/380 on macOS 26.5.2 (25F84), Xcode 26.6 (17F113), and Xmake 3.0.9+HEAD.2b184e178, including native Cocoa, Metal primitive/clip/text-image pixel capture, accessibility, and public-example smoke coverage.

Phase H required macOS gaps: 0. Phase I Step 759 X11/XCB platform boundary.

Phase I preserves the native macOS evidence while completing strict Linux
backend parity. Phase I Step 798 completes strict Linux backend parity with
Windows full Debug at 478/478, WSLg full Debug at 470/470, an isolated Xvfb
X11 matrix at 11/11, cross-platform/macOS source guards, and preserved Phase H
native macOS evidence at 380/380. Phase I required X11 gaps: 0. Phase J Step 799 re-run upstream
extractor against the pinned revision.
