# Accepted: opaque DX12 frame admission and runtime readiness

2026-11-24. Baseline `3f2df20f`. Evidence directory:
`D:/Dev/shots/cgpui-dx12-frame-pacing-20261124`.
This milestone is complete; the overall DX12 completion goal remains active.

## Implemented contract

Opaque HWND chains own one DXGI frame-latency handle, configure maximum latency1,
and nonblockingly poll before constructing each frame, including the first.
Pending returns begin-only `frame_not_ready` without poisoning the renderer,
publishing a frame or changing the last successful capture/diagnostic/generation.
An observed grant survives abandonment/pre-Present validation failure; each real
Present attempt consumes it immediately on return, regardless of HRESULT.
Same-chain resize retains it; retirement/recovery closes and reconfigures it.
This handle is not GPU-fence evidence; original GPU retirement stays intact.

WindowRuntime coalesces one window-bound readiness timer, initially2ms and64ms
after250ms sustained pending. Early success, close, shutdown and recovery cancel
pending bookkeeping. Token/delivery guards handle synchronous transport reentry;
success checks lifecycle again after cancellation. Existing typed recovery is
still immediate retry; present-returned pending is a fatal contract violation.
Win32 app initialization fails if its real delayed timer cannot be allocated,
instead of changing future timers into immediate wake loops.

Admission-call CPU time travels by value with the successful frame and seeds the
existing saturating timing ledger once. It excludes prior pending/recovery,
event-loop and frame-holding time. The pacing field also contains pre-record GPU
reuse synchronization, not an exclusive DXGI wait or GPU/input-latency measure.

## Red/green and structure evidence

- Real hidden/visible production begin initially returned frames without a native
  readiness observation (exit6); after activation both observed real ready and
  passed native captures without showing the hidden window.
- Three scripted-wait/native-renderer tests initially RED, then GREEN: pending,
  actual64->96 resize, abandon/overlap/pre-Present errors, retained pixels, real
  poll-boundary RemoveDevice recovery and handle lifetime.
- Native runtime recovery observers return pending unchanged; actual runtime
  handles retries, once-uploaded image/glyph recovery and independent windows.
- Synthetic-poll/native-Win32 tests cover input, actual resize, timer progress,
  child/root close and quiet sustained-pending backoff. The64->2ms mutation was
  rejected; restored original source passed. No CPU/power/refresh-rate claim.
- CPU timing17ns seed RED/GREEN and saturation/unpublished-snapshot controls.
- 142 successful test acquisitions migrated;54 classified raw occurrences kept.
  Negative/recovery/Present/pixel assertions unchanged; default helper only
  yields, explicit matching owner alone may pump at most32 messages.
- 19 DX12 architecture guards moved into a genuine owner directory; discovery
  and CI use exact bidirectional inventories. Manifest232<=240, renderer85<=100,
  Win32 application170<=170; no original cap relaxed. Architecture222/222.
- Independent Standards/Spec reviews each0 confirmed findings. Review limits and
  retained intermediate failures are recorded in review.md.

## Final frozen-source matrix

`verify-final.ps1` exited0. No final-stage retries or native SKIP markers.

| Gate | Actual result |
| --- | --- |
| Windows full suite / all-member build |818/818; build pass |
| Dedicated WSL mirror suite / all-member build |664/664; build pass |
| Intel / NVIDIA / WARP native DX12 |116 each, debug enabled; WARP GPU validation |
| Common renderer, DX12 Intel / NVIDIA |94 each |
| Common renderer, explicit both on WARP |Vulkan94 + DX1294 |
| ASan native DX12 / common both |116 / 94+94 |
| Driver fixtures PS7 / PS5 |17 / 15+2 named PS7-only skips |
| SDK graph / header fixtures |9 / 15 |
| Installed Windows consumer |Actual /MT compile/link/run; real opaque/Popup/client-alpha captures and texture lifecycle; no Vulkan import/load, clean and invalid-app-local-loader cases |
| CMake |Configure-only, not binary/CRT/release verification |
| Source consistency |All2940 paths: Windows raw and WSL normalized match frozen source |

User `.vscode` and planning records excluded from source manifest. Installed
libraries are local hardlinks, not immutable published release artifacts.

## Remaining scope

Composition chains still explicitly disable admission pacing (flags0); external
probe candidates are not yet applied or executed. Dynamic ordinary alpha/menu
compatibility, cross-thread/busy recovery boundaries, DRED/RAM diagnostics,
performance/CI/release/default-switch gates remain separate incomplete work.
Default/auto stays Vulkan, explicit DX12 never silently falls back, and Popup
stays native-first with failure-only Overlay. No push/publication performed.
