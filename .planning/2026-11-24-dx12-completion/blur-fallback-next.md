# DX12 unsupported backdrop: next bounded slice

2026-11-24. **Read-only design, not executed acceptance.** Current source stays
frozen. `diagnosing-bugs` was read first; parent prohibits builds/tests in this
investigation, so the native RED below is a proposed loop, not an observed result.

## Contract and smallest change

- `include/cgpui/renderer/renderer_frame.hpp` and `renderer_effect_draws.hpp`
  promise tint-only drawing without blur support. The Vulkan override delegates
  to `RenderFrame::draw_backdrop_blur(backdrop)`; DX12 currently latches
  `unsupported_platform`, making `present()` fail even when no tint was requested.
- Keep ownership in `src/renderer/dx12/dx12_backdrop_blur_frame.cpp`. After the
  existing released/error guard, make the existing device-scaled command copy
  and run `dx12_valid_backdrop_blur_input(device)` **before** the support branch.
  Unsupported: call `RenderFrame::draw_backdrop_blur(backdrop)` and return.
  Supported: keep the existing neutral plan, plan validation and typed append.
- Pass the **original logical command**, not `device`, into the base fallback:
  virtual dispatch reaches DX12 `draw_rounded_rect`, which scales geometry,
  radii, clips and transform translations once. Passing `device` double-scales.
  Existing recursive operation guard supports this nested draw call.
- Reuse `src/renderer/renderer_effect_fallback.cpp` unchanged. It forwards rect,
  tint, radius, clip rect/stack, composition stack and metadata. Do not duplicate
  field copying or add a public leaf/header API, environment switch, factory
  option, new primitive kind or production catch-and-fallback path.
- "Raw validation" means the current validator's command-field checks, before
  geometry/planner normalization: finite rect/endpoints, tint and every clip.
  Preserve device-scaled validation (including overflow at device scale), even
  for no tint, alpha-zero tint, fully clipped draws and nonpositive dimensions.
  Do not rely solely on rounded geometry validation: empty/clipped/normalized
  geometry can otherwise hide invalid raw inputs.
- Do not expand this fix into a new finite policy for sigma/radii/metadata.
  Existing sigma normalization and composition opacity/transform normalization
  stay owned by shared planners. Supported blur validation must not weaken.

## Deterministic native RED seam

Use the real private-state setup already exercised by
`dx12_geometry_upload_budget_test.cpp` / `dx12_texture_upload_budget_test.cpp`:
real HWND + shared `Dx12State`, configure surface, create native device/pipeline/
swapchain, prepare frame, set `frame_active`, instantiate real `Dx12Frame`.
Require valid DX12 native identity/capture; WARP requests must report software.
Failure to initialize is failure, not a skip.

The RGBA8 capability is normally available on local hardware/WARP, so relying on
an actually incapable adapter is not a reproducible gate. Probe/save the real
support value, then set only private `state->backdrop_blur->support` to
`{.supported = false, .reason = "test-forced unsupported backdrop"}` on this
single-thread fixture. Do not replace the device, recorder, frame or GPU output.
This is capability-state injection, **not** proof of an incapable physical GPU.

First native regression: white clear, black left-half rectangle, half-red tinted
rounded panel covering the black/white boundary, then a small opaque green
foreground marker. Request capture and require successful present. Independent
interior samples must be `{128,0,0,255}` / `{255,128,128,255}`, with the original
hard boundary, unchanged outside/corner samples and green foreground on top.
Use existing `dx12_pixel` tolerance 1 away from AA edges. Current code should
fail at present with `unsupported_platform`; record that exact error before B.

After adding only A (test/guard/manifest), root runs:

```powershell
$env:CGPUI_DX12_ADAPTER = 'warp'
$env:CGPUI_DX12_DEBUG = '1'
scripts/dev/test.ps1 -Renderer dx12 -p tests/renderer-dx12 dx12_backdrop_blur_fallback_pixels_test
```

Record native initialization, requested/actual adapter, assertion and exit code;
do not substitute a source-token failure or failing stub for this RED. Then apply
B's small owner-only change and rerun the same executable followed by the matrix.

## Required behavioral boundaries

1. **Tint variants/order:** absent tint is a no-op that does not poison surrounding
   draws; alpha-zero tint preserves pixels but remains a real rounded command;
   half/opaque tint and nonzero rounded corners have independent color/mask
   oracles. No smoothing is permitted when unsupported, even for large sigma.
2. **Coordinates/forwarding:** run scales 1, 1.5 and 2 with matching framebuffer
   extent; use hand-computed interior/outside probes to detect double scaling.
   Separate standalone metadata translation+opacity from nonempty composition
   stack current metadata; give them deliberately different values to prove the
   stack wins rather than applying both. Include nonuniform transform or shear,
   rounded corner, explicit clip, nested clip current intersection and a fully
   clipped panel. Do not compute expected pixels with the production planner.
