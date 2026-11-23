# DXGI pacing and its runtime prerequisites

2026-11-24. Baseline `3f2df20f`; overall goal remains active. No new commit yet.
Root owns all builds/native execution; preserve user .vscode. No default/Popup
policy changes; no native wait can substitute for GPU retirement evidence.

## Current integration (not final acceptance)

1. Win32 delayed-timer allocation failure:
   - Applied external A from `D:/Dev/shots/cgpui-win32-timer-readiness-20261124`.
   - Creation-error control passed. Actual message-queue test RED exit2: failed
     future timer posted immediate wake. Actual factory RED exit1: exposed app
     without native delayed timer. Original logs retained under evidence root.
   - Applied B; complete platform suite72/72 passes. Architecture214/214 passes.
   - Private creation seam only; production factory always real native API.
     Captured GetLastError preserved; failed app not exposed; old immediate
     delayed-wakeup fallback removed. Module caps unchanged. Full matrix pending.
2. DXGI private core stageA:
   - Applied reviewed stage-a.patch from `D:/Dev/shots/cgpui-dx12-frame-pacing-20261124`.
     Merged architecture registration additively, preserving timer/blur guards.
   - Four tests build and pass on WARP+debug: core/error scripted contracts and
     genuine native waitable chain/capture/lifetime. Native log: set1/get1,
     polls5, ready3, pending2, close1; counts are one run, not timing guarantees.
   - StageA is NOT production pacing: manual test fixture coordinates admission,
     Present consumption and reset. No production state/begin/present hook yet.
3. Neutral runtime readiness remains external, unapplied:
   - `D:/Dev/shots/cgpui-frame-readiness-20261124/candidates` A/B prepared.
   - Independent review found P2: successful child cancellation can reenter
     transport and close child before dereferencing its renderer. Author is
     adding post-cancel root/child lifetime rechecks and actual transport tests.
   - Present-returned pending must stay fatal; strengthened exact error assertions
     will make that A test RED too. Wait for revised candidate/review before apply.

## Required next order

- [done] Timer A real RED and B local platform/architecture GREEN.
- [done] Core A compiled/scripted/native bounded acceptance; not production activation.
- [pending] Revised neutral readiness A meaningful actual-runtime RED, B GREEN.
- [pending] Actual production begin-frame admission RED before B hook activation.
- [pending] Integrate owned core with swapchain, begin, every actual Present attempt,
  same-chain resize and safe retirement/recovery; zero-time pending must schedule
  future runtime redraw, not hot-loop or publish fake successful frames.
- [pending] Direct/native runtime semantics, root/child/worker, abandoned frames,
  device recovery and no lost uploaded images on pending.
- [pending] Honest timings; composition route is currently flags0/disabled and is
  not claimed paced. Need separate capability/native validation before enabling.
- [pending] Independent final reviews, frozen full Windows/WSL/hardware/WARP/ASan/
  SDK and source-consistency matrix, local commit. No push.

External candidates are review material only; do not copy old manifests over
current registrations. Original design/analysis is in completion/frame-pacing-next.md.

## Neutral readiness integration follow-up (2026-11-24)

This update supersedes item 3's external/unapplied status and completes the
neutral-readiness A RED / B GREEN prerequisite above; it is not native pacing
activation or final matrix acceptance.

- Root applied revised A and B, preserving shared timer/pacing-core registrations.
  Observed `D:/Dev/shots/cgpui-frame-readiness-20261124/stage-a-red.log`:
  0 passed / 6 failed at actual-runtime assertions, not compilation failures;
  Present failed specifically at the strengthened exact code/message contract.
- Observed `D:/Dev/shots/cgpui-frame-readiness-20261124/stage-b-test.log`:
  6 passed / 0 failed. `stage-b-build.log` separately records all six built.
  The cancellation program includes root/child synchronous registration delivery
  and close-during-success-cancellation cases; Present also protects unchanged
  ordinary errors. A/B use identical test support and behavior sources.
- Read-only applied-payload audit: 28/30 paths match the final stage-B candidate
  as normalized source text. The only differences are the two shared architecture
  registration files, containing the intended additional Win32 timer and DX12
  pacing-core registrations. No readiness source, test, contract, or guard omitted.
- Independent spec review's post-cancel lifetime P2 is fixed: root/child recheck
  quit/shutdown and live resources after cancellation; child retains its original
  pre-cancel active check. A delivered marker prevents stale fired TimerId writeback.
