# Retained-image payload acceptance

2026-11-24. Baseline `140cde6b`, branch `phase-v`. Overall DX12 goal active.
This slice is accepted after the complete frozen matrix. No push/release.

## Implemented

Neutral diagnostic data leaf appends optional retained_image_byte_count and its
independent saturation flag. DX12's focused weak tracker counts unique live
immutable tight RGBA CPU snapshots once, regardless of cache/active/finished
frame or submission references. Caller assets, row padding, glyphs, capacity,
bookkeeping, native textures/descriptors, staging/readback and backdrop storage
are excluded. This is not total renderer RAM/RSS/VRAM, a peak or a memory budget.

Unknown differs from known zero. Only tracker bookkeeping allocation/capacity
exceptions are caught and latch unavailable for the image-cache lifetime;
unobserved snapshots never become a known partial sum. Other allocations retain
existing exception behavior. Arithmetic saturates independently at SIZE_MAX.

Focused successful completion prepares allocating diagnostics first, ages the
cache exactly once, samples without allocation, then moves history without
throwing. The accepted capture candidate publishes afterward without throwing.
Diagnostic-only CPU ledger publication does not age cache or report native
success. Native recovery retains the CPU tracker; future successful samples
reflect current ownership without rewriting old history.

No image mutation, capture-history, backend selection, Popup or recovery policy
change. Public fields extend SDK data layout; this is not an ABI guarantee.

## Observed focused evidence

Stage A actual WARP/debug succeeds in Present/blue readback, then reports absent
retention with zero image upload and testcase exit40. Stage B same test reports
known16/unsaturated/zero upload, then checks real padded four-color pixels,
first-copy16/warm-copy0 and glyph exclusion. Both raw logs retained.

Architecture227/227 and native131/131 WARP+debug passed, including CPU weak
identity/expiry/cache/arithmetic/completion/equality, finished immutable image
owners, failed/abandoned history, eviction120/121 and actual idle-device removal
with peer lazy rebuild and later payload release. Neutral schema test passes.
Independent actual-source Standards and Spec reviews0; see `review.md`.

The OOM catch is source-reviewed, not a run fault-injection test. Synthetic
overflow does not allocate enormous images. Idle RemoveDevice is not hardware
reset/TDR. These limitations are not replaced by a whole-suite green result.

## Final frozen matrix

Evidence: `D:/Dev/shots/cgpui-dx12-image-retention-20261124/final`.
2986 source paths frozen, excluding planning/user .vscode/builds. Root only,
serial no-run jobs4/execution jobs1. The driver refreshes all-member build graph
before SDK closure. Installed consumer adds fixed4 and3076-byte payload oracles
alongside existing native pixels, upload counts and Vulkan-loader isolation.

`verify-final.ps1` exit0, first run, no source change/retry. `matrix-audit.json`
validates native131/shared95 exact manifest inventories and no native/common SKIP.

| Gate | Result |
| --- | --- |
| Windows workspace / all-member build |839 passed,0 failed / passed |
| Dedicated WSL mirror / all-member build |670 passed,0 failed / passed |
| Architecture |227 passed, existing caps unchanged |
| Intel / NVIDIA native + debug |131 passed each |
| WARP native + debug + GPU validation |131 passed |
| Shared suite requested DX12 Intel / NVIDIA |95 passed each |
| WARP shared Vulkan then DX12 |95+95 passed |
| ASan WARP native / shared both |131 /95+95 passed |
| PS7 driver fixtures |17 passed |
| Windows PS5 driver fixtures |15 passed,2 explicit PS7-only skips |
| SDK graph / header fixtures |9 /15 passed |
| Installed Windows consumer |actual /MT compile/link/run, native opaque/Popup/client-alpha captures |
| Installed retained bytes |known4 baseline,3076 each cold/warm/replaced pass, unsaturated |
| Vulkan-loader isolation |no import/delay import or loaded DLL at entry/exit; owned invalid-DLL run passed |
| CMake consumer |configure-only passed; not CMake binary/release validation |
| Source consistency |2986 raw Windows and normalized WSL paths match frozen source |

Shared95 includes neutral/private tests as well as13 shared pixel scenes; it is
not95 distinct DX12 pixel scenes. Native131 includes CPU/native controlled seams.
WSL is not native macOS or Windows GPU proof. SDK library staging uses hardlinks,
not an immutable release; later rebuilds can replace their source bytes.

After freeze the user explicitly chose Vulkan-compatible same-frame last-upload
and retained-successful-capture semantics. This matrix intentionally validates
the current retained-image diagnostic change without silently modifying those
contracts mid-run. Follow-up decision is in completion/public-contract-decision.md;
backend-specific image-order tests will migrate in that separate slice while
retention ownership and history invariants remain protected.

## Still open

The DRED device-wide lifetime boundary, neutral image/capture operation choices,
orphan-frame compatibility, ordinary-window/chrome/menu transitions, cross-thread
recovery, command reuse/SRV pooling/dirty glyph optimization, native noncapture
benchmarks, actual CI and release/default-switch gates remain separate work.
