# macOS Validation

## Current target and verification boundary

The supported target is Cocoa + Metal on Apple Silicon (arm64), with
`MACOSX_DEPLOYMENT_TARGET=26.0`. Intel and Universal SDKs are not deliverables.
CI is configured for `macos-26`; that configuration alone is not evidence
that the current checkout has passed on that operating system.

Use the current [dependencies](platform-dependencies.md),
[SDK instructions](macos-sdk.md), and `scripts/ci/macos-*.sh` entrypoints.
Test registration belongs to `tests/*/mcpp.toml`, not a retired build-system
inventory.

## Native validation (2026-11-25)

Apple M4, macOS 27.2.1 (26B131), Xcode 27.2 (27B52), Apple clang 21.0.0,
mcpp 2026.11.21.1, `MACOSX_DEPLOYMENT_TARGET=26.0`, CMake 4.5.1. Source:
`fc53fc9e` plus the fixes below, committed together with this record.

Fixes found by this run:

- Apple clang leaves C++20 module syntax off, so the shared `cgpui.build`
  module (`tools/mcpp/cgpui_build.cppm`) and every `build.mcpp` that imports
  it failed to compile. The macOS toolchain prefix is now `tools/macos-clang`,
  whose `clang++` runs `/usr/bin/clang++ -fcxx-modules`; mcpp's build-program
  sub-build receives only the toolchain path, so the flag lives in the driver
  (as `tools/linux-gcc/bin/g++` does on Linux). The SDK and ASan workspace
  generators rewrite the prefix path. `tests/ci/macos_clang_toolchain_test.py`
  checks the three workspaces and builds a `build.mcpp` that imports a module.
- `runtime_async_io_shutdown_reentry_test` crashed (SIGSEGV): runtime
  shutdown retired task callbacks with `std::move`, which libc++ implements
  as a copy for small `std::function` targets, so the moved-from slot kept
  its captures and destroyed them later against the dismantled runtime.
  Callback retirement now uses `std::exchange(callback, {})` (shutdown,
  async I/O cancellation, task completion and cancellation).

| Check | Command | Result |
|---|---|---|
| Shared renderer after DX12 extraction | `scripts/dev/build.sh -p packages/renderer`; `scripts/dev/test.sh -p tests/renderer-neutral`, `-p tests/header_cleanliness`, `-p tests/architecture renderer_planning_ownership_structure_test` | exit 0; 16, 47 and 1 passed, as expected; no skips |
| Debug | `scripts/ci/macos-debug.sh` | 39 members, 632 passed, 0 failed |
| Example smoke | `scripts/ci/macos-example-smoke.sh` | passed (12 test programs) |
| Architecture and headers | `scripts/ci/macos-architecture-header.sh` | 261 + 47 = 308 passed |
| ASan / UBSan | `scripts/dev/test.sh --asan` | 632 passed, no sanitizer report (no LeakSanitizer on arm64) |
| Performance | `scripts/ci/macos-performance-baseline.sh` | eight metrics at 64 iterations, passed |
| Stress | `scripts/ci/macos-stress-matrix.sh` | eight scenarios at scale 1, passed |
| Release SDK | `scripts/ci/macos-phase-j-release.sh arm64` | two archives byte-identical (sha256 `d523a40b...`, before the schema 4 change), `hello_window` and `sdk_consumer` arm64 `minos 26.0` with system dependencies only, CMake consumer built outside the checkout, SVG pixel checked, `sdk_consumer: first frame presented` |

Not covered: a macOS 13-25 runtime (the deployment target is 26.0),
cross-display Retina scaling (one 1x display), the hosted `macos-26` CI
jobs, and LeakSanitizer. Performance and stress use `TestApp` timings.

## Recorded native evidence (2026-11-21)

These results were recorded before the current checkout. They are retained
as historical evidence, not a fresh certification of subsequent changes.

- Native arm64 Debug and example smoke passed with mcpp 2026.11.20.1:
  466 tests, 21 examples, and 12 focused platform/renderer/lifecycle modes.
  The recorded machine used macOS 27.2.1 and Xcode. The original Debug/SDK
  runs used deployment target 13.0; Mach-O `minos 13.0` did not establish
  runtime compatibility with macOS 13, which was unavailable.
- Hosted arm64 CI passed six jobs at `d5013518`:
  [Phase H](https://github.com/dreamyloong2005/cgpui/actions/runs/44880423975)
  and [Phase J](https://github.com/dreamyloong2005/cgpui/actions/runs/44880423908).
  The latter uploaded the
  [Release SDK](https://github.com/dreamyloong2005/cgpui/actions/runs/44880423908/artifacts/14697923104)
  and [performance/stress reports](https://github.com/dreamyloong2005/cgpui/actions/runs/44880423908/artifacts/14697961504).
- A separate native arm64 sanitizer run used macOS 27.2.1, Xcode 27.2,
  deployment target 26.0, and passed 466/466 with ASan/UBSan and libc++
  hardening. Apple's arm64 ASan did not check leaks; no LeakSanitizer
  validation is claimed.
- Release SDK validation compared deterministic archives, checked manifest
  hashes, extracted outside the checkout, and built the CMake consumer.
  The consumer checked an SVG pixel and presented a Cocoa/Metal frame.
  These local results were not a published SDK or a macOS 13 runtime run.
- The architecture/header matrix passed 183 + 31 tests. Performance passed
  eight budgets at 64 iterations; stress passed eight scenarios at scale 1.
  Those timings used `TestApp`, not native startup/presentation latency.
- Generated Slang-to-Metal shaders compiled and primitive, clip/transform,
  text/image pixel tests passed. Cross-display Retina scaling was not verified.

The earlier Phase H 380/380 and Phase J `19c51989` results used xmake and
deployment target 13.0. They must not be substituted for current mcpp runs.

## Revalidation after source or toolchain changes

Re-run native build, Debug, architecture/header, example smoke, sanitizer,
Release consumer, and performance/stress entrypoints with the current
toolchain before claiming a new macOS verification result. Record the
commit, toolchain versions, OS, deployment target, and limitations.

The 2026-11-25 run still put an `ld64.lld` launcher delegating to
`/usr/bin/ld` on `PATH`: mcpp's host build programs ask for it, and LLD
22.1.2 rejects Xcode 27 SDK `arm64e.x1` targets. Recheck with newer mcpp.
