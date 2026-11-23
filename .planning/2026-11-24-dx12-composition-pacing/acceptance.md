# Composition admission acceptance

2026-11-24. Baseline `4d29c4e5`, branch `phase-v`. This slice is accepted;
the overall DX12-completion goal remains active. No push or release publication.

## Implemented

- One focused private constexpr flags owner for opaque HWND and composition
  creation plus same-chain ResizeBuffers. Pacing configuration derives from the
  actual descriptor flag. Resize has a focused implementation and never acquires
  another latency handle or resets a cached grant.
- Nonblocking composition admission, including the first frame before any native
  submission/Present/Commit. Native failure remains failure; no flags-zero retry,
  unpaced success, public window mode or altered Popup/backend selection policy.
- Actual hidden/visible Renderer first-begin RED/GREEN, cold/granted/post-Present
  real resize, abandon/validation cache, removal/replacement and both orphan
  destruction orders. Existing desktop Popup tests additionally observe exact
  failed-construction, peer purge/lazy rebuild and destroyed-HWND handle epochs.
- Explicit experimental native bin remains outside test discovery. Its six
  capability rows are separate evidence, not production recovery/performance proof.
- Live docs describe both enabled routes, cached-grant zero-poll behavior, honest
  CPU timing and unchanged ordinary-window/menu limitations. Three new structure
  guards preserve existing renderer/platform/manifest caps and exact CI discovery.

## Final frozen matrix

Final evidence: `D:/Dev/shots/cgpui-dx12-composition-pacing-20261124/final`.
Root alone executed the serial driver; no-run jobs4, native execution jobs1.
`verify-final.ps1` exited0. `matrix-audit.json` additionally checks the exact122-name
native manifest inventory, no duplicate/omitted cases and no native/common SKIP.

| Gate | Result |
| --- | --- |
| Windows whole workspace |827 passed,0 failed; all-member build passed |
| Dedicated WSL validation mirror |667 passed,0 failed; all-member build passed |
| Architecture |225 passed, including unchanged existing caps |
| Intel / NVIDIA DX12 suite + debug |122 passed each |
| WARP DX12 suite + debug + GPU validation |122 passed |
| Shared renderer suite, requested DX12 on Intel / NVIDIA |94 passed each |
| WARP shared suite, Vulkan then DX12 |94+94 passed |
| ASan WARP DX12 / shared both |122 and94+94 passed |
| Driver fixtures, PowerShell7 |17 passed |
| Driver fixtures, Windows PowerShell5 |15 passed,2 explicitly PS7-only skips |
| SDK graph / package-header fixtures |9 and15 passed |
| Installed Windows consumer |actual /MT compile/link/run, real opaque/Popup/client-alpha captures |
| Vulkan loader isolation |no PE import/delay import, no loaded Vulkan at entry/exit; owned invalid-DLL run passed |
| CMake SDK fixture |configure only; no CMake binary build or release claim |
| Source consistency |2969 paths, raw Windows and normalized WSL matches |

The shared suite contains neutral and backend-private tests as well as the13
shared real-pixel scenes;94 is not a claim of94 distinct DX12 pixel scenes.
The native suite likewise includes controlled CPU/native seams. WSL is not native
Windows/macOS or GPU evidence. Native host build26300.9974 is recorded in the raw
probe row JSON; neither those rows nor this matrix certify all OS/driver versions.

## Earlier evidence and corrections retained

The parent evidence directory preserves A's real unpaced hidden/visible exit40,
its paced GREEN, six Intel/NVIDIA/WARP raw-probe rows and incremental native118,
121,122 plus structure225 passes. No warmup Commit/Present was used for A.

The first whole matrix's SDK verifier failed before staging because selective
test invocation replaced build.ninja and removed the workbench link edge. A
fresh all-member build restored exactly that edge; the same verifier then passed
with unchanged2969 source paths. Before/after graphs and results are retained.
The final driver fixes ordering by refreshing the production graph before SDK
closure; it did not loosen the SDK assertions or reuse a fabricated link list.

Two late documentation corrections triggered a fresh complete run in `final/`.
`refreeze-delta.json` proves only the two current docs changed between snapshots.
The final run above has no SDK retry or source mismatch. Static-library staging
uses hardlinks and is not an immutable SDK release; later builds may change it.

Independent Standards and Spec reviews, including closure of cold-resize and
partial/cohort accounting gaps, are recorded in `review.md`. No remaining
actionable finding in this slice. Historical milestone acceptance was not rewritten.

## Still open

This is not all-DX12 completion or default-switch readiness. Diagnostic/DRED and
retained-image accounting, successful capture publication, cross-backend operation
contracts, broader ordinary-window/chrome/menu/background transitions, coordinated
foreign-thread/busy recovery, performance, actual CI provisioning/execution and
release/default-switch gates remain. Prepared follow-ups are indexed in
`../2026-11-24-dx12-completion/next-slices.md`; none is accepted merely as a candidate.
