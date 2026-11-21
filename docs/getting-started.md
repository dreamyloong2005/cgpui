# Getting Started

## 1. Install Dependencies

Install [mcpp](https://github.com/mcpp-community/mcpp) 2026.11.20.1 or later,
a C++23 compiler (MSVC 14.44 on Windows, GCC 16 on Linux) and the native
dependencies for your target. Use
[Platform Dependencies](platform-dependencies.md) for exact Windows and Linux
requirements. LunaSVG and plutovg are vendored under `third_party/`; the
Vulkan SDK (Windows) and the display libraries (Linux) are host-owned.

On Windows, `scripts/dev/build.ps1` and `scripts/dev/test.ps1` set
`MCPP_HOME` for their own process; set it yourself to run `mcpp` directly.
The compiler may speak any language: mcpp 2026.11.19.1 and later record
header dependencies and read MSVC's diagnostics in every language and code
page (mcpp issue #970).

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

## 7. AddressSanitizer

```bash
scripts/dev/test.sh --asan                     # every test, ASan + UBSan
scripts/dev/test.sh --asan -p tests/ui         # one suite
scripts/dev/test.sh --asan --per-member-cleanup  # member by member, low disk
```

On Windows: `scripts/dev/test.ps1 -Asan` with the same arguments, and
`-AsanSymbols` for function, file and line in the reports (a PDB per test
program, in `build/asan-symbols`; use it with `-p`). The run
replaces the checks of the MSVC debug CRT, which mcpp does not build
(`_ITERATOR_DEBUG_LEVEL=2`, debug heap, leak report; see
[SDK packages](sdk-packaging.md#c-runtime-windows)).

`scripts/dev/asan_workspace.py` writes `build/asan/mcpp.toml`: the
repository's members, by paths relative to `build/asan`, with the sanitizer
flags in `[workspace.build]`, so the whole graph (vendored lunasvg and
plutovg too) is instrumented and has its own build directory under
`build/asan/target`. An mcpp profile would not do: its `cxxflags` reach only
the units of the selected member, not of its dependencies, and MSVC refuses
to link instrumented with uninstrumented STL containers (LNK2038
`annotate_vector`). `-p <member>` is rewritten to `-p ../../<member>`.

| | Windows (cl.exe) | Linux (GCC) |
|---|---|---|
| compile | `/fsanitize=address` | `-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS` |
| link | `build/asan/asan_entry.obj` (below); `-AsanSymbols` adds `/DEBUG:FULL` | `-fsanitize=address,undefined` |
| runtime | `/MD`; `clang_rt.asan_dynamic-x86_64.dll` from the MSVC toolset's `bin/Hostx64/x64`, which `test.ps1 -Asan` puts first on `PATH` (`scripts/dev/msvc-asan-runtime.ps1`) | `-static-libstdc++` as the normal build; `libasan` / `libubsan` from GCC |
| debug info | `/Z7` (line tables) | `-g1` (`dev-lines`) |
| leaks | not checked (MSVC ASan has no LeakSanitizer) | LeakSanitizer, on |

`_GLIBCXX_DEBUG` is not used: it changes the layout of the standard
containers, and the system libraries a test links (HarfBuzz, FreeType, ...)
are not built with it. A sanitizer report fails the test program (non-zero
exit). `ASAN_OPTIONS` and `UBSAN_OPTIONS` given by the caller are kept;
`test.sh` sets defaults otherwise. The default leaves `strict_string_checks` off:
with it, `xkb_x11_keymap_new_from_device` of libxkbcommon-x11 1.13.2
(Arch) reads an XCB reply string without a terminating NUL, a
heap-buffer-overflow report inside that system library on every X11 test
(reproduced by a 15-line C program; cgpui passes it nothing but the
connection).

On Windows every ASan program begins at `scripts/dev/asan_entry_windows.cpp`:
the workspace is compiled with `/Dmain=cgpui_asan_main`, and that file's
`main` calls the program's own, runs `_cexit()` (atexit handlers, static
destructors) and ends the process with `TerminateProcess` and the same exit
code. Without it a GPU test can hang forever in `ExitProcess`: Windows
terminates the other threads first, and when a GPU driver thread (Intel
`igvk64`, NVIDIA `nvgpucomp64`) held a lock of the ASan runtime's allocator,
the next `free()` of the DLL detach waits on it, and the process cannot be
killed until reboot (1 of 8 parallel copies of `vulkan_frame_lifetime_test`
within 10 rounds, with `windows_hook_legacy_allocators` on and off). Only the
DLL detach is skipped, and Windows ASan checks nothing there (no
LeakSanitizer). The normal build does not use the entry.

`test.sh --asan` also sets `VK_LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING=1`
(a Vulkan driver unloaded before exit leaves leak reports with unnamed
frames) and reads `scripts/dev/lsan-suppressions.txt`, which suppresses
leaks inside Mesa's Vulkan drivers only: `libvulkan_lvp.so` and
`libvulkan_dzn.so` (Mesa 26.1.6) keep 96 bytes per Wayland event queue
from `wl_display_read_events` in their WSI after
`vkQueuePresentKHR` / `vkDestroySwapchainKHR`. A suppression names a
system library and the report it covers; cgpui code is never suppressed.

No test is skipped under ASan: every test program passes as it is. `asan_workspace.py` defines `CGPUI_ASAN` in these builds, for a
future test that cannot run instrumented; such a test must print why it is
skipped, and the skip must be listed here.

A full Linux ASan build of every test program needs more disk than a small
WSL disk has. `scripts/dev/test.sh --asan --per-member-cleanup` runs one
member with tests at a time and deletes that member's test programs and
objects after it (the libraries stay built); `CGPUI_ASAN_STOP_USED_GB=<n>`
stops before the next member once the file system uses more than `n` GB.
