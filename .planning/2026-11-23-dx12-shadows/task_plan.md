# DX12 milestone: outer box shadows

Baseline `e4d551a2`, branch `phase-v`. User requested continuation. Preserve
pre-existing untracked `.vscode/`; no push/publication or separate-design edits.

## Scope and existing acceptance seams

Implement native outer box shadows using shared neutral CPU geometry and the
existing analytic Slang shader, not a new shadow algorithm. Preserve CSS sigma,
spread, rounded corners and box clip-out; DPI, composition transform/opacity,
clipping, and original ordering with rectangles/text/images. Validate at the
previously established public Renderer/RenderFrame/pixel-capture and neutral
planner seams, with mandatory module-ownership/independent-header checks.

Vulkan remains default, explicit DX12 does not silently fall back, and Popup
failure-only policy remains unchanged. Backdrop blur, transparent presentation,
device recovery, exceptional synchronization retirement safety, upload pooling
and default-switch gates are outside this milestone. Completing shadows is not
proof of full DX12 parity or production readiness.

## Sequence

1. [done] Native hard-shadow pixels and neutral/shader ownership RED.
2. [done] Shared geometry, DXIL and focused DX12 PSO/draw integration GREEN.
3. [done] Blur falloff/rounded/spread/clip/DPI/order/input-validation regressions.
4. [done] Hardware/WARP/debug, Windows/WSL, installed SDK consumer and scoped ASan.
5. [done] Independent Standards/Spec reviews, documentation and local commit.

## Ownership

- popup_structure: neutral shadow planning, thin Vulkan compatibility, header
  and ownership tests; Standards review.
- gpui_windows_design: native shadow pipeline/draw/frame integration, focused
  native module tests; no public Windows mode or backend policy changes.
- repo_progress: public native pixel/behavior regressions and Spec review.
- root: shared Slang/DXIL, test registration, all build/test execution, evidence,
  docs and commit. Builds run sequentially; no-run MCPP_JOBS=4, native runs=1.

Use mcpp wrappers only. Existing module size caps must not be increased.
