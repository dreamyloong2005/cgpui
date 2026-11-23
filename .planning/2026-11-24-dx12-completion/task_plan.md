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
