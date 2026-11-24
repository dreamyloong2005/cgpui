# Orphan-frame compatibility acceptance

2026-11-24. Baseline `d893fdee`, branch `phase-v`. Overall DX12 goal active.
This slice is accepted after full frozen validation. No push/release/default switch.

## Implemented

An already-acquired unfinished frame retains healthy native state and may record,
request capture and Present once after its Renderer facade is destroyed, with
the native window still valid. This restores the original plan/ledger's lifetime
compatibility, not a new optional backend policy. It does not own the HWND.

Focused private preflight/late-failure routes distinguish this right to finish
from permission to rebuild. Ownerless states never reconstruct or advertise
retry; removed/purged/stale epochs are terminal. Proven removal and operation
depth<=1 allow local native retirement; live incomplete work remains owned for
the existing conservative teardown. Native objects retire before alpha lease.

Private renderer seam forwards a nullable borrowed synchronization API to the
state constructor; public factory remains native-default. No native callbacks or
storage algorithms were added to the facade. Existing module caps unchanged.
Retained-image completion and capture publication are untouched in this slice;
user-chosen capture-history/image batching are subsequent independent changes.

## Actual focused evidence

StageA WARP/debug actual renderer warms with verified captures; native orphan
Present returns exit40 in both retained-frame destruction orders. Shared exact
test passes Vulkan/Intel but fails DX12/WARP exit40. After B all5 native orphan
tests and shared Vulkan/DX12 exact contract pass. Structure228 and native136
WARP/debug pass, including all five migrated prior healthy-orphan rejections.

Coverage includes cold Popup first production Commit with fresh desktop alpha
bands and same-HWND replacement, managed client-alpha new desktop color while
lease/topology remains occupied, preflight removal, late unique InfoQueue error,
and genuine late same-device RemoveDevice after native Present/Signal. The latter
requires real UINT64_MAX fence completion and failed removal query; its old
handle is closed and a live peer rebuilds/captures green while orphan still lives.

No fake readiness/HRESULT, dead-facade pixel getter or manual warming Commit.
The late fault is not proof of work still in flight or hardware TDR. IsWindow is
not an instance-identity guarantee for externally destroyed/reused numeric HWNDs.
Existing managed-alpha cookie/thread checks remain; no new thread-safety claim.

Independent actual-source Standards and Spec/native-safety reviews0. External
installed consumer addition separately source-reviewed0; see `review.md`.

## Frozen matrix

Evidence: `D:/Dev/shots/cgpui-dx12-orphan-frame-20261124/final`.
2997 source paths, excluding planning/user .vscode/builds. Root serial driver,
no-run jobs4/execution jobs1, all-member graph refresh before SDK dependency
extraction. Installed consumer adds actual facade-first opaque Present-once and
post-facade capture-request API checks, without claiming orphan capture pixels.

Driver exit0, first run; exact native136/shared96 inventories, no native/common
SKIP and raw Windows/normalized WSL source checks pass (`matrix-audit.json`).

| Gate | Result |
| --- | --- |
| Windows workspace / all-member build |846 passed,0 failed / passed |
| Dedicated WSL mirror / all-member build |671 passed,0 failed / passed |
| Architecture |228 passed, existing caps unchanged |
| Intel / NVIDIA native + debug |136 passed each |
| WARP native + debug + GPU validation |136 passed |
| Shared suite requested DX12 Intel / NVIDIA |96 passed each |
| WARP shared Vulkan then DX12 |96+96 passed |
| ASan WARP native / shared both |136 /96+96 passed |
| PS7 / Windows PS5 driver fixtures |17 /15 passed,2 explicit PS7-only skips |
| SDK graph / header fixtures |9 /15 passed |
| Installed Windows consumer |actual /MT compile/link/run, opaque/Popup/client-alpha pixels |
| Installed orphan contract |post-facade capture request and exactly-one Present, both runs |
| Installed retained payload |known4 baseline and3076 per cold/warm/replacement pass |
| Vulkan isolation |no import/delay import or loaded DLL; owned invalid-DLL run passed |
| CMake consumer |configure-only passed, not a CMake binary/release build |
| Source consistency |2997 raw Windows and normalized WSL paths match frozen source |

Shared96 includes13 common pixel scenes plus neutral/private/contract tests,
not96 distinct DX12 pixel scenes. Native136 includes CPU and controlled native
seams. WSL does not certify native Mac or Windows GPU. SDK libraries are mutable
hardlinked staging, not an immutable release. No benchmark or all-driver claim.

## Remaining goal

This is a bounded lifetime contract, not all DX12 parity. User-approved image
batching/capture history, safe DRED, broader ordinary-window/menu/background
transitions, cross-thread recovery, recorded-command/SRV/glyph optimization,
noncapture benchmarks, actual CI and release/default-switch gates remain.
