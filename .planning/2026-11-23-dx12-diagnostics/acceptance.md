# Acceptance: DX12 last-successful-frame diagnostics

Started 2026-11-23; final validation continued 2026-11-24 (Asia/Shanghai).
Baseline `9d80e51d`, branch `phase-v`.
Status: **PASS for this bounded DX12 diagnostic milestone**, with the initial
unresolved intermittent platform-test failures retained below.
Evidence: `D:/Dev/shots/cgpui-dx12-diagnostics-20261123/`.

## Delivered bounded contract

DX12 now implements the existing public diagnostic snapshot getter. A focused
private candidate ledger separates eligible planning from actual draw/upload
observations. Original command identity and all eight primitive categories
survive lowering, including selection/caret and multiple atlas runs from one
text command. Resources are identified by original command and resource index.
Backend payload groups and logical resources are not portable GPU draw-call
counts. Empty payloads and invisible scissors do not become missing/unsupported
resources; accepted culled commands can remain pending. Emitted degenerate image
quads still count even without pixel coverage.

Upload bytes measure actual unique tightly packed immutable glyph/image texture
payloads recorded in the successful frame, not CPU cache writes, alignment
padding, vertices/indices, effect scene copies or readback. Cache hits contribute
zero; immutable replacements and actual recovery reuploads count again. Blur
counts distinguish final composites from their three offscreen filter passes,
and report actual retained targets.

The local candidate publishes only after complete present success, including
fence/readback/debug/composition checks. Initial getter is null; failed/abandoned
frames, resize and device recovery preserve the previous successful CPU snapshot.
That snapshot may describe an older size/generation; it is not proof that the
current device is healthy. Peer native retirement preserves borrowed backing
storage. Existing caller synchronization and renderer lifetime still apply.

CPU timings are six non-overlapping stage laps inside submission, including
diagnostic planning in resource preparation and the synchronous fence wait in
presentation. They exclude prior frame construction/text rasterization and do
not measure GPU timestamps, scanout or end-to-end latency. No public API, default
backend, native Popup policy or window topology changed.

## Red-to-green and targeted verification

- `diagnostics-red.log`: two real rectangle captures succeeded but the public
  getter was null (exit 8). `diagnostics-green.log`: same test now passes.
- `diagnostics-targeted-green.log`: **7 passed** with WARP/debug, serial native
  execution. Covers all categories and native pixels, empty/scissor-cull cases,
  zero-area emitted image geometry, padded image upload/dedup/cache, actual text
  coverage, multi-atlas return runs, CPU timing sum, blur passes, snapshot lifetime
  through real removal and lazy peer reconstruction, private mismatch/saturation.
- `diagnostics-recovery.log`: **2 passed**; existing image/glyph/effect recovery
  and interrupted-present tests now also assert actual texture reuploads and
  retention of the entire previous diagnostic snapshot.
- `diagnostics-structure.log`: new private-module structure guard passed.
- Initial expansion found test-oracle issues, not missing DX12 pixels:
  `diagnostics-empty-probe.log` proves the zero-size image still records one native
  draw/16 upload bytes; `diagnostics-text-probe.log` proves canonical-key lookup
  mismatch before pixel comparisons. Corrected tests retain real pixel checks.

## Frozen-source validation

Snapshot: **2,795** source paths, excluding `.planning/` and user `.vscode/`.
Root runs all builds/tests serially; no-run builds use `MCPP_JOBS=4` and native
execution uses `MCPP_JOBS=1`. WSL uses only the dedicated validation mirror,
`/root/cgpui-logic-cleanup-20261123`, not a user checkout.

| Validation | Final actual result | Evidence |
| --- | --- | --- |
| Windows full no-run/execution | **752 built; 752 passed, 0 failed** | `windows-final-build.log`, `windows-final-test.log` |
| Architecture, included above | 208 passed | `windows-final-test.log` |
| Renderer, included above | 89 passed | `windows-final-test.log` |
| Windows all-member build | PASS | `windows-all-build.log` |
| WSL full execution/all-member build | **643 passed, 0 failed**; build PASS | `wsl-full-test.log`, `wsl-all-build.log` |
| Native DX12 adapter 0 + debug (Intel) | **79 passed, 0 failed** | `native-0.log` |
| Native DX12 adapter 1 + debug (NVIDIA) | **79 passed, 0 failed** | `native-1.log` |
| Native DX12 WARP + debug + GPU validation | **79 passed, 0 failed** | `native-warp.log` |
| ASan DX12, WARP + debug | **79 passed, 0 failed** | `asan-test.log` |
| ASan renderer | **89 passed, 0 failed** | `asan-renderer-test.log` |
| SDK graph/README fixtures | 9 passed | `sdk-graph-fixtures.log` |
| SDK header fixtures | 15 passed | `sdk-headers-fixtures.log` |
| External Windows installed consumer | compile/link/run PASS | `installed-sdk-windows.log` |
| Windows frozen-source check | all 2,795 paths match raw bytes | `source-match.json` |
| Dedicated WSL source check | all 2,795 paths match; CRLF/LF normalized | `wsl-source-match.json` |

