# DX12 owned DRED scalar status

Baseline: `6ee4998b16a7876d2a206bed75b42ca1352d08af` (P0 idle foreign native
retirement probe). Root evidence: `D:/Dev/shots/cgpui-dx12-dred-20261124/root-attempt-1/`.

## Scope and ownership

Implement the independently reviewed `scalar-status-r0/implementation` and
`native-controls-revision-1`, using only `p0-basis-integration` additive registration.
Five private declaration/data leaves and five focused implementation leaves own
setup observation, local-lease policy, synchronous native scalar reads, formatting,
and debugger output. No public API or backend/window selection change.

This is **not complete DRED**: no borrowed breadcrumb/page-fault output is fetched,
traversed or retained. Device-wide capture-once and external-owner lifetime remain
open. Debug-off records typed skips; debug-on setup failure does not suppress a
later interface query. Actual native removal reason, not cached retirement proof,
determines reporting. Reporting does not authorize teardown.

## Execution

- [x] Verify HEAD, clean source baseline (user `.vscode/` preserved), all 3243
  source hashes and three candidate seals.
- [x] Integrate 30 new sources and eight existing-file deltas; verify every output
  against reviewed hashes. Combined one-sided-context patch requires
  `git apply --unidiff-zero`; initial normal apply-check failed without changes.
- [x] Compile no-run jobs4: eleven DRED executables built, no tests run.
- [ ] Run focused tests serial jobs1 after the combined implementation batch.
- [ ] Observe six separately compiled CPU/API semantic RED controls and restore
  exact GREEN bytes; compile/link errors cannot count as RED.
- [ ] Observe three native miswiring RED controls and restored GREEN, then seven
  native programs on actual hardware/WARP with per-program identity and requested
  debug/GPU-validation classification. Nodebug always forces GPU-validation0.
- [ ] Freeze new actual source and execute expanded regression matrix. Full suite:
  no-run jobs4 before jobs1. ASan object cache was previously cleaned and must rebuild.
- [ ] Independent actual-source Standards and Spec/evidence review; document real
  failures and limitations, commit locally without pushing.

Only root runs engineering builds/tests, GPU work and WSL. Agents prepare external
helpers or review read-only for this slice. Existing P0/reuse/capture/pacing/native policy tests
must remain intact. No completion claim follows from candidate review or hashing.

## Cadence update

The user's later instruction, `加速开发，全部实现后再验证`, defers all remaining
execution to the combined implementation batch. DRED scalar code is integrated,
not runtime-accepted. Recovery and mailbox implementation may add legitimate
lease lifecycle wiring alongside it; the original integration binding is a
historical snapshot, not a hash assertion for subsequent source changes.
