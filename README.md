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

The current foundation defines architecture boundaries and compiles backend targets with Xmake. Windows now presents a Vulkan-cleared frame through the Win32 demo path. Linux and macOS still use renderer skeletons in this milestone, and Linux Wayland surface completion is planned for a later pass.

Solid rectangle GPU drawing is planned separately; this milestone keeps draw commands behind backend interfaces while proving the Windows Vulkan present path.
