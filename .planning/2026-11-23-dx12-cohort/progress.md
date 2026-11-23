# Evidence and progress

Baseline `fd125c63`; actual public native tracer:

```powershell
scripts/dev/test.ps1 -p tests/renderer-dx12 dx12_coordinated_recovery
```

WARP+Debug `cohort-red.log`: exit8/0.13s, same-thread peer still blocks recovery.
The same tracer passes after private registry/cohort retirement: `cohort-green.log`,
0.10s. Warmed same-ID/different-color caches remain isolated; peer generation
advances lazily; borrowed peer CPU capture remains alive before local mutation.

`native-green.log`:43/43 programs pass, including orphan-state cleanup, lazy
begin/resize/abandon, foreign-window-thread blockers, actual different-LUID
hardware/WARP isolation (not skipped) and private nonblocking busy-gate checks.
The first new structure guard incorrectly expected GetCurrentThreadId inside
retirement rather than registry metadata filtering. The check was corrected to
its actual owner (not weakened); `cohort-structure-green.log` passes. The full
final architecture suite will rerun all203 programs.

## Reentrant operation safety

Root identified that a recursive mutex can be acquired again on its owning
thread even while an outer peer entry is active. A behavior-preserving private
Dx12OperationGuard first replaced raw public-entry locks. `diagnosing-bugs` was
loaded; the minimal deterministic test uses that same production guard and real
cohort cleanup, not a separate modeled cleanup policy.

```powershell
scripts/dev/test.ps1 -p tests/renderer-dx12 dx12_cohort_reentrant
```

`reentrant-red.log`: exit6/0.14s, recursive acquisition mistaken for idle peer.
The test's native devices are really removed, but no actual DXGI message callback
or in-flight GPU command is injected. It checks the private operation invariant.

Ranked hypotheses were communicated: recursive-lock idle misclassification;
stale activity accounting; wrong thread eligibility. Adding balanced operation
depth inside the same guard and excluding active peers addresses the first
hypothesis without changing the test seam or expected behavior. The test also
requires the peer to become eligible after the outer guard exits, catching stale
depth. Foreign-thread eligibility has its own public native test.

`reentrant-green.log` passes after the peer-depth fix. Extending the same seam to
the origin exposed a second gap: `reentrant-origin-red.log`, exit8/0.09s, nested
recovery could retire resources belonging to its still-active outer operation.
The recovery helper now rejects confirmed/pending recovery at depth > 1 with
ERROR_BUSY before any mutation, including pending/null-device state. Peer purge
itself holds an operation guard so reentry during native Release is also active.
`reentrant-origin-green.log` passes in 0.07s, including a successful bounded
rebuild on a later safe outer call. These are private invariant tests with real
device removal, not an injected DXGI callback or in-flight GPU/TDR certification.

Final targeted Standards/native-lifetime and Spec reviews both report zero
outstanding findings; both reentrancy issues are closed. No source-size caps were
raised. Full frozen-source validation is the next acceptance gate.

## Final frozen-source validation

The complete architecture rerun passed 203/203. After the last source/doc edit,
2711 files were frozen. Serial final and native drivers both exited 0:
Windows 707/707, WSL 636/636; all libraries/examples and installed-only SDK
consumers on both platforms; DX12 ASan+WARP+Debug 44/44; Intel+Debug,
NVIDIA+Debug and WARP+Debug+GPU validation each 44/44; SDK fixtures 4+15.
Every non-ASan native matrix explicitly exercised distinct hardware/WARP LUIDs.
Windows byte comparison and independent WSL sync-normalized comparison both
match all 2711 frozen paths. See acceptance.md for exact logs and limitations.
No product source changed after freezing; only these closeout notes changed.
