# CGPUI

CGPUI is a C++23-native UI framework inspired by GPUI's architecture. This foundation milestone focuses on clean platform, renderer, UI, and public authoring API boundaries.

## Build

```bash
xmake f -c -m debug
xmake build hello_window
```

## Run

```bash
xmake run hello_window
```

## Platform Targets

- Windows: Win32 + Vulkan
- Linux: Wayland + Vulkan, with X11 reserved in the platform abstraction
- macOS: Cocoa + Metal

## Current Foundation

The current foundation defines architecture boundaries and compiles backend targets with Xmake. The hello window demo now uses the public `cgpui/cgpui.hpp` prelude, `run_app`, `AppContext`, `View::render`, free element factories, fluent builder shortcuts, and `ViewContext` model/text helpers. Windows presents Vulkan frames through the Win32 demo path, and Linux uses the Wayland xdg-shell window path with Vulkan Wayland surface creation wired to the shared renderer path. macOS still uses a renderer skeleton in this milestone.

Linux Wayland runtime verification requires a Linux Wayland session or WSLg:

```bash
xmake f -c -m debug
xmake build hello_window
xmake run hello_window
```

For a deterministic first-frame smoke check, ask the demo to exit after the
first successful present:

```bash
CGPUI_EXIT_AFTER_FIRST_FRAME=1 xmake run hello_window
```

To exercise swapchain resize and a second presented frame, run the lifecycle
smoke check:

```bash
CGPUI_RESIZE_AFTER_FIRST_FRAME=1 xmake run hello_window
```

To exercise the close-request callback after the first presented frame, run:

```bash
CGPUI_CLOSE_AFTER_FIRST_FRAME=1 xmake run hello_window
```
