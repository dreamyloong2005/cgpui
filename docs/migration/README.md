# xmake -> mcpp migration (branch `mcpp-migration`)

`xmake-targets.json` is the inventory of every xmake target at `31607247`
(`scripts/migration/build_inventory.py`); `mcpp-target-map.json` maps each
of them to its mcpp counterpart (`scripts/migration/reconcile.py`). All 674
targets map; none is dropped. Test programs: Windows 579 (xmake: 579 test
entries), Linux 559 (559). macOS manifests exist but were not built.

## Layout

| xmake | mcpp |
|---|---|
| `cgpui_<lib>` static targets | `packages/<lib>/mcpp.toml`, sources in `src/` in place |
| OS-specific backends (`platform_win32`, `platform_linux_*`, `renderer_vulkan`, macOS) | same members, sources in `[target.'cfg(os = ...)'.build]`; `tools/mcpp/empty_library.cpp` on the other OSes (mcpp refuses an empty archive) |
| `add_requires("lunasvg")` (+ plutovg) | `third_party/{lunasvg,plutovg}` vendored, `VENDORED.md` |
| `vulkansdk`, system packages | `build.mcpp` per member: `VULKAN_SDK` (Windows), pkg-config (Linux), via `tools/mcpp/cgpui_build.hpp` (a `[build-dependencies]` package) |
| examples | `examples/*/mcpp.toml` bin members, same target names |
| `hello_window` test cases with run envs | `examples/hello_window/tests/<case>.cpp`, one program per case |
| test targets | `tests/<dir>/mcpp.toml`, one member per directory, every test in `[test] discover` |
| `rundir` / `CGPUI_SOURCE_ROOT` run env | `CGPUI_SOURCE_ROOT` define from each suite's `build.mcpp` |
| `phase_g_test_runner_test` (2 env cases) | `test_runner_test` + `test_runner_invalid_environment_test` |
| structure tests reading `xmake.lua` / `build/xmake` | read the manifests that state the same registration |

## Behaviour differences

- `src/ui/text_font.cpp` was compiled into both `cgpui_platform` and
  `cgpui_renderer`; mcpp links dependencies' objects (not archives), so it
  is compiled once, by `packages/platform`, and `packages/renderer` depends
  on `packages/platform`.
- Windows dev/test builds: `/MT` (`[workspace.build] cxx_runtime`), as
  before. SDK packages: `/MD` in both modes; mcpp has no debug CRT (`/MDd`,
  upstream #718 closed by design). The debug package's debug information is
  in its libraries (`/Z7`): no `.pdb` files, the consumer's PDB receives it
  (verified, `docs/sdk-packaging.md`, Debug information).
- The MSVC debug-CRT checks (`_ITERATOR_DEBUG_LEVEL=2`, debug heap, leak
  report) are replaced by AddressSanitizer runs: `scripts/dev/test.ps1
  -Asan`, `scripts/dev/test.sh --asan` (`docs/getting-started.md` section 7).
  They build a generated workspace in `build/asan` because profile flags
  reach only the selected member's own units.
- xkbcommon: system library instead of the xmake source build.
- Builds and tests run in parallel; the `-j 1` convention is gone.

## mcpp issues met

- MSVC localized `/showIncludes` => no header dependencies (upstream #970):
  wrappers set `VSLANG=1033`.
- A `build.mcpp` is not rebuilt when a header it includes by relative path
  changes: repro and draft issue in `D:/Dev/shots/mcpp-spike/bug-buildmcpp-header`;
  workaround: the header is a `[build-dependencies]` path package.

## AddressSanitizer findings (first runs)

Fixed in cgpui:

- `src/ui/app_runner.cpp`: `run_app` scoped its `AppContext` to the
  `setup_context` branch while callbacks installed there used it during
  `run()` (stack-use-after-scope, Windows ASan, `app_runner_test`).
- `src/ui/runtime_context.cpp`: `WindowRuntime::context()` bound the
  context's window and renderer references to null pointers before
  `run()` (UBSan; `async_spawn_result_conventions_test`,
  `animation_cancellation_test`).
- `tests/platform/wayland_test_compositor.cpp`: the test compositor leaked
  its clients (LeakSanitizer, 25 Wayland tests).

System libraries (not patched, see `docs/getting-started.md` section 7):
libxkbcommon-x11 1.13.2 under `strict_string_checks` (option left off) and
Mesa 26.1.6 Vulkan driver leaks (`scripts/dev/lsan-suppressions.txt`).
Vendored lunasvg / plutovg: no report. No test is skipped under ASan.

Open: `vulkan_frame_lifetime_test` (Windows) hung once in a full parallel
ASan run (300 s timeout) and once in 40 runs of 8 parallel ASan copies;
0 hangs in 24 ASan runs of 4 copies and 40 runs of 8 normal-build copies.
The hung process has one thread in a kernel wait with the NVIDIA Vulkan
driver loaded and cannot be terminated, so the wait is inside the driver;
not diagnosed further (no kernel debugger on the machine).
