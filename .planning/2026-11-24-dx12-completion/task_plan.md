# DX12 completion goal

Date: 2026-11-24. User goal: 做到DX12全部完成再停下.
Status: ACTIVE; no claim of full parity, default readiness or release.

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
loss. Default/auto stays Vulkan until full readiness gates actually pass.
Explicit DX12 never silently falls back. Local commits only; no remote push.

## Completion worklist (must be audited against actual code)

- [complete] Finish reusable texture staging pages, preserve geometry and
  synchronous GPU lifetime semantics; quantify allocation and correctness.
- [pending] Audit every public Renderer/RenderFrame operation against Vulkan and
  original acceptance, separating implemented behavior from missing regression
  coverage. Add common real-pixel parity tests, not renamed tests alone.
- [pending] Complete shared CPU ownership and neutral public planner vocabulary
  where still required; avoid speculative extraction of API-specific code.
- [pending] Audit frame pacing/command reuse/descriptor allocation/dirty glyph
  uploads and actual memory/work behavior. Implement missing specified work and
  establish reproducible performance gates without claiming FPS from allocation
  counts. Optional readback optimization is not a substitute for semantic parity.
- [pending] Resolve required ordinary-window background transitions/chrome/menu
  compatibility using new evidence, not rejected recipes repeated unchanged.
  Maintain explicit unsupported errors until strict native desktop/interaction
  tests pass. Do not waive this gate just because primitive captures pass.
- [pending] Audit recovery contract end to end (runtime redraw after retry,
  multiwindow/foreign-thread peers, retained resources, Vulkan parity).
- [pending] Add reproducible WARP/debug/GPU-validation CI and full example/UI
  acceptance; verify diagnostics, retained-image accounting and release packaging.
- [pending] Only after preceding gates, implement and verify automatic creation
  preference/fallback, repository+installed SDK behavior, docs and release build
  acceptance. No runtime backend switching; no publication without authorization.
- [pending] Final independent Standards/Spec review, Windows/WSL regression,
  hardware+WARP/debug+ASan, installed/release SDK and source consistency checks.

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
