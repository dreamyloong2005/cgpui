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

## Revalidation still required after source or toolchain changes

The recorded evidence predates the workspace's mcpp 2026.11.21.1 requirement
and shared `cgpui.build` module. Re-run native build, Debug, architecture/header,
example smoke, sanitizer, Release consumer, and performance/stress entrypoints
with the current toolchain before claiming a new macOS verification result.
Record the commit, toolchain versions, OS, deployment target, and limitations.

The historical mcpp host-build-program linker required `ld64.lld` on `PATH`.
LLD 22.1.2 rejected Xcode 27 SDK `arm64e.x1` targets; the recorded workaround
used a launcher delegating to `/usr/bin/ld`. Treat this as a historical
toolchain constraint to recheck, not a universal requirement of newer mcpp.
