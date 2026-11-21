# xmake -> mcpp migration (branch `mcpp-migration`)

`xmake-targets.json` is the inventory of every xmake target at `31607247`
(`scripts/migration/build_inventory.py`); `mcpp-target-map.json` maps each
of them to its mcpp counterpart (`scripts/migration/reconcile.py`). The
merge of `origin/master` (`19c51989`) added its two new targets (its
`build/xmake/phase_j_macos_targets.lua`, recorded as `merged_targets`; the
file itself is not kept).
All 676 targets map; none is dropped. Test programs: Windows 581 (xmake: 581
test entries), Linux 561 (561). macOS manifests exist but were not built; see
[macOS follow-up](#macos-follow-up).

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

Windows ASan hang in process exit (fixed):
`vulkan_frame_lifetime_test` ran past its timeout in 1 of 8 parallel ASan
copies, in the first round each time it was measured (an earlier note here
said "40 runs of 8 copies"; that loop had stopped in its first round).
Stacks of the live process (cdb): the main thread in the DLL detach after
`common_exit`, in a `free()` from `ucrtbase!destroy_fls`, `nvspcap64` or
`nvgpucomp64`, waiting on a lock of the ASan allocator
(`__asan_win_moveable::IsOwned`, `Quarantine::Drain`) held by a driver
thread that `ExitProcess` had terminated. The normal build never hung (40
runs). `windows_hook_legacy_allocators=false` moved the wait to the
quarantine lock (round 10 of 20). Fixed by `scripts/dev/asan_entry_windows.cpp`,
which ends ASan programs without the DLL detach (docs/getting-started.md
section 7).

## macOS follow-up

No Mac was available, so nothing below has been built or run with mcpp.
The last macOS evidence was produced with xmake: Phase H (380/380) and the
Phase J macOS completion of `origin/master` (`19c51989`, arm64 and x86_64
at deployment target 13.0), merged after the migration.

To do, in order:

1. **mcpp on the Mac.** mcpp releases macOS builds only for Apple silicon
   with macOS 14 or later (mcpp README). `[workspace.toolchain]` in
   `mcpp.toml` has no `macos` entry: check which toolchain mcpp picks and
   add one (Apple clang from Xcode) if it is not that.
2. **Build the workspace.** `mcpp build` on macOS. The manifests written
   for it but never built:
   - `packages/platform-macos` (Cocoa, `.mm` sources; frameworks AppKit,
     CoreText, UniformTypeIdentifiers, QuartzCore);
   - `packages/renderer-metal` (frameworks Metal, QuartzCore). Its
     `build.mcpp` embeds `src/renderer/metal/shaders.metal` and compiles it
     with `xcrun metal` / `metallib`; a failure there is only a warning,
     so read the build output for it;
   - `packages/platform` (`macos_clipboard.mm`, AppKit), `packages/app`,
     `examples/hello_window` (`macos_*` cases) and the `cfg(os = "macos")`
     sections of `tests/platform`, `tests/renderer`, `tests/ui`,
     `tests/api_parity`, `tests/architecture` and the other examples;
   - vendored `third_party/lunasvg` and `plutovg` (built so far on Windows
     and Linux only).
3. **Test count.** The xmake inventory (`xmake-targets.json`, `31607247`)
   has 487 macOS targets with 465 test entries (463 at `31607247`, plus
   `phase_j_macos_completion_test` and
   `phase_j_macos_completion_structure_test` from `19c51989`). Run
   `mcpp test --list` on the Mac and
   `python scripts/migration/reconcile.py --listed macosx=<file>`; the count
   of mcpp test programs must be 465, as it is 581 / 561 on Windows / Linux.
4. **Run the tests.** `scripts/ci/macos-debug.sh` (`mcpp build`, `mcpp test`)
   and `scripts/ci/macos-example-smoke.sh` were rewritten for mcpp and
   never run. The Phase J macOS scripts merged from `19c51989` were ported
   the same way and never run: `macos-dependencies.sh` (exports `MCPP`,
   `MCPP_HOME`, `TMPDIR`; the xmake-repo pin is gone), `macos-package.sh`
   (sdk-package.py workspace, `mcpp build --profile`; the package's
   `metadata/` holds `mcpp.toml` and `VENDORED.md` instead of
   `xmake-requires.lock`; it reads the one `target/*/*/` build directory,
   whose triple name on macOS is unverified), `macos-phase-j-release.sh`,
   `macos-performance-baseline.sh` and `macos-stress-matrix.sh`
   (`mcpp test --release -p tests/performance|stress -- <runner args>`) and
   `macos-architecture-header.sh` (`mcpp test --list` against
   `architecture-header-targets.txt`). They rely on mcpp reading
   `MACOSX_DEPLOYMENT_TARGET=13.0` (mcpp docs 04, `macos_deployment_target`;
   mcpp's own default is 14.0): check `LC_BUILD_VERSION minos` of a built
   program with `vtool -show`. The xmake smoke script also ran
   `phase_f_multi_window_lifecycle_churn_test/default`, which is no xmake
   target (only `phase_f_multi_window_lifecycle_churn_structure_test` and
   `window_runtime_multi_window_churn_test` exist); the port leaves it out. `scripts/dev/build.sh` / `test.sh` and `mcpp-env.sh` were
   written for Linux; check them on macOS or add macOS wrappers.
5. **CI.** `.github/actions/setup-mcpp` has Windows and Linux steps only;
   add a macOS step (arm64 and x86_64: `phase-j-desktop.yml` runs both, on
   `macos-15` and `macos-15-intel`, and mcpp releases macOS builds for Apple
   silicon only), then the jobs of `.github/workflows/phase-h-macos.yml` and
   `phase-j-desktop.yml` can run.
6. **Sanitizers.** `scripts/dev/asan_workspace.py` has flags for `windows`
   and `linux` only. Add a `macos` row (`-fsanitize=address,undefined`,
   Apple clang) if macOS should get ASan runs. The Windows ASan entry
   (`scripts/dev/asan_entry_windows.cpp`) is Windows-only and not needed.
7. **SDK packages.** `scripts/ci/sdk-package.py` accepts `--platform
   windows|linux` only; there is no macOS package or consumer smoke test.
   Decide whether macOS ships one.
8. **Docs.** Once the Mac runs pass, replace the xmake-era evidence in
   `docs/platform-mac-readiness.md` and `docs/platform-dependencies.md`
   (the Phase H closeout tests check some of those sentences; keep them or
   update the tests with them) and remove the "planned only, not built"
   comments in the macOS manifests.
