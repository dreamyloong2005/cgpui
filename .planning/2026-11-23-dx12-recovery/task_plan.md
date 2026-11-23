# DX12 milestone: bounded single-owner device recovery

Baseline `c1b72784`, branch `phase-v`; user requested continuation. Preserve
untracked `.vscode/`, separate design checkout and user's Linux checkout.
Local commit only, no push/release. Vulkan/auto remains default; no change to
Vulkan Popup/failure-only fallback, DX12 transparent/Popup rejection or public
window modes.

## Scope and contract

Recover only confirmed native device removal. Preserve HWND, framebuffer/DPI,
CPU image/glyph caches and immutable recorded snapshots. Retire all old native
resources, including texture realizations formerly pinned by finished frames.
Recreate on the original adapter LUID, never re-read adapter selection or silently
choose another backend/software device. Attempt reconstruction at most once per
public call. Publish generation+1 only after device, pipeline, swapchain and
native validation all succeed; failed attempts preserve generation and CPU state.

begin_frame/present report frame_retry_required only AFTER successful rebuild;
the lost frame is not presented or counted as a cache-success frame. The next
frame must genuinely render. resize cannot return a retry that runtime does not
understand: after successful recovery it must complete the requested resize or
return an error. Generic live-device sync/validation errors remain terminal.

D3D12 uses a process/adapter singleton. This slice is single-owner recovery, not
a multi-window coordinator: other renderers or external native references can
block replacement of a removed device. Return the actual bounded failure, never
fake progress/retry. A later call may attempt reconstruction after blockers are
released. Existing unprovable live-device retirement remains safely blocking.

## Intended ownership

- Immutable CPU texture snapshots contain no native resources. Private per-state
  texture realizations own native resource/descriptor/upload status, keyed by
  snapshot identity with weak CPU ownership. Retained draw handles keep pending
  submissions safe; idle frame-start prunes expired realizations.
- Private adapter/device initialization supports pinned-LUID reconstruction.
- Dedicated native cleanup and recovery modules own teardown and one bounded
  complete rebuild. Renderer/frame entries only integrate typed outcomes.
- New architecture tests guard module boundaries, existing caps, registrations
  and no public/native-mode leakage. No size cap is raised.

## Team and acceptance seams

- popup_structure: CPU/native texture ownership split only.
- gpui_windows_design: pinned adapter, native cleanup/rebuild, recovery state,
  focused architecture test. No frontend integration.
- repo_progress: public Renderer/RenderFrame real RemoveDevice tests, first tracer
  then expanded cache/lifetime/blocker/resize tests; runtime retry contract test.
- root: frontend integration, all registrations, docs/evidence and serial tests.

Use the existing public renderer/native-removal/pixel-capture seams and mandatory
structure checks. No new public or environment fault hook. All builds are owned
by root: MCPP_JOBS=4 no-run,1 execution. Evidence root:
`D:/Dev/shots/cgpui-dx12-recovery-20261123/`.

## Sequence

1. [done] Public native recovery tracer RED on baseline.
2. [done] Native ownership/rebuild and frontend GREEN.
3. [done] Cache/snapshot/blocker/resize/runtime regressions and reviews.
4. [done] Frozen Windows/WSL/native/SDK/ASan verification.
5. [done] Acceptance and local commit (this changeset); no push/release.
