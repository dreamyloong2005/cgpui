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
  before. SDK packages: `/MD` in both modes; mcpp has no debug CRT (`/MDd`).
- xkbcommon: system library instead of the xmake source build.
- Builds and tests run in parallel; the `-j 1` convention is gone.

## mcpp issues met

- MSVC localized `/showIncludes` => no header dependencies (upstream #970):
  wrappers set `VSLANG=1033`.
- A `build.mcpp` is not rebuilt when a header it includes by relative path
  changes: repro and draft issue in `D:/Dev/shots/mcpp-spike/bug-buildmcpp-header`;
  workaround: the header is a `[build-dependencies]` path package.
