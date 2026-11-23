# Acceptance: same-UI-thread DX12 coordinated device recovery

Date: 2026-11-23 (Asia/Shanghai). Baseline `fd125c63`, branch `phase-v`.
This milestone extends bounded recovery to eligible same-thread peers; it does
not complete transparent windows, native Popup or cross-thread recovery.

## Delivered behavior

- A private registry records weak state ownership, adapter LUID and HWND-owning
  UI thread. Its metadata mutex is released before promoting weak handles,
  calling native methods or destroying state. Registration follows successful
  renderer initialization and retains no native COM reference.
- Fresh actual native removal retires eligible peers on the same adapter and UI
  thread before rebuilding the triggering renderer. Each peer must itself
  report removal. Pending/null-device retries never purge healthy replacements.
- Only the triggering renderer reconstructs immediately. Peers remain pending
  with unchanged generation until their own begin/present/resize attempts one
  bounded rebuild. Generations advance only after complete native validation.
- Native-only peer retirement preserves CPU caches, framebuffer metadata,
  capabilities, active CPU frames and previously borrowed capture storage. A
  pending peer reports no current capture. The old borrowed storage survives
  until that peer's own next mutating operation, not arbitrary concurrent use.
- Frame generation is captured on creation and checked before submission. An
  old active frame returns a typed new-frame retry rather than using retired
  native resources. CPU recording may continue until present/abandon; finished
  frame release remains idempotent. Orphan states can be purged without being
  rebuilt for a dead renderer.
- Per-state recursive operation gates serialize state access. Nonblocking peer
  acquisition plus balanced operation depth excludes both foreign-held gates
  and same-thread reentrant active entries. Native peer retirement is itself a
  guarded operation. Nested origin recovery returns ERROR_BUSY before the
  recovery helper mutates state or native ownership, including pending/null
  state. A later safe outer call can reconstruct normally.
- External COM owners, foreign-UI-thread peers, busy peers and concurrently
  finishing native destructors remain real bounded blockers. Failed replacement
  returns its actual terminal error with unchanged generation. A skipped peer
  must recover or release native ownership; merely unlocking does not trigger
  background rescanning. The normal runtime still exits on terminal errors.

No public window modes or new fault environment controls were added. Vulkan and
auto remain default; explicit DX12 never silently falls back. Vulkan Popup and
failure-only Overlay policy are unchanged. DX12 transparent/Popup requests still
fail as unsupported. No DirectComposition implementation was reintroduced.

## Verification

Evidence root: `D:/Dev/shots/cgpui-dx12-cohort-20261123/`.
Both serial validation drivers exited 0. All builds/tests were root-owned and
serial; no-run builds used MCPP_JOBS=4 and native execution MCPP_JOBS=1.

| Check | Result | Evidence |
| --- | --- | --- |
| Public cohort tracer RED / GREEN | expected exit8 / PASS | `cohort-red.log`, `cohort-green.log` |
| Same-thread active peer RED / GREEN | expected exit6 / PASS | `reentrant-red.log`, `reentrant-green.log` |
| Nested origin RED / GREEN | expected exit8 / PASS | `reentrant-origin-red.log`, `reentrant-origin-green.log` |
| Complete architecture suite | 203 passed, 0 failed | `architecture-final-test.log` |
| Frozen Windows complete suite | 707 passed, 0 failed | `windows-final-test.log` |
| Windows all libraries/examples | PASS | `windows-all-build.log` |
| Windows installed-only SDK compile/link/run | PASS | `installed-sdk-windows.log` |
| Frozen Linux/WSL complete suite | 636 passed, 0 failed | `wsl-full-test.log` |
| Linux/WSL all libraries/examples | PASS | `wsl-all-build.log` |
| Linux installed-only SDK compile/link/run | PASS | `installed-sdk-linux.log` |
| DX12 ASan + WARP + Debug | 44 programs passed, 0 failed | `asan-dx12-test.log` |
| Intel Graphics + Debug | 44 passed, 0 failed | `native-0.log` |
| NVIDIA RTX 5070 Ti Laptop + Debug | 44 passed, 0 failed | `native-1.log` |
| WARP + Debug + GPU validation | 44 passed, 0 failed | `native-warp.log` |
| Independent public headers / neutral renderer | 47 / 15 passed | `windows-final-test.log` |
| SDK graph/header fixtures | 4 + 15 passed | `sdk-graph-fixtures.log`, `sdk-headers-fixtures.log` |
| Windows frozen-source comparison | all 2711 files match | `source-match.json` |
| WSL frozen-source comparison | all 2711 manifest paths match after sync-rule normalization | `wsl-source-match.json` |

The Windows source manifest excludes build outputs, dated planning notes and the
user's pre-existing `.vscode/`. No source paths were added, removed or changed
after freezing. The WSL comparison independently verifies archive bytes against
the manifest and compares every archived source path using the actual sync
rule: CRLF becomes LF only for data without NUL. Unlisted build outputs are not
part of that comparison. Only closeout planning notes changed after validation.

Seven new native programs cover same-ID/different-color renderer cache isolation,
finished and active frame lifetimes, borrowed capture retention, orphan states,
lazy begin/resize/abandon, real foreign-thread HWND ownership, different-LUID
adapter isolation, deterministic busy gates and nested-operation safety. All
three non-ASan native matrix logs explicitly confirm distinct hardware/WARP
LUIDs were exercised, not skipped. ASan's custom process exit can suppress
buffered stdout, so its program pass count is not independently used as that
no-skip evidence.

The first new architecture attempt failed because its test expected the thread
eligibility check in retirement rather than its actual registry owner. The
assertion was corrected without weakening the boundary; the final full suite is
green. Historical `architecture-green.log` contains that initial failure and is
not final acceptance evidence. Historical `native-green.log` has 43 passes and
predates the last reentrancy test; final matrices above contain 44.

Native removal tests use real idle-device RemoveDevice and post-recovery pixel
checks. Busy/reentrancy tests use the production private operation guard and
cleanup seam, not actual injected DXGI callbacks. None certifies in-flight GPU
failure, physical hardware reset or arbitrary driver TDR. Native macOS was not
tested. Permanent live-device Signal failure without completion/removal evidence
can still block safe teardown, as documented by the previous milestone.

## Reviews

### Standards

Zero outstanding findings. Focused registry, operation and retirement modules
remain under existing limits; no size caps were raised. Structure tests enforce
private ownership and header boundaries. Follow-up review confirmed lock/depth
lifetime, peer weak-handle promotion and native destruction ordering.

### Spec

Zero outstanding findings against the same-UI-thread contract. Follow-up review
confirmed lazy generations, bounded real blockers, capture/frame lifetimes and
no healthy-peer re-purge. Both discovered reentrancy risks have RED/GREEN evidence
and are closed. Ordinary begin/present/resize do not falsely trigger the nested
origin rejection.

Only the dedicated agent-owned WSL mirror was updated. The separate design
checkout, user's Linux checkout and `.vscode/` were untouched. Local commit only;
no push, release, performance-parity claim or default backend switch.
