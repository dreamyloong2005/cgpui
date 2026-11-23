# Next bounded scope: common real-pixel parity

Date: 2026-11-24. Status: read-only audit recommendation, not implementation or acceptance evidence.

The current texture-staging milestone remains frozen for validation. This note
changes planning only. It does not authorize production/test/document edits or
builds during that freeze. No claim of full DX12 parity, default readiness,
performance parity or release is made here.

## Recommendation

Make the next bounded slice the same 13 real-pixel scenes and independent pixel
oracles actually executed against both named backend factories. Do not prioritize
descriptor/readback micro-optimization ahead of this contract gate. All public
Renderer and RenderFrame operations already have DX12 overrides; the important
remaining work is common semantic coverage, ordinary-window compatibility,
recovery and the explicit performance/readiness gates.

Existing DX12 tests cover the broad primitive categories, but they are not the
same 13 scenarios and assertions. In particular, the common native Latin/CJK
silhouette test and the PaintList/DirectWrite 48 px ink-extent test (normal and
italic blur/f/W/j, scale 1 and 1.5, negative bearings and final-glyph overhang)
provide stronger/different coverage than the current DX12 M/B atlas cases.
The dark-color/runtime-clear/50% white blend UNORM regression also needs to run
unchanged against DX12.

## Exact 13 existing tests

The following are the source scenes to migrate, not 13 newly invented or copied
tests. Preserve each scene, oracle and its existing tolerance. Suggested names
replace the initial `vulkan_` with `renderer_`.

1. `D:/Dev/Projects/cgpui/tests/renderer/vulkan_backdrop_blur_pixel_test.cpp`
2. `D:/Dev/Projects/cgpui/tests/renderer/vulkan_box_shadow_pixel_test.cpp`
3. `D:/Dev/Projects/cgpui/tests/renderer/vulkan_clip_pixel_test.cpp`
4. `D:/Dev/Projects/cgpui/tests/renderer/vulkan_color_encoding_pixel_test.cpp`
5. `D:/Dev/Projects/cgpui/tests/renderer/vulkan_frame_pixel_capture_test.cpp`
6. `D:/Dev/Projects/cgpui/tests/renderer/vulkan_image_pixel_test.cpp`
7. `D:/Dev/Projects/cgpui/tests/renderer/vulkan_opacity_pixel_test.cpp`
8. `D:/Dev/Projects/cgpui/tests/renderer/vulkan_resize_pixel_test.cpp`
9. `D:/Dev/Projects/cgpui/tests/renderer/vulkan_rounded_rect_pixel_test.cpp`
10. `D:/Dev/Projects/cgpui/tests/renderer/vulkan_scale_pixel_test.cpp`
11. `D:/Dev/Projects/cgpui/tests/renderer/vulkan_text_pixel_test.cpp`
12. `D:/Dev/Projects/cgpui/tests/renderer/vulkan_text_ink_extent_pixel_test.cpp`
13. `D:/Dev/Projects/cgpui/tests/renderer/vulkan_transform_pixel_test.cpp`

Keep these tests in the Windows discover section of
`D:/Dev/Projects/cgpui/tests/renderer/mcpp.toml`. A separate suite would make
`-p tests/renderer` miss the intended gate unless the runner explicitly expands
it; there is no need to introduce that complexity for this slice.

## Current false-evidence trap

`vulkan_pixel_test_support.hpp` includes `vulkan_pixel_test_window.hpp` and its
capture helper calls `create_vulkan_renderer` directly. The existing
`scripts/dev/test.ps1 -Renderer dx12` only sets `CGPUI_RENDERER`; named factories
bypass that environment-based production selection. Consequently running the
old suite under that flag does not run its common scenes on DX12.

Changing only the helper is still insufficient. Three scene files also call
`create_vulkan_renderer` directly and must use the new common creation entry:

- `tests/renderer/vulkan_backdrop_blur_pixel_test.cpp`
- `tests/renderer/vulkan_frame_pixel_capture_test.cpp`
- `tests/renderer/vulkan_resize_pixel_test.cpp`

The ink-extent scene also calls the compatibility
`vulkan_build_textured_glyph_quads`; migrate that caller to the existing neutral
`build_textured_glyph_quads`, not a new planner implementation.

## Test-support module ownership

Suggested focused owners, all under `tests/renderer/`:

- `renderer_pixel_test_backend.hpp/.cpp`: selector parsing, named factory
  dispatch, actual backend/selection validation, and actionable creation/error
  reporting. Non-template implementation belongs in the cpp.
- `renderer_pixel_test_window.hpp/.cpp`: the existing Win32 test HWND host,
  resize/client extent and message pumping. Keep native window work out of the
  backend selector and capture helper.
