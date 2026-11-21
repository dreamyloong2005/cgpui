# xmake -> mcpp migration (branch `mcpp-migration`)

`xmake-targets.json` is the inventory of every xmake target at `31607247`
(`scripts/migration/build_inventory.py`); `mcpp-target-map.json` maps each
of them to its mcpp counterpart (`scripts/migration/reconcile.py`). The
merge of `origin/master` (`19c51989`) added its two new targets (its
`build/xmake/phase_j_macos_targets.lua`, recorded as `merged_targets`; the
file itself is not kept).
All 676 targets map; none is dropped. At migration time Windows listed 581
test programs and Linux 561, matching xmake. On arm64 macOS, mcpp now lists
466: the 465 migrated entries plus `shader_source_structure_test`. Listing
does not run the tests; see [macOS follow-up](#macos-follow-up).

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

- MSVC localized `/showIncludes` => no header dependencies, and mcpp
  crashed on GBK diagnostics (upstream #970, reported 2026-11-19). The
  wrappers set `VSLANG=1033` until the fix: mcpp 2026.11.19.1 takes cl.exe's
  inputs from `/sourceDependencies` and decodes the compiler's text in the
  console code page. Checked with 2026.11.20.1 on the reference machine
  (zh-CN Build Tools, `VSLANG=2052`): a header edit rebuilds and a localized
  C5050 warning reaches the log as valid UTF-8. The workspace requires
  2026.11.20.1 (`mcpp.toml`) and the wrappers no longer set `VSLANG`.
- A `build.mcpp` is not rebuilt when a header it includes by relative path
  changes (upstream #991, reported 2026-11-21; reproduced with 2026.11.20.1
  on Windows and Linux). Workaround, kept until it is fixed: the header is a
  `[build-dependencies]` path package (`tools/mcpp`).

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

Only native arm64 is supported for macOS builds, CI, and the Release SDK.
Intel (x86_64) and Universal SDKs are not deliverables or sign-off requirements.

The minimum macOS was raised from 13.0 to 26.0 on 2026-11-21, because no
macOS 13 machine was available for runtime testing. CI runs on `macos-26`
runners, so the minimum is now tested at runtime. The evidence below was
recorded at 13.0 and has not been re-run at 26.0.

The pre-migration macOS evidence was produced with xmake: Phase H (380/380) and the
Phase J macOS completion of `origin/master` (`19c51989`, arm64 and x86_64
at deployment target 13.0), merged after the migration.

Native arm64 evidence (2026-11-21) and remaining work:

1. **mcpp on the Mac.** The arm64 check used mcpp 2026.11.20.1 on macOS
   27.2.1 (26B131), Xcode 26.6 (17F113). The workspace selects Xcode's
   compiler through `/usr/bin` and enables ARC for its Objective-C++ sources.
   This mcpp version also invokes `clang++ -fuse-ld=lld` for the generated
   **host build programs**, even when the workspace specifies Apple's linker.
   Put a real `ld64.lld` on `PATH` for them. If Rust is installed, one option is
   `export PATH="$(rustc --print sysroot)/lib/rustlib/$(rustc -vV | sed -n 's/^host: //p')/bin/gcc-ld:$PATH"`.
   With Xcode 27 this does not work: its SDK `.tbd` files list an
   `arm64e.x1` target that LLD 22.1.2 (rust-lld) rejects as an unknown
   architecture, so a host build program that has to be relinked fails.
   An `ld64.lld` on `PATH` that runs `exec /usr/bin/ld "$@"` links them
   with Apple's linker instead (used for the 2026-11-21 runs).
2. **Workspace build: passed.** All 33 members build in Debug, including
   the Cocoa and Metal Objective-C++ files and vendored LunaSVG and plutovg.
   Xcode's `ar` does not expand mcpp's `@objects.rsp`; the workspace selects
   `scripts/dev/macos-ar.py` to expand it before invoking the native archiver.
   The shared backend-selection test needs `linux_backend_selection.cpp` in
   macOS test support, just as on Windows; it has no Linux system dependencies.
3. **Test count.** The xmake inventory (`xmake-targets.json`, `31607247`)
   has 487 macOS targets with 465 test entries (463 at `31607247`, plus
   `phase_j_macos_completion_test` and
   `phase_j_macos_completion_structure_test` from `19c51989`). Run
   `mcpp test --list > macos-tests.txt` on the Mac and
   `python3.11 scripts/migration/reconcile.py --listed macosx=macos-tests.txt`
   (Python 3.11 or newer). Verified on arm64: 466 programs, including
   `shader_source_structure_test` added after migration. The checker accounts
   for that test separately; the historical xmake count stays 465. Tests
   added later are listed in `POST_MIGRATION` with the platforms that build
   them.
4. **Debug and example smoke: passed.** `scripts/ci/macos-debug.sh` reports
   466 passed, 0 failed. With `MACOSX_DEPLOYMENT_TARGET=13.0`,
   `scripts/ci/macos-example-smoke.sh` runs 21 examples and 12 focused tests
   (8 native platform/renderer tests and 4 window lifecycle modes). The `hello_window`
   executable under `target/aarch64-macos/` reports `LC_BUILD_VERSION minos
   13.0` in `xcrun vtool -show-build`. A macOS 13 runtime is unavailable:
   keep the deployment-target check, but do not require a macOS 13 run
   for sign-off or claim runtime compatibility from the Mach-O metadata.
5. **Hosted arm64-only CI: 6/6 passed at `d5013518`.**
   [Phase H](https://github.com/dreamyloong2005/cgpui/actions/runs/44880423975)
   passed full Debug and example smoke.
   [Phase J](https://github.com/dreamyloong2005/cgpui/actions/runs/44880423908)
   passed Debug/examples, architecture/headers, Release SDK and performance/stress;
   no Intel job was scheduled. Both the
   [arm64 Release SDK](https://github.com/dreamyloong2005/cgpui/actions/runs/44880423908/artifacts/14697923104)
   and the [performance/stress reports](https://github.com/dreamyloong2005/cgpui/actions/runs/44880423908/artifacts/14697961504)
   were uploaded for that commit.

   At `5385aaaa`, [Phase H](https://github.com/dreamyloong2005/cgpui/actions/runs/44876227518)
   and all four arm64 jobs in [Phase J](https://github.com/dreamyloong2005/cgpui/actions/runs/44876227613)
   passed, but Phase J still failed overall because the pinned mcpp release had
   no native Intel binary for its four Intel jobs.

   On `f56ce548`, the
   [Phase H run](https://github.com/dreamyloong2005/cgpui/actions/runs/44864775450)
   installed mcpp on arm64, but the examples step exited 3. The workflow
   did not set `MACOSX_DEPLOYMENT_TARGET`; it now sets `13.0` for both jobs.
   In the [Phase J run](https://github.com/dreamyloong2005/cgpui/actions/runs/44864775416),
   arm64 annotations showed the missing ARC flag and the unsupported
   `mcpp test --release` option.
6. **Sanitizers: passed on native arm64.** `scripts/dev/asan_workspace.py`
   has a `macos` row (Apple clang, `-fsanitize=address,undefined`, libc++
   extensive hardening) and `scripts/dev/test.sh --asan` selects it on macOS.
   On 2026-11-21 (macOS 27.2.1, Xcode 27.2, deployment target 26.0) it
   passed 466/466 with no sanitizer report. Leaks are not checked: Apple's
   ASan has no LeakSanitizer on arm64. The Windows ASan entry
   (`scripts/dev/asan_entry_windows.cpp`) is Windows-only and not needed.
7. **Phase J Release SDK: passed on native arm64.** The original archive
   passed its hash checks but a standalone consumer could not include
   `cgpui.hpp` or link `rasterize_svg`. The package now includes the 16
   transitive `src/ui` headers and all nine link archives, including LunaSVG
   and plutovg. `scripts/ci/macos-phase-j-release.sh` compares two archives
   of the same staged tree, checks entries against the release manifest,
   then extracts the archive outside the checkout and builds its CMake
   sample. The sample checked an SVG pixel and presented a Cocoa/Metal
   frame; a separate xmake consumer run also passed. The demo and consumer
   report `minos 13.0` and only system dynamic dependencies. This was a
   local working-tree build, not a published C10 SDK or a run on macOS 13.
   The [hosted arm64 Release job](https://github.com/dreamyloong2005/cgpui/actions/runs/44876227613/job/136761411061)
   also passed on `5385aaaa` and uploaded `cgpui-macos-arm64-release`.
   See the [SDK instructions](../macos-sdk.md).
8. **Metal shaders from Slang.** All five generated sources compile with
   Apple's `metal -c`. The four runtime pipelines initialize; the bootstrap,
   primitive, clip/transform and text/image pixel tests pass on this Mac.
   The generated vertex shader still warns about a writable device buffer.
   This run does not verify Retina scaling across displays.
9. **Phase J architecture, performance and stress: passed on native arm64.**
   `macos-architecture-header.sh` matched the checked-in target list and
   passed 183 architecture tests and 31 public-header tests (214 total).
   `macos-performance-baseline.sh` passed all eight metric budgets at the
   default 64 iterations; `macos-stress-matrix.sh` completed all eight
   scenarios at scale 1. Both produced fresh `macos-arm64.json` reports
   accepted by the repository validators. With mcpp 2026.11.20.1 both
   runners require `mcpp test --profile release`, not `mcpp test --release`.
   The timings are regression measurements using `TestApp`, not native
   Cocoa startup or Metal presentation latency.
