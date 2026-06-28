# CGPUI

CGPUI is a C++23-native UI framework inspired by GPUI's architecture. This foundation milestone focuses on clean platform, renderer, and UI boundaries.

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

The current foundation defines architecture boundaries and compiles backend targets with Xmake. Windows and Linux use Vulkan renderer skeletons; macOS uses a Metal renderer skeleton. Linux Wayland has a platform boundary, while full xdg-shell and native surface completion are planned for a later pass.

Real GPU draw submission is also planned separately; this milestone keeps rendering calls behind backend interfaces without promising visible Vulkan or Metal drawing yet.