- These six tests use controlled clock/event transport and actual WindowRuntime,
  not native DXGI admission, GPU retirement, performance, or cross-platform GPU
  acceptance. Full UI/architecture regressions remain owned/running by root;
  this audit ran no build/test and changed no source. Production admission RED,
  activation, native runtime semantics and the frozen complete matrix remain open.

## Neutral scheduling integration regression

Actual revised A compiled all six and all six failed on their intended runtime
contract assertions; B with the identical behavior tests passed6/6. UI full suite
then passed114/114. Architecture initially215/216: renderer_platform_recovery
structure still demanded direct runtime_retry_frame calls in the two redraw
owners. Tight isolated RED exit2 reproduced it. Guard now checks the actual chain
root/child→focusedreadiness→unchangedimmediaterecoveryhelper, preserves30-line cap,
typed recovery/no string or backend matching and forbids pending in that helper;
the same guard is GREEN. Original failed logs retained. Full matrix not yet run.

## Production admission activation (2026-11-24, not final acceptance)

- R1 configured ownership in the real renderer but left begin/Present untouched.
  Hidden and visible native begin cases both RED exit6: frame returned with
  polls0/ready0. Actual recovery/orphan ownership control passed.
- Root added three controlled-wait real-renderer regressions: initial/post-Present
  pending and retained capture, abandon/overlap/pre-Present validation, actual
  ResizeBuffers64->96, live poll failure and poll-boundary real RemoveDevice.
  All three RED exit3 before activation (`control-red-test.log`). Only their
  poll result is scripted, not native creation, handle ownership, draws/capture
  or device removal; they are explicitly not native-ready proof.
- Applied reviewed B focused admission leaf and post-Present consumption hook.
  `stage-b-test.log`: all10 pacing tests pass on WARP+debug. Strict native hidden
  and visible first-begin each observed polls1/ready1 without making hidden visible.
  Independent review's no-op resize coverage finding fixed with96x96 capture.
- Existing actual-runtime recovery tests first failed because their observer
  classified new pending as fatal. Added a separate begin_pending count, still
  returning the identical error and with no observer wait/retry. Both real native
  root-first/child-first tests now pass with pending1..4 and exactly one typed
  recovery retry per window (`runtime-adoption-green.log`). Original RED kept.
- Shared test-only bounded success acquisition helper reviewed0, applied; pure
  contract test passes. Only pending retries, absolute3s budget and1ms yield;
  native pump only for explicit matching owner. Existing negative/recovery/raw
  pacing calls must remain unwrapped during migration. No production wait loop.
- Full architecture run built219, passed211/failed8: new manifest registration
  reached243 lines and tripped historical240-line module caps. Other checks
  passed, including recovery routing and admission ownership. Resolve actual
  registry/module growth without relaxing caps; not final green/frozen source.

Still pending: caller migration and native message-pump control, real-runtime
controlled pending fairness/input/resize/close, timing sample semantics,
composition capability, independent final review and full frozen matrix/commit.

## Freeze/matrix follow-up (supersedes the partial list above)

- Caller migration applied/reviewed:142 successful acquisitions in62files,
  54raw typed-error/recovery/overlap/special cases retained. Original assertions
  unchanged. Native WARP/debug114/114 after migration; old84/28 log preserved.
- Helper contract and real Win32 queue controls2/2; R1 native-counter helper
  additionally preserves WM_QUIT and checks its absolute deadline after dispatch
  and yield. Neither helper retries recovery or Present.
- New actual Win32/runtime synthetic-admission fairness and close tests2/2.
  Native-timer backoff control also passes; temporary64->2 mutation fails, restored
  source passes. See review.md for observation/claim boundaries.
- Frame-owned admission CPU sample implemented: known17ns ledger A RED, B GREEN.
  Only successful current admission contributes; disabled0, no accumulated
  pending/holding/recovery time; public layout and shared saturation unchanged.
- Architecture registry genuinely grouped19DX12guards into its owner directory,
  root discover glob and exact recursive inventory/CI equality. All prior caps
  retained; manifest232<=240. Full222/222 after common-capture guard migration.
- Standards and Spec independent reviews0 confirmed findings; actual source
  frozen2940paths. Final Windows all-member build and818/818 tests PASS.
  Serial full-matrix driver is still running; WSL/hardware/WARP/ASan/installed SDK
  and final source matches are not yet accepted. No new commit yet.
- Next composition probe and DRED/RAM planning are external evidence-directory
  notes/candidates only, not applied production or completed capability.

Final matrix now completed exit0 on identical2940paths. All results and limits
are recorded in acceptance.md; no final-stage retry or native skip. Ready for
local commit, then lift freeze for the separate composition capability experiment.