- `renderer_pixel_test_support.hpp`: a thin capture template that delegates to
  the selected backend helper, plus thin/shared pixel-probe declarations. If
  non-template probe implementation needs space, use a focused cpp/leaf rather
  than growing the aggregate support header.
- Existing mathematical shadow and text-ink-shape support may take neutral
  names; their algorithms/oracles must not change during the migration.

Preserve the existing module caps; do not solve integration by raising them.
`renderer_source_structure_test.cpp` currently caps the pixel support header at
90 lines and the window support at 110 lines. Existing live capture is capped at
100 lines; the per-scene caps include text 60, rounded rect 35, image 80, clip
35, transform 40, opacity 40 and resize 90. Allocate new focused leaves with
explicit small caps instead of stuffing parsing, diagnostics and hosting into
the template header or weakening those guards.

### Metal cross-reference

`D:/Dev/Projects/cgpui/tests/renderer/metal/metal_box_shadow_pixel_test.mm`
includes `vulkan_box_shadow_pixel_support.hpp` and mentions it in a comment.
If that pure-math support becomes `renderer_box_shadow_pixel_support.hpp`,
update the Metal include/comment too. Do not change the Metal scene or expected
coverage, and do not claim native macOS verification from a Windows/WSL build.

## Dependency and platform boundary

In `tests/renderer/mcpp.toml`:

- Retain the existing renderer-vulkan dependency and all backend-specific tests.
- Add `cgpui.renderer-dx12` only in Windows dependencies.
- Register new non-template support cpp files explicitly in the Windows build
  sources; do not expose Win32 or DX12 symbols to Linux/macOS compilation.
- A renderer-windows dispatcher dependency is not necessary if the test helper
  calls the two named factories directly.
- Keep the 13 native HWND cases Windows-only for this slice. Preserve existing
  Linux Vulkan/Wayland/X11 tests and macOS Metal registration unchanged.

This is shared Windows Vulkan/DX12 scene coverage, not a claim that all thirteen
scenes have acquired portable Linux/macOS window hosts.

## Strict backend selection and evidence

A minimal selector can reuse existing `CGPUI_RENDERER`; a second environment
variable is not necessary and would need a conflict policy. Suggested harness
semantics:

| Value seen by common test helper | Factory |
| --- | --- |
| Unset, empty, `auto` | Named Vulkan, preserving the current common-test default |
| `vulkan` | `create_vulkan_renderer` |
| `dx12` | `create_dx12_renderer` |
| Any other value, including literal `both` | Hard failure |

This test-only default is deliberately explicit. Existing backend-selection
tests remain responsible for the production automatic dispatcher contract.
Do not modify production selection/default/fallback in this slice.

Every successful creation must validate and report:

- requested selector and expected backend;
- `backend_info().backend` strictly equals that expected backend (unknown fails);
- explicit selection metadata matches the named backend;
- `fallback_reason` is empty;
- actual adapter name/LUID/vendor/software status; a WARP request must establish
  that the selected adapter is software rather than just printing the variable.

The harness's `auto` request is resolved to named Vulkan, so its resulting
selection metadata is Vulkan rather than automatic; do not conflate this with a
production dispatcher test.

Window creation, renderer initialization, capability, capture, present and
identity failures return nonzero and preserve useful original errors. No silent
skip, fallback or missing optional capture can count as a successful case.

## PowerShell runner: explicit both, unchanged default

Extend `D:/Dev/Projects/cgpui/scripts/dev/test.ps1` to accept explicit
`-Renderer both`, without adopting the old design's default-both recipe:

- The same mcpp test arguments run twice, sequentially, in independent test
  process runs, first with `CGPUI_RENDERER=vulkan`, then `dx12`.
- Never put the literal `both` into `CGPUI_RENDERER` for a test process.
- Any failed run makes the aggregate result nonzero. A successful second run
  must not overwrite a failed first exit code; fail-fast is also acceptable.
- Preserve no-selector and explicit-auto behavior. Do not silently double all
  existing test runs by changing the default.
- Preserve ASan path conversion/runtime setup, mcpp argument forwarding and
  compile-command export. Scope/restore environment changes safely, especially
  if test script internals become callable instead of terminating their process.
- Add script parser/runner tests using a fake invocation boundary, not actual
  GPU builds: both makes two correctly selected calls; arguments including `-p`
  and filters remain intact; invalid/missing/duplicate selectors fail; either
  backend's failure propagates; auto/no-selector remain single-run; ASan member
  paths still translate exactly once.

`-Renderer dx12 -p tests/renderer` must truly select DX12 for the migrated common
pixel cases. Existing `vulkan_*` private pipeline, descriptor, synchronization and
device-policy tests still test Vulkan; they are not to be rewritten to obey a
global DX12 selector. Running the whole suite twice repeats those tests but does
not make them DX12 tests. Logs/evidence must distinguish that from the 13 common
scenes' verified selected backend.

