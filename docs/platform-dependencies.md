# Platform Dependencies

## Common

- Xmake 3.x
- Git
- A C++23 compiler
- Python 3 for parity, package, performance, and stress validators
- LunaSVG, resolved by Xmake

Repository CI commands keep Xmake build and test work serial with `-j 1`.

## Windows: Win32 + Vulkan

- Windows 10 or later
- Visual Studio 2022 Build Tools with the x64 C++ workload
- PowerShell 7 for CI scripts
- Vulkan SDK, resolved through Xmake's `vulkansdk` package
- A Vulkan-capable driver

`scripts/ci/windows-dependencies.ps1` keeps Xmake package state under the
repository-local build root. `scripts/ci/windows-package.ps1` is the package
entrypoint.

## Linux: Wayland + Vulkan

Required development/runtime components are a C++ compiler, Vulkan loader and
driver, Wayland client development files, DBus, libxkbcommon, Meson, Ninja,
and Python virtual-environment support. Fontconfig is used when available.

Ubuntu CI installs:

```bash
sudo apt-get install -y libdbus-1-dev libfontconfig1-dev libvulkan-dev \
  libwayland-dev mesa-vulkan-drivers meson ninja-build python3-venv weston
```

Use a real Wayland session, WSLg, or Weston for native smoke and pixel tests.

## Linux: X11/XCB + Vulkan

The X11 backend additionally requires XCB and XKB Common's X11 integration.
Headless tests require Xvfb. Typical packages are `libxcb`/`libxcb1-dev`,
`libxkbcommon-x11`/`libxkbcommon-x11-dev`, and `xorg-server-xvfb`/`xvfb`.

Use `CGPUI_LINUX_BACKEND=x11`; do not force that override across
Wayland-specific tests. The repository's X11 runners isolate Xvfb acceptance
from the full Wayland suite.

## macOS: Cocoa + Metal

The production target uses Xcode, AppKit, QuartzCore, CoreText, and Metal.
Phase J pins Xmake 3.0.9 and targets macOS 13.0 on native arm64 and x86_64
runners. `scripts/ci/macos-dependencies.sh` owns repository-local dependency
roots; both architectures pass final native verification.
