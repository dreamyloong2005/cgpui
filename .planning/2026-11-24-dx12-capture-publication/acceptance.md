# Capture publication acceptance

2026-11-24. Baseline `a51c09a8`, branch `phase-v`. This slice is accepted;
overall DX12-completion goal remains active. No push/release.

## Implemented boundary

Readback returns an owned pixel candidate from its focused private owner.
Allocation precedes Map; Unmap precedes return. Presentation keeps that value
local through debug validation, required composition Commit and diagnostic
preparation/publication; its final optional move is compile-time nonthrowing.
The single experimental direct reader follows the same native success boundary.

This explicitly strengthens DX12 failed-Present safety. Public headers did not
already promise success-only capture history. Vulkan behavior is unchanged;
DX12 still invalidates old pixels at Present start and does not restore them
after failure. No getter masking, public mode, default or fallback changes.

## Direct regression and review

Actual WARP/debug device, both cold and warm cases: before fix, raw Present
rejects the exact injected late InfoQueue error yet exposes candidate pixels
(both exit18). After fix both pass (result0); device remains live, generation
and successful diagnostic history remain unchanged. Every native ERROR and
CORRUPTION is inspected before clearing, permitting exactly one injected error.
This reproduces late debug rejection, not actual Commit failure or hardware TDR.

Expanded structure226 and native123 WARP/debug passed. Rebuilt explicit native
composition probe passed all six Intel/NVIDIA/WARP x hidden/visible rows,
including real readback after resize. Hidden rows do not claim desktop pixels.
Probe results are capability experiments on host build26300.9974, not production
recovery/performance or arbitrary OS/driver certification.

Independent Standards and Spec/native-safety final reviews: 0 remaining findings;
see `review.md`. Existing entry/module caps remain unchanged.

## Frozen matrix

Evidence: `D:/Dev/shots/cgpui-dx12-capture-publication-20261124/final`.
2971 source paths, excluding planning records/user `.vscode`/builds. Root runs
the matrix serially: no-run jobs4, execution jobs1. The driver refreshes the
full production graph immediately before installed SDK dependency extraction.

`verify-final.ps1` completed exit0. `matrix-audit.json` checks exact native and
shared inventories, no duplicate/omitted cases and no native/common SKIP.

| Gate | Result |
| --- | --- |
| Windows workspace / all-member build |829 passed,0 failed / passed |
| Dedicated WSL mirror / all-member build |668 passed,0 failed / passed |
| Architecture |226 passed, caps unchanged |
| Intel / NVIDIA native + debug |123 passed each |
| WARP native + debug + GPU validation |123 passed |
| Shared suite, requested DX12 Intel / NVIDIA |94 passed each |
| WARP shared Vulkan then DX12 |94+94 passed |
| ASan WARP native / shared both |123 /94+94 passed |
| PowerShell7 driver fixtures |17 passed |
| Windows PowerShell5 driver fixtures |15 passed,2 explicit PS7-only skips |
| SDK graph / header fixtures |9 /15 passed |
| Installed Windows consumer |actual /MT compile/link/run; opaque/Popup/client-alpha captures |
| Loader isolation |no Vulkan import/delay import or entry/exit load; invalid-DLL run passed |
| CMake consumer |configure-only passed; no CMake binary/release claim |
| Source consistency |2971 raw Windows and normalized WSL paths match frozen source |

The shared94 includes neutral/backend-private tests as well as13 shared pixel
scenes, not94 distinct DX12 pixel scenes. Native123 includes CPU and controlled
native seams. WSL does not certify native macOS or Windows GPU behavior. SDK
libraries are hardlinked staging, not an immutable release package.

The first auxiliary inventory audit omitted the manifest's Windows-or-Linux
conditional block when forming the shared expected names. It correctly failed
on52 extra actual cases. The audit now evaluates that explicit block; the same
unchanged logs/source pass exact94-name validation. No test was rerun or omitted
to hide this audit-script correction; the full driver itself passed first run.

## Remaining goal

Retained-image accounting, safe DRED integration, neutral public-operation
contracts, ordinary-window/chrome/menu transitions, foreign-thread recovery,
performance, actual CI provisioning/execution and release/default-switch
verification remain separate gates. Passing this slice will not complete DX12.
