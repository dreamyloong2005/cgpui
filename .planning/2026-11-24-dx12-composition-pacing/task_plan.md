# Composition frame admission (accepted)

2026-11-24. Baseline `4d29c4e5` (opaque admission milestone fully accepted).
Overall DX12 goal remains active. Root is sole build/native executor; no push.

Evidence: `D:/Dev/shots/cgpui-dx12-composition-pacing-20261124`.
Candidate/design source: prior frame-pacing evidence `candidates/composition-probe`
and `composition-pacing-probe-plan.md`. External sources were independently
reviewed before application; that is not execution evidence.

## Observed

1. Applied focused tests-only A/current-renderer first-begin and B/explicit-native
   probe, incrementally registered support/test/bin and architecture CI entries.
2. Actual A WARP+debug hidden/visible both semantic RED exit40: returned real
   composition frame with sets/gets/polls/ready0. No setup failure/warmup/Commit.
3. B built through scripts/dev/build.ps1. Actual six rows Intel/NVIDIA/WARP x
   hidden/visible all exit0: flagged composition chain, SetMaximumFrameLatency1,
   app-owned handle, first native ready before any Commit/submission, same-chain
   64->96 and96->64 resize preserving creation flags/grant policy, three distinct
   premultiplied GPU captures and exact acquired1/close1. Visible rows additionally
   validate real desktop RGB; hidden rows explicitly do not sample desktop.
   This is a manual native experiment, not production begin/recovery acceptance
   or a universal OS/driver guarantee. Per-row JSON records actual OS and exit.
4. Minimal production change in a focused private constexpr flags owner requests the same
   waitable flag for both routes and both creation/resize, configures pacing true.
   No fallback on timeout, failed wait or initialization error. Existing failures
   remain failures; Popup's normal native-first failure handling is not changed.
   Creation derives pacing enablement from its actual descriptor flag. Same-chain
   resize has its own focused implementation and does not reacquire the handle.
5. A hidden/visible actual-renderer WARP GREEN with one Set/Get/ready/close per
   case; first native regression118/118 passed. Three new strict-forward actual
   renderer resize/cache, RemoveDevice/replacement and orphan lifetime cases then
   passed with native121/121. These are pre-freeze checks, not the final matrix.
6. Independent Spec review found missing strict cold-resize and partial/cohort
   handle-epoch accounting. Added a zero-poll64->96 first-begin plus post-Present
   96->64 resize case. Existing visible Popup lifetime/recovery tests now retain
   their pixel/borrowed-capture assertions while observing exact failed-construction,
   orphan purge, live peer purge/lazy rebuild and final native handle close counts.
   Source re-review0 remaining; expanded native122/122 WARP+debug and
   architecture225/225 passed.2969source paths frozen; final serial matrix passed.

## Completed gates

- Same A semantic GREEN through actual renderer (done in pre-freeze WARP).
- Strict-forward real composition lifecycle/resize/abandon/removal/orphan test
  (three new tests and added review-gap coverage passed).
- Existing full native ordinary/client-alpha/Popup/recovery matrix and exact
  composited desktop pixels, plus structure. Do not infer ordinary-window alpha
  compatibility expansion from the Popup probe.
- Updated live docs to new policy and measured limits, independent reviews,
  freeze and complete Windows/WSL/hardware/WARP/ASan/SDK consistency matrix.
- Local commit follows full gates; explicit experimental bin is not a default
  required-pass capability test and must remain clearly marked as experimental.

Final source2969 matrix accepted in `acceptance.md`: Windows827,WSL667,
native122 each adapter,common94 per requested backend,ASan and installed SDK.
Final driver exit0; exact native inventory/no skips and raw/normalized source
consistency audited. Preserve user .vscode, Linux and design checkouts. Overall
DX12 goal is still active; this acceptance does not close the remaining worklist.
