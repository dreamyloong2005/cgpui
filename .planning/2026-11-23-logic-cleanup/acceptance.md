# Acceptance: logic and ownership cleanup

Date: 2026-11-23 (Asia/Shanghai). Baseline `91e68993` on `phase-v`.
User authorized the audit fixes and migration-directory deletion. No DX12 backend,
external design checkout, remote push, package publication or merge is included.

## Delivered contracts

- Win32 window registry tokens expire with the native window; dispatch snapshots
  safely tolerate callback-driven closure.
- X11 drains already-buffered protocol events before blocking, including events
  queued by synchronous replies inside a wakeup callback.
- Stable runtime records, callback lifetime scopes and coherent retirement batches
  protect self-close, cross-window closure and creation during event/render callbacks.
  Owned renderers release before their native windows; trees release before views.
- Redraw and asynchronous completion retain their originating window ID. Closed
  windows skip callbacks without leaving task/I/O completion bookkeeping pending.
- Typed recovered-frame retry reaches the runtime without becoming a fatal quit;
  unsubmitted Vulkan uploads survive acquisition retry.
- Terminal callbacks release captured resources outside locks. True shutdown drains
  all user-resource cohorts while task metadata is alive and only then detaches
  TaskGroup lifetime tokens; reentrant task/I/O registration is rejected.
- `WindowRuntime` is pointer-sized, with thirteen focused private state groups.
  Public templates use non-template bridges. Installed headers no longer include
  private source files. Existing public method signatures and compatibility types
  remain, but the changed ABI requires rebuilding libraries and consumers together.
- Text physical/build ownership agrees across core/platform/renderer; neutral CPU
  planning lives in renderer, retaining legacy Vulkan-named compatibility symbols.
- Removed `docs/migration` and `scripts/migration`; retained the separate current
  GPUI-to-C++ API migration guide. Historical prose locks were removed from affected
  architecture/renderer checks without removing their behavior/module constraints.
- Vulkan still attempts independent Popup; overlay is used for missing capability
  or actual creation/initialization/placement failure, not merely a Vulkan backend.
  DComp remains an internal Windows host/presentation detail, not a public mode.
- Metal drawables are readable for capture; native regression added.

## Verified results

| Verification | Result | Evidence under `D:/Dev/shots/cgpui-logic-fixes-20261123/` |
| --- | --- | --- |
| Windows all workspace libraries/examples | PASS | `windows-final-build.log` |
| Windows all no-run test targets | 628 built | `windows-final-no-run.log` |
| Windows serial complete suite | **628 passed, 0 failed** | `windows-final-test.log` |
| SDK packaging Python fixtures | 15 passed | `sdk-header-closure-green.log` and local rerun |
| Isolated public-include consumer compilation (MSVC) | PASS | `installed-headers-green.log` |
| External installed include + static-library link/run (MSVC /MT, no source headers, /OPT:NOREF) | PASS | `installed-sdk-windows.log`, `installed-sdk-windows-v3/verification.json` |
| WSL renderer-only neutral/compatibility tests | 3 passed | `wsl-neutral-test.log` |
| WSL complete suite (GCC 16.2.1, private Xvfb) | **601 passed, 0 failed** | `wsl-final2-test.log` |
| External installed include + static-library link/run (Linux) | PASS | `installed-sdk-linux.log` |
| X11 queued-event regression, repeated | 20/20; all seven X11 tests passed | `wsl-queued-repeat.log`, `wsl-x11-green.log` |
| Linux SDK header fixtures / unsupported-Mac-request guard | 15 + 1 passed | `wsl-final2-driver.log` |
| Scoped Windows AddressSanitizer regression | **54 passed, 0 failed** (41 window-runtime, 12 async, 1 native registry) | `asan-ui-test.log`, `asan-async-test.log`, `asan-registry-test.log` |

The complete Windows run includes 195 architecture checks, 33 public-header checks,
105 UI tests, 67 native platform tests, 87 renderer tests, 100 API parity tests,
12 async tests, neutral planning, animation/assets, examples and stress/performance
runners. Native Intel/NVIDIA Vulkan Popup pixel coverage is in the renderer log.

## Review and limitations

Independent Standards and Spec reviews found no outstanding actionable findings.
Review previously caught retirement-cohort reentry, shutdown I/O re-registration,
and late resource-capture destruction; each was fixed with a demonstrated RED
regression followed by GREEN. Original effective module caps were not increased.

This is not an exhaustive proof of the entire program. Terminal task metadata
compaction and raw TaskHandle lifetime after runtime destruction are unchanged;
TaskGroup's safe late destruction does not support concurrent runtime deletion.
There is no native macOS environment here: Metal source/test changes are not a
claim of native Mac validation. SDK link checks are local acceptance packages,
not a clean release publication or `/MD` SDK certification.

All product/test source hashes match the snapshot used by the final WSL run;
only this acceptance/planning closeout changed afterward. `git diff --check`
reports no whitespace errors.

The user's pre-existing `.vscode/` is preserved and excluded from commits. WSL
validation uses a new `/root/cgpui-logic-cleanup-20261123` mirror and leaves the
existing dirty `/root/cgpui-linux` checkout untouched. Temporary migration
inventories/generators, source previews and probe/cache directories are outside
the repository in `planning-history/` beneath the evidence directory.
