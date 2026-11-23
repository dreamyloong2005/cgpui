# DX12 milestone: backdrop blur

Baseline `f81fa683`, branch `phase-v`. User requested continuation after outer
shadows. Preserve pre-existing untracked `.vscode/`. No push/publication or
changes to the separate design checkout.

## Scope and established acceptance seams

Implement native backdrop blur using shared neutral planning and the existing
Slang downsample/separable Gaussian/rounded composite algorithms. Preserve the
public BackdropBlurDraw contract: blur previously drawn scene content only,
mix tint, then allow later primitives on top; sigma is blur_radius (not the
outer-shadow CSS radius/2). Include rounded mask, clip, transforms/opacity,
DPI, overlapping effects and normal resize/lifetime behavior.

Continue the previously approved public Renderer/RenderFrame/pixel-capture,
neutral planning and independent-header seams, plus mandatory structure tests.
Use vertical RED/GREEN slices and independent numerical/literal expectations.

Native strategy: copy the current swapchain scene at each blur command, run
downsample and two Gaussian passes into private targets, composite back onto
that swapchain target. No-blur frames keep the direct path. Effect resources
are retained through the existing GPU completion boundary; no new promise of
exceptional synchronization/device-loss safety. Per-backdrop allocations are
acceptable for this correctness-first milestone, not a pooling/performance claim.

Vulkan remains default, explicit DX12 never silently falls back, and the
existing Vulkan Popup/failure-only overlay policy stays unchanged. Transparent
presentation/Popup, device recovery, exceptional-sync retirement, full diagnostic
parity and performance/default-switch gates remain outside this slice.

## Sequence

1. [done] Public native backdrop pixel tracer RED (unsupported, exit5).
2. [done] Neutral planner and embedded DXIL ownership RED/GREEN.
3. [done] Focused native target/pipeline/recording/frame integration GREEN.
4. [done] Public effects/order/DPI/lifetime/invalid-input regressions.
5. [done] Review; native hardware/WARP/debug, Windows/WSL, installed SDK, ASan.
6. [done] Documentation, frozen-source acceptance and local commit.

## Ownership

- popup_structure: neutral backdrop planning, Vulkan compatibility adapter,
  neutral/header and existing backdrop structure tests.
- gpui_windows_design: focused private native effects implementation and its
  structural boundaries. No shader binary/generator changes.
- repo_progress: public native pixel/behavior tests, one slice at a time.
- root: all builds/tests, Slang/DXIL, registrations, integration, docs/evidence.

Builds run serially. Use mcpp wrappers, MCPP_JOBS=4 for no-run builds and1 for
native execution. Never raise existing source-size caps.
