# Platform Dependencies

## Common

- [mcpp](https://github.com/mcpp-community/mcpp) 2026.11.20.1 or later
  (`[workspace.package] mcpp` in `mcpp.toml`)
- Git
- A C++23 compiler
- Python 3.11 or later for parity, package, performance, and stress
  validators (the SDK scripts read TOML with `tomllib`)
- LunaSVG v3.5.0 and plutovg v1.3.3, vendored under `third_party/`
  (provenance in `third_party/VENDORED.md`)

mcpp builds and runs tests in parallel. It must not download a toolchain:
the workspace uses the installed one (`[workspace.toolchain]`).

## Windows: Win32 + Vulkan

- Windows 10 or later
- Visual Studio 2022 Build Tools with the x64 C++ workload
- PowerShell 7 for CI scripts
- The LunarG Vulkan SDK (1.4.341.1 here) with `VULKAN_SDK` set;
  `packages/renderer-vulkan/build.mcpp` takes the headers and `vulkan-1.lib`
  from it
- A Vulkan-capable driver

The toolchain is `msvc@system` (the installed Build Tools), in any UI
language: mcpp takes header dependencies from cl.exe's `/sourceDependencies`
record and decodes the compiler's text in the console code page (mcpp
2026.11.19.1, issue #970). `scripts/dev/mcpp-env.ps1`, used by the dev
wrappers and `scripts/ci/windows-dependencies.ps1`, sets `MCPP_HOME` for its
own process only. `scripts/ci/windows-package.ps1` is
the package entrypoint.

## Linux: Wayland + Vulkan

Required development/runtime components are a C++ compiler, Vulkan loader and
driver, Wayland client/server/cursor development files, DBus, libxkbcommon
and pkg-config. Native text shaping requires HarfBuzz, FreeType and
fontconfig and at least one installed font; CJK text needs a CJK font. The
toolchain is the system GCC (`[toolchain] linux = { path = "/usr" }`).
Each library's `build.mcpp` asks pkg-config for the modules it uses
(`wayland-client`, `wayland-cursor`, `wayland-server` (tests), `dbus-1`,
`xkbcommon`, `xkbcommon-x11`, `xcb`, `xcb-xkb`, `xcb-cursor`, `harfbuzz`,
`freetype2`, `fontconfig`, `vulkan`); `scripts/ci/linux-dependencies.sh`
checks they are all present. xkbcommon is now the system library (xmake
built it from source).

Arch (the WSL2 `archlinux` gate) installs:

```bash
pacman -S --needed gcc pkgconf wayland libxkbcommon libxkbcommon-x11 \
  libxcb xcb-util-cursor dbus harfbuzz freetype2 fontconfig vulkan-icd-loader \
  vulkan-headers ttf-dejavu wqy-zenhei
```

Ubuntu CI installs:

```bash
sudo apt-get install -y libdbus-1-dev libfontconfig1-dev libfreetype-dev \
  libharfbuzz-dev libvulkan-dev fonts-dejavu-core fonts-wqy-zenhei \
  libwayland-dev libxkbcommon-dev libxkbcommon-x11-dev libxcb1-dev \
  libxcb-xkb-dev libxcb-cursor-dev pkg-config mesa-vulkan-drivers \
  meson ninja-build python3-venv weston
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
That was the xmake build (`19c51989`). The scripts now build with mcpp,
which reads `MACOSX_DEPLOYMENT_TARGET`, and have not run on a Mac
([macOS follow-up](migration/README.md#macos-follow-up)).
