# Actual-source Spec / native-safety review

2026-11-24. Reviewer: `popup_structure`. **0 actionable findings.** This is
source-review acceptance of the image-batching slice, not runtime acceptance or
full Vulkan image parity.

## Scope and provenance

Reviewed `git diff 0f6b2c6e` plus the thirteen new implementation/test leaves
against `task_plan.md`, `../2026-11-24-dx12-completion/public-contract-decision.md`
and the external `images/design.md`. HEAD remains
`0f6b2c6eb4e24b5437ad559d1923ae697ef3d49a`. The thirty Stage B overlay files match
the actual tree after CRLF/LF normalization; this supplements independent source
inspection, not execution. Unrelated `.vscode` content was not reviewed or changed.

## Contract and safety evidence

- `dx12_image_batch.cpp` copies caller bytes before replacing the final value per
  ID and deduplicates invalidations. Recording does not mutate the image cache.
  Abandonment and already-latched recording errors never apply the prefix.
- `dx12_image_prepare.cpp` applies invalidations, final uploads, then resolution.
  Local swapped vectors release raw upload copies on every preparation exit,
  including exceptions. A discarded malformed intermediate is not validated;
  a malformed final upload still rejects. No arbitrary attempted-Present rollback
  is advertised.
- Pending draws own value data, are device-scaled once at recording, and resolve
  in their original payload slots without changing command order, kind, index or
  resource offset. Resolution retains finite-draw validation. Explicit guards
  reject unresolved markers before diagnostics and at native dispatch.
- Frame recording/native preflight precedes CPU preparation. CPU preparation
  errors return directly instead of poisoning native state. Healthy acquired
  orphans retain recording/preparation permission; removed/stale/window-invalid
  orphans reject before the prefix, without rebuilding.
- Native realization/retirement, orphan recovery, immutable cache installation,
  weak retention and success-only exactly-once aging owners are unchanged from
  the accepted baseline. CPU preparation does not move native pruning or upload
  reuse before idle/removal proof. Capture publication ordering is preserved.
- Test migrations retain cross-frame old/current immutable snapshots, actual
  red/green pixels, distinct-ID two-payload upload/staging assertions, and narrowed
  abandonment/early-rejection history checks. Removed-orphan, late-removal,
  late-error and capture-publication tests remain unchanged. Existing 120/121
  aging, recovery and upload-pool coverage still requires execution.

## Acceptance boundary

Missing-image skip is the explicitly accepted **next slice**; current rejection
is not a defect in this bounded batch slice. Timing documentation correctly
excludes the new prefix from selected diagnostic stage sums.

Root reports Stage A Vulkan 3/3 and DX12 three semantic exit40 results. This
reviewer did not independently inspect those runtime logs and executed no build,
test or GPU workload. Stage B GREEN and the full frozen acceptance matrix remain
root-owned and pending this source review.