These are separate runs, not additive totals. The seven new DX12 diagnostic tests
run on each selected adapter; the existing loader children intentionally select
their own WARP/Vulkan configurations. Full WSL native windows use the available
WSLg environment, not proof of arbitrary Linux compositors or native macOS.
No production/test/document source changed after the final snapshot; only these
excluded planning records were updated from the resulting logs.

### Failures retained rather than hidden

The first Windows full attempt (`windows-final-test-attempt1.log`) reported
749 passed/3 failed. Two were integration omissions: the new architecture test
needed its CI header-matrix registration, and the existing text structure guard
still required the old metadata-free append signature. Both were corrected,
with the text guard now additionally enforcing command/run identity. All three
targeted structure checks passed (`diagnostics-structure-followup.log`).

The third was the unchanged `win32_popup_window_test` global hit-test at exit 23,
which also failed once in isolation. Failure-only diagnostics did not reproduce
it in six bounded standalone runs. Those probes were removed; the test is byte-
equivalent to baseline and retains its original `WindowFromPoint` assertion.
The final full Windows run passes it. Its initial intermittent failure is **not
claimed fixed**; no platform production behavior was changed for this milestone.
Evidence: `popup-isolated.log`, `popup-hit-probe.log`, `popup-hit-repeat.log`.

The first WSL full attempt (`wsl-full-test-attempt1.log`) reported 642 passed/1
failed: the second application connection in `x11_queued_event_loop_test` could
not connect to DISPLAY. Isolated unchanged rerun passes; DISPLAY is `:0` and
WAYLAND_DISPLAY is `wayland-0`, not deliberately unset to skip native tests.
No Linux source or test change was made in response. Evidence:
`x11-queue-isolated.log`; final unchanged WSL full rerun passed all 643 tests.
This initial connection failure is also not claimed fixed.

## Installed SDK evidence

The direct external consumer used copied public headers, same-volume hardlinked
static libraries and the actual production **/MT** CRT. It compiled and linked
without Vulkan SDK include/lib/PATH entries or Vulkan import/delay-load flags.
The final executable's PE import check has no `vulkan-1.dll`. Both clean and
owned-invalid-app-local-Vulkan-DLL runs passed real opaque/text/image/shadow/blur,
Popup and client-title-bar alpha pixel captures while Vulkan remained unloaded.
The new diagnostic assertion verifies three original commands, image/shadow/text
categories, real image/glyph upload bytes, exact work agreement and no drops via
the installed public getter. It proves external static dependency closure, not
just an in-tree test link.

Generated CMake configuration also passed with Vulkan discovery disabled;
this is configure-only, **not** a CMake binary build or release CRT validation.
The stage records the hardlink/non-immutable policy and actual CRT in
`installed-sdk-windows/verification.json`. About 5.92 GiB remained free on D:
after validation; no new 700 MiB SDK archive copy was made.

## Limits

WSL logs retain dzn testing-only/non-conformance and mcpp runtime-facts/host
sysroot warnings; passing these tests is not Vulkan conformance or a hermetic
toolchain certification. Source hashes establish source consistency, not an
independent proof of executable provenance.

This is a diagnostic milestone, not complete DX12/GPU-performance parity.
Dynamic opaque-to-transparent upgrades, client-title-bar plus HMENU alpha,
broader OS/native-window compatibility, cross-thread device-loss coordination,
performance/default-switch and release gates remain separate work. No native
macOS, arbitrary driver TDR or release validation is claimed.

Installed verification uses copied headers and same-volume hardlinked libraries,
not an immutable SDK release snapshot. The already authorized old-SDK cleanup
was completed in the preceding milestone; no additional old SDK deletion is part
of this change. Independent Standards and Spec reviews are in `review.md`.
