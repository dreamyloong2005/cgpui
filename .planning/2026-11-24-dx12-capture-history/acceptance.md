# Capture history acceptance

2026-11-24. Baseline `58e16f71`, branch `phase-v`. User explicitly chose the
existing Vulkan-compatible history contract. Second frozen matrix accepted;
overall DX12 goal remains active. No push/release/default switch.

## Implemented

DX12 preserves the last successful requested capture through uncaptured success,
input rejection and abandonment on an unchanged valid surface/device. Fresh
renderers remain null; a later successful requested capture replaces history.
Frame no longer pre-clears pixels; successful submission only moves an engaged
candidate. Readback/debug/Commit/diagnostic completion still precede publication.
Existing extent-change resize, recovery and recovery-pending invalidation remain.
Public leaf comments and guide describe support and borrowed-storage boundaries.

No orphan/retention/image-batch/recovery/fallback/Popup policy change. Custom
renderers opt in; no base no-op capture promise. No native Mac verification claim.

## Root evidence and independent review

Actual shared A: Vulkan/Intel passes; DX12/WARP succeeds through requested red
capture and uncaptured green Present, then exit8 for lost history. B same exact
test passes both. Native cold/warm late InfoQueue regression passes, preserving
deep old capture in warm case, null in cold case, exact one error and stable
live device/generation. Early invalid blur cases preserve old pixels/diagnostics.

Full structure228/native136 WARP-debug pass. Actual-source independent Standards
and Spec/native-safety reviews0, existing caps preserved. SDK history addition
source-reviewed0 and both installed runs passed. See `review.md` for review scope.

## Frozen matrix

Accepted evidence: `D:/Dev/shots/cgpui-dx12-capture-history-20261124/verified`.
Driver exit0; exact inventory audit and raw Windows/normalized WSL source checks
pass for2998 paths. Root ran serial jobs4 no-run/jobs1 execution; full graph
refreshed before SDK closure. Installed consumer preserves orphan/retention/native
pixels and verifies no-capture/rejected same-epoch deep history in both runs.

| Gate | Result |
| --- | --- |
| Windows workspace / all-member build |847 passed,0 failed / passed |
| Dedicated WSL mirror / all-member build |671 passed,0 failed / passed |
| Architecture |228 passed, existing caps unchanged |
| Intel / NVIDIA native + debug |136 passed each |
| WARP native + debug + GPU validation |136 passed |
| Shared suite requested DX12 Intel / NVIDIA |97 passed each |
| WARP shared Vulkan then DX12 |97+97 passed |
| ASan WARP native / shared both |136 /97+97 passed |
| PS7 / Windows PS5 driver fixtures |17 /15 passed,2 explicit PS7-only skips |
| SDK graph / header fixtures |9 /15 passed |
| Installed Windows consumer |actual /MT compile/link/run, opaque/Popup/client-alpha pixels |
| Installed capture history |uncaptured and rejected retain deep history, same epoch, both runs |
| Installed orphan / retained payload |one-shot orphan completion; known4 then3076, preserved |
| Vulkan isolation |owned invalid-DLL run passes, no Vulkan import/load |
| CMake consumer |configure-only passed, not CMake binary/release build |
| Source consistency |2998 raw Windows and normalized WSL paths match |

Shared97 is a mixed suite, not97 distinct DX12 pixel scenes. Native136 includes
CPU and controlled native seams. SDK libraries are mutable hardlinks, not an
immutable release. WSL does not certify native Mac or Windows GPU behavior.

### First-run failure retained, not declared repaired

Original evidence remains in `final/`: Windows846pass/1fail, ordinary native-menu
cancel exit4. The opaque reference timed out before constructing a DX12 renderer,
with BEGIN but no OPEN logged. One isolated replay plus five bounded unchanged
replays passed. No source, timeout, oracle or skip policy was changed. The first
and second frozen manifests have identical SHA256 (rerun-source.json).

The complete second matrix passed, but this does not establish or fix the menu
timeout root cause. Existing logging cannot distinguish discovery/highlight/
pixel-stability waits; this remains a native-menu fixture reliability gap.
No first-run-all-green or fixed-menu claim is made.

## Remaining work

Same-frame final-ID image batching is separately user-approved and has reviewed
external A/B candidates, not implemented here. DRED, broader window/menu/background
transitions, cross-thread recovery, work/performance optimizations/measurements,
actual CI and release/default gates remain. This is not whole-DX12 completion.
