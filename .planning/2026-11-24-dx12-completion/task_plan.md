# DX12 completion goal

Date: 2026-11-24. User goal: 做到DX12全部完成再停下.
Status: ACTIVE; no claim of full parity, default readiness or release.

## Current execution cadence

The user now requests **implementation first, consolidated verification afterward**
(`加速开发，全部实现后再验证`). See
`.planning/2026-11-24-dx12-implementation-batch/task_plan.md` for current parallel
implementation. Older entries below describe their historical acceptance only;
they do not justify repeated full matrices during this unfinished batch.

## Authority and boundaries

The original phased design remains available read-only at
`D:/Dev/Projects/cgpui-dx12-design/docs/superpowers/plans/2026-11-22-dx12-renderer-backend-plan.md`.
Its topology details and some proposed API/packaging/recovery mechanisms predate
implemented decisions. The current owning code, tests, current backend document,
and later acceptance records take precedence over stale implementation recipes.
Do not rewrite or build the separate design checkout or the user's Linux checkout.

Preserve modular development, private presentation policy, native menus and
native-first Popup with failure-only Overlay. No app-selected window mode,
backend blacklist, hidden menu replacement, or false successful present after
loss. Windows default/auto is now a local DX12-first validation candidate after
focused readiness; complete activated-source acceptance is still required.
Explicit DX12 never silently falls back. No remote push without human approval.

## Current completion checklist: implementation versus acceptance

This source audit supersedes stale "candidate/unimplemented" descriptions in the
historical progress entries below. An implementation check is not acceptance of
the current combined source, a default switch, or a release claim.

### Implemented

- [x] Every public DX12 Renderer/RenderFrame operation has an implementation;
  there is no remaining primitive stub. Final-upload-per-ID image batches,
  valid-missing-image skipping, successful capture publication/history and
  healthy acquired-frame lifetime are implemented. The thirteen shared native
  pixel scenes select the requested backend; current regression is a separate gate.
- [x] Required shared CPU geometry/planning and neutral public leaf ownership
  are implemented. Historical `compat_*` forwarders intentionally preserve the
  old entry points without duplicating algorithms; deleting them is not a goal.
- [x] Geometry/texture staging pages, SRV descriptor pooling, conditional dirty
  glyph transport, command recording reuse and opaque/composition frame admission
  are implemented. The steady-state probe exists; allocation reductions are not
  FPS, GPU-time or latency parity evidence.
- [x] DX12 retained-image accounting and bounded owned DRED status exist.
  Debug breadcrumbs now use guarded exact reads into bounded owned numeric
  samples, explicitly best-effort/non-atomic; final runtime verification remains.
- [x] Managed-owner token dispatch, bounded DX12 native retirement/reconstruction
  and Runtime delayed recovery/latest-desired-resize handling are implemented.
  Busy owners never authorize purge; unsupported endpoints and timeout remain
  explicit failures. Vulkan device reconstruction parity is also required by the
  original plan (4.7 / phase6 / decision3), and is now being implemented. The
  earlier exclusion here was incorrect; this batch is not yet validated.
- [x] Eligible fresh system-title-bar and menu-free client-title-bar ordinary windows have a private
  composition-first opaque route and dynamic alpha replacement. Existing flip
  history remains refused. Native Popup/initial-alpha sticky behavior remains.
- [x] Manual hosted WARP/common CI source and desktop prerequisite/runner command
  owners exist. Desktop scheduling deliberately remains blocked without externally
  audited authorization; local workflow files are not deployed-CI evidence.

### Remaining implementation and capability boundaries

- [ ] Finish and validate Vulkan exact-device-loss reconstruction, retained
  image/glyph restoration, stale-frame rejection and terminal failed-rebuild policy.
- [ ] Validate strict DX12 high_performance/minimum_power adapter preferences
  and the strict Vulkan index selector (original plan 4.1); explicit requests
  never silently fall back or change their selected adapter during recovery.

- [x] Extend the existing dynamic route to eligible client-title-bar ordinary
  windows without HMENU. Promotion rechecks current menu/chrome eligibility;
  four additional native regressions are authored and awaiting execution.
- [ ] Resolve broader required window compatibility with an actual supported
  mechanism, or retain an explicit documented capability boundary: client title
  bar plus HMENU, hidden/undecorated ordinary alpha and pre-26100 Windows are not
  made supported by primitive captures or removal of a guard. Dynamic return to
  opaque currently retains the active alpha lease as a bounded safety constraint;
  releasing it needs a compositor-completion/lifetime design, not merely a
  successful asynchronous DComp Commit.
