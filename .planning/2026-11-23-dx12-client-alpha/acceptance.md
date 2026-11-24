# Acceptance: initial DX12 alpha with client-drawn title bars

Date: 2026-11-23. Baseline: `4ce08c0a`; branch: `phase-v`.
Status: **PASS for the bounded no-application-HMENU milestone**, not completion
of every DX12 transparency topology or the pre-existing opaque client/menu bug.
Evidence: `D:/Dev/shots/cgpui-dx12-client-alpha-20261123/`.

## Delivered behavior

- A CGPUI-owned ordinary window with an initially visible, decorated client
  title bar can acquire initial DX12 alpha without an application HMENU, on
  Windows build 26100+ after the real DWM alpha attribute succeeds. No manual
  chrome setter is needed to make the initial descriptor work.
- Keep existing `WS_CAPTION`, `WS_SYSMENU`, client geometry, resize and caption
  controls. The alpha lease fixes its initial system/client title-bar mode;
  switching modes, hiding/removing chrome or entering fullscreen is refused.
- Existing HMENU causes client-alpha factory refusal without menu mutation.
  A nonempty app-wide menu install is refused before any HWND/model/accelerator
  change while a client-alpha lease is held; empty clearing remains allowed.
- The private RAII menu-policy leaf spans native install/clear callbacks and
  rejects reentrant installers and client-alpha creation even on temporarily
  menu-free targets. Reservation follows allocation-bearing read-only preflight.
- System-title-bar alpha with native menus, opaque HWND flip/history,
  Vulkan/default selection and native-first Popup policy are unchanged.

Public headers/API, renderer implementation, native styles and menu geometry
are not rewritten by this milestone. The broad application owner adds only
policy delegation. Existing line caps remain intact.

## Regression evidence

| Validation | Result | Evidence |
| --- | --- | --- |
| Windows full no-run | 739 built, 0 failed | `windows-final-build.log` |
| Windows full execution | **739 passed, 0 failed** | `windows-final-test.log` |
| Architecture (included above) | 206 passed, 0 failed | same log |
| Windows all-member build | PASS | `windows-all-build.log` |
| WSL full execution | **641 passed, 0 failed** | `wsl-full-test.log` |
| WSL all-member build | PASS | `wsl-all-build.log` |
| Intel adapter 0 + DX12 debug | **70 passed, 0 failed** | `native-0.log` |
| NVIDIA adapter 1 + DX12 debug | **70 passed, 0 failed** | `native-1.log` |
| WARP + debug + GPU validation | **70 passed, 0 failed** | `native-warp.log` |
| ASan DX12 no-run/execution | 70 built; **70 passed, 0 failed** | `asan-build.log`, `asan-test.log` |
| DX12 package graph fixtures | 4 passed | `sdk-graph-fixtures.log` |
| SDK header fixtures | 15 passed | `sdk-headers-fixtures.log` |
| External installed-only SDK consumer | compile/link/run PASS | `installed-sdk-windows.log` |
| Frozen Windows source | 2,770 files match byte-for-byte | `source-match.json` |
| Frozen WSL source | 2,770 manifest paths match, text CRLF/LF normalized | `wsl-source-match.json` |

Counts are separate runs and not additive; architecture and the default native
DX12 run are included in the Windows full count. The native host reports build
26300. Actual adapters were Intel(R) Graphics, NVIDIA GeForce RTX 5070 Ti Laptop
GPU and Microsoft Basic Render Driver (software=1 for WARP). Each explicit
adapter log contains all six new client tests exactly once, with no skipped
tests; `matrix-summary.json` records their names and elapsed times.

The six public native cases cover initial frame metrics/hit tests and GPU plus
desktop alpha in both body and title row; fixed-mode refusal, resize/renderer
scale 1/2, Mica/none, retained frames and destruction; two idle-device removal
epochs; menu refusal preserving real accelerators and application state;
actual menu install/clear resize callbacks with menu-free targets; maximize,
restore, minimize, close cancellation and initially fixed-size windows.

The SDK consumer uses copied external headers and same-volume hardlinked
libraries after all production builds. It compiles without source include paths
and with unused consumer code retained, then exercises opaque/Popup behavior
and initial client-title-bar alpha. Its output includes
`client_title_bar=1 native_menu=0 captured_alpha=128,0`; it also requires menu
installation refusal. This is installed-header/static-link closure and GPU
capture acceptance, **not a release package or immutable independent library
snapshot**, and the consumer does not separately establish desktop pixels.
No production build runs after the SDK stage in the driver.

## Red-to-green and harness history

- `client-initial-red.log`: initial no-menu client factory failed with exit 7,
  after initial geometry/hit tests passed. The bounded admission change made
  the same public factory and strict GPU/desktop alpha assertions pass.
- `client-refusal-red.log`: menu-bearing client alpha was incorrectly admitted
  before the new menu policy (exit 4). Refusal/preservation now passes.
- `client-menu-red.log` and `client-menu-candidate.log`: the opaque HMENU/client
  rectangles overlap, including after the rejected NCCALCSIZE adjustment.
- The captionless raw probe v2 passes locally but violates the documented
  `WS_SYSMENU` requirement for `WS_CAPTION`, so no such style change ships.
  Prototype source is archived only at
  `codex/prototype-dx12-client-menu-20261123`,
  `bb343e0a77a9ad1d99b56d51c31b6a3c31b410bc`.
  (That branch was deleted on 2026-11-25 without merging; its source is gone.)
- The controls test originally expected an event the existing Win32 backend
  does not emit. Its corrected oracle checks real `IsIconic` plus public
  lifecycle display state; no unrelated event implementation was added.
- The first external WSL driver failed before sync because CRLF made Bash
  parse `pipefail\r`. Only that evidence harness was normalized to LF; the
  failure remains in `wsl-driver-crlf.log`. `verify-remaining.ps1` resumed at
  WSL after the successful Windows stages. No source, test oracle or failed
  production result was discarded.

The source snapshot excludes `.planning/` and user `.vscode/`. The WSL mirror is
only `/root/cgpui-logic-cleanup-20261123`; no user or design checkout was changed.
The normalized WSL check verifies every manifest path, not arbitrary unlisted
build/cache files. The final Windows check also detects added/removed source
paths. The production snapshot did not change during validation.

## Review and limitations

Independent Standards: 0 hard violations, 0 actionable smell findings.
Independent Spec: 0 actionable findings. Documentation cross-check: 0 findings.
See `review.md`; reviewers did not substitute their inspection for root's tests.

Still not supported or not established by this milestone:

- Client-title-bar alpha with application HMENU. The pre-existing opaque
  client/HMENU layout defect remains unresolved; menus are not hidden/replaced.
- Dynamic opaque-to-transparent upgrades, hidden/undecorated ordinary alpha,
  or older Windows. Older-OS refusal branches were not run on an old OS.
- Physical mouse dragging/Snap Layouts, monitor-DPI migration, arbitrary TDR,
  all-driver acceptance, native macOS or a full workbench interaction session.
- Renderer scale 1/2 is not a physical monitor-DPI test. Maximized controls
  assert GPU alpha; restored windows re-establish the desktop-pixel oracle.
- Mica may be explicitly refused by OS/policy with a reason. The test always
  requires fresh GPU pixels and desktop alpha after returning to `none`.

No previous installed SDK copy was removed; no cleanup authorization was
received. D: remained critically low (about 146 MiB after validation). Changes
are intended for a local commit only, with no push and user `.vscode/` preserved.
