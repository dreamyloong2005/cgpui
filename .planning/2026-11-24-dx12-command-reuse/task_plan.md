# DX12 native command reuse

Date: 2026-11-24. ACTIVE slice; overall complete-DX12 goal remains ACTIVE.
Base commit:79f4146e375273dae2bcc5d70cae944b26ef86fc (accepted glyph transport).
Its3158non-planning source files match the candidate's glyph final-freeze basis.

## Ownership and policy

Private immutable BUNDLE owner, independent frozen geometry, exact bounded scene
selection/signature, strong immutable image pins retaining native nodes/sole SRV
leases, and focused G/P proof integration. DIRECT shell remains frame-local.
Only opaque HWND ready rect/image subset is eligible; capture/mutation/missing/
cold/glyph/blur/shadow/composition use unchanged DIRECT behavior. Ordinary blend
paint on an opaque HWND is not a transparent-surface exclusion. No public mode,
backend-default/Popup policy changes or silent native-error fallback.

G comes from actual completed ticket/confirmed removal. P comes from successful
parent Reset or actual parent release. Unknown completion and failed Reset retain
ownership. Success-only publication must not remain sticky across later attempts.

## Staged integration

- [complete] External native-integration-1 revision1 and supplemental revision1
  independently source-reviewed Standards/Spec. Original compile/structure/oracle
  findings and their separate revisions are retained; no runtime claim.
- [complete] Apply revision1 tests-red + observation/legal-emitter red-seams only;
  compile and prove real native pixels/queue order/identity/debug before expected
  missing-reuse exit40 on Intel/NVIDIA/WARP. No compiler/setup failure counts RED.
- [complete] Incremental GREEN only; unchanged oracle proves actual immutable
  BUNDLE Close plus two later native executions without re-recording or aliasing
  transient geometry. Queue-tail pixels use two queue submissions, not one.
- [complete] Apply supplemental controls: slot/P recycling, true transient poison,
  late InfoQueue failure/publication, actual completed-bundle cohort/orphan removal.
  Completed/CPU-unsubmitted controls do not prove executed-unknown removal.
- [complete] Focused/structure regressions; close remaining required failure,
  ineligible/mutation and executed-unknown removal boundaries with actual tests.
- [complete] Actual-source independent reviews, full frozen matrix, docs/acceptance,
  local commit. No push; no FPS/total-memory/full-frame reuse claim.

External basis:
`D:/Dev/shots/cgpui-dx12-command-reuse-20261124/candidates/native-integration-1/revision-1/`.
Supplement:
`D:/Dev/shots/cgpui-dx12-command-reuse-20261124/candidates/supplemental-tests/revision-1/`.
Root exclusively owns all builds/tests/GPU; agents prepare/review externally only.

## Actual execution notes

- Applied revision1 tests-red plus red-seams after3158basis-byte verification;
  all27overlay paths match reviewed normalized source. Initial real A build found
  C2397: observer aggregate assigned variable DXGI_FORMAT to uint64_t implicitly.
  This is a compilation failure, not semantic RED; prior source review missed it.
- Minimal explicit uint64_t cast preserves the actual IASetIndexBuffer argument
  and recorded enum value. Rebuild succeeded: one native oracle built, not run.
  A3170source manifest and the actual executable are retained in stage-red-actual.
- Real A oracle now passes targeted RED on Intel73219/NVIDIA74481/WARP74333:
  each first proves setup/pixels/native identity/debug and three same-frame tail
  captures (two queue submissions), then the exact missing-BUNDLE exit40. WARP
  GPU validation enabled. Runner exit0 and final PASS observed; all three use
  the same pinned executable and source manifest. Incremental GREEN is applied,
  all61reviewed paths match except the separately reviewed explicit enum cast;
  compilation and actual GREEN execution remain pending.
- Incremental GREEN built seven focused tests; unchanged native oracle succeeds
  on all three adapters with exact same scoped pixels/history checks. Seven
  focused WARP/debug/GPU-validation tests pass. Supplemental revision1 applied
  additively, seven new paths match; all11focused tests pass on the same settings.
- Actual execution sensitivity: omit only real ExecuteBundle while preserving
  observer metadata. Mutation compiles, WARP returns ordinary exit4 before the
  queue-tail pixel success marker. Exact source restoration/rebuild followed by
  all-three-adapter native GREEN succeeds. No compilation failure or telemetry
  counter alone is used as semantic RED.
- Full242architecture run reports241pass/1fail: the shadow ownership guard still
  expects direct dispatch in presentation rather than the new focused frame-body
  owner. Original failure retained; targeted guard migration preserving the full
  callchain and both owners' PSO exclusion is being reviewed. No cap is relaxed.
- Reviewed shadow guard repair is now applied; rebuild and complete242architecture
  rerun both pass. Original241/242 failure remains in supplement-actual evidence.
- Added independently reviewed executed-unknown/actual-RemoveDevice and exact
  injected Close-denial controls. Real native producer/tail plus successful queue
  Signal precede the hidden-completion event denial; actual removal, not a forged
  completion, then authorizes recovery. This proves CPU knowledge is unknown, not
  that the GPU was physically still executing. Exact Close control proves both
  actual resets succeeded before the single target denial and no queue execution.
  All13focused tests pass on WARP with debug/GPU validation. These are preliminary
  tests, not the pending final frozen matrix.
- Public backend documentation now records the bounded opaque-HWND subset,
  private fallback/admission and G/P lifetime rules, separate queue-tail fixture,
  and no whole-frame/FPS/total-memory/default-readiness claims. Remaining authored
  signature/policy/native-transition/public-facade tests are under independent
  review before integration. No source mutation experiment is left active.
- Signature stage1 and stage2 revision1 are integrated after independent Spec
  and Standards review. The initial stage2 omitted undrawn invalidation; retained
  revision1 adds actual invalidate9999 with unchanged pixels/zero uploads and
  DIRECT->fresh->hit. All20focused WARP/debug/GPU-validation tests and all247
  architecture tests now pass. The earlier16-test source also passed Intel and
  NVIDIA; the final full matrix will test all20 on each adapter.
- Final Standards found two unused, undefined proof-friend authorities. Root
  first tightened the structure guard (actual successful build then exact RED),
  removed only their declarations/friendships, and obtained GREEN. Actual proof
  algorithms/API are unchanged. Independent final actual-source Standards has
  0remaining findings, with author exclusions covered by the other reviewer.
- Final proposed inventory is derived: Windows944, Linux697, native200,
  common107, architecture247,39members. This is not a pass count yet. Final
  tooling bundle is independently reviewed, preserving original-system-TEMP
  fixtures, owned WSL mirror, wslpath fix and reviewed ASan actual identity rule;
  fresh /MT graph validation now covers all DX12 production CPPs, not only glyphs.
- First frozen3231file matrix and strict audit pass: Windows944/WSL697,
  native200 on each adapter/common107 both per adapter/ASan200+107+107,
  real-system-TEMP fixtures, all39member builds and installed SDK bound to104
  actualDX12 /MT objects. See acceptance.md for exact bindings, expected PS5-only
  skips and disk-maintenance correction. No whole-DX12/default/performance claim.
