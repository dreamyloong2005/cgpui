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
  from it. Its `slangc` and `spirv-val` are needed only to compile edited
  shaders ([Getting Started](shaders.md)).
- A driver and loader with Vulkan 1.1 or later: the shaders are SPIR-V 1.3,
  and the renderer skips devices below 1.1.

The toolchain is `msvc@system` (the installed Build Tools), in any UI
language: mcpp takes header dependencies from cl.exe's `/sourceDependencies`
record and decodes the compiler's text in the console code page (mcpp
2026.11.19.1, issue #970). `scripts/dev/mcpp-env.ps1`, used by the dev
wrappers and `scripts/ci/windows-dependencies.ps1`, sets `MCPP_HOME` for its
own process only. On hosted Phase G runners the shared setup action installs
the SHA-256-pinned x64 Vulkan SDK 1.4.341.1 and exports `VULKAN_SDK` before
checking the build environment. `scripts/ci/windows-package.ps1` is the package
entrypoint.

## Linux: Wayland + Vulkan

Required development/runtime components are a C++ compiler, Vulkan loader and
driver, Wayland client/server/cursor development files, DBus, libxkbcommon
and pkg-config. Native text shaping requires HarfBuzz, FreeType and
fontconfig and at least one installed font; CJK text needs a CJK font. The
toolchain is the system GCC. `tools/linux-gcc/bin` exposes the installed GNU
tools from `/usr/bin`, but not Clang. Both the main workspace and mcpp's
native `build.mcpp` sub-build therefore select `g++`: the latter receives the
toolchain path, not the manifest's `family` setting. This does not change the
system compiler or `PATH`.
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
  vulkan-headers ttf-dejavu wqy-zenhei xorg-server-xvfb xorg-xdpyinfo
```

On Ubuntu the Phase G setup action installs the build dependencies before
checking pkg-config. The example smoke job additionally installs the software
Vulkan driver, fonts, and headless Weston. To install both sets:

```bash
sudo apt-get install -y g++ binutils libdbus-1-dev libfontconfig1-dev libfreetype-dev \
  libharfbuzz-dev libvulkan-dev fonts-dejavu-core fonts-wqy-zenhei \
  libwayland-dev libxkbcommon-dev libxkbcommon-x11-dev libxcb1-dev \
  libxcb-xkb-dev libxcb-cursor-dev pkg-config mesa-vulkan-drivers \
  meson ninja-build python3-venv weston
```

Use a real Wayland session, WSLg, or Weston for native smoke and pixel tests.

## Linux: X11/XCB + Vulkan

The X11 backend additionally requires XCB and XKB Common's X11 integration.
Typical packages are `libxcb`/`libxcb1-dev`, `libxkbcommon-x11`/
`libxkbcommon-x11-dev`, and `xorg-server-xvfb`/`xvfb` with `xorg-xdpyinfo`/
`x11-utils`. `scripts/dev/test.sh` runs the X11 tests on a private Xvfb
display (`scripts/dev/test-display.sh`), not the session's: on WSLg or a
desktop, `x11_clipboard_test` would replace the host clipboard (WSLg copies
the X11 selection into Windows) and fail Windows clipboard tests running at
the same time. `CGPUI_TEST_DISPLAY=session` uses the session's `DISPLAY`.

Use `CGPUI_LINUX_BACKEND=x11`; do not force that override across
Wayland-specific tests. The repository's X11 runners isolate Xvfb acceptance
from the full Wayland suite.

## macOS: Cocoa + Metal

The production target uses Xcode, AppKit, QuartzCore, CoreText, and Metal.
Only native arm64 is supported. `scripts/ci/macos-dependencies.sh` configures
the CI environment. The mcpp build uses Xcode's clang,
`ld64.lld` for host build programs, and `MACOSX_DEPLOYMENT_TARGET=13.0`.
See the [macOS migration record](migration/README.md#macos-follow-up).
