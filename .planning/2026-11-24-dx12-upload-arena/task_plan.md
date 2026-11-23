# DX12 geometry upload page reuse

Date: 2026-11-24. Baseline `27605927`, branch `phase-v`.
Evidence: `D:/Dev/shots/cgpui-dx12-upload-arena-20261124/`.

## Scope

Replace per-draw vertex/index committed upload buffers with a focused private
persistent-mapped geometry page owner. Keep rectangle/shadow, textured-run and
backdrop-composite recorders responsible for native draws; they consume borrowed
slices from the owner. No public API or diagnostic semantics change. Texture
footprint staging, immutable glyph/image snapshots, readback, descriptors, draw
ordering, batching, synchronous presentation and window/backend policy are not
part of this milestone. Existing `.vscode/` remains untouched.

## Contract

- Standard upload pages are 256 KiB. Slices are 16-byte aligned and nonoverlapping
  until a proven safe reset. Only valid nonempty data bounded by native UINT
  view size may be copied; arithmetic is checked before allocation/memcpy.
- Pages are persistently mapped; native allocation/map failures do not publish
  a usable slice or page. Actual native page creations are counted privately for
  deterministic tests, not exposed as a public performance interface.
- `dx12_submit_frame` proves idle completion before resetting cursors. No reset
  occurs in public begin-frame or failed wait. State helpers reject pending,
  failed, removed or recovery-pending device generations before mutation.
- On the next safe frame reset, retain at most four standard pages (1 MiB);
  release surplus standard and oversize 64-KiB-rounded temporary pages. This
  bounds idle retained memory, not current-frame peak demand.
- Native retirement clears arena resources before releasing the old device,
  after the existing retirement path has proved completion/removal. No old-device
  pages survive peer retirement or reconstruction. Resize on a live device may
  retain reusable pages; it cannot rewrite them before completion.
- Public frame diagnostics still count original commands/resources and only
  glyph/image texture payload uploads, not geometry slice bytes or page capacity.

## Feedback loop and progress

1. [x] Native RED: 128 rectangles, real capture and exact diagnostic counts,
   then `resources=256` exceeds the bounded geometry upload budget (exit 5).
   Command: `scripts/dev/test.ps1 -p tests/renderer-dx12 dx12_geometry_upload_budget_test`.
2. [x] Private page owner + guarded frame integration + old helper removal.
3. [x] Real slice/multipage/cache/retirement/recovery tests and module guards.
4. [x] Independent Standards/Spec review, docs, frozen full regression.
5. [x] Native adapters/ASan/installed SDK, evidence and local commit; no push.

Success means reduced actual resource creation with unchanged pixels/lifetimes,
not a claimed FPS speedup, GPU-performance parity or default-backend switch.
The ordinary client-titlebar HMENU and dynamic opaque-to-alpha blockers were
rechecked; this change does not relax their evidence-backed restrictions.
