# DX12 implementation: first executable milestone

Baseline: `4d49b9ec`, branch `phase-v`. User requested implementation after
reviewing the DX12 status. Preserve the pre-existing untracked `.vscode/`.
The separate `dx12-design` checkout is a design reference, not the edit target.
(The dx12-design branch and worktree were deleted on 2026-11-25; that plan is gone.)

## First milestone and acceptance seams

Implement the first usable, explicitly requested opaque-window DX12 backend:
backend selection/reporting; actual device/queue/swapchain; solid and rounded
rectangle rendering with shared geometry, clipping, transform/opacity and DPI;
pixel readback; resize and normal-path resource ownership. Test at the planned public
`create_renderer` / named factories, `Renderer`, `RenderFrame` and pixel-capture
seams, plus mandatory module-ownership guards. The approved design's native
pixel-test seam is used, not a mocked renderer claiming DX12 support.

Vulkan remains the Windows default, including `auto`; DX12 cannot become an
automatic fallback until full parity. An explicit DX12 request must either use
DX12 or return its error. Unsupported primitives/surfaces fail explicitly, not
silently draw nothing. No public native/composition window mode. The current
Vulkan Popup default and failure-only overlay policy remain intact.

This milestone is not phase 4-7 completion: text/images/shadows/blur, transparent
presentation, device-loss recovery and the default-switch/release gates remain
separate work. Hardware/WARP and debug validation cover the locally tested
configurations, not every driver or exceptional synchronization failure.
Complete only what actual tests demonstrate; see acceptance.md for final evidence.

## Work sequence

1. [complete] Register and run failing selection/geometry/native acceptance tests.
2. [complete] Implement backend selection and the reusable geometry slice.
3. [complete] Implement DX12 rendering, shared-shader DXIL, capture and resize.
4. [complete] Run targeted native tests, architecture/header checks, then full Windows
   regression and relevant WSL non-regression serially. Validate SDK link closure.
5. [complete] Independent review, documentation and local commit; no remote push.

## Ownership

- repo_progress: public backend information/selection and dispatcher package,
  named Vulkan factory, consumer build dependencies.
- popup_structure: shared CPU geometry/clip/transform extraction, neutral tests.
- gpui_windows_design: focused DX12 GPU modules, shader DXIL generation/native tests.
- root: all test/build execution, suite registration, acceptance tests, SDK closure,
  documentation and integration.

No simultaneous build processes. Windows native tests execute with MCPP_JOBS=1;
no-run builds may use four workers. Do not raise existing source-size caps.
