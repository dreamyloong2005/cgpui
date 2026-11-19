# Getting Started

## 1. Install Dependencies

Install [mcpp](https://github.com/mcpp-community/mcpp) 2026.11.18.1 or later,
a C++23 compiler (MSVC 14.44 on Windows, GCC 16 on Linux) and the native
dependencies for your target. Use
[Platform Dependencies](platform-dependencies.md) for exact Windows and Linux
requirements. LunaSVG and plutovg are vendored under `third_party/`; the
Vulkan SDK (Windows) and the display libraries (Linux) are host-owned.

On Windows, mcpp needs `VSLANG=1033` (English compiler notes; without it
mcpp records no header dependencies, mcpp issue #970) and a `MCPP_HOME`.
`scripts/dev/build.ps1` and `scripts/dev/test.ps1` set both for their own
process; set them yourself to run `mcpp` directly.

## 2. Build

From the repository root (a workspace: `mcpp.toml` lists the members in
`packages/`, `examples/`, `tests/` and `third_party/`):

```bash
scripts/dev/build.sh                          # every member
scripts/dev/build.sh -p examples/hello_window # one member and what it needs
```

On Windows use `scripts/dev/build.ps1` with the same arguments. Builds run
in parallel. Windows builds use the static CRT (`/MT`); Linux builds use
the `dev-lines` profile (line tables only) to keep test binaries small.

## 3. Run a Desktop Backend

Windows selects Win32. Linux selects `WAYLAND_DISPLAY` before `DISPLAY`, or an
explicit `CGPUI_LINUX_BACKEND` override:

```bash
mcpp run -p examples/hello_window hello_window
CGPUI_LINUX_BACKEND=wayland mcpp run -p examples/hello_window hello_window
CGPUI_LINUX_BACKEND=x11 mcpp run -p examples/hello_window hello_window
```

Wayland requires a compositor such as a desktop session, WSLg, or Weston. X11
requires an X server such as a desktop session or Xvfb.

## 4. Run Deterministic Smoke Modes

```bash
CGPUI_EXIT_AFTER_FIRST_FRAME=1 mcpp run -p examples/hello_window hello_window
CGPUI_RESIZE_AFTER_FIRST_FRAME=1 mcpp run -p examples/hello_window hello_window
CGPUI_CLOSE_AFTER_FIRST_FRAME=1 mcpp run -p examples/hello_window hello_window
CGPUI_DEMO_SMOKE_FLOW=1 mcpp run -p examples/hello_window hello_window
```

PowerShell uses `$env:NAME = "1"` before the command and
`Remove-Item Env:NAME` afterward. The same cases run as test programs of
`examples/hello_window` (`tests/<platform>_first_frame.cpp` and so on).

## 5. Run Tests

```bash
scripts/dev/test.sh                     # every test program, in parallel
scripts/dev/test.sh -p tests/ui         # one suite
scripts/dev/test.sh -p tests/api_parity gpui_parity_ledger_test
```

On Windows use `scripts/dev/test.ps1`. A test program is a file listed in
its suite's `[test] discover` (`tests/<suite>/mcpp.toml`) and is named after
the file. Tests that read repository files get the repository root from the
`CGPUI_SOURCE_ROOT` define of their suite's `build.mcpp`.

## 6. Start an Application

Application code includes `cgpui/prelude.hpp`, implements a `View`, creates a
platform application, and calls `run_app(...)`. The complete executable example
is `examples/hello_window/main.cpp`; public API-only examples live under
`examples/api_parity/`.

Use public contexts and capability facades from callbacks. Do not include
`src/` headers or construct `WindowRuntime` directly in application code.
