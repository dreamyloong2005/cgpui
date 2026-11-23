# Acceptance: DX12 exceptional synchronization retirement

Date: 2026-11-23 (Asia/Shanghai). Baseline `1964c77c`, branch `phase-v`.
This is an ownership/synchronization prerequisite, not complete device recovery.

## Delivered behavior

- Successful queue fence tickets survive event-registration or wait failure.
  Retrying observes the original ticket rather than submitting a replacement.
- Event wake is only a notification. The completed fence value must reach the
  pending ticket; otherwise the frame fails while resources remain retained.
- A focused noexcept teardown owner polls completion with backoff and releases
  state-owned resources only after completion or actual removal. It performs no
  allocation or event registration. The state destructor only delegates.
- Device-removal evidence remains terminal across later waits; UINT64_MAX with
  no failed removal reason is normalized to DXGI_ERROR_DEVICE_REMOVED. Reserved
  UINT64_MAX is never queued. Wait errors are captured immediately and failures
  without an error code use ERROR_GEN_FAILURE, not a success HRESULT.
- Native API calls, synchronization evidence and teardown policy have separate
  private owners. No public API, environment fault switch, driver reset, native
  window mode, backend default or Popup-policy change was introduced.

If Signal permanently fails on a device still reported live, with no valid
completion evidence, destruction can block indefinitely. Backoff avoids a busy
loop, but this is not an availability guarantee. Neither timeout nor leaking
resources is treated as successful retirement. Automatic device recreation,
transparent windows/Popup, diagnostics and performance parity remain unfinished.

## Evidence and validation

Evidence root: `D:/Dev/shots/cgpui-dx12-retirement-20261123/`.
Frozen source manifest: 2679 files, excluding `.planning/`, build output and the
pre-existing untracked user `.vscode/`. Both serial verification drivers exited0.
Final `source-match.json` confirms no changed/new/removed source after validation;
only these dated closeout notes changed.

| Check | Result | Evidence |
| --- | --- | --- |
| Original deterministic RED (twice) | 3 failing programs, expected symptoms | `sync-red.log`, `sync-red-repeat.log` |
| Expanded deterministic RED | 6 failing programs | `sync-expanded-red.log` |
| Structural RED/GREEN | missing owner fails; final owner passes | `structure-red.log`, `structure-green.log` |
| Initial WARP + Debug native suite | 32 passed, 0 failed | `native-green.log` |
| Early-release mutation sensitivity | expected exit3 | `resource-sentinel-mutation.log` |
| Frozen complete Windows suite | 693 passed, 0 failed | `windows-final-test.log` |
| Windows all libraries/examples | PASS | `windows-all-build.log` |
| Windows installed-only SDK compile/link/run | PASS | `installed-sdk-windows.log` |
| Frozen complete Linux/WSL suite | 634 passed, 0 failed | `wsl-full-test.log` |
| Linux/WSL all libraries/examples | PASS | `wsl-all-build.log` |
| Linux installed-only SDK compile/link/run | PASS | `installed-sdk-linux.log` |
| Scoped native DX12 ASan + WARP + Debug | 32 passed, 0 failed | `asan-dx12-test.log` |
| Architecture / independent public headers | 201 / 47 passed | `windows-final-test.log` |
| Original neutral renderer tests | 15 passed | `windows-final-test.log` |
| Intel Graphics + Debug | 32 passed, 0 failed | `native-0.log` |
| NVIDIA RTX5070Ti Laptop + Debug | 32 passed, 0 failed | `native-1.log` |
| WARP + Debug + GPU validation | 32 passed, 0 failed | `native-warp.log` |
| SDK graph/header Python fixtures | 4 + 15 passed | `sdk-graph-fixtures.log`, `sdk-headers-fixtures.log` |
| Frozen-source final comparison | all2679 files match | `source-match.json` |

The synchronization tests use real idle native objects but scripted native-call
results and synthetic submitted state; they never submit GPU commands. The
actual state destructor runs on a controlled thread. A real sole-owned upload
resource carries a COM final-release sentinel: it must remain alive before
completion is permitted, then be released after retirement. The temporary
early-release mutation proved that these checks fail even if destruction still
blocks. Original source bytes were restored before the final matrix.

The independent public-renderer removal test warms up actual image/shadow/blur
GPU resources, acquires the same adapter's native singleton device and calls
Device5::RemoveDevice while idle. Present must fail terminally, invalidate old
capture and reject further begin/resize. Debug error inspection exempts only
explicit removal messages. This is not a real pending-GPU failure/TDR proof.

## Standards review

Independent review against baseline and AGENTS.md: zero hard violations and
zero actionable judgment smells. Module ownership and private declarations are
focused, mcpp registration is explicit, and existing size caps are not raised.
The actual structure suite is the execution gate, not the static review alone.
The first full Windows run found an omitted CI-matrix row for the new test;
the registration was repaired and reviewed without relaxing assertions, then
the matrix was re-frozen and the entire driver rerun successfully.

## Spec and native ownership review

Independent review found zero outstanding actionable findings. The initially
identified test gap (destructor may block after resources were released) was
closed with the COM resource sentinel and proven by mutation. No unsupported
automatic-recovery or real pending-GPU validation claim is made.

No release/push, native macOS claim, or modification of the separate design
checkout or the user's Linux checkout is included in this milestone.
