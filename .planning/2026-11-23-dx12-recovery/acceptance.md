# Acceptance: bounded DX12 single-owner device recovery

Date: 2026-11-23 (Asia/Shanghai). Baseline `c1b72784`, branch `phase-v`.
This milestone is not coordinated multi-window recovery or full DX12 parity.

## Delivered behavior

- Confirmed native removal triggers at most one reconstruction attempt per
  public operation, using the original adapter LUID. No backend, adapter,
  software fallback or construction-only adapter preference is reselected.
- Old GPU owners are completely released before replacement: commands and
  allocator, PSOs/root signatures, uploads/capture, texture/descriptor/sampler
  realizations, backdrop targets, backbuffers/swapchain, queue/fence/event,
  device/adapter/factory. CPU image and glyph caches, HWND and dimensions/DPI
  survive. Partial new initialization has submitted nothing and is cleaned up.
- Immutable texture snapshots now contain CPU data only. Weakly keyed native
  realizations belong to the renderer and are pruned after idle; retained CPU
  draw handles keep pending uploads safe. Recovery can discard every native
  realization even when older frames keep invalidated snapshots alive.
- Generation advances only after the complete device/pipeline/swapchain/debug
  chain succeeds. A recovered begin/present returns typed frame_retry_required,
  never successful presentation for the discarded frame. Image idle ages are
  advanced only by successful frames. Resize completes its requested size/DPI
  after rebuilding or returns an error, never a frame retry.
- External device references or another renderer on the same adapter can block
  replacement of the removed singleton. Attempts fail with the real error and
  unchanged generation, with no retry spin. Once blockers are released, an
  explicit later call on the same renderer may attempt again. The normal UI
  runtime treats terminal frame errors as fatal; it does not coordinate peers.
- Live-device synchronization/validation failures are not generalized into
  removal. Previous safe retirement remains in force, including potentially
  blocking teardown when live-device completion cannot be proven.

No public API or environment fault hook was added. Vulkan/auto stays default;
explicit DX12 does not silently fall back. Vulkan Popup/failure-only overlay and
DX12 transparent/Popup rejection are unchanged. Diagnostics/performance parity,
transparent ordinary windows/native-menu compatibility, coordinated multi-window
recovery and default-switch/release gates remain unfinished.

## Verification

Evidence root: `D:/Dev/shots/cgpui-dx12-recovery-20261123/`.
Frozen manifest contains2698 source files, excluding build output, dated planning
and the user's pre-existing `.vscode/`. Both serial validation drivers exited0.
Final `source-match.json` confirms all2698 files match the frozen sources, with
no new/removed/changed source files after verification; only closeout notes changed.

| Check | Result | Evidence |
| --- | --- | --- |
| Original public recovery tracer RED | expected exit9 | `recovery-red.log` |
| Same tracer GREEN | PASS | `recovery-green.log` |
| Initial WARP + Debug native suite | 37 passed, 0 failed | `native-green.log` |
| Architecture including recovery and CI registration | 202 passed, 0 failed | `architecture-green.log` |
| Runtime root/child begin/present retry | 12 cases in1 passing program | `runtime-retry-green.log` |
| Frozen Windows complete suite | 699 passed, 0 failed | `windows-final-test.log` |
| Windows all libraries/examples | PASS | `windows-all-build.log` |
| Windows installed-only SDK compile/link/run | PASS | `installed-sdk-windows.log` |
| Frozen Linux/WSL complete suite | 635 passed, 0 failed | `wsl-full-test.log` |
| Linux/WSL all libraries/examples | PASS | `wsl-all-build.log` |
| Linux installed-only SDK compile/link/run | PASS | `installed-sdk-linux.log` |
| Scoped DX12 ASan + WARP + Debug | 37 passed, 0 failed | `asan-dx12-test.log` |
| Intel Graphics + Debug | 37 passed, 0 failed | `native-0.log` |
| NVIDIA RTX5070Ti Laptop + Debug | 37 passed, 0 failed | `native-1.log` |
| WARP + Debug + GPU validation | 37 passed, 0 failed | `native-warp.log` |
| Independent public headers / neutral renderer | 47 / 15 passed | `windows-final-test.log` |
| SDK graph/header fixtures | 4 + 15 passed | `sdk-graph-fixtures.log`, `sdk-headers-fixtures.log` |
| Frozen-source final comparison | all2698 files match | `source-match.json` |

Five new native tests cover:

1. Real idle-device RemoveDevice, both current and invalidated image snapshots
   retained by finished frames, cached glyph/image reuse without image reupload,
   literal image/shadow/blur/ink checks plus full captured scene equality.
2. Loss while a CPU frame is active, typed present retry, no repeated present,
   and old-frame destruction cannot unlock a newer active frame.
3. Loss before resize, requested96x48 framebuffer and DPI2 applied synchronously,
   verified by capture dimensions and rectangle/device-pixel samples.
4. External Device5 and same-adapter peer blockers independently: two hard errors
   do not consume generations; releasing blockers allows the same renderer to
   rebuild and render its cached image correctly.
5. Three successive removals with all finished frames retained, one image upload
   across four generations, unchanged LUID/software flag, and an invalid changed
   adapter-selection environment value that must not affect reconstruction.

Native helpers release their Device5/InfoQueue before successful reconstruction;
blocker tests intentionally retain them. Debug audit exempts exactly three native
removal IDs only during intentional removal; new-generation audit allows no
ERROR/CORRUPTION messages. Old-generation release-time messages cannot be observed
through a retained InfoQueue during successful replacement without itself
blocking creation. These tests remove idle native devices, not physical resets,
in-flight native execution or arbitrary hardware TDR; no such certification is
claimed. Native macOS was not tested by this Windows/WSL milestone.

## Reviews

Standards: zero hard violations and zero outstanding judgment findings. One
duplicated watchdog/debug-message-filter suggestion was fixed by sharing the
native test helper, preserving the original removal test's long-lived blockers
and exact error whitelist. Module boundaries and existing size limits remain
enforced by architecture tests.

Spec: zero actionable findings against the single-owner scope. Independent
native-lifecycle review verified the full ownership purge, no-allocation cleanup,
partial-init cleanup, pinned selection, failure persistence and generation
publication. Root additionally reviewed frame/resize retry control flow.

Only agent-owned WSL validation storage is updated. No modifications to the
separate design checkout or user's Linux checkout, no release and no push.