- [x] Implement the original debug-breadcrumb requirement without unsafe direct
  borrowed traversal: ReadProcessMemory, bounded exact reads, overflow/ring/
  mutation checks and explicit partial outcomes. No diagnostic result controls
  recovery. This does not promise an atomic/coherent device-wide snapshot or
  full DRED; page-fault enrichment was not required by the original plan.
- [x] Implement automatic DX12-first creation with typed capability failure plus
  explicit checked same-HWND rollback before initialization-only Vulkan fallback.
  Focused native 23, architecture 257 and neutral 16 passed before applying the
  reviewed local default/doc/test activation. Repository default contracts are
  migrated; installed-SDK verification is being updated. Full activated-source
  acceptance is pending, not a release claim. Explicit requests never fall back;
  runtime backend switching is out of scope.

Readback/blur-target pooling and deeper frame overlap are optional optimizations,
not new mandatory completion items. Keep the current synchronous lifetime
contract honest rather than adding work solely to extend this checklist.

### Acceptance and external prerequisites

- [ ] Consolidated current-source behavior/structure review and Windows/WSL,
  available hardware/WARP/debug/GPU-validation/ASan regression. Historical slice
  counts do not certify the new combined source.
- [ ] Real desktop window/menu/transition and example/UI acceptance, comparable
  performance observations, Release build and installed SDK/source consistency.
  Different backend-native work counts are not required to be numerically equal.
- [ ] Actual hosted CI execution; desktop runner server-side authorization and
  the subsequently reviewed scheduling change. Do not replace authorization with
  repository booleans, labels or a local successful run.
- [ ] Reconcile current documentation and readiness claims with the resulting
  evidence; preserve unavailable old-Windows/physical-monitor/driver/macOS limits.
  No remote push, publication or goal-complete claim follows from source completion.

## Evidence limits

Do not interpret 'all complete' as permission to fabricate arbitrary-driver,
physical-monitor, old-Windows or native-macOS validation unavailable locally.
Concrete platform/API constraints must be demonstrated and reported; continue
independent work rather than prematurely stopping or calling the goal complete.
Current compatibility forwarders and typed retry replace older plan recipes;
do not mechanically delete them merely to match obsolete names.
The existing synchronous model or unsupported capability must not be silently
redefined as complete solely to close the checklist.

## Progress

- Baseline before current staging work: `241b226a`, branch `phase-v`.
- Texture staging native RED: 128 resources after 768 texels and 3072 uploaded
  payload bytes pass. Implementation and frozen-source expanded regression passed in
  `.planning/2026-11-24-dx12-texture-upload-pages/`.
- Existing milestones are individually validated; their historical test counts
  and limitations are not current full-completion evidence.

- Next common-pixel/runtime/driver candidate code is being prepared outside the
  repository under `D:/Dev/shots/cgpui-dx12-common-parity-20261124/candidates/`
  while this source stays frozen. Candidates are not applied, built or accepted.

- Common pixel/runtime/driver slice accepted after a fully refrozen rerun:
  `.planning/2026-11-24-dx12-common-parity/acceptance.md`. Thirteen real shared
  scenes now select DX12; native runtime root/child retry acceptance is present.
  This closes those bounded test gaps, not the remaining public-operation audit,
  cross-thread recovery or full window compatibility. Unsupported-blur fallback
  has reviewed external A/B candidates; pacing/CI proposals remain unimplemented.
- Unsupported backdrop contract now matches public tint-only semantics; accepted
  `.planning/2026-11-24-dx12-blur-fallback/acceptance.md`. Native injected-capability
  RED/GREEN, full783/648 and102-per-adapter matrix passed. No default/Popup changes.
- Opaque HWND admission/runtime readiness accepted in
  `.planning/2026-11-24-dx12-frame-pacing/acceptance.md`: real first-frame RED/GREEN,
  native scheduling/lifetime controls, honest frame-owned CPU samples and bounded
  direct-client test migration. Frozen2940path matrix818Windows/664WSL,
  116native per adapter and ASan, common94-per-backend plus installed SDK passed.
  Composition pacing was disabled at that milestone; the following slice is now
  being validated in `.planning/2026-11-24-dx12-composition-pacing/`. This is not
  full pacing/performance, whole-backend or default-switch completion.
- Composition admission accepted in
  `.planning/2026-11-24-dx12-composition-pacing/acceptance.md`: native first-ready
  without warmup, real resize/partial construction/cohort/orphan handle epochs,
  final2969path matrix Windows827/WSL667 and122native per adapter plus ASan/SDK.
  Prepared diagnostics/CI/window/recovery work and remaining public-operation
  contract differences are indexed in `next-slices.md`, not marked implemented.
