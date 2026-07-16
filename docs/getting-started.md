# Getting Started

## 1. Install Dependencies

Install Xmake, a C++23 compiler, and the native dependencies for your target.
Use [Platform Dependencies](platform-dependencies.md) for exact Windows and
Linux requirements. Xmake resolves LunaSVG and the configured Vulkan SDK
package; system display libraries remain host-owned.

## 2. Configure and Build

From the repository root:

```bash
xmake f -c -m debug
xmake build -j 1 hello_window
```

The configure command does not accept this repository's `-j 1` convention.
Build and test commands do.

## 3. Run a Desktop Backend

Windows selects Win32. Linux selects `WAYLAND_DISPLAY` before `DISPLAY`, or an
explicit `CGPUI_LINUX_BACKEND` override:

```bash
xmake run hello_window
CGPUI_LINUX_BACKEND=wayland xmake run hello_window
CGPUI_LINUX_BACKEND=x11 xmake run hello_window
```

Wayland requires a compositor such as a desktop session, WSLg, or Weston. X11
requires an X server such as a desktop session or Xvfb.

## 4. Run Deterministic Smoke Modes

```bash
CGPUI_EXIT_AFTER_FIRST_FRAME=1 xmake run hello_window
CGPUI_RESIZE_AFTER_FIRST_FRAME=1 xmake run hello_window
CGPUI_CLOSE_AFTER_FIRST_FRAME=1 xmake run hello_window
CGPUI_DEMO_SMOKE_FLOW=1 xmake run hello_window
```

PowerShell uses `$env:NAME = "1"` before the command and
`Remove-Item Env:NAME` afterward.

## 5. Run Tests

```bash
xmake build -j 1
xmake test -j 1 -v
```

Focused tests use their registered names, including `/default`:

```bash
xmake test -j 1 -v gpui_parity_ledger_test/default
```

## 6. Start an Application

Application code includes `cgpui/prelude.hpp`, implements a `View`, creates a
platform application, and calls `run_app(...)`. The complete executable example
is `examples/hello_window/main.cpp`; public API-only examples live under
`examples/api_parity/`.

Use public contexts and capability facades from callbacks. Do not include
`src/` headers or construct `WindowRuntime` directly in application code.