No DX12 selection is introduced in Linux `test.sh` for this bounded slice.

## Preserve diagnostic strength without equating backend work

The backdrop scene currently combines independent pixel expectations with
Vulkan-specific diagnostic topology:

- one blur expects `offscreen_pass_count == 5`;
- two blurs expect `offscreen_pass_count == 9`.

DX12's existing diagnostic definition is three offscreen filter passes per
effect, hence 3/6 for those scenes. These are different implemented work
topologies, not justification to falsify counts, force them equal, remove all
diagnostics or weaken an assertion until green.

Move the Vulkan-specific 5/9 assertions into a focused Vulkan-only diagnostic
test while preserving its exact expectations. Keep exact DX12 3/6 expectations
in its focused DX12 diagnostic coverage. The shared scene retains the original
pixels, clipping, ordering, tint, scale and common semantic counts (blurred
backdrop count and submitted backdrop draw count). Keep the zero-effect
no-offscreen-work assertion where it is a genuine common contract.

Do not generalize the old design's stated tolerance of one over actual current
oracles. Existing tests deliberately use different tolerances for specific
effects and edge/interior samples; preserve those exact tolerances initially,
and investigate a real mismatch instead of raising them for parity.

## Structure-test changes required alongside migration

Existing owning guards with hard-coded references:

- `D:/Dev/Projects/cgpui/tests/architecture/renderer_source_structure_test.cpp`
- `D:/Dev/Projects/cgpui/tests/architecture/backdrop_blur_structure_test.cpp`
- `D:/Dev/Projects/cgpui/tests/architecture/box_shadow_structure_test.cpp`
- `D:/Dev/Projects/cgpui/tests/architecture/device_pixel_scale_structure_test.cpp`

Update paths/class names/registration expectations rather than dropping checks.
Add a focused structure guard requiring the 13 scene files, their Windows
registration, the focused selector owner and backend identity verification.
Forbid direct named factories in the common scene/capture owners; only the
backend helper may select between them. Guard the Windows-only DX12 dependency,
absence of silent success/skip paths, and unchanged file-size limits.

## Architecture decisions that supersede mechanical old-plan recipes

The existing neutral ownership is `src/renderer/planning/`, with public leaves
under `include/cgpui/renderer/planning/`, compiled by the renderer package.
`tests/renderer-neutral` links only renderer and checks neutral ownership.
The old planned `src/renderer/frame/` path and forced `renderer_` spelling are not
completion requirements by themselves.

Compatibility forwarders are deliberate current architecture, not leftovers to
delete mechanically. `tests/renderer-neutral/planning_compatibility_test.cpp`
explicitly proves old `vulkan_*` entry points are source-compatible and link
without renderer-vulkan, while focused `compat_*.cpp` leaves own them neutrally.
New common tests should use neutral names, but the old plan's blanket removal of
all forwarders is superseded and is outside this parity slice.

Other superseded recipes: typed `frame_retry_required` plus runtime redraw
replaces falsely successful lost-frame present; bounded geometry/texture upload
pools replace the proposed unified ring; RGBA UNORM capture needs no old BGRA
channel-swap recipe. None of those substitutions closes remaining behavior or
performance gates merely by documentation.

## Subsequent gates remain separate

This slice must not be called full DX12 completion even if all thirteen scenes
pass on hardware and WARP:

- Ordinary opaque-to-alpha transition, client-titlebar plus application HMENU,
  alpha chrome/fullscreen transitions and broader ordinary-window combinations
  remain compatibility/functionality work.
- Unsupported-blur semantics differ: the public comment/base implementation
  and Vulkan use tint-only fallback, while DX12 currently returns unsupported
  for the whole frame. This needs an explicit contract resolution, not an
  unreported assumption of parity.
- Runtime retry has root/child mock regression, and DX12 has real idle-device
  RemoveDevice tests, but a real WindowRuntime automatic-redraw recovery test
  was not found. Vulkan still lacks the planned full device-loss reconstruction;
  cross-thread/busy DX12 peers can still cause terminal errors.
- Retained-image system-memory accounting is absent from the public diagnostic
  snapshot; upload bytes do not represent retained residency.
- WARP/debug/GPU-validation runtime CI is absent. Existing hosted example
  smoke uses SwiftShader/Vulkan and does not certify a full DX12 workbench.
- Actual waitable-object pacing, command reuse, descriptor pooling and dirty
  glyph subregion uploads remain performance work. Current synchronous fence
  safety and reduced committed upload allocations do not establish FPS parity.
- Full example/UI acceptance, installed/release SDK, native desktop/menu
  validation, automatic preference/fallback and final release readiness remain
  independently gated. No push/publication follows from this note.

## Audit evidence limits

This recommendation was derived from owning code, manifests and scripts only.
No builds, tests, production changes or acceptance runs were performed by this
audit. The only file written is this planning note.