3. **Validation before fallback:** table-drive the existing ten nonfinite
   bounds/tint/clip mutations from `dx12_backdrop_blur_validation_test.cpp`, plus
   finite endpoint/device-scale overflow. With unsupported capability require
   `invalid_argument`, no new capture and last successful snapshot unchanged.
   Repeat relevant bounds/clip failures with absent/transparent tint and hidden
   geometry, then a valid frame to prove no poisoning. Negative/nonfinite sigma
   remains valid under the existing normalization policy, not a new hard error.
4. **No stale resources:** run supported blur control (actual capability required
   on the chosen WARP run), force false for a subsequent fallback frame, restore
   the saved support and render supported blur again. Check fallback frame has no
   retained frame targets/offscreen allocation, while supported control again
   smooths the edge and reports its existing exact 1-effect/3-pass diagnostics.
5. **Failure remains failure:** after successful fallback capture, queue another
   fallback and perform real native `RemoveDevice` using existing recovery test
   helpers before present. Require `frame_retry_required`, no new capture, same
   adapter/generation+1 and unchanged last-success diagnostics; never success.
   A separate prior-invalid-draw case must retain its original error. Do not
   intercept actual pipeline/target/upload/debug/submit/present HRESULT errors.

## Diagnostic oracle (do not redefine existing accounting)

Let N be the visible surrounding solid-rectangle commands/draws; clear is not one.
The table shows additions caused by one unsupported backdrop call:

| Case | Planned commands | Submitted commands | Planned/submitted rounded draws |
|---|---:|---:|---:|
| no tint | 0 | 0 | 0/0 |
| visible tint, including alpha 0 or effective opacity 0 | 1 | 1 | 1/1 |
| tint with empty geometry or fully invisible scissor | 1 | 0 | 0/0 |

Every fallback frame: planned/submitted `backdrop_blur_count == 0`, blurred
backdrops/offscreen passes zero, targets-allocated false, native frame-targets
empty, logical texture upload bytes zero, and no dropped/unexpected resources.
Visible/no-tint cases have exact work; the clipped/empty case deliberately has
one pending command, zero pending draw/batch/upload work. Do not demand
`work.exact_match()` there or manufacture a submitted resource. Validate totals
and primitive kind, not just nonzero pixels. Geometry arena uploads are not
logical image/glyph upload bytes and need not be zero for rounded fallback.

## Focused implementation/registration plan

- A: `dx12_backdrop_blur_fallback_pixels_test.cpp`, focused validation and
  lifecycle tests, plus a declaration-only `dx12_backdrop_fallback_test_support.hpp`
  and bounded `.cpp` native fixture/scenes as needed. Do not enlarge generic
  `dx12_test_support.hpp` or production frame entry files to hold test helpers.
- Register tests in Windows `[test].discover` and support `.cpp` in the explicit
  Windows `[build].sources` of `tests/renderer-dx12/mcpp.toml` (not glob assumptions).
- Extend existing `dx12_backdrop_blur_structure_test.cpp` to require validation
  before support branch, base fallback with original `backdrop`, and focused
  test registration. Keep all original <=100-line production module caps and
  existing supported-plan/recording checks; do not weaken them for fallback.
- B: only the small ordering/delegation edit in the existing focused frame owner.
  Native matrices after GREEN: new tests, all DX12 backdrop/diagnostic/recovery
  tests, common Vulkan+DX12 pixels, architecture/headers; root selects appropriate
  hardware/WARP/debug/GPU-validation/ASan regression and final full-matrix gates.

## Explicit limits / one pre-existing API ambiguity

`dx12_backdrop_blur_support()` currently maps a failed `CheckFeatureSupport`
HRESULT to the same public `{supported=false, reason}` type as missing format
bits. This slice cannot claim to distinguish those causes; do not add string
parsing or conflate that probe result with later native execution errors.
Device removal is still checked by `dx12_prepare_present`; all recording/API
errors retain their normal typed failure/recovery path. A stricter capability-
query error contract would be a separate diagnosed design, not a silent side
change. No production default switch, all-adapter certification or full DX12
completion follows from this bounded fallback fix.

## External candidate handoff (not applied)

Root approved the validation/delegation/accounting boundary above. Prepared
`D:/Dev/shots/cgpui-dx12-blur-fallback-20261124/candidates/A.patch` (12 files:
five native tests, focused support, new structure guard and registrations) and
`B.patch` (one production owner). Candidate README contains the RED/GREEN order,
coverage and limitations; baseline/manifests preserve exact file hashes.
Both `git apply --check --whitespace=error` calls succeeded. Static TOML parsing
confirmed all prior registrations preserved; baseline hash comparison confirmed
the four existing checkout files unchanged. No candidate native build/test or
source application was performed. Root still must establish native RED then GREEN.
