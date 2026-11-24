# Frame-batched image compatibility

2026-11-24. Overall DX12 goal remains active; local commit/no push.
The direct human choice in completion/public-contract-decision.md requires
Vulkan-compatible final-upload-per-ID within a frame, without developer options.
Capture history is a separate preceding slice and must remain intact.

Ownership: frame-local pending owned ImageAsset bytes and deduplicated invalidation
IDs, private pending-draw variant at its original command position, focused
queue/preparation/resolution leaves. Present preflight precedes the CPU prefix:
invalidations -> final uploads -> resolution. Native submission/retirement,
success-only completion and weak immutable-snapshot accounting keep their owners.
Abandonment does not apply the batch; arbitrary failed Present rollback is not
promised. Missing-image policy is unchanged, not claimed to have full parity.

## Execution sequence

1. Finish frozen capture-history matrix and commit before applying image changes.
2. Apply only Stage A from the rebased external candidate, compile through mcpp,
   run identical explicit Vulkan and DX12 last-upload/prefix/abandonment tests.
   Actual successful native Present followed by semantic exit40 is the expected
   DX12 RED, not renderer setup/build failure.
3. Apply Stage B after checks, preserving capture/orphan/retention. Rerun the
   exact scenes; new CPU/validation/orphan tests plus retained-image/upload-pool
   migrations and all structure tests. No caps or unrelated assertions weakened.
4. Independent actual-source Standards and Spec review, full frozen serial
   Windows/WSL/Intel/NVIDIA/WARP/debug/GPU validation/ASan/fixtures/installed SDK
   acceptance, exact inventory/source audits, then local commit.

External source candidates and reviews:
`D:/Dev/shots/cgpui-dx12-public-contract-20261124/images/candidates/rebased-capture/`.
Implementation candidates are not runtime evidence. Root alone runs project
build/test/GPU. User .vscode and unrelated checkouts remain untouched.

## Actual progress

- Preceding capture slice committed as0f6b2c6e after full unchanged-source rerun.
- Stage A applied alone and compiled. Identical shared scenes pass Vulkan/Intel
  (3/3) and fail DX12/WARP (3 semantic exit40). Last-upload reaches successful
  native Present/capture with wrong pixels; invalidation-prefix positive control
  passes before cache loss; both abandoned replacement and invalidation differ.
  Logs: `D:/Dev/shots/cgpui-dx12-image-batching-20261124/stage-a-both.log`.
- Stage B applied after whitespace/applicability check; first build in progress.
  Independent actual-source two-axis reviews requested, full acceptance pending.
- Human now accepted missing-resource skip with honest diagnostics and strict
  invalid-input/native-error boundaries. This is a separate next slice; do not
  claim it from this batch implementation's intentionally unchanged rejection.
- Stage B exact shared three scenes now pass Vulkan/Intel and DX12/WARP. Full
  native139 WARP/debug and architecture229 pass. Independent actual-source
  Standards0/Spec0; all30 slice files match reviewed candidate normalized text.
  Installed image-batch verifier independently source-reviewed0 (not yet run).
- Source frozen at3011 files; full serial matrix started in
  `D:/Dev/shots/cgpui-dx12-image-batching-20261124/final`. No further production,
  test or documentation edits until source/inventory audits finish. Expected
  Windows854/WSL672; derive native139/common100 inventory from actual manifests.
- Full frozen driver exit0, all expected counts observed (not inferred), hardware/
  WARP/debug/GPU/ASan/fixtures/SDK pass. Native139/common100 exact inventories,
 3011 source consistency pass. Accepted locally; missing-image follow-up next.
