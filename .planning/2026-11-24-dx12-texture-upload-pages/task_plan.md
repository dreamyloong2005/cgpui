# DX12 texture staging upload pages

Date: 2026-11-24. Baseline `241b226a`, branch `phase-v`.
Status: implementation, independent review and frozen-source full validation PASS; local commit follows.
Evidence: `D:/Dev/shots/cgpui-dx12-texture-upload-pages-20261124/`.

## Scope and ownership

Replace per-texture committed staging buffers with bounded reusable upload pages.
Extract the already implemented native page allocation/mapping/retirement policy
into private `Dx12UploadPages` (`dx12_upload_pages_internal.hpp/.cpp`). Keep
geometry and texture staging in independent pool instances; sharing policy must
not share storage or couple their cursor/reset schedules. Geometry retains its
existing focused adapter and 16-byte/UINT-sized vertex/index slice contract.
Texture footprint calculation and CPU row copies belong in the new
focused texture-upload adapter (`dx12_texture_upload_internal.hpp/.cpp`).
GPU copy/barrier recording remains in `dx12_texture_resources.cpp`.

The root agent alone runs builds and tests. Preserve user `.vscode/`. No public
API, renderer default, window/Popup policy, GPU draw ordering or diagnostic
resource identity changes. Readback reuse, native texture pooling, glyph partial
updates, descriptor pooling, per-draw texture bookkeeping optimization, batching
and asynchronous frame overlap are explicitly outside this milestone.

## Required behavior

- Low-level pages own persistently mapped native upload resources and return
  bounded borrowed slices. They do not know geometry types, texture formats,
  footprints, row-copy policy or GPU recording commands.
- Both pools use 256-KiB ordinary pages and retain at most four ordinary pages
  after a proven-safe reset. The combined retained payload capacity is at most
  2 MiB. This is not a per-frame peak-memory or whole-renderer memory limit.
  Surplus and oversized pages are transient until safe retirement.
- Geometry alignment and native UINT view-size validation remain unchanged.
  Texture slice placement meets 512-byte alignment, with the device-provided
  footprint row pitch and checked copy bounds. Copy source offsets reference the
  selected slice, not the beginning of the shared resource.
- Each immutable image/glyph snapshot still uploads lazily and only once when
  first prepared. Cache hits and same-frame duplicate draws do not copy again;
  recovery realizes and uploads the current CPU snapshots on the new device.
- Row padding, slice alignment and page capacity do not enter public upload-byte
  diagnostics. Those remain actual tightly packed image/glyph payload bytes.
- Neither pool may rewind on failed/unknown completion or public begin-frame.
  Presentation proves idle completion first; native teardown clears both pools
  before the device, covering recovery, cohort peers and final destruction.
- Remove the obsolete `state.uploads` ownership path rather than retaining an
  unused alternate staging implementation. Preserve existing sentinel-based
  lifetime coverage by observing the actual new pool-owned resources.

## Quantitative acceptance criteria

First capture a real native RED using many distinct small immutable textures:
verify their rendered cell pixels and exact diagnostics before asserting a
bounded number of actual staging resources. Record the old per-texture resource
count from runtime ownership, not an estimate from draw count.

After implementation, the same scene must show fewer actual native staging
allocations while preserving the pixel/diagnostic checks. A following frame with
fresh texture snapshots must reuse retained page storage without allocating a
new page when the same footprints fit; an unchanged cached frame must upload
zero texture bytes. Count texture and geometry pages separately, including any
temporary/legacy resources so an ownership move alone cannot satisfy the budget.
Do not use CPU elapsed time, FPS or GPU-performance parity as pass thresholds.

Also require byte/offset checks for multiple uploads in one page, 512-byte
placement, padded rows, oversize and page crossings; safe retention limits;
invalid/overflow rejection; failed-completion retention; same-device resize;
real device removal/recovery and unaffected geometry reuse.

## Work and validation ledger

1. [x] Root confirms the real native resource-budget RED before implementation:
   128 actual staging resources exceed the budget after 768 texel checks and
   3,072 diagnostic upload bytes pass; exit 5. See `texture-budget-red*.log`.
2. [x] Extract one shared low-level owner; integrate independent staging pool.
3. [x] Native pixels, accounting, boundary and lifetime tests pass.
4. [x] New structure guard and affected geometry/shadow guards registered in both
   mcpp discovery and the architecture/header CI matrix; documentation updated.
5. [x] Independent Standards and Spec reviews of the fixed-baseline working diff.
6. [x] Freeze source manifest; root serially validates full Windows/WSL suites,
   native adapters/debug/GPU validation, ASan and external installed consumers.
7. [x] Record actual evidence, retained failures and limitations; local commit
   only when accepted. No push, release or default-backend switch.

All full-regression and installed-consumer logs were inspected; see acceptance.md. Source hashes establish consistency, not binary provenance;
hardlinked libraries are not an immutable release SDK, and configure-only CMake
checks must not be described as binary or release validation.
