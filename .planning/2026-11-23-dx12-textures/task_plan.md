# DX12 milestone: text and images

Baseline: `4e0455f0`, branch `phase-v`. User requested continuation.
Keep the pre-existing untracked `.vscode/`; no push/publication or edits to the
separate DX12 design checkout.

## Scope and seams

Continue the design's primitive phase with native text and image rendering,
shared CPU image/text geometry, embedded shared-Slang DXIL, GPU texture uploads,
samplers and cache lifetime. Preserve order across rectangles, glyphs and images,
including same-frame image replacement/invalidation. Validate through the existing
public Renderer/RenderFrame/native pixel-capture seams and neutral planner APIs,
plus mandatory module-ownership guards. Native rasterization is reused, not
replaced by fake text. Default Vulkan and Popup failure-only fallback remain.

Shadows, blur, transparent presentation, device-loss recovery, upload-ring
optimization and switching the default remain later work. The bootstrap's known
repeated-synchronization-failure safety gate is not certified by normal texture
lifetime tests. This milestone must not be described as complete DX12 parity.

## Sequence

1. [complete] Public image/text pixel regressions and neutral/shader ownership RED.
2. [complete] Native texture/image slice and shared CPU geometry GREEN.
3. [complete] Native glyph/text slice and mixed-order/cache regression GREEN.
4. [complete] Hardware/WARP/debug checks, full Windows/WSL regression, scoped ASan,
   installed-consumer closure; sequential builds only.
5. [complete] Independent Standards/Spec review, documentation and local commit.

## Ownership

- gpui_windows_design: native texture substrate, image cache/draws, PSOs and
  shared DX12 state/frame/presentation integration.
- popup_structure: neutral image/text CPU geometry, Vulkan thin compatibility,
  behavior/ownership tests at shared seams.
- repo_progress: native text/glyph planning modules and public text pixel tests;
  shared-file integration coordinated with native owner.
- root: test registration/execution, shared shaders/DXIL, integration, evidence,
  documentation and commit. Agents do not build concurrently.

Use `scripts/dev/build.ps1` / `test.ps1` (and `.sh` in validation mirror).
MCPP_JOBS=4 for no-run builds, 1 for native test execution. Never raise caps.
