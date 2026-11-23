# DX12 last-successful-frame diagnostics

Date: 2026-11-23. Baseline: `9d80e51d`. Branch: `phase-v`.

## Scope and ownership

Implement the existing public `RendererFrameDiagnosticSnapshot` seam for DX12.
Keep the public interface unchanged. A private frame-diagnostics ledger owns
planning, actual native recording observations, CPU stage timing and publication.
Frame enqueue paths retain original primitive/command identity before lowering;
presentation only coordinates the ledger. Focused structure tests guard this
module layout. Default Vulkan/auto, explicit-backend errors and native-first
Popup behavior do not change. Preserve the user's untracked `.vscode/`.

## Contract

- Commands are accepted original draw calls (not clear/upload/invalidate/capture).
  Submitted commands have at least one eligible recorded resource.
- Batches are eligible DX12 payload recording groups, not portable GPU batches.
- Draw resources are nonempty, scissor-visible geometry ranges, contiguous glyph
  atlas runs, images, shadows and blur composites. Original selection/caret kinds
  survive lowering; split text retains one command and distinct resource IDs.
- Empty/fully clipped resources are excluded from planned resources, not reported
  unsupported. Their accepted command can remain pending without an error.
  Empty means an empty native payload or invisible scissor, not zero pixel
  coverage: the existing image builder emits a degenerate quad for zero bounds,
  which still records a native draw and texture upload and must be counted.
- Upload counts measure unique actual tightly packed glyph/image texture payload
  bytes recorded this successful frame, not padding or geometry buffers. Cache
  hits contribute zero; recovery reuploads count again.
- Blur diagnostics count actual composites and three offscreen passes per effect;
  allocated means retained targets actually exist.
- Six CPU stages are measured during submission; no GPU timestamps, prior text
  rasterization, or end-to-end latency claim. Stage totals are saturating sums.
- Initial snapshot is null. Only complete present success (including readback,
  validation and composition commit) publishes. Failure, abandon, resize and
  recovery retain the previous successful snapshot, possibly from an older
  surface/device generation. Native peer retirement does not invalidate backing
  CPU storage. Borrowing remains subject to renderer lifetime/next successful
  publication and the existing caller synchronization requirements.

## Progress

1. [x] Confirm public RED: real pixels succeed but snapshot is null.
2. [x] Implement private ledger and narrow enqueue/record/presentation hooks.
3. [x] Green public tests for all kinds, clipping, caches, effects and lifecycle.
4. [x] Structure guards, docs and independent Standards/Spec reviews.
5. [x] Frozen Windows/WSL full regression, adapter matrix, ASan and installed SDK.
6. [x] Record evidence and commit locally; no push/release/default switch.

Evidence directory: `D:/Dev/shots/cgpui-dx12-diagnostics-20261123/`.
RED: `diagnostics-red.log`, exit 8, missing public diagnostic after successful
two-rectangle frame. No complete-backend or GPU/performance-parity claim.