- Capture publication accepted at
  `.planning/2026-11-24-dx12-capture-publication/`: true cold/warm native late-debug
  RED/GREEN, structure226/native123 and six rebuilt composition probes passed.
  Frozen2971path Windows829/WSL668, both all-member builds, native123 per adapter,
  common94 per selected backend, ASan123/common94+94 and installed SDK passed.
  Source matching and exact inventories pass; this is not whole-DX12 completion.
- Retained-image candidate was rebased around capture publication and independently
  source-reviewed on both axes; no runtime evidence yet. The DRED breadcrumb
  candidate must not be applied as-is: a new device-wide retirement review found
  peer-release-before-later-copy ordering is not protected by per-state gates.
  This is a proved ordering gap, not a proved UAF. See the external
  `cgpui-dx12-dred-20261124/candidates/retirement-boundary-review.md` before integration.
- Retained-image payload has actual StageA exit40 / StageB GREEN, structure227,
  native131 WARP-debug and neutral schema passes, plus actual-source two-axis
  review0. Accepted frozen2986path matrix Windows839/WSL670, native131 per adapter,
  shared95 per selected backend, ASan131/95+95 and fixed installed SDK counts;
  exact inventories/source matches. See `.planning/2026-11-24-dx12-image-retention/`.
- Public-operation follow-up now has primary-source contract evidence and an
  external revised orphan-frame A/B candidate. Original plan/ledger explicitly
  preserve frame-outlives-facade; healthy same-epoch completion must be separated
  from permission to rebuild. Candidate only, no applied/runtime claim.
- Performance audit identifies actual missing work, not only missing numbers:
  recorded-command reuse, pooled SRV slots and dirty-region glyph GPU copies.
  Existing staging allocation wins/pacing are not FPS or steady-state latency
  proof. External report: `D:/Dev/shots/cgpui-dx12-performance-20261124/audit.md`.
  A small noncapture benchmark is being prepared, not executed or accepted.
- User explicitly chose existing Vulkan-compatible last-upload-per-frame and
  latest-successful-capture history. Decision in `public-contract-decision.md`;
  prepare separate exact common regressions and deliberate DX12 migration, not
  source changes during the completed retention freeze.
- Orphan lifetime compatibility implemented with actual RED/GREEN and independent
  actual-source two-axis review0: focused ownerless finish/no-rebuild policy,
  true late native removal, first-Commit cold Popup and alpha lease desktop
  checks. Accepted structure228/native136; frozen2997path Windows846/WSL671,
  all builds, native136 per adapter, common96 per selection, ASan136/96+96,
  SDK and exact inventory/source checks pass. See orphan-frame acceptance.
- Capture-history candidate now independently rebased onto the actual orphan
  source; image-batching A/B has independent Standards/Spec0. Both external only,
  neither compiled/applied/accepted. Benchmark revised candidate closes three
  source-review gaps but is likewise unexecuted. Shared SRV-pool design exists
  with same-page blur tuple and safe retirement requirements, not implementation.
- Capture history accepted after a second unchanged-source frozen matrix:
  `.planning/2026-11-24-dx12-capture-history/acceptance.md`. Windows847/WSL671,
  native136, common97, ASan/SDK and2998 source consistency pass. First-run native
  opaque-menu timeout remains recorded/unlocated, not claimed repaired. Image
  batching is next. Human also explicitly accepted engineering recommendation
  to skip valid not-yet-uploaded images with honest diagnostics while rejecting
  invalid requests/native errors; a separate follow-up candidate is underway.
- Frame-batched images accepted in `.planning/2026-11-24-dx12-image-batching/`:
  actual shared A three Vulkan pass / three DX12 semantic exit40 -> B both pass;
  focused native139/structure229, first full frozen3011path Windows854/WSL672,
  all builds, native139 per adapter, common100 per selection, ASan/SDK all pass.
  Next is approved missing-image skip + truthful Vulkan diagnostics and strict
  malformed-input controls. Source-reviewed external candidates include added
  native Vulkan reuse oracle; its first version had an observer identity gap
  and a separately preserved revised candidate is awaiting re-review/execution.

- Valid missing images accepted: DX12 skips legal unavailable images with honest
  diagnostics, Vulkan accounts actual successful fresh/reused selection and
  rejects invalid inputs before cache mutation. Final verified-flush frozen3038
  matrix Windows864/WSL677/native140/common107+ASan+SDK passed strict inventories
  and sixphase native reuse trace. Two earlier evidence attempts retained: Xvfb
  reset race fixed independently6b2de264; ASan stdout tail fixed explicitly in
  the test, not by weakening oracle. See missing-image acceptance. Baseline
  benchmark is next, SRV pool afterwards; overall completion gates remain open.

