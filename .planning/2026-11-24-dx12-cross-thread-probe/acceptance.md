# Cross-thread probe bounded acceptance record

Baseline279a2e8c; evidence root:
`D:/Dev/shots/cgpui-dx12-cross-thread-recovery-20261124/root-attempt-1/`.
No production recovery filter or native owner is modified by this slice.

## Source and compilation

`root-source-binding.json` verifies all3231 source paths and bytes against the
command-reuse final freeze, plus17 dependencies and18 reused artifact hashes.
`integration-actual.json` records12 exact sealed test sources and2 additive
registrations. Git initially applied CRLF; normalized content was checked and
the permitted exact LF bytes restored before compilation. Native no-run succeeds;
all248 architecture tests pass, including foreign-probe/cohort/recovery guards.

## Independent supervision prerequisite

The separately sealed non-GPU fixture remains untouched. The work copy carries
the reviewed assertion-only overlay: WAIT_TIMEOUT alone produces205/214;
unexpected wait errors produce206/217. All six planned cases use that overlay.

The first external runner's85s effective target allowance was rejected statically
before execution. Revision1 gives full90s target waits (1200s for compilation),
120/1230s outer budgets and up to10s extra cleanup confirmation. Creation-time
JOB_LIST + HANDLE_LIST, noninherited kill-on-close jobs and file-backed output
remove dependence on pipe draining. Top-level bootstrap/final WinAPI calls are
not claimed to have universal hard-real-time bounds.

Revision1's first real compilation succeeded, but its run was NOT accepted:
the worker had exited while the outer Job still reported an active member.
The minimal Python pass reproduced that rejection. Two read-only diagnostics
preserve the reason: after the exact created process signaled, a job-owned
`conhost.exe` remained briefly and naturally exited about10.34ms later; the
exact process/thread handles remained held and no termination was requested.
This is not proof that every driver/helper exits within that time.

Reviewed revision2 permits at most1s natural settling within the existing cleanup
deadline. It still requires actual job active-zero; needing forced termination
after an otherwise normal target exit still fails. Timeout/exception cleanup
remains immediate. No executable-name/PID exception or global process kill exists.
Seven scripted cleanup-policy tests and the real minimal pass are GREEN. A real
30s lingering child is forcibly cleaned up and correctly rejected (wrapper125,
target0, guard false, both owned jobs empty). Original failures are preserved;
successful forced cleanup is never relabeled as successful execution.

## Actual focused results

Six serial non-GPU cases pass their exact oracles: original interruption0,
original no-drain0 (48.219s), late-job mutation205 (3.235s), restored then only
blocking-output mutation214 (60.250s), restored interruption0 and no-drain0
(48.203s). Both mutations compiled successfully; setup/wait failures are not RED.
All runner-owned jobs confirmed actual active-zero without forced runner cleanup.

Native WARP74333 with debug/GPU-validation, pumping and parked: omit-purge alone
produces complete real-removal stages1–5 followed by CHILD_FAIL44/parent94.
Restoring exact source and rebuilding gives full eight-stage CHILD_PASS0/parent0
for both modes; Intel73219 hardware passes both restored modes too. Independent
raw-log review verifies distinct caller/owner threads, matching actual LUIDs,
native HRESULT0x887A0005, both generations1→2 and final cleanup. The tests also
assert exact pixels, preserved CPU/capture ownership and pacing epoch closes.
No source mutation remains. Source/binary/runner/selector hashes are per run.

Actual-source Standards and Spec/evidence independent reviews have no remaining
actionable findings within this scope. The final source freeze has3243 files;
exact discovery is Windows946/Linux698, including native201 and architecture248.

## Regression and disk-maintenance status

The initial full native attempt used the root compile-budget launcher's unintended
MCPP_JOBS4 default:188 passed/13 window-interaction tests failed. That attempt is
retained and excluded from serial acceptance. The native-menu case passes unchanged
with explicit jobs1. The first full jobs1 workspace run then exhausted its1200s
budget compiling cold test targets; it is partial, not accepted. Owned cleanup
confirmed empty jobs. All logs and intermediate results remain unchanged.
No production fix or proved universal cause is inferred from that contrast.

The same frozen source was checked in full before a jobs4 --no-run build (36.907s)
and again before the warmed jobs1 run. That run completes in275.32s: all39 members,
**946 passed,0 failed**, including native201, architecture248, common107 and the
unchanged production foreign-thread refusal regression. Exact inventories and
member/test ordering, no skips,52 real WARP74333 markers, Vulkan six-phase reuse,
runner/worker/command bindings and all3243 post-run source hashes pass the strict
separate-stream auditor. The warm run has its own full preflight; the earlier
timed-out run's snapshot was created during that run and is not mislabeled preflight.

An independent auditor review found a missing manifest-self-hash/same-set gate.
The original auditor is preserved. A pure external control removes one manifest
production row: the old auditor falsely accepts it, the corrected auditor rejects
at the manifest SHA gate. The corrected auditor accepts the actual complete run.
This fixture is auditor evidence, not another native test run.

The existing authorized WSL mirror was source-checked against its old3231 marker,
updated to this3243 source and checked afterward. The new portable structure test
passes. An initial direct-mcpp invocation used the full-debug profile; the accepted
replay uses the repository `scripts/dev/test.sh` wrapper and its dev-lines profile
(one test,3.05s). The existing SubOS metadata warning is retained; no hermeticity,
full698 Linux-suite or new native macOS result is claimed.

D: again reached0 during full compilation. Authorized cleanup removed1176 exclusively
opened, unused **true ASan** `.obj` cache files (1,334,752,485 logical bytes). Missing
objects will be rebuilt at the next ASan validation. No source, SDK, evidence,
library or executable was deleted. Earlier non-forced compact calls changed no
file bytes but recompressed0 files; they are not credited with reclaimed space.
Subsequent explicit LZX recompression preserved SHA256 for970 completed normal
executables. The in-use files were skipped; no global process was killed. Complete
maintenance logs are external and the C:-saved deletion log was copied hash-exactly
after space became available. Observed free space afterward exceeded3GB onD:.

No automatic production foreign recovery, active BUNDLE safety, DRED completeness,
hardware-reset/TDR, default-backend readiness or overall DX12 completion follows.