- Dedicated steady-state baseline accepted: manual bin/four portable pure tests,
  structure232, checked-in bounded runner and docs, verified optimized368-unit
  Release build.18 Intel/Vulkan+DX12 series/10800 steady samples plus real native
  negative controls pass; source-backed exactbatch auditor correction preserves
  original failure and source semantics. First frozen3063 full Windows869/WSL682,
  native140/common107+ASan+SDK/fixtures/sourcechecks pass. See steady-state
  acceptance for scope and lack of speedup claim. Next SRV baseline rebase is
  independently preserved/reviewed, still uncompiled/unexecuted.

- SRV descriptor pool accepted in `.planning/2026-11-24-dx12-descriptor-pool/`:
  real six native topology RED/GREEN; stable4096-slot pages and move-only leases,
  exact16new tests, architecture233. Frozen3093source Windows886/WSL683,
  native156 per adapter and ASan, common107 per selection and installed SDK pass.
  Initial short-path fixture skips were NOT accepted: same-source full PS5/PS7
  suites reran with real system TEMP and strict combined audit passed. Independent
  Standards0/Spec0 after two coverage repairs. New optimized371TU Release plus
  18fresh runs, six negative controls and two adapter smokes pass. Independent
 36run comparison supports heap topology reduction, not FPS/memory-byte gains.
  All original evidence/failed preparation attempts preserved. Glyph dirty upload
  and command reuse remain external candidates; full completion remains ACTIVE.

- Glyph dirty transport accepted in `.planning/2026-11-24-dx12-glyph-dirty/`:
  real held/released transport RED/GREEN, InfoQueue/readback and actual-copy
  mutation controls; COW cloneP+dirtyD with separate G/P lifetime and checked
  geometric CPU lineage metadata growth. Frozen3158source second matrix passes
  Windows915/WSL688/native180 per adapter/common107 both per adapter/ASan/SDK.
  First matrix stopped at a proved WSL path-marshalling script bug; second
  pipeline passed but its initial SRV-only ASan identity audit was too narrow.
  Independent actual-source review and18negative/positive controls validate the
  separate audit-only correction using existing flushed native FIRST_BEGIN facts;
  all original strict gates and failed evidence remain. No performance gain or
  whole-DX12 completion claim. Command reuse native revision1 plus supplemental
  lifetime controls are independently source-reviewed external candidates only.

- Conditional native command recording reuse accepted in
  `.planning/2026-11-24-dx12-command-reuse/`: actual native RED/GREEN on three
  adapters, real ExecuteBundle-omission pixel RED,20focused lifetime/signature/
  mutation/parent/facade controls and247structure. Independent final actual-source
  Standards0/Spec0 with author exclusions separately covered. First3231file
  frozen matrix and strict audit pass Windows944/WSL697/native200 each adapter/
  common107both each adapter/ASan200+107+107/real-TEMP drivers/SDK104DX12object
  closure. No FPS/latency/total-memory/default selection claim. Unused proof
  authorities removed after actual structure RED/GREEN. P0 foreign-thread
  tests-only integration and DRED scalar no-traversal candidates remain external,
  unexecuted; window/background/chrome/HMENU, CI/native-platform and release
  completion gates remain OPEN. Overall complete-DX12 goal remains ACTIVE.

- Idle foreign-thread native-retirement prerequisite accepted in
  `.planning/2026-11-24-dx12-cross-thread-probe/`: tests-only source,6 supervision
  controls, WARP pumping/parked true omit-purge RED44/parent94 then restored GREEN,
  Intel pumping/parked GREEN; independent actual-source/evidence review0. Frozen
  3243 source passes warmed jobs1 Windows946/native201/architecture248 and the new
  WSL structure guard. Incorrect jobs4 native run and cold-build1200s timeout are
  preserved, not counted as acceptance. Production foreign-thread refusal remains;
  captured DIRECT does not prove active-BUNDLE/origin-gate-held automatic recovery.
  Next: independently reviewed private DRED scalar implementation plus actual-native
  controls revision1 (actual identity/request classification) and P0-preserving
  additive bridge. All remain external/unbuilt, and complete DRED is still OPEN.

- Consolidated local acceptance progress (2026-11-25) is recorded in the implementation-batch task_plan.md: Windows1004, native240 and common107both on Intel/NVIDIA/WARP, ASan240+107both, full39Release/localSDK127objects and typed-WSL715 pass. Performance desktop precondition and external CI remain open; no overall completion/release claim. Evidence and failed attempts are retained.
