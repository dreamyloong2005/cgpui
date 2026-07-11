# CGPUI GPUI-Core Findings

## 2026-07-10 Phase E Step 511 Swapchain Recovery

- `present_frame(...)` currently sets `presentation_blocked_` for out-of-date or
  suboptimal acquire/present results and relies on a later external resize to
  recreate the swapchain. If no platform resize follows, presentation remains
  permanently blocked even though the current framebuffer descriptor is usable
  for an immediate recreation attempt.
- Step 511 should separate `VkResult` policy from Vulkan state mutation. A
  focused policy can distinguish continue, recreate-before-frame,
  recreate-after-frame, and fatal result handling; a focused renderer-state
  method can reuse the existing `resize(current_size, current_scale)` resource
  replacement path.
- Out-of-date acquire/present should recreate automatically and return a
  retryable frame error when the current frame was not displayed. Suboptimal
  acquire/present can finish the submitted frame, recreate afterward, and
  return success. Non-surface acquire/present failures retain existing recovery
  behavior. Step 512 retains present pacing ownership.
- Phase E Step 511 adds automatic swapchain recreation from acquire/present result plans. The out-of-date results return a retryable frame error after recreating, suboptimal frames recreate after submission, and presentation remains unblocked after successful recovery. Step 512 present pacing is next.

## 2026-07-10 Phase E Step 510 Resource-Barrier Batching

- Glyph-atlas and image-texture upload recording duplicate the same layout,
  access-mask, stage-mask, and `VkImageMemoryBarrier` construction logic. Each
  upload also emits one barrier before and one after its copy, so a batch of N
  independent images records 2N `vkCmdPipelineBarrier` calls.
- Step 510 should centralize upload-image barrier planning and recording in a
  focused private module. Ordered barrier waves can transition all unique image
  targets to transfer layout together, execute their copies, and transition the
  whole wave to shader-read layout together.
- A repeated image target cannot appear twice in one Vulkan image-barrier call.
  The planner should preserve upload order by starting a new wave at a repeated
  target and should treat that target's later old layout as shader-read because
  the preceding wave completed the readable transition. Step 511 retains
  swapchain recovery ownership.
- Phase E Step 510 adds ordered upload barrier waves with batched transfer and shader-read transitions. Unique glyph-atlas or image-texture targets share two barrier calls around their copies, while duplicate image targets start a new wave with shader-read old-layout continuity. Step 511 swapchain recovery is next.

## 2026-07-10 Phase E Step 509 Pipeline-Switch Batching

- `vulkan_record_frame_draws(...)` already skips all draw-state binding for
  consecutive draws of the same resource kind, but it treats solid and rounded
  rectangles as different active kinds even though both use the same rounded
  rectangle Vulkan pipeline. An authored solid/rounded transition therefore
  performs a redundant `vkCmdBindPipeline` while still requiring different
  vertex/index buffers.
- Step 509 should preserve authored order and split pipeline changes from
  geometry-resource changes. A focused planner can coalesce adjacent
  solid/rounded pipeline use without moving Vulkan command ownership out of the
  existing primitive recording modules. Resource barriers remain Step 510.
- Phase E Step 509 adds pipeline-switch batching with authored draw order preserved. Adjacent solid and rounded rectangle draws reuse the shared rounded-rectangle pipeline while rebinding only their distinct geometry buffers. Step 510 resource barriers are next.

## 2026-07-10 Phase E Step 508 Recorded Command Reuse

- Vulkan already retains one command-buffer handle per swapchain image, but
  `record_vulkan_frame_command_buffer(...)` resets and records it every frame
  with `VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT`. Step 508 must implement
  recorded-command reuse, not relabel handle reuse.
- Reuse is unsafe for command buffers that contain glyph-atlas or image staging
  uploads because the staging resources are destroyed/rebuilt on later frames.
  Any non-empty upload set must force recording and leave the slot ineligible.
- For upload-free frames, exact compact signatures can cover render pass,
  framebuffer, extent, clear color, pipeline/layout handles, geometry-buffer
  handles, solid/rounded ranges, planned text/image descriptor commands, clip
  state, and authored draw order. Vertex/index bytes may change behind retained
  handles after the in-flight fence without changing recorded commands.
- Command reuse state belongs per swapchain image and must be cleared on
  swapchain install/destroy and record failure. Step 509 retains pipeline-switch
  batching ownership.
- Phase E Step 508 adds per-swapchain-image recorded command reuse guarded by an exact semantic command signature. Matching upload-free frames resubmit the recorded buffer without reset or recording, while pending uploads force recording and invalidate reuse state. Step 509 pipeline-switch batching is next.

## 2026-07-10 Phase E Step 507 Frame Geometry Buffers

- Steps 507-514 are specified as one band covering vertex/index buffers,
  command reuse, pipeline switches, resource barriers, swapchain recovery, and
  present pacing. Step 507 should therefore close the first listed buffer gap
  through a focused renderer module and matching structure coverage.
- The current renderer already has primitive-specific geometry and recording
  leaves. The Step 507 boundary must compose those owners rather than moving
  their implementation into `vulkan_command_recording.cpp`,
  `vulkan_renderer.cpp`, or `vulkan_state.cpp`.
- Text and image vertex uploads and rounded/solid vertex-index uploads currently
  destroy and recreate their host-visible Vulkan buffers every frame. The
  renderer already waits for its single in-flight fence before these uploads,
  so retaining allocations and remapping them is safe on the current schedule.
- Step 507 will add a focused private `vulkan_frame_geometry_buffer` policy and
  implementation. Primitive-specific resources keep their draw/count metadata,
  while the shared leaf owns capacity planning, reuse, growth replacement,
  host-coherent mapping, and destruction for vertex/index allocations.
- Empty geometry should clear active byte/count metadata without discarding an
  existing allocation; a later non-empty frame can reuse that capacity. Actual
  command-buffer reuse remains Step 508.
- Phase E Step 507 adds `VulkanFrameGeometryBufferResources` and reusable host-visible vertex/index buffers for text, image, solid, and rounded geometry. Empty frames keep retained capacity, matching-capacity uploads remap the existing allocation, and growth replaces buffers geometrically after the single in-flight fence. Step 508 command reuse is next.

## 2026-07-10 Phase E Step 506 SVG Integration Closeout

- Step 506 is audit-only. Steps 499-505 already own the full behavior chain,
  so closeout should add no renderer API, raster behavior, or Vulkan code.
- The closeout must freeze module and focused-test evidence for planning,
  LunaSVG rasterization, cache ownership, viewport scaling, current-color
  recolor, frame upload, and the prelude-only example before batching begins.
- Phase E Step 506 SVG integration closeout is audit-only in `tests/api_parity/phase_e_svg_integration_closeout_test.cpp`. It freezes Steps 499-505 across bounded RGBA raster planning, the LunaSVG raster backend, cache-owned raster results, viewport-aware scaling, currentColor recolor, RenderFrame image upload, and the prelude-only public example. Step 507 batching and frame scheduling is next.

## 2026-07-10 Phase E Step 505 SVG Public Example

- The example can remain platform-neutral and executable by using a small
  public `RenderFrame` implementation that records uploads/draws. This proves
  the complete public workflow without reaching into Vulkan or private runtime
  APIs.
- `ImageAssetRegistry::register_svg(...)` already provides the asset id,
  logical size, and owned SVG source needed to construct the public
  `SvgRasterizationRequest`.
- The example should exercise both cache miss and cache hit, then draw with the
  returned raster descriptor so the upload/draw relationship is explicit.
- Phase E Step 505 now adds the prelude-only `public_svg_raster_upload` executable. It turns a registered SVG source into a viewport-aware raster request, proves cache miss/hit behavior, and performs cached upload and image draw. Step 506 SVG integration closeout is next.

## 2026-07-10 Phase E Step 504 SVG Image Upload Integration

- `RenderFrame::upload_image(...)` is the backend-neutral upload boundary. The
  Vulkan frame already converts the `ImageAsset` to an `ImageUploadBatch`,
  deduplicates same-frame asset ids, and lets the persistent texture cache
  reuse ready GPU resources.
- Step 504 should therefore own only the bridge from `SvgRasterCache` lookup to
  `RenderFrame`, not duplicate Vulkan staging or make renderer code depend on
  UI `ImageSource`/`ImageAssetRegistry` types.
- A cache hit still submits the cached asset to the current frame. This keeps
  frame construction stateless while the Vulkan upload/cache layer decides
  whether GPU work is necessary.
- Phase E Step 504 now adds `SvgImageUploadResult` and consumes a cache-owned raster ImageAsset through RenderFrame::upload_image(...) integration. Cache hits resubmit the ready asset, while failed rasterization skips upload. Step 505 SVG public example is next.

## 2026-07-10 Phase E Step 503 SVG Recolor/Tint

- The existing Vulkan image path already resolves optional `ImageDraw::tint`,
  applies composition opacity once, transports tint through vertex color, and
  multiplies the sampled texture in `image.frag.glsl`. That path should remain
  the dynamic tint owner.
- LunaSVG exposes `Document::documentElement()` and
  `Element::setAttribute(name, value)`. Raster-time SVG recolor can therefore
  set the root CSS `color` for authored `currentColor` without rewriting source
  markup or duplicating a parser.
- Step 503 should add explicit current-color planning/validation and include the
  normalized current color in raster cache identity. It should also freeze that
  multiplicative image tint remains draw-time and does not create tinted raster
  cache variants.
- Phase E Step 503 now adds `SvgRasterColorizationPlan` with a validated RGBA current color normalized to RGBA8. The LunaSVG currentColor recolor sets document-root CSS `color`, while existing image color remains draw-time multiplicative tint. Step 504 SVG image upload integration is next.
- The complete Windows run passed 183/184. The only failure was the visible
  `vulkan_solid_rect_test` desktop sample returning RGB(73,108,132). A clean
  detached worktree at the Step 502 commit `65bc81db` reproduced the exact same
  exit 5 and RGB value outside the sandbox, so this failure predates Step 503.

## 2026-07-10 Phase E Step 502 SVG Viewport Scaling

- `ImageElement::layout(...)` resolves a content size from preferred style or
  intrinsic asset size, and paint stretches the selected asset/source rect into
  that content rectangle. There is no existing object-fit/contain/cover policy
  to reuse or extend in this slice.
- The useful Step 502 boundary is therefore an explicit raster viewport that
  can differ from intrinsic SVG logical size. The planner should derive device
  pixels from viewport size plus DPI, while preserving logical source metadata
  for cache identity and later Step 504 image upload integration.
- LunaSVG v3.5.0 exposes `renderToBitmap(width, height)`, with `-1` reserved for
  intrinsic auto-scaling. Step 502 can keep the mature backend call unchanged
  while feeding it explicit device dimensions from a focused viewport-scaling
  plan.
- The focused module should own viewport fallback/validation, dimension and byte
  limits, ceil-rounded device extents, and effective x/y raster scales. The
  raster request can add an optional viewport size at the end of its aggregate
  fields, preserving existing designated initializers.
- Phase E Step 502 now adds `SvgViewportScalingPlan`. An optional raster viewport falls back to intrinsic logical size, produces ceil-rounded viewport device pixels, and records effective x/y raster scales. The raster plan, LunaSVG dimensions, and normalized cache key consume the result. Step 503 SVG recolor/tint is next.
- Final Step 502 Windows verification passes the focused 6/6 gate and complete
  183/183 suite. The focused viewport leaf is 47 lines, its implementation is
  85 lines, and the renderer aggregate remains 11 lines. WSL still has no
  registered distribution.

## 2026-07-10 Phase E Step 501 SVG Raster Cache

- The roadmap gives Step 501 cache ownership before Step 502 scaling and Step
  503 recolor/tint. Cache identity therefore needs to preserve the current
  request inputs without pre-claiming later scaling or tint policy.
- `svg_rasterization.hpp` is already the focused request/plan/result leaf, and
  the LunaSVG implementation is isolated. The cache should begin in its own
  public leaf and focused `.cpp`, with structure coverage, instead of placing
  container state or lookup bodies into the existing rasterization module.
- Existing explicit caches use caller-owned objects, sequential entry storage,
  hit/miss/lookup counters, and `clear()`. Step 501 should follow that surface,
  but return a pointer to the cache-owned raster result so cache hits do not
  copy the potentially large RGBA pixel vector.
- The cache key should own SVG source bytes together with asset id, logical
  size, and scale. Only ready rasterizations should be stored; invalid requests,
  invalid SVG, and backend failures should remain misses and should not poison
  later lookups.
- Phase E Step 501 now adds `SvgRasterCache` with cache identity over asset id, logical size, scale, and SVG source.
  Repeated requests return a cache-owned raster result without pixel copies,
  while failed rasterizations are not cached. Step 502 SVG viewport scaling is next.
- Cache entries use `std::deque` so result pointers remain stable when another
  raster is inserted; `clear()` remains the explicit invalidation boundary.
  Final Windows verification passes the focused 4/4 gate and the complete
  182/182 suite. WSL remains unavailable because no distribution is registered.

## 2026-07-10 Phase E Step 500 LunaSVG Raster Backend

- Step 499 is committed at `e0b22c04 feat: plan svg rasterization`; tracked
  `master` is clean with only the existing `.vscode/` directory untracked.
- The first xrepo install attempt successfully installed PlutoVG, then timed
  out fetching the GitHub-hosted LunaSVG prebuilt artifact. Before changing
  backend strategy, inspect the package recipe for a source-build or reachable
  mirror path.
- The package recipe has no alternate URL, and the guessed Gitee mirror is not
  publicly accessible. A checksum-verified codeload tarball in a temporary
  xmake search directory is the next source-build path.
- The codeload v3.5.0 tarball matches the xmake recipe SHA-256. Windows tar
  cannot create the extraction tree under `C:\tmp` in this sandbox, so use a
  repository-local `.tmp_deps` directory and remove only that created tree
  after package installation.
- LunaSVG v3.5.0 was successfully source-built from the checksum-verified local
  archive. Its API provides length-aware `Document::loadFromData(...)`, sized
  `renderToBitmap(...)`, and in-place `Bitmap::convertToRGBA()`; third-party
  types can remain private to one focused backend source.
- Phase E Step 500 now adds `SvgRasterizationResult` and `rasterize_svg(...)`
  over LunaSVG v3.5.0. The backend returns
  plain RGBA pixel output after length-aware parse, sized render, conversion,
  and stride validation. Step 501 SVG raster cache is next.
- Focused backend/plan/structure/ledger verification passes 4/4; the public
  leaf is 65 lines and the LunaSVG backend is 68 lines.
- On session recovery, the WinGet-linked `rg.exe` again failed to start, so
  repository searches should continue with PowerShell `Select-String`. The
  Step 500 tracked diff remains intact and excludes the pre-existing untracked
  `.vscode/` directory.
- Step 500 closes with a successful complete Windows debug build and 181/181
  full-suite result. A fresh final pass also confirms the focused 4/4 gate,
  30/30 documentation phrases, valid JSON, module line limits, and diff hygiene.
  WSL still has no registered distribution, so Linux execution remains a hard
  Phase E closeout gate rather than a Step 500 claim.

## 2026-07-10 Phase E Step 499 SVG Rasterization Strategy

- Step 498 is committed at
  `3aea175b test: close vulkan image integration band`; tracked `master` is
  clean and only the existing `.vscode/` directory remains untracked.
- Existing Phase C SVG support owns markup metadata only through `ImageSource`,
  `ImageAssetRegistry::register_svg(...)`, and the public source example. The
  repository currently has no SVG parser/raster dependency or renderer backend.
- The Steps 499-506 ownership split is: strategy/request/result boundary,
  raster backend, caller-visible raster cache, viewport scaling, recolor/tint,
  image-upload integration, public example, and audit closeout.
- Step 499 should begin in focused public renderer leaves plus a focused
  implementation file; it must not place parser bodies in `image_source.cpp`,
  `image_asset_registry.cpp`, broad renderer entry files, or aggregate headers.
- The local xmake repository exposes `lunasvg v3.5.0`, a cross-platform C++ SVG
  rendering library. Use it as the Step 500 backend while keeping its types out
  of public headers and preserving a focused implementation boundary.
- Step 499 should define a public planning boundary rather than a backend stub:
  `SvgRasterizationRequest` and `SvgRasterizationPlan` expose RGBA8 output,
  device-pixel dimensions, stride/byte budget, validity, and strategy. Step 500
  then consumes that plan through the LunaSVG implementation.
- `cgpui_renderer` already owns focused `src/renderer/*.cpp` implementations.
  The compatibility aggregate `renderer.hpp` should include the new leaf only;
  structure coverage should cap the leaf/implementation and reject bodies in
  the aggregate.
- Phase E Step 499 now adds `SvgRasterizationRequest` and
  `SvgRasterizationPlan`. The focused planner emits an
  explicit RGBA8 output plan with ceil-rounded device pixels and a
  bounded raster byte budget. Step 500 LunaSVG raster backend is next.
- Focused Step 499 behavior/structure/ledger coverage passes 3/3 with the
  public header at 48 lines and focused implementation at 77 lines.
- The complete Windows debug build and full 180/180 test suite pass. WSL has no
  installed distribution, so Linux verification remains deferred to the final
  Phase E gate.

## 2026-07-10 Phase E Step 498 Image Integration Closeout

- Step 497 is committed at `5d77f965 feat: invalidate vulkan image textures`;
  tracked `master` is clean and only the existing `.vscode/` directory remains
  untracked.
- Step 498 is an audit-only closeout. It should add a focused API-parity test
  and xmake target that freeze the Steps 491-497 resource, upload, sampling,
  draw, tint/opacity, cache lifetime, and invalidation evidence without adding
  renderer behavior.
- Existing Phase E closeouts use one standalone source-reading test that checks
  the band targets, focused module symbols, behavior-test symbols, five exact
  documentation surfaces, and roadmap completion state. Step 498 should follow
  that established structure as
  `phase_e_image_integration_closeout_test`.
- The authoritative Steps 491-497 targets are
  `vulkan_image_texture_resources_test`, `vulkan_image_texture_upload_test`,
  `vulkan_image_texture_descriptor_test`, `vulkan_image_draw_recording_test`,
  `vulkan_image_tint_opacity_test`, `vulkan_image_texture_cache_test`, and
  `vulkan_image_texture_invalidation_test`.
- The standalone Step 498 audit target compiles. Its initial test is RED only
  because the five closeout documents and completed roadmap state have not yet
  been authored.
- The closeout should move the Phase E handoff to Step 499 SVG rendering
  strategy while retaining the explicit Windows/WSL final verification gate.
- Phase E Step 498 image integration closeout is now audit-only in
  `tests/api_parity/phase_e_image_integration_closeout_test.cpp`. It freezes
  Steps 491-497 across persistent image texture resources,
  explicit bitmap upload transport, nearest/linear sampling descriptors,
  stable authored image interleaving, multiplicative image tint, the
  frame-generation image cache, and deduplicated image invalidations.
  Step 499 SVG rendering strategy is next.
- The focused closeout/structure/ledger gate passes 3/3, exact documentation
  phrases pass 55/55, ledger JSON parses, and diff hygiene is clean.
- Windows verification passes the complete build, focused image integration
  regressions 11/11, and the full debug suite 179/179. WSL remains unavailable
  because the current host has no installed distribution.

## 2026-07-10 Phase E Step 497 Image Invalidation

- Step 496 commit `4fa1981f` leaves tracked `master` clean with only the
  existing untracked `.vscode/` directory.
- Existing uploads already refresh same-allocation texture pixels and replace
  allocation-mismatched resources. The remaining invalidation gap is an
  explicit frame command for resource unregister/forced refresh.
- The public ownership boundary is the focused `RenderFrame` leaf with a
  backward-compatible default `invalidate_image(ImageAssetId)` method. Vulkan
  frames should deduplicate ids in frame-owned storage.
- The private ownership boundary should be a focused image-texture invalidation
  module. It must run after the in-flight fence and staging cleanup, but before
  cache touch/eviction and draw/upload resource ensure, so same-frame uploads
  rebuild cleanly and invalidated draw-only resources stay safely unreadable.
- Step 498 remains the image integration closeout audit.
- Phase E Step 497 now adds `RenderFrame::invalidate_image(...)` and VulkanFrame
  stores deduplicated image invalidations. The focused invalidation module
  performs fence-safe resource destruction before cache touch and ensure.
- Pure invalidation is idempotent for duplicate/missing ids. Live coverage passes
  upload/draw, invalidate/draw-only, and invalidate/upload/draw refresh frames.
  Step 498 image integration closeout is next.
- Focused Windows regression coverage passed 12/12, the complete Windows debug
  suite passed 178/178, and WSL remains unavailable because no distribution is
  installed on the current host.
- Final closeout checks passed Step 497 behavior/structure/ledger 3/3, exact
  documentation phrases 25/25, JSON parsing, module line limits, and diff
  hygiene.

## 2026-07-10 Phase E Step 496 Image Cache Lifetime

- Step 495 commit `6472f778` leaves tracked `master` clean with only the
  existing untracked `.vscode/` directory.
- Current image textures persist indefinitely in `VulkanImageTextureResources`
  until renderer teardown or allocation-identity replacement. Step 496 should
  make retention and eviction explicit without taking Step 497 invalidation.
- The focused boundary should be a private frame-generation image cache leaf.
  Each texture records its last-used frame; the default policy retains 120 idle
  frames and evicts only during frame preparation after the in-flight fence.
- Draw and upload requests must touch existing resources before eviction so an
  asset used by the new frame cannot be reclaimed. New/reused resources then
  record the current generation through the existing ensure path.
- Live coverage should prove an uploaded texture survives an idle frame and can
  be drawn again without re-upload. Step 497 image invalidation remains next.
- Phase E Step 496 now provides a frame-generation image cache with a default
  retention of 120 idle frames. Draw and upload ids are touched before
  fence-safe eviction, resource creation/reuse records the current generation,
  and generation wrap conservatively rebases live entries.
- The live upload -> idle -> draw path passes without a second upload, proving
  retained shader-readable layout and descriptor reuse. Step 497 image invalidation
  remains next.
- The complete Windows debug build succeeds, focused cache and image regressions
  pass 9/9, and the full suite passes 177/177. WSL has no installed distribution,
  so Linux verification remains a final Phase E requirement.
- Final Step 496 audits pass: behavior/structure/ledger 3/3, required phrases
  25/25, valid parity JSON, module line counts 76/32/72/59/96/149 within their
  limits, and clean diff hygiene.

## 2026-07-10 Phase E Step 495 Image Tint And Composition Opacity

- Step 494 commit `99e54f52` leaves tracked `master` clean with only the
  existing untracked `.vscode/` directory.
- Public image tint already propagates through `ImageBuilder`, `ImageElement`,
  paint metadata, and `ImageDraw`; Step 495 should add production GPU
  consumption rather than broaden the public widget surface.
- The focused ownership boundary is a private `vulkan_image_color` leaf that
  resolves an absent tint to opaque white and reuses
  `vulkan_apply_composed_opacity(...)` so precomposed opacity is applied once to
  tint alpha.
- `VulkanImageVertex` should gain an explicit RGBA color attribute. The vertex
  shader forwards that color and the fragment shader performs multiplicative
  image tint over the sampled full-RGBA texel. Step 496 retains image cache
  lifetime ownership.
- Phase E Step 495 now resolves absent tint to opaque white, applies composition
  opacity through the existing precomposed helper for single application to
  tint alpha, and carries the result in every image vertex. The shaders perform
  multiplicative image tint against the sampled full-RGBA texel.
- Optimized embedded SPIR-V was regenerated at 289 vertex words and 141
  fragment words, matches `glslc -O` output word-for-word, and passes
  `spirv-val`. Step 496 image cache lifetime is next.
- The complete Windows debug build succeeds, focused regressions pass 10/10,
  and the full suite passes 176/176. WSL remains unavailable because no
  distribution is installed, so Linux verification stays required at Phase E
  closeout.
- Final Step 495 audits pass: behavior/structure/ledger 3/3, required phrases
  25/25, valid parity JSON, module line counts 9/14/59/114/104/183 within their
  limits, and clean diff hygiene.

## 2026-07-10 Phase E Step 494 Image Pipeline And Draw Recording

- Step 493 commit `c1fe890c` leaves tracked `master` clean with only the existing
  untracked `.vscode/` directory.
- The authoritative roadmap groups Steps 494-498 as image pipeline recording,
  tint/opacity, cache lifetime, and invalidation. Step 494 should close the
  first production boundary only: dedicated image graphics pipeline resources,
  sampling-descriptor selection, and actual Vulkan draw recording.
- The implementation should reuse the proven text/rounded pipeline ownership
  shape while keeping image shaders, vertex contract, pipeline resources, and
  recording in focused private modules. Tint/opacity remains Step 495.
- UI image builder sampling propagation must be located by symbol/file inventory
  and wired through the existing `ImageDraw::sampling` field without broadening
  runtime or renderer aggregate files.
- Existing production ownership is explicit: swapchain lifecycle owns text and
  rounded pipeline resources, `vulkan_command_recording.cpp` plans draw commands,
  and `VulkanFrameDrawOrderCursor` preserves authored interleaving. Step 494 must
  extend those focused boundaries with an image resource kind rather than sort
  or record image draws separately.
- The UI image path lives in `include/cgpui/ui/element_image_nodes.hpp`,
  `src/ui/element_image_nodes.cpp`, `src/ui/element_image_paint.cpp`, and
  `src/ui/render_view_commands.cpp`; there is no `src/ui/image_builder.cpp`.
- The public builder leaf is `include/cgpui/ui/image_builder.hpp`. Sampling must
  flow through `ImageElement`, `ImagePaint` in `include/cgpui/ui/paint.hpp`,
  `PaintList::draw_image` in `src/ui/paint_image.cpp`, and finally
  `submit_paint_command_to_frame(...)`.
- `ImageBuilder` implementation lives in `src/ui/widgets/image_builder.cpp`.
  The focused public change is an rvalue-qualified `sampling(...)` setter plus a
  stored mode on `ImageElement`; `PaintList::draw_image` should accept the mode
  with a linear default for direct callers.
- Image vertices can mirror the text position/UV contract, but the fragment
  shader must sample full RGBA. Reusing the text fragment shader would treat the
  texture red channel as glyph coverage and is therefore incorrect.
- Vulkan SDK `glslc.exe` and `glslangValidator.exe` are available under
  `D:/VulkanSDK/1.4.341.1/Bin`. Step 494 can compile readable image GLSL and
  embed validated SPIR-V using the existing renderer pattern.
- The focused RED gate should require a new image draw-recording private leaf
  and cover quad vertices/UVs, sampling descriptor selection, draw ranges,
  stable frame-order integration, and source-structure ownership.
- The Step 494 RED build fails exactly at the missing focused
  `vulkan_image_draw_recording_internal.hpp` boundary before any production
  image pipeline or recording implementation exists.
- Optimized GLSL compilation produced validated embedded SPIR-V contracts with
  260 vertex words and 119 fragment words. The readable GLSL remains beside the
  focused shader binary/module sources.
- The first Step 494 implementation build succeeds. Direct execution reaches
  exit `61`, proving public sampling propagation, source-rect UVs, pipeline
  state, embedded shader magic, descriptor-mode draw planning, and stable image
  order all pass before the expected architecture-source gate.
- Phase E Step 494 now provides an image graphics pipeline, sampling descriptor
  selection, stable authored image interleaving, and actual Vulkan image draw recording.
  Full-RGBA shaders, transformed normalized source-UV vertices, and
  clip-stack scissors remain in focused renderer modules. Step 495 owns image
  tint and composition opacity.
- Embedded image SPIR-V matches the optimized `glslc` output word-for-word and
  passes `spirv-val`. Renderer/UI focused regressions pass 18/18, the five
  required phrases pass across five authoritative documents (25/25), and the
  complete Windows debug suite passes 175/175.
- WSL verification remains unavailable because `wsl.exe -l -q` returns success
  with an empty distribution list. Linux image pipeline verification remains in
  the required final Phase E gate.
- Final Vulkan layout review found that descriptor-only first-frame resources
  would otherwise be sampled from `VK_IMAGE_LAYOUT_UNDEFINED`. The planner now
  records only shader-readable textures or textures with a pending upload and
  safely skips unready draws; focused layout/lifetime coverage passes 4/4.
- After that layout-readiness guard, the complete Windows debug suite remains
  green at 175/175. The final cross-document phrase audit is 25/25, the parity
  JSON parses, all focused image modules and `vulkan_presentation.cpp` remain
  within their structure limits, and `git diff --check` passes.

## 2026-07-10 Phase E Step 493 Image Sampler Modes And Descriptors

- Step 492 commit `9aa9dea1` leaves tracked `master` clean with only the existing
  untracked `.vscode/` directory.
- Step 493 should add a focused public `ImageSamplingMode` leaf and carry the
  mode on `ImageDraw`, defaulting to linear without expanding broad renderer
  type aggregates.
- Private Vulkan ownership should mirror the proven glyph atlas descriptor
  pattern: one descriptor layout and pool, fixed capacity for 256 textures, two
  descriptor sets per texture, and persistent nearest/linear samplers.
- Resource creation will allocate and write both sampler-mode descriptor sets
  after image/view creation. Actual image pipeline creation, UI builder
  propagation, and draw command recording remain Step 494.
- The Step 493 RED build fails exactly at the missing focused private descriptor
  header before any public sampling or Vulkan descriptor implementation exists.
- Phase E Step 493 now adds `ImageSamplingMode`, persistent nearest/linear
  samplers, and descriptor set binding for both modes on every cached texture.
  The fixed pool supports 256 textures and 512 sets. Step 494 owns the image
  graphics pipeline and draw recording.
- The complete Windows debug build succeeds, and the full suite passes 174/174.
  Descriptor sets are freed before image/view/memory teardown; descriptor
  layout, pool, and sampler ownership remains in the focused Vulkan image
  texture resource state.
- WSL verification remains unavailable because `wsl.exe -l -q` returns success
  with an empty distribution list. Linux verification remains a required final
  Phase E gate when a distribution is available.
- Final Step 493 focused verification passes 9/9. The five required phrases are
  present across all five authoritative documents (25/25), the ledger JSON
  parses, all focused modules remain within structure limits, and
  `git diff --check` passes.

## 2026-07-10 Phase E Step 492 Image Upload Staging

- Step 491 commit `9e903644` leaves tracked `master` clean with only the existing
  untracked `.vscode/` directory.
- `ImageUploadBatch` already owns copied RGBA bytes and region metadata, while
  `ImageDraw` intentionally stays descriptor-only. Step 492 should preserve that
  lightweight draw command and add an explicit frame upload operation rather
  than embedding a pixel-owning container in every draw.
- The focused Vulkan path can collect frame-owned upload batches before
  `present()`, reconcile the Step 491 image resource by descriptor, allocate a
  host-visible staging buffer, record buffer-to-image copy plus undefined/readable
  layout transitions, and commit layout state only after successful submit.
- `ImageAssetRegistry::raster_assets()` exposes the retained bitmap owners
  explicitly, so callers can select assets for `RenderFrame::upload_image(...)`
  without a renderer-global registry or descriptor-to-bytes lookup.
- The Step 492 RED build fails exactly at the missing focused private upload
  header, before production staging or command recording exists.
- The focused upload target now compiles. Direct execution exits `50`, exactly
  at the intentionally missing documentation gate, after explicit frame
  transport, RGBA validation, copy metadata, layout barriers, and source-boundary
  checks pass.
- Phase E Step 492 now routes `RenderFrame::upload_image` into frame-owned
  batches, host-visible RGBA staging, buffer-to-image copy recording, layout
  barriers, and submit-time readable-layout commit. The live upload frame and
  structure/resource regressions pass. Step 493 owns sampler modes and descriptor
  binding.
- Final focused Step 492 verification passes 5/5, all 25 required phrases are
  present across the five authoritative documents, and the parity JSON parses.
- The complete Windows debug build succeeds and the full suite is GREEN at
  173/173. New upload modules remain at 42/168/121 lines, the expanded frame leaf
  is 79 lines, and presentation remains exactly at its 150-line structure limit.
- WSL verification remains unavailable because `wsl.exe -l -q` returns an empty
  distribution list. Shared image upload and public frame API coverage remain in
  the final Phase E Linux gate.
- Samplers, descriptor sets, textured image pipeline/draw recording, tint, and
  cache eviction remain later Steps 493-497.

## 2026-07-10 Phase E Step 491 Image Texture Resources

- Step 490 commit `0cbb4915` leaves tracked `master` clean with only the existing
  untracked `.vscode/` directory.
- The public/UI path already carries `ImageDraw` bounds, asset descriptor,
  optional source rectangle/tint, clip stack, composition stack, and paint
  metadata. Existing CPU-side `ImageUploadBatch` planning does not own Vulkan
  images or device memory.
- `VulkanFrame` already collects `image_draws_` and records image entries in its
  compact authored order, but the current presentation path does not yet prepare
  or record image GPU resources.
- Step 491 should establish focused private Vulkan texture resource ownership
  (`VkImage`, memory, view, descriptor-facing metadata, dimensions/format, and
  destruction/reconciliation boundaries) without pulling Step 492 upload
  staging or later sampler-mode behavior into the same slice.
- The real public owners are `renderer_types.hpp` for `ImageAsset` and
  `ImageAssetDescriptor`, `image_uploads.hpp` for CPU upload batches, and
  `vulkan_report_image_uploads.cpp` for the existing report/planning adapter.
- `ImageAssetDescriptor` deliberately carries dimensions/stride/format/byte size
  but not pixel bytes. Step 491 resource reconciliation can therefore key by
  descriptor identity and allocate device-local RGBA image/view ownership while
  leaving actual bytes and layout transitions to Step 492.
- Existing glyph atlas code demonstrates the intended module split: resource
  structs and image creation/destruction live in focused private leaves, frame
  orchestration owns reconciliation, and staging is a separate sibling module.
- The current widget/paint path loses bitmap bytes intentionally: `ImageElement`
  paints an `ImageAssetDescriptor`, and `ImageDraw` carries no pixel owner.
  Step 491 must either connect an existing asset registry to Vulkan resource
  requests or introduce a narrow immutable asset-reference transport that Step
  492 can consume; descriptor-only allocation cannot complete real uploads.
- `ImageAssetRegistry` does retain raster `ImageAsset` values, but no runtime or
  renderer consumer currently observes the registry. It cannot be treated as an
  implicit global upload source.
- Conservative Step 491 scope: allocate/reconcile persistent device-local
  `VK_FORMAT_R8G8B8A8_UNORM` image, memory, and view resources keyed by valid
  descriptors; preserve `VK_IMAGE_LAYOUT_UNDEFINED` and no sampler/descriptor
  ownership. Step 492 must explicitly connect pixel bytes, staging, copy, and
  layout transitions.
- To preserve modular boundaries, use one private resource header plus focused
  image creation/destruction, reconciliation, and renderer-frame integration
  sources. Add renderer source-structure gates immediately.
- `cgpui_renderer_vulkan` already compiles focused `.cpp` leaves by wildcard;
  only the focused test target and explicit architecture file inventory need
  xmake/structure updates.
- The Step 491 RED build fails exactly at the missing focused private resource
  header, before any production implementation exists.
- Step 491 now has separate resource contract, image creation/destruction,
  request reconciliation, and frame integration leaves. Resources remain
  device-local, sampler-free, descriptor-free, and in undefined layout for the
  Step 492 staging handoff.
- The focused target compiles, and direct execution exits `40` exactly at the
  missing documentation gate. Resource behavior, state lifecycle, and source
  boundary assertions pass.
- Reused allocations refresh their descriptor metadata even when dimensions and
  format are unchanged, so Step 492 receives current stride/byte-size metadata
  without forcing a VkImage recreation.
- Phase E Step 491 now implements `VulkanImageTextureResources` with persistent
  device-local RGBA image/view ownership, descriptor request preflight, frame
  reconciliation, and device teardown. Resources deliberately keep undefined
  layout and no sampler/descriptor/upload bytes. Step 492 owns image upload
  staging and explicit pixel transport.
- Focused resource, renderer architecture, frame lifetime, and stable-order gates
  pass 4/4. All four new implementation leaves stay below their structure limits.
- A live frame should still exercise the real Vulkan allocation path by
  submitting a valid image descriptor and presenting successfully; this proves
  resource creation without claiming pixel upload or image draw recording.
- Added that live image resource frame to `vulkan_frame_lifetime_test` and froze
  it in the focused Step 491 structure gate.
- The live Vulkan frame lifetime test passes with the descriptor-only image
  command, proving actual image/memory/view allocation and teardown before Step
  492 upload work.
- Final focused Step 491 verification passes 4/4 across resource behavior,
  renderer structure, parity ledger, and live Vulkan allocation.
- The complete Windows debug build succeeds and the full suite is GREEN at
  172/172.
- The final Step 491 audit passes across all five authoritative documents; the
  parity ledger JSON parses, the four new module line counts remain
  53/134/119/11, and `git diff --check` is clean.
- WSL verification remains unavailable because `wsl.exe -l -q` returns an empty
  distribution list. The shared Vulkan image texture work remains in the final
  Phase E Linux gate.

## 2026-07-10 Phase E Step 490 Clip/Composition Integration Closeout

- Step 489 commit `c4ef7e92` leaves tracked `master` clean with only the existing
  untracked `.vscode/` directory.
- The roadmap defines Step 490 as closing the clip/composition band before image
  production begins at Step 491. No new renderer behavior is implied.
- The correct Step 490 boundary is an audit-only closeout test over Steps
  475-489: rounded geometry/resources/recording/antialias/radii/borders/fill,
  clip stacks, composed opacity, solid geometry, transforms, transformed clips,
  stable draw interleaving, and explicit z/layer ordering.
- The closeout should follow the existing Phase E audit pattern: source/test
  evidence, xmake target registration, synchronized roadmap/ledger/planning
  phrases, and an explicit Step 491 image-texture handoff.
- All fifteen focused band targets already exist in the platform-gated Vulkan
  section. The Step 490 closeout target itself should remain a portable
  source-audit binary beside the Step 466/474 closeouts.
- Representative behavior evidence spans contiguous rounded geometry, paired
  buffers, fixed pipeline state, validated indexed draws, coverage fringes,
  normalized radii, inset strokes, fill variants, nested scissor resolution,
  composed opacity, blended solid geometry, affine vertex transforms,
  push-time clip AABBs, authored interleaving, and UI z/layer order reaching
  the Vulkan cursor.
- The focused closeout audit will require the exact phrases `Steps 475-489`,
  `non-rectangular clip`, and `Step 491 image texture resources` across all five
  authoritative planning/ledger documents.
- The Step 490 audit builds successfully and direct execution exits `40`, the
  first required-document phrase. Target, module, behavior, and roadmap-status
  code paths therefore compile before documentation synchronization.
- Phase E Step 490 clip/composition integration closeout now freezes Steps
  475-489 in
  `tests/api_parity/phase_e_clip_composition_integration_closeout_test.cpp`:
  contiguous rounded geometry, allocation-free dynamic scissor,
  single-application composed opacity, blend-capable solid geometry, push-time
  framebuffer AABB capture, stable authored interleaving, and explicit z/layer
  command ordering. A future non-rectangular clip remains a stencil or
  shader-mask boundary. Step 491 image texture resources is next.
- The corrected closeout audit is GREEN, all twelve required phrases are present
  in all five authoritative documents, and `git diff --check` passes.
- The full Steps 475-490 focused band passes 19/19. The complete Windows debug
  build succeeds and the full suite is GREEN at 171/171.
- WSL remains unavailable because `wsl.exe -l -q` returns an empty distribution
  list. The portable closeout audit is registered for Linux, but the final Phase
  E WSL gate remains required.

## 2026-07-10 Phase E Step 489 Explicit Z/Layer Command Ordering

- `Element::z_order()` already gives explicit nonzero `z_index` precedence over
  `layer`.
- `ElementTree` and direct `StyledElement` child traversal already sort paint
  children by ascending z-order and stable sibling index, while hit testing and
  event dispatch consume the reverse visual order.
- Step 488 preserves the authored `PaintList` interleaving through
  `VulkanFrameDrawOrderCursor`, so Step 489 must not add a second renderer-side
  sort that could flatten or reorder nested traversal semantics.
- The focused Step 489 slice should prove the existing UI z/layer order survives
  mixed solid, rounded, and text submission into the Vulkan frame cursor,
  including negative layers, explicit z-index override, and equal-order stable
  siblings.
- The authoritative roadmap defines Steps 489-490 as completing z/layer order
  in actual command recording and then closing the clip/composition band; it
  does not require renderer-owned z metadata or a second sorting stage.
- `RenderView` iterates `paint_list.commands()` directly,
  `submit_paint_command_to_frame(...)` forwards each primitive immediately, and
  `VulkanFrame::append_draw` is therefore the correct end-to-end observation
  boundary for the focused ordering gate.
- `ElementTree::paint_subtree(...)` uses `std::stable_sort` on `z_order()`, and
  `paint_ordered_children(...)` uses the original sibling index as its equal-z
  tie-break. These are the two production ordering paths the gate must freeze.
- A focused test can use small test-only `Element` subclasses to author solid,
  rounded, and text commands with negative layer, explicit z-index override,
  and equal-order siblings. It should pass the resulting PaintList through the
  real `submit_paint_command_to_frame(...)`, capture the real `RenderFrame`
  virtual calls, and feed their compact primitive order into the real Vulkan
  cursor with matching resource spans.
- The production Vulkan frame indexes each primitive before pushing it into its
  per-type vector (`.command_index = draws.size()`), so the recording test frame
  mirrors that exact compact-index rule before exercising the real cursor.
- The new Step 489 gate compiles and its behavior/source-boundary checks pass;
  direct execution exits `40`, exactly at the intentionally missing
  cross-document evidence gate. No production renderer change is required.
- Phase E Step 489 now freezes explicit z/layer command ordering across the
  existing UI ownership boundary and Vulkan recording path. The focused gate
  proves stable UI paint order for negative layers, explicit z-index override,
  and equal-order mixed primitive siblings. Step 490 owns clip/composition
  integration closeout.
- The Step 489 focused gate is GREEN after documentation sync. The parity JSON
  parses, `git diff --check` is clean, and the tracked diff contains only the
  focused test/build entry plus roadmap, ledger, and planning updates.
- The complete Windows debug build passes and the full suite is GREEN at
  170/170, including the new end-to-end layer ordering gate.
- WSL remains unavailable because `wsl.exe -l -q` returns an empty distribution
  list. Linux compilation of the shared test/build entry remains part of the
  final Phase E gate.
- The corrected cross-document phrase audit passes for all five authoritative
  files. No `clang-format` executable is installed on this host; the new test
  was manually checked against adjacent C++ test style.

## 2026-07-10 Phase E Step 488 Stable Renderer Command Ordering

- UI submission already calls `RenderFrame` in authored order, but
  `VulkanFrame` stores per-primitive vectors and command recording currently
  replays all solid draws, then all rounded draws, then all text draws.
- A compact `{RendererPrimitiveKind, command_index}` record can preserve frame
  order without duplicating clip/composition metadata or introducing type
  erasure. The existing per-type vectors remain the owning storage.
- Solid and rounded GPU ranges already retain `source_index`. Text page-run
  commands need to retain `text_draw_index` so one authored text draw can expand
  to multiple atlas-page draw calls in place.
- A zero-allocation `VulkanFrameDrawOrderCursor` can merge the compact order
  with sorted per-type ranges, skip commands that emitted no geometry, and
  expand text page runs while preserving stable authored interleaving.
- Step 489 should own explicit z/layer command ordering after stable authored
  order reaches actual Vulkan command recording.
- Phase E Step 488 now implements compact frame order entries and a
  zero-allocation `VulkanFrameDrawOrderCursor`. `VulkanFrame` appends only kind
  and per-type index, leaving existing vectors as the sole command owners.
- Rounded/solid draw ranges and text commands retain their source indices. The
  cursor skips ranges absent after geometry filtering and expands every text
  atlas page run at the authored text position, preserving stable authored interleaving.
- The focused ordered recorder reuses bind/single-draw helpers and only rebinds
  state when switching between solid buffers, rounded buffers, and text.
  Step 489 owns explicit z/layer command ordering.

## 2026-07-10 Phase E Step 487 Transform/Clip Interaction

- `PaintList` call order establishes clip scope: styled elements push their
  precomposed metadata before an overflow clip, while callers may intentionally
  push a framebuffer clip before later transforms.
- `RendererClipStackRecord` retains rectangles but no per-entry transform. The
  correct zero-allocation boundary is therefore clip push time, not final draw
  recording: capture each clip using the transform active at that scope, then
  intersect only framebuffer-space rectangles.
- `paint_clip.cpp` already owns nested clip intersection. Step 487 should keep
  it unchanged, add `transform_clip_rect_to_framebuffer_aabb` in a focused
  `paint_clip_transform` sibling, and keep broad `paint.cpp` to one call.
- Rotation or skew cannot be represented exactly by a Vulkan scissor. A
  conservative push-time framebuffer AABB preserves visible content; exact
  non-rectangular clipping still requires the later stencil or shader-mask path.
- Step 488 should continue with stable renderer command ordering after the
  transform/clip interaction policy is frozen.
- Phase E Step 487 now implements
  `transform_clip_rect_to_framebuffer_aabb` in a focused private leaf. It
  validates the active precomposed matrix, transforms four clip corners, and
  captures their conservative push-time framebuffer AABB before intersection.
- Nested clips retain the transform active at each push, while clips pushed
  before later transforms stay framebuffer-space. Invalid transforms and
  non-finite transformed corners retain the authored clip.
- The Vulkan clip resolver remains transform-free and allocation-free; it only
  intersects captured framebuffer rectangles and emits the dynamic scissor.
  Step 488 owns stable renderer command ordering.

## 2026-07-10 Phase E Step 486 Composed Affine Transform Application

- UI metadata already stores the composed parent/child affine transform, just as
  it stores precomposed opacity. Production geometry must select current stack
  metadata when present and apply the resulting matrix once.
- A focused transform leaf should own matrix validation and point application:
  `x' = scale_x*x + skew_x*y + translate_x` and the corresponding y equation.
  Non-finite matrices should fall back to identity before reaching GPU buffers.
- Solid and rounded vertices can transform during CPU geometry construction.
  Text must first apply its subpixel/pixel-snap positioning policy, then transform
  all four quad corners before emitting six triangle-list vertices.
- Existing clip rectangles are framebuffer-space. Step 486 should not silently
  transform them; Step 487 must define transform/clip interaction explicitly.
- Phase E Step 486 now implements `vulkan_apply_composed_transform` in a focused
  private renderer leaf. It selects current stack metadata when present,
  otherwise scalar metadata, validates every matrix component, and resolves a
  non-finite precomposed affine transform to identity.
- Solid quads and each rounded draw range transform their CPU vertices once.
  Text first applies the selected positioning policy, then transforms all four
  quad corners before emitting the six triangle-list vertices.
- Clip rectangles remain framebuffer-space. Step 487 owns the explicit
  transform/clip interaction policy.

## 2026-07-10 Phase E Step 485 Blend-Capable Solid Geometry

- `vkCmdClearAttachments` cannot implement source-alpha blending, so solid
  rectangles must leave the clear path to complete nested opacity semantics.
- The existing rounded pipeline already provides the required position/color/
  coverage ABI, straight-alpha blend state, dynamic scissor, and indexed draw
  recording. Solid rectangles can reuse it with a compact four-vertex/six-index
  geometry leaf rather than creating another graphics pipeline.
- Separate solid and rounded buffer resources avoid a per-frame temporary vector
  of adapted draw records. The shared upload module can accept either focused
  geometry result, and command recording can submit the two buffer sets in the
  current solid-before-rounded order until authored ordering work lands later.
- Step 485 should delete the obsolete solid clear recorder, retain clip and
  precomposed opacity through the geometry path, and hand Step 486 to affine
  transform application.
- Phase E Step 485 now implements `vulkan_build_solid_rect_geometry`. Each valid
  solid draw produces four vertices, six indices, one draw range, baked opacity,
  and one effective clip without adapting into a temporary rounded-draw vector.
- Separate `solid_rect_buffers_` reuse the existing upload helper, rounded
  pipeline, dynamic scissor recorder, and fence lifetime. The old authored
  rectangle clear recorder is deleted, completing blend-capable solid geometry.
  Step 486 owns composed affine transforms.

## 2026-07-10 Phase E Step 484 Nested Opacity Command Recording

- `PaintList::push_metadata(...)` already composes parent and child opacity, so
  the current stack metadata and scalar command metadata carry the final product.
  The renderer must clamp and apply that value once rather than multiplying all
  retained stack entries again.
- A focused private opacity leaf can keep the policy shared and allocation-free.
  Solid clear colors should resolve at recording, rounded fill/stroke colors at
  geometry construction, and production text quad colors before vertex upload.
- Renderer reports should retain authored color plus composition metadata; the
  production text draw-data path is the appropriate place to bake opacity into
  GPU vertex color without changing diagnostic quad semantics.
- `vkCmdClearAttachments` writes alpha but does not execute pipeline blending.
  Therefore clear writes do not blend even after applying the opacity policy;
  Step 485 must promote solid rectangles to blend-capable geometry before
  composed affine transforms move to Step 486.
- Phase E Step 484 now implements `vulkan_apply_composed_opacity`. The resolver
  uses current stack metadata when present, otherwise scalar metadata, treats
  non-finite opacity as 1, clamps to `[0, 1]`, and multiplies authored alpha once.
- Solid clear attachment color, rounded fill/stroke vertex colors, and
  production text atlas quad colors now consume precomposed opacity. Rounded
  and text pipelines blend; solid clear writes do not blend. Step 485 owns
  blend-capable solid rectangle geometry.

## 2026-07-10 Phase E Step 483 Clip-Stack Command Recording

- Paint commands already carry an effective nested `clip_rect` plus bounded
  `RendererClipStackRecord` metadata through render submission. Solid rectangles
  consume only the scalar clip in their clear rectangle, while rounded and text
  recorders currently reset one full-frame scissor for every batch.
- A focused private Vulkan clip-scissor leaf should resolve the retained stack,
  current clip, scalar fallback, and framebuffer bounds without allocation.
  Rounded draw ranges and text draw commands can retain only the resulting
  effective rectangle, avoiding repeated copies of the vector-backed stack.
- Actual rounded and text command recording must set a dynamic scissor before
  each visible draw and skip empty clip results. Solid rectangle clear recording
  should reuse the same resolver so all three production primitive paths agree.
- Non-rectangular clipping, stencil/shader masks, nested opacity, transforms, and
  cross-primitive authored ordering remain later work in the Steps 484-490 band.
- Phase E Step 483 now implements `vulkan_resolve_clip_stack_scissor` and
  per-draw dynamic scissor recording. The resolver intersects retained stack,
  current, and scalar clips, clamps/rounds to the framebuffer, and marks empty
  results invisible without allocating.
- Rounded geometry ranges and text page-run commands retain only one effective
  rectangle; their recorders set `vkCmdSetScissor` per visible draw. Solid clear
  rectangles consume the same resolver. Step 484 owns nested opacity command
  recording.

## 2026-07-10 Phase E Step 467 Text Pipeline Ownership

- The Vulkan backend currently has no graphics pipeline, shader module, or
  `vkCmdDraw` path; solid rectangles are still emitted with
  `vkCmdClearAttachments`.
- A text graphics pipeline is compatible with the swapchain render pass, so its
  handle-bearing resources should live in the private swapchain resource
  aggregate and be created after the render pass, not in broad command
  recording or public renderer headers.
- The glyph atlas descriptor-set layout remains renderer-owned and can be
  supplied to the focused text-pipeline creator when it builds the pipeline
  layout. Shader source/binary availability must be verified on both Windows
  and WSL before choosing build-time compilation versus checked-in SPIR-V.
- Windows exposes `glslc`, `glslangValidator`, `spirv-as`, and `spirv-val`
  through the Vulkan SDK. WSL exposes only `spirv-as` and `spirv-val`, so the
  production build cannot require GLSL compilation on every host.
- Step 467 should freeze a private `VulkanTextVertex` ABI plus fixed pipeline
  state helpers: triangle-list input, no culling/depth, one sample, straight
  alpha blending, and dynamic viewport/scissor. Step 468 can add validated
  embedded SPIR-V over that stable contract.
- Phase E Step 467 now owns `VulkanTextVertex` and the focused fixed state:
  triangle-list input, no culling or depth, one-sample rasterization,
  dynamic viewport/scissor, and straight alpha blending. Step 468 adds
  validated embedded shader modules; graphics pipeline handles and draw calls
  remain later slices.
- Phase E Step 468 adds reviewable vertex/fragment GLSL and validated
  embedded SPIR-V. `vulkan_text_vertex_shader_spirv` and the fragment companion
  feed focused `vulkan_create_text_shader_modules`, destroy, and stage helpers.
  The optimized 289-word vertex and 189-word fragment binaries pass
  `spirv-val` on Windows and WSL; Step 469 creates pipeline-layout and graphics-
  pipeline handles.
- Phase E Step 469 pipeline resources are render-pass compatible and therefore
  belong to the private swapchain resource lifetime. A focused resource header
  and source can own the pipeline layout and graphics pipeline, create them
  after the swapchain render pass using the glyph-atlas descriptor-set layout,
  move them through install/resize, and destroy them before the render pass.
- The text vertex shader's `vec2 framebuffer_size` maps to an explicit 8-byte
  vertex-stage push-constant contract. Shader modules remain transient during
  graphics-pipeline creation; only the layout and pipeline handles persist.
  `VulkanTextPipelineResources` and `vulkan_create_text_pipeline_resources`
  now implement that contract. Step 470 adds text vertex-buffer upload
  resources before command-recorded textured glyph draws.
- Phase E Step 470 can expand each flat `TexturedGlyphQuad` into six triangle-
  list `VulkanTextVertex` records without changing page-run ordering. Because
  `present_frame(...)` waits for the single in-flight fence before frame
  preparation, a focused host-visible/coherent vertex buffer can be rebuilt
  there without per-frame overlap or an extra staging resource.
- The vertex-buffer module should retain only Vulkan buffer/memory handles plus
  byte and vertex counts. `first_quad_index * 6` remains a zero-allocation draw
  offset for Step 471 descriptor-bound `vkCmdDraw` recording.
  `VulkanTextVertexBufferResources` and `vulkan_upload_text_vertex_buffer` now
  implement this fence-safe upload path.
- Phase E Step 471 should plan one draw command per existing contiguous page
  run. The planner can validate descriptor handles and convert quad ranges to
  32-bit vertex ranges before command recording; the focused recorder then owns
  all text-specific Vulkan binds and draws while the broad frame recorder only
  calls `vulkan_record_text_draws(...)` inside the render pass.
- Full-frame viewport/scissor and the 8-byte framebuffer-size push constant are
  stable per frame, so they should be bound once before iterating page runs.
  Pipeline and vertex buffer are also frame-global; only descriptor set and
  `vkCmdDraw` range change per contiguous page run. The focused
  `vulkan_record_text_draws` path now implements this contract, with all range
  validation completing before the render pass begins. Step 472 owns explicit
  subpixel positioning policy.
- Phase E Step 472 should preserve the current fractional device coordinates by
  default because shaping and DPI conversion already produce positioned device
  geometry. A separate deterministic pixel-snap policy remains useful for UI
  text or diagnostics, but it should round the quad's outer edges and derive the
  snapped size rather than rounding origin and size independently.
- Positioning policy belongs before vertex expansion and must not alter atlas UV
  coordinates. Recording the selected policy in vertex-buffer resources keeps
  the choice inspectable without adding public renderer state or shader branches.
  `VulkanTextPositioningPolicy` now defaults to `preserve_subpixel`, with the
  optional snap mode rounding outer device edges. Step 473 owns coverage gamma
  and alpha policy.
- Phase E Step 473 should keep the glyph atlas as linear `alpha8_unorm`
  coverage, make the coverage transfer explicit before alpha composition, and
  preserve straight RGB with `source_alpha` blending. This separates coverage
  gamma from color-space conversion and avoids double-premultiplying text color.
- `VulkanTextCoveragePolicy` now formalizes that contract. Production uses a
  linear transfer over clamped `R8_UNORM` samples, the reference helper covers
  deterministic power transfer, and `straight_color_coverage_alpha` preserves
  RGB while multiplying only output alpha. The optimized 212-word fragment
  shader payload passed Vulkan 1.0 `spirv-val`; Step 474 owns integration
  closeout rather than more shader behavior.
- Phase E Step 474 should be an audit-only closeout for Steps 467-473. The
  landed pipeline already owns fixed state, embedded shaders, swapchain
  resources, fence-safe vertices, descriptor-bound draws, positioning, and
  coverage/alpha policy; adding another renderer behavior in the closeout would
  blur the Step 475 rounded rectangle geometry boundary.
- Phase E Step 474 text pipeline integration closeout is implemented by
  `tests/api_parity/phase_e_text_pipeline_integration_closeout_test.cpp`. It
  freezes Steps 467-473, including descriptor-bound textured glyph draws,
  `preserve_subpixel`, and `straight_color_coverage_alpha`, then hands Phase E
  to Step 475 rounded rectangle geometry without adding renderer behavior.
- Phase E Step 475 should turn the existing report-only rounded-rectangle
  tessellation counts into one private contiguous vertex/index geometry buffer.
  The current runtime merely collects `RoundedRectDraw`; command recording
  ignores it, while solid rectangles still use `vkCmdClearAttachments`.
  Tessellation ownership therefore belongs in a new focused Vulkan geometry
  module, with GPU upload deferred to Step 476.
- `VulkanRoundedRectGeometry` now emits contiguous vertex/index buffers and
  stable per-source draw ranges from one pre-reserved allocation. A center plus
  clockwise corner-perimeter fan keeps the first production geometry simple and
  inspectable; GPU resource upload remains Step 476.
- Phase E Step 476 should mirror the fence-safe text upload lifecycle while
  keeping paired vertex/index allocation inside a rounded-rectangle-specific
  resource module. Draw ranges must remain with the resources for Step 477+
  pipeline and indexed-recording work.
- `VulkanRoundedRectBufferResources` now owns paired vertex/index buffers and
  keeps the geometry draw ranges beside them.
  `vulkan_upload_rounded_rect_buffers` runs through
  `prepare_rounded_rect_frame(...)` after the in-flight fence, and a live Win32
  first frame exercised real allocation/upload. Step 477 can build the shader
  pipeline over this stable resource ABI.
- Phase E Step 477 should use a dedicated swapchain-owned pipeline because the
  rounded-rectangle vertex ABI and future anti-aliasing inputs differ from text.
  Reviewable GLSL and embedded validated SPIR-V preserve the Step 468 build-time
  policy without coupling rounded primitives to glyph descriptors.
- Phase E Step 477 now owns `VulkanRoundedRectPipelineResources`, transient
  shader modules, fixed graphics state, and swapchain create/install/destroy
  integration in focused private files. The reviewable sources compile into
  embedded rounded rectangle SPIR-V validated for Vulkan 1.0, and the pipeline
  uses no descriptors. Step 478 can bind the paired buffers and record indexed
  rounded rectangle draws without changing resource ownership.
- Phase E Step 478 validates the retained ranges without allocating a second
  command vector, then `vulkan_record_rounded_rect_draws` binds the dedicated
  pipeline, paired vertex/index buffers, and framebuffer push constants before
  `vkCmdDrawIndexed`. Solid clear recording moved out of the general command
  entry to preserve its module-size guard. Step 479 should define the rounded
  rectangle anti-aliasing strategy without weakening this indexed path.
- Phase E Step 479 uses `VulkanRoundedRectAntialiasingPolicy` to keep the
  coverage fringe explicit. Geometry owns the extra inner/outer rings, the
  pipeline vertex ABI owns scalar coverage, and the fragment shader multiplies
  only straight alpha. The optimized embedded Vulkan 1.0 SPIR-V is 293 vertex
  words and 155 fragment words. Step 480 should normalize overlapping corner
  radii before both the inner contour and coverage fringe are generated.
- Phase E Step 480 centralizes radius clipping in
  `VulkanRoundedRectRadiiResolution`. The resolver clamps negative values and
  applies a single scale derived from all four adjacent corner sums, preserving
  relative asymmetry while preventing overlap. Both fill and coverage fringe
  now use the same resolved radii. Step 481 can derive an inset stroke contour
  from this stable normalization boundary.
- Phase E Step 481 keeps stroke policy in
  `VulkanRoundedRectStrokeResolution` and contour sampling in separate private
  leaves. The border width clamps to half the smaller dimension, an inset stroke
  contour derives from the Step 480 radii, and duplicated inner rings preserve a
  hard fill/border color boundary without adding a blended transition band.
  Step 482 should connect fill variants and close the rounded-rectangle band.
- Phase E Step 482 closes rounded rectangle fill variants without forcing
  nonuniform style edges into a uniform renderer contract. `PaintList` owns
  fill-only, fill-plus-stroke, and stroke-only APIs in a focused source leaf;
  styled boxes coalesce only positive uniform widths, and Vulkan stroke-only geometry
  uses three perimeter rings with no center/fill ring. Step 483 can
  start clip-stack command recording over the completed rounded draw path.

## 2026-07-10 Phase E Step 466 Glyph Atlas Integration Closeout

- Step 466 is an audit-only closeout for the production glyph-atlas band. The
  runtime implementation already lives in focused resource, staging, upload
  recording, frame, draw-binding, and draw-data modules from Steps 459-465.
- The closeout should freeze both behavior and ownership evidence: alpha8 page
  resources, descriptor capacity and preflight, dirty uploads, acquired-buffer
  recording, three-page reuse, renderer-owned descriptor bindings, contiguous
  flat quad ranges, and live range/page validation.
- No text shader, pipeline, vertex-buffer, or draw-call implementation belongs
  in this slice. Step 467 starts that separate renderer band.
- Phase E Step 466 glyph atlas integration closeout is frozen by
  `tests/api_parity/phase_e_glyph_atlas_integration_closeout_test.cpp`. The
  audit covers Steps 459-465 evidence for alpha8 page resources, descriptor
  capacity, dirty uploads, acquired command buffer recording, three atlas
  pages, private `VulkanGlyphAtlasDrawBinding` and
  `VulkanGlyphAtlasDrawData` ownership, and contiguous page runs.
  Step 467 text shader pipeline is the next implementation slice.

## 2026-07-10 Phase E Step 465 Glyph Atlas Draw Data

- Descriptor bindings alone are insufficient for the future text pipeline; the
  renderer also needs stable glyph quad ranges that preserve authored order.
- `VulkanGlyphAtlasDrawData` owns one flat quad vector. `first_quad_index` and
  glyph counts describe contiguous page runs, so a page sequence such as
  `2 -> 0 -> 2` remains three runs rather than being incorrectly merged.
- Quad planning lives in `vulkan_glyph_atlas_draw_data.cpp`; descriptor lookup
  and stale-handle checks remain in the Step 464 binding module.
- The live command buffer validates range bounds and every quad's page identity
  before the render pass. Step 466 can now close the atlas integration band.

## 2026-07-10 Phase E Step 464 Glyph Atlas Draw Bindings

- The runtime gap after Step 463 was not resource creation; it was converting
  text draw page usage into the actual descriptor set owned by renderer state.
- `VulkanGlyphAtlasDrawBinding` stays private because it carries
  `VkDescriptorSet`. Public submission records continue to expose only scalar
  `atlas_page_index` metadata.
- `vulkan_resolve_glyph_atlas_draw_bindings(...)` makes missing page resources
  an explicit error, while live command buffer validation detects a stale
  descriptor identity before the render pass begins.
- Page-usage planning reuses `vulkan_build_textured_glyph_quads(...)`, so glyph
  cache allocation and draw binding share one atlas-page decision instead of
  duplicating placement logic. Step 465 continues atlas draw-data integration.

## 2026-07-10 Phase E Step 463 Multi-Page Glyph Atlas

- Nine deterministic 128x128 glyphs fill a 256x256 page four-at-a-time, giving
  stable coverage of three atlas pages with page indices 0, 1, and 2.
- The focused staging design already handles cross-page uploads: each production
  upload command resolves its batch by page index and appends one independent
  staging allocation with its own copy list.
- Descriptor capacity must be a private shared contract, not a local magic
  number. `vulkan_glyph_atlas_descriptor_capacity` now sizes the pool and gates
  resource reconciliation before any existing page is destroyed.
- The Win32 frame smoke submits the same three-page synthetic workload so the
  test covers actual image, memory, view, descriptor, staging, command-buffer,
  and queue submission paths. Step 464 continues remaining atlas integration.

## 2026-07-10 Phase E Step 462 Multi-Frame Atlas Lifecycle

- Phase E Step 462 records only the acquired command buffer. Re-recording every
  swapchain command buffer duplicated upload commands and retained unnecessary
  staging references in command buffers that would not be submitted.
- The safe order is wait/prepare/acquire/record/reset-fence/submit. The fence is
  not reset until acquisition and command recording both succeed, so early
  failures do not leave the renderer waiting forever on an unsignaled fence.
- A record failure after image acquisition consumes the image-available binary
  semaphore. `recover_after_failed_record(...)` therefore blocks presentation,
  waits the device idle, rebuilds synchronization, and requires swapchain
  recreation before another frame.
- Fence reset now occurs after acquisition as well, so a reset failure uses the
  same post-acquire recovery instead of returning with a signaled binary
  semaphore still attached to the acquired image.
- The `ab -> ab -> abc` smoke exercises initial atlas upload, a no-dirty frame,
  and an incremental upload from SHADER_READ_ONLY_OPTIMAL back through
  TRANSFER_DST_OPTIMAL.
- Step 463 should force multiple atlas pages and verify descriptor capacity and
  cross-page upload planning/recording.

## 2026-07-10 Phase E Step 461 Dirty Glyph Atlas Uploads

- Phase E Step 461 uses private `VulkanGlyphAtlasUploadResources`; the public
  glyph-atlas plan remains free of Vulkan handles and staging allocations.
- `vulkan_glyph_atlas_staging.cpp` owns staging allocation, mapping, aligned
  payload repacking, and fence-safe retirement.
- Vulkan buffer-to-image offsets require 4-byte alignment. Dirty glyph payloads
  therefore cannot directly reuse arbitrary offsets from the tightly packed
  public alpha batch; staging repacks each glyph at a 4-byte-aligned buffer
  offset and builds copy regions against those offsets.
- Atlas images transition from UNDEFINED on first upload or
  SHADER_READ_ONLY_OPTIMAL on incremental upload to TRANSFER_DST_OPTIMAL, then
  back to SHADER_READ_ONLY_OPTIMAL after `vkCmdCopyBufferToImage`.
- A single in-flight fence allows prior staging buffers to be destroyed before
  new frame recording. Planner state and tracked image layout still must not
  advance until `vkQueueSubmit` succeeds, because recording alone does not
  change device state.
- Moving atlas preparation/commit to `vulkan_glyph_atlas_frame.cpp` keeps
  `vulkan_presentation.cpp` under its existing 150-line structure limit and
  leaves acquire/submit/present orchestration readable.
- Step 462 should cover multi-frame no-op reuse, incremental glyph additions,
  and acquired command-buffer lifetime.

## 2026-07-10 Phase E Resume And Step 460 Descriptor Binding

- The authoritative Phase E scope is Steps 459-538 in the complete replication
  roadmap, not only the first 20-step wording retained in older planning
  history. Step 459 is complete; 79 steps remain.
- Phase E Step 460 introduces private `VulkanGlyphAtlasResources` ownership
  while keeping the public production plan handle-free.
- Step 460 preserves the Step 459 ownership split: public atlas records stay
  free of Vulkan handles, while descriptor pools/layouts/sets, R8_UNORM images,
  device-local memory, image views, samplers, and renderer-owned lifetime state
  live in focused private Vulkan files.
- Descriptor allocation/update belongs in the descriptor module, not the image
  module. The first GREEN audit caught this boundary mismatch with exit 22;
  moving `vkAllocateDescriptorSets` and `vkUpdateDescriptorSets` to
  `vulkan_glyph_atlas_descriptors.cpp` made the module ownership explicit.
- Atlas resource reconciliation runs only after `vkWaitForFences` succeeds, so
  dropped or resized page resources are not destroyed while a prior submitted
  frame may still reference them.
- Planner state is copied before resource reconciliation and committed only
  after Vulkan resource creation succeeds. A failed allocation therefore does
  not advance `uploaded_count` and hide the same dirty glyphs on retry.
- Step 461 is the remaining production gap for dirty glyph pixels: staging
  buffers, image layout transitions, `vkCmdCopyBufferToImage`, and upload
  resource retirement are not part of Step 460.
- The current tracked `master` baseline is clean at `12753ba5` with only the
  pre-existing untracked `.vscode/`. Step 459 already passed Windows 140/140
  and WSL Arch Linux 137/137.
- The local WinGet `rg.exe` launcher is broken in this PowerShell environment;
  repository searches for this Phase should use `git ls-files`,
  `Get-ChildItem`, and `Select-String` unless the launcher is repaired.

## 2026-07-04 Complete GPUI Replication Planning

- Official GPUI sources checked for the roadmap were the Zed GPUI README,
  gpui.rs, the GPUI crate root, context docs, and key dispatch docs. The useful
  planning conclusion is that full replication must be pinned to a specific
  upstream revision because upstream GPUI is pre-1.0 and can break between
  versions.
- The official scope is broader than the current CGPUI Step 218 core: it
  covers state/entities, views/rendering, low-level elements, contexts,
  actions/key dispatch, platform services, async executor integration, test
  context support, and the public example set.
- The roadmap should not start macOS or X11 immediately. The durable order is
  upstream parity ledger first, Windows/Linux production parity second,
  macOS/Cocoa + Metal third, and optional X11 only if the user later wants
  strict upstream Linux backend matrix parity.
- A complete roadmap was written to
  `docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md`.
  It estimates roughly 460 more slices for strong Windows/Linux production
  parity, roughly 540 for Windows/Linux plus macOS, and roughly 580-620 for a
  strict all-upstream-platform interpretation, with the exact count corrected
  after Phase A generates the parity ledger.

## 2026-07-03 Aggressive Structural Optimization Plan

- The user clarified that the optimization target should be thorough even when
  the risk is higher than the earlier conservative structure pass. The next
  plan should therefore optimize for long-term file boundaries over short-term
  source compatibility.
- A fresh scan of `.worktrees/structural-optimization` shows implementation
  files are no longer the main problem: `src/platform/linux` has a 177-line
  maximum with four files over 120 lines, `src/renderer/vulkan` has a 151-line
  maximum with four files over 120 lines, `src/ui` has a 128-line maximum, and
  `src/platform/win32` has a 149-line maximum.
- The dominant remaining issue is public header size. `include/cgpui/ui` has
  29 headers, 6252 total lines, 13 headers over 200 lines, and 11 headers over
  300 lines. The biggest are `text_model.hpp` 688 lines,
  `runtime_types.hpp` 567, `text_layout.hpp` 481, `window_runtime.hpp` 464,
  and `element_tree.hpp` 461.
- The preferred aggressive sequence is private declaration surgery first,
  remaining long bridge implementation files second, public API header surgery
  third, and test-suite structure fourth. This avoids starting with public API
  churn while private implementation boundaries are still broad.
- The public header pass should move non-template inline bodies out of headers,
  split template implementation headers by domain, keep compatibility
  aggregate headers under 40 lines, and reduce public leaf headers toward a
  220-line ceiling. For especially large public classes, PIMPL/private state or
  focused facade domains are acceptable because compatibility is not the top
  constraint for this pass.
- A standalone design was written to
  `docs/superpowers/specs/2026-07-03-structural-optimization-design.md`, and
  an executable plan was written to
  `docs/superpowers/plans/2026-07-03-structural-optimization-execution-plan.md`.

## 2026-07-03 Structural Optimization Audit

- With the latest aggressive split, `win32_application.cpp` is no longer a
  Win32 window monolith. It is now the Win32 application/factory layer; window
  lifecycle/chrome/DPI, input/test drag hooks, IME placement, and OLE drag-drop
  ownership have separate files guarded by `win32_window_source_test`.
- `runtime_context.cpp` was a pure forwarding pile. It now only constructs
  `WindowRuntimeContext`; input/focus/tree forwarding, action/command
  forwarding, text/clipboard forwarding, scheduling/diagnostics forwarding,
  and native platform/theme forwarding are separate files guarded by
  `ui_source_structure_test`.
- This continuation also split `WaylandApplication` construction from window
  creation, event-loop forwarding, platform service facades, and the platform
  factory; split renderer report key comparison helpers out of command and
  submission planning; split Wayland pointer axis/scroll handling from
  enter/move/button handling; split `PaintList` text/image command emission
  from core paint stack commands; split `WindowRuntime` run/shutdown from the
  constructor; and split Win32 drag/drop payload extraction by source.
- The later aggressive pass then split Wayland text-input protocol bindings,
  Wayland DnD drag actions/events, runtime animation start/state/tick,
  Vulkan presentation recovery, Vulkan swapchain creation, Win32 window
  chrome/size handling, Wayland window input/drag/text event forwarding,
  AppContext/run_app responsibilities, and Win32 clipboard text/read/write
  responsibilities. The old aggregate files for those areas are now 1-line or
  very small placeholders guarded by architecture tests.
- After the latest scan, the implementation-oriented files still above about
  150 LF are `wayland_application_internal.hpp` (186),
  `vulkan_internal.hpp` (170), `vulkan_swapchain_create.cpp` (161),
  `win32_application.cpp` (159), `wayland_window.cpp` (156),
  `wayland_application_windows.cpp` (155), `wayland_window_internal.hpp` (154),
  `win32_window_proc.cpp` (154), and `vulkan_report_submission.cpp` (153).
  These are now mostly private declaration headers, single long lifecycle
  flows, or platform message bridges rather than old mixed-responsibility
  monoliths.
- The next implementation-file optimization pass should focus on private
  declaration surgery (`wayland_application_internal.hpp`,
  `vulkan_internal.hpp`, `wayland_window_internal.hpp`) and then decide whether
  single-flow files like `vulkan_swapchain_create.cpp` should be split by
  helper extraction or left intact for readability. The remaining large public
  headers are a separate public API surgery pass with higher include-order and
  source-compatibility blast radius.

## 2026-07-03 UI Runtime Structure Split

- `include/cgpui/ui/ui.hpp` is now a compatibility aggregate over
  `paint.hpp`, `view.hpp`, and `runtime.hpp` instead of owning the full public
  UI declaration surface.
- Paint declarations now live with paint command types and `PaintList`; view
  declarations carry `ViewContext` / `Context<T>` and the `View` base class;
  runtime declarations keep `WindowRuntime`, `WindowRuntimeContext`,
  `AppContext`, templates, and runner APIs.
- The former `src/ui/ui.cpp` monolith is split into focused implementation
  files. `ui.cpp` is now only the small RAII/handle implementation layer, while
  paint, render-view, app context, runtime core, events, scheduling,
  diagnostics, and context forwarding have separate compilation units.
- `src/ui/ui_internal.hpp` is deliberately private to the UI implementation and
  keeps shared helper code out of the public headers. This avoids inventing a
  public helper API just to share render/routing/accessibility internals.
- `ui_source_structure_test` now guards the split so future work does not
  silently grow `ui.hpp` or `ui.cpp` back into monoliths.
- The largest remaining implementation unit is `runtime_events.cpp`; a later
  structure-only cleanup can split event routing, input/text, clipboard, and
  action/command-palette logic without changing the public API.

## 2026-07-03 Threaded Async Executor And Cancellation

- Step 218 keeps the existing manual `spawn_task(...)` / `complete_task(...)`
  queue and adds `spawn_background_task(...)` for cancellable background work
  that completes back on the main runtime queue.
- `TaskCancellationToken` is a small cooperative token over shared atomic
  state. `TaskHandle` now reports cancellation and can request cancellation
  through `cancel()`.
- Background tasks are backed by `std::jthread`; the runtime protects task
  records and the completion queue with a task-scoped mutex. Completion
  callbacks are still copied out and executed without holding that mutex, so
  callbacks can safely call normal runtime/context APIs.
- `WindowRuntime::~WindowRuntime()` requests cancellation and joins workers
  before member teardown, avoiding background threads reaching back into a
  partially destroyed runtime.
- Runtime diagnostics now expose task, active, queued, completed, cancelled,
  and background task counters. This makes the executor visible in the same
  diagnostics snapshot used by existing frame/platform state.
- This is a small threaded executor, not a full GPUI async runtime: task pools,
  priorities, async I/O integration, structured task groups, and cross-thread
  entity access remain future work.

## 2026-07-03 Asset And Image Pipeline Skeleton

- Step 217 adds deterministic in-memory RGBA8 bitmap assets, image asset ids,
  descriptors, and soft validation metadata without adding external PNG, JPEG,
  or SVG decoders.
- Image paint commands now flow through `PaintList::draw_image(...)`,
  `render_view(...)`, and `RenderFrame::draw_image(...)` with clip stack,
  composition stack, bounds, sampling, tint, and opacity metadata preserved for
  renderer-facing records.
- Frame statistics now count image commands, and the Vulkan diagnostic path
  treats image commands as a supported primitive for reporting, batching, image
  render summaries, submission plans, and upload byte/region counters.
- `vulkan_plan_image_uploads(...)` produces deterministic upload batches and
  regions from image assets. It is still an upload plan, not real Vulkan image
  allocation, GPU upload, shader sampling, texture cache lifetime management,
  or SVG/image decoder integration.
- Step 218 moves to threaded async executor and cancellation so background work
  can leave the current main-thread/manual-completion skeleton.

## 2026-07-03 Animation Clock And Tween Primitives

- Step 216 adds deterministic animation primitives without introducing a full
  GPUI animation subsystem yet: `AnimationEasing`, clamped progress, easing,
  float/color/transform/style tween helpers, and `StyleTween`.
- Runtime animation state is owned by `WindowRuntime` and driven through the
  existing deterministic timer path. `AnimationHandle` exposes id, active,
  completion, snapshot, and cancellation queries; `WindowRuntimeContext`
  forwards start/snapshot/cancel APIs.
- Animation snapshots report elapsed duration, linear progress, eased progress,
  easing mode, and completion. Repeating timer catch-up can invoke the same
  animation timer more than once for one coarse `advance_time(...)`; the
  runtime deduplicates callbacks per current runtime timestamp so tests see
  one animation tick per time advance.
- Animation callbacks run with the normal root `WindowRuntimeContext`, so
  callback-driven `request_render()` uses the existing invalidation and redraw
  path. The current test harness observes one redraw/paint per animation tick.
- Style opacity tweening normalizes the final opacity to a small decimal grid
  to keep deterministic exact-float tests stable, while color interpolation
  keeps the raw float path so color channel fractions remain exact for existing
  RGB/RGBA expectations.
- This slice is deterministic clock/tween infrastructure. It does not include
  transitions attached to element lifecycle, spring animation, compositor
  frame pacing, cancellation propagation across async work, or a full upstream
  GPUI animation graph. Step 217 moves to asset and image pipeline skeletons.

## 2026-07-03 Runtime Theme Inheritance And Switching

- Step 215 adds runtime-owned theme hierarchy on top of the existing inert
  `Theme` token map: an app theme provides defaults and per-window themes
  override individual color/spacing tokens by `WindowRuntimeId`.
- `WindowRuntimeContext` now carries its `window_runtime_id`, so root and
  additional-window contexts can resolve theme tokens through the same
  window-then-app fallback path.
- `AppContext`, `WindowRuntime`, and `WindowRuntimeContext` all expose theme
  setters, clearers, and token lookups. Missing tokens continue to soft-fail
  with `std::nullopt`.
- Theme changes request full render/layout/paint invalidation and redraw.
  `clear_invalidation()` now also clears pending redraw bookkeeping, making it
  a complete observable reset point for tests and diagnostics between theme
  changes.
- This is runtime theme storage and lookup, not a full design-system cascade
  installation into every element style. Step 216 moves to deterministic
  animation clock and tween primitives.

## 2026-07-03 Additional Window Lifecycle Cleanup

- Step 214 makes app-opened child-window close deterministic instead of only
  flipping the child record inactive.
- `WindowRuntime::handle_native_additional_window_event(...)` now records the
  close lifecycle event before cleanup, so diagnostics still see the child
  window and renderer context that existed at close time.
- `cleanup_closed_additional_window(...)` removes the matching owned native
  child window from `native_additional_windows_`, removes and destroys an owned
  child root view, erases subscriptions for that root view, and clears the
  child record's window, renderer, active, and ownership flags while preserving
  the record as historical runtime metadata.
- The Step 213 child routing test needed its child-view counter snapshot moved
  before emitting close, because Step 214 intentionally destroys the owned
  child root during close. This avoided a WSL-only dangling-read failure while
  preserving the routing assertions.
- Renderer objects created by `run_app` still live in the app renderer owner
  vector until `run_app` exits; Step 214 releases the runtime record's renderer
  ownership pointer/flag rather than introducing a renderer-owner removal API.
  Full child render loops and production OS window-loop integration remain
  future multi-window depth.

## 2026-07-03 Additional Window Event Routing

- Step 213 adds record-specific child-window event routing for app-opened
  native windows keyed by `WindowRuntimeId`.
- `WindowRuntime` can now build a `WindowRuntimeContext` from a child
  `WindowRuntimeRecord`, route child focus/pointer/keyboard events to the
  child root view, and emit `EventDispatchRecord`s whose context and route use
  the child root `ViewId` instead of the root window view.
- Child redraw requests now render through the child renderer and child root
  view, and child resize events forward to the child renderer while updating
  the child descriptor's logical size. Child close events mark only the child
  record inactive and emit child-routed lifecycle diagnostics without quitting
  the app.
- This is routing, not full child-window lifetime ownership. Closed child
  roots, subscriptions, native windows, and renderers are still retained until
  the broader runtime cleanup path runs. Step 214 moves to deterministic
  child-window lifecycle teardown.

## 2026-07-03 Additional Window Renderer Ownership

- Step 212 moves app-opened child windows from native-window-only records to
  records with independent child renderers created from each child
  `PlatformWindow` native surface, framebuffer size, and scale.
- `run_app` now retains all renderers produced by the app renderer factory in a
  vector, so root and app-opened child renderers share the same ownership
  boundary without changing the public `RendererFactory` pointer-returning
  contract.
- Child renderer creation remains graceful: if the factory fails or returns an
  empty renderer, the child record stores `native_window_error`, clears the
  transient window/renderer pointers, and avoids adding the native child window
  to the active native window list.
- This still does not implement child-window event routing, render loops, or
  lifecycle teardown. Step 213 moves to routing pointer, keyboard, focus,
  redraw, resize, and close events by `WindowRuntimeId`.

## 2026-07-03 Accessibility Value And Live Update Events

- Step 211 adds `PlatformAccessibilityLiveUpdateKind` and
  `PlatformAccessibilityLiveUpdate` as platform-neutral event records for
  value, text, and focus changes at the platform accessibility boundary.
- `WindowRuntime` now keeps the previous platform accessibility update and
  emits a `live_updates` batch by comparing same-element node values, text, and
  focused state across redraw-driven accessibility snapshots.
- Win32 UIA and Wayland AT-SPI adapters retain the latest live update batch
  alongside their provider/object facade node records, so later production
  event bridges can raise UIA/AT-SPI notifications from the same source of
  truth.
- This remains deterministic event metadata, not production accessibility
  event emission. No COM UIA provider events or D-Bus AT-SPI signals are
  emitted yet. Step 212 moves to additional-window renderer ownership.

## 2026-07-03 AT-SPI Object Model Facade

- Step 210 mirrors the Step 209 Win32 UIA provider-node facade on
  Linux/Wayland. `WaylandAtspiAccessibilityAdapter` now builds
  `WaylandAtspiObjectNode` records from the latest
  `PlatformAccessibilityTreeUpdate` instead of only retaining summary counts.
- AT-SPI object paths are deterministic and element-id based:
  `/org/a11y/atspi/accessible/<element-id>`. Child records also retain optional
  parent object paths, so later D-Bus provider work can navigate the object
  model without inventing a separate public accessibility identity.
- The facade preserves role, name, text, value, enabled/focusable/focused
  state, bounds, and child counts while keeping the existing root/node/focus
  summary counters intact.
- This remains an internal object model, not production AT-SPI. It does not
  register on the accessibility bus, expose D-Bus object paths, implement
  AT-SPI interfaces, or emit accessibility events. Step 211 moves to
  accessibility value and live update event records shared by the platform
  adapters.

## 2026-07-03 UIA Provider Tree Facade

- Step 209 adds `PlatformAccessibilityNodeUpdate::value` so text-input
  accessibility nodes expose their editable value separately from name/text at
  the platform boundary.
- `platform_accessibility_update_from(...)` now maps text-input snapshot text
  into the platform `value` field while leaving non-text-input values empty.
- The Win32 UIA adapter now owns internal `Win32UiaProviderNode` records built
  from the latest `PlatformAccessibilityTreeUpdate`. The facade retains stable
  element ids, parent ids, role, name, text, value, enabled/focusable/focused
  state, bounds, and child counts while preserving the existing root/node/focus
  counters.
- This remains a facade, not production UI Automation. It does not expose
  `IRawElementProvider*`, raise UIA events, implement navigation methods, or
  register COM provider objects. Step 210 mirrors the object-model facade on
  Linux/Wayland for AT-SPI.

## 2026-07-03 Platform Diagnostics Event Stream

- Step 208 adds `PlatformDiagnosticKind` and `PlatformDiagnosticEvent` as
  platform-neutral event metadata for platform-facing runtime hooks.
- `EventKind` moved from `ui.hpp` to `core/events.hpp`, so platform diagnostic
  metadata can live at the platform boundary without depending on UI-only
  declarations.
- `WindowRuntime` now keeps a bounded 32-event platform diagnostics stream,
  exposes it through `platform_diagnostics()`, and copies it into
  `RuntimeDiagnosticsSnapshot`; `WindowRuntimeContext` forwards the same
  read-only stream.
- The runtime records deterministic diagnostics for clipboard copy/cut/paste
  outcomes, drag/drop events and payload counts, IME placement updates,
  accessibility tree updates, window lifecycle dispatch, native-menu
  installation, and native file-dialog requests.
- This is an observability layer, not full native telemetry. Backend names are
  currently runtime/platform-result summaries, and deeper Win32 UIA, Wayland
  AT-SPI, menu, dialog, chrome, and renderer-backed diagnostics remain future
  production-depth work. Step 209 moves to the Win32 UIA provider facade.

## 2026-07-03 App Command Palette Registry

- Step 207 adds `CommandPaletteEntry` as platform-neutral command metadata:
  action name, title, group, intended action scope, enabled state, and optional
  view/element ids.
- `WindowRuntime`, `WindowRuntimeContext`, and `AppContext` can register
  command palette entries, enumerate the stable registry, filter by group, and
  dispatch a registered command through the existing scoped action registry.
- Disabled entries remain searchable but dispatch as unhandled without calling
  their action handler, and missing palette action names update the last action
  dispatch with an unhandled result.
- This is a registry and dispatch layer, not a full command palette UI. Fuzzy
  search, keyboard palette presentation, menu/palette unification,
  accelerator display, platform diagnostics, and native command routing remain
  future work. Step 208 moves to a bounded platform diagnostics event stream.

## 2026-07-03 Window Chrome Customization Skeleton

- Step 206 adds platform-neutral window chrome metadata:
  `WindowChromeOptions` now lives on `WindowDescriptor` with titlebar
  visibility, decoration, resizable, and transparent-background flags.
- `WindowOptions` exposes fluent `.titlebar_visible(...)`, `.decorations(...)`,
  `.resizable(...)`, and `.transparent(...)` helpers, and app-opened child
  windows preserve those descriptors through the runtime registry and native
  `PlatformApplication::create_window(...)` path.
- `PlatformWindowChromeState` records requested/applied chrome state, backend
  name, support state, and unsupported reason. The base `PlatformWindow`
  returns an explicit unsupported result so fallback backends stay graceful.
- Win32 computes window `style` / `extended_style` from the chrome request,
  uses those styles during `CreateWindowExW`, and can reapply them through
  `apply_window_chrome(...)`. Wayland records the request and applied default
  chrome while reporting unsupported xdg-decoration behavior until decoration
  negotiation is implemented.
- This is still a skeleton. Full frameless hit testing, resize grips,
  drag-to-move regions, Wayland xdg-decoration/client-side decoration policy,
  transparent swapchain/compositor behavior, and native runtime diagnostics
  remain future platform-depth work. Step 207 moves to command palette metadata
  over the existing action registry.

## 2026-07-03 Native File Dialog API Skeleton

- Step 205 adds platform-neutral native file dialog descriptors:
  `NativeFileDialogKind`, `NativeFileDialogFilter`,
  `NativeFileDialogOptions`, and `NativeFileDialogResult`.
- The public surface can express open-file, open-files, and save-file requests
  with title, default directory, suggested name, and extension filters. Results
  carry support/acceptance state, backend name, requested kind, selected paths,
  optional error text, and filter count diagnostics.
- `AppContext`, `WindowRuntimeContext`, and `WindowRuntime` now forward
  `show_native_file_dialog(...)` through `PlatformApplication` and retain the
  last dialog result for frame/runtime inspection.
- Win32 and Wayland own inert native file-dialog state objects that report
  `backend = "win32"` or `backend = "wayland"` plus requested kind and filter
  count while returning `supported = false`. Real `IFileOpenDialog`/
  `IFileSaveDialog`, xdg-desktop-portal, non-blocking dialog lifetimes, and
  sandbox/desktop integration remain future platform-depth work.

## 2026-07-03 Native Menu And Accelerator API Skeleton

- Step 204 adds the first platform-neutral native menu surface:
  `NativeMenuModel`, `NativeMenuItem`, `NativeMenuItemKind`,
  `NativeMenuAccelerator`, `PlatformMenuInstallationResult`, and
  `NativeMenuInstallation`.
- `AppContext`, `WindowRuntimeContext`, and `WindowRuntime` can now install a
  native menu model and retain the last installation record. The retained model
  keeps submenu hierarchy, command action names, enabled/checked state, and
  accelerator metadata visible to later command-palette and native-menu depth
  work.
- Win32 and Wayland now both own inert native menu state objects that report
  backend names, root menu count, recursive item count, and accelerator count
  while returning `supported = false`. This preserves graceful behavior until
  real Win32 `HMENU`/accelerator tables and Wayland/desktop-shell menu
  integration are designed.
- Recursive `native_menu_item_count(...)` and
  `native_menu_accelerator_count(...)` live at the platform-neutral boundary,
  so Step 205 can add file dialog request/result APIs without reopening menu
  model counting or runtime forwarding.

## 2026-07-03 Win32 OLE Drop Target Skeleton

- Step 203 adds an internal `Win32OleDropTarget` implementing `IDropTarget`,
  with `QueryInterface`/`AddRef`/`Release` and `DragEnter`/`DragOver`/
  `DragLeave`/`Drop` methods that route into the existing platform drag events.
- `Win32Application` now initializes OLE, `Win32Window` registers and revokes
  the drop target with per-window `Win32OleDropTargetRegistrationState`
  diagnostics, and the Win32 platform target links `ole32` alongside the
  existing Win32 system libraries.
- `drag_payload_from_ole_data_object(...)` creates the conversion boundary for
  `CF_UNICODETEXT` and `CF_HDROP`, while `drag_action_from_drop_effect(...)`
  maps `DROPEFFECT_COPY` and `DROPEFFECT_MOVE` into the public
  `DragDropAction` metadata added in Step 200.
- The deterministic Win32 drag/drop test hook now carries a drop effect, so
  text enter/update and file drop coverage verifies public copy/move action
  metadata without relying on a real shell drag gesture.
- This remains a skeleton, not full production OLE drag/drop. Richer effect
  negotiation, non-text/non-file formats, async shell edge cases, and deeper
  user-facing drag policy remain future Windows platform-depth work. Step 204
  moves to native menu and accelerator API scaffolding.

## 2026-07-03 Wayland XDG Configure Lifecycle State

- Step 202 adds deterministic Wayland XDG configure lifecycle records:
  `WaylandXdgConfigureState` tracks pending size, pending serial, last acked
  serial, and current/pending toplevel state; `WaylandXdgToplevelState` tracks
  activated, maximized, and fullscreen flags parsed from the compositor state
  array.
- `handle_surface_configure(...)` now records the acked serial before updating
  configured state, and lifecycle dispatch uses existing public events:
  activation changes emit `WindowActivated`, while leaving maximized/fullscreen
  state emits `WindowRestored`. Resize delivery still waits for the matching
  surface configure path.
- The Wayland test compositor can now send stateful resize configures and
  report the last configure state, including the serial observed by
  `ack_configure`. The resize test asserts size, activation, max/fullscreen
  flags, and exact ack serial instead of only checking a boolean ack.
- This is still a deterministic lifecycle slice, not full shell-policy
  integration. It does not yet add native maximize/fullscreen APIs, window
  state queries in the public `WindowState`, tiled states, compositor serial
  policy, or production configure-loop edge handling. Step 203 pivots to the
  Win32 OLE drop target skeleton.

## 2026-07-03 Wayland Cursor Theme Image State

- Step 201 adds deterministic internal Wayland cursor theme/image state records
  before real cursor-theme loading exists. `WaylandCursorThemeState` records the
  current theme status, requested shape, mapped cursor name, serial, apply
  count, hotspot, and graceful unavailable reason.
- The cursor-shape mapping now has explicit Wayland names for default arrow,
  pointing hand, text, crosshair, horizontal/vertical resize, and not-allowed
  cursors. The runtime still calls the existing null `wl_pointer_set_cursor`
  path, so this slice improves observability and state ownership without
  claiming loaded cursor images.
- The Wayland pointer test now requests a pointing-hand cursor during drag
  enter and expects an additional compositor-visible cursor set request,
  proving cursor application records are updated outside the initial pointer
  enter path.
- Real `libwayland-cursor`/theme loading, cursor surfaces, shared-memory cursor
  buffers, animated cursors, scale-aware images, and compositor edge cases
  remain future Wayland platform-depth work. Step 202 moves to XDG configure
  lifecycle state.

## 2026-07-03 Wayland Drag Action Negotiation

- Step 200 adds public `DragDropAction::{none, copy, move}` metadata to
  `DragEntered`, `DragUpdated`, `DragDropped`, and `DragExited`, keeping the
  default action as `none` for existing call sites.
- `WaylandDataDevice` now records `wl_data_offer.source_actions` and
  `wl_data_offer.action`, accepts the preferred supported MIME type, advertises
  destination copy/move support through `wl_data_offer_set_actions`, and calls
  `wl_data_offer_finish` after drop payload extraction.
- The Wayland test compositor now sends source/selected DnD action events and
  records client-side `accept`, `set_actions`, and `finish` requests. The
  platform test covers text move and URI-list copy flows, while the runtime
  test verifies action metadata survives normal event routing.
- The slice intentionally maps only Wayland copy/move/none into the public API.
  Wayland ask actions, richer drag-effect policy, non-local URI handling, and
  production shell edge cases remain future platform-depth work.

## 2026-07-03 Wayland Clipboard Ownership And Send Offers

- Step 199 promotes standalone `WaylandClipboard::Connection::write_text(...)`
  from memory fallback to a real Wayland selection owner when connected to an
  available data device. It creates a `wl_data_source`, offers
  `text/plain;charset=utf-8` and `text/plain`, and installs it with
  `wl_data_device_set_selection`.
- The clipboard connection keeps a small dispatch loop alive while it owns a
  source so compositor `wl_data_source.send` events can be received after
  `write_text(...)` returns. The send callback copies the current owned UTF-8
  payload under a mutex and writes it to the compositor-provided fd.
- `MemoryClipboard` remains the fallback/read-back path, so unsupported,
  no-seat, or failed protocol write cases keep the existing local clipboard
  contract while connected Wayland displays now have a protocol-backed write
  path.
- The Wayland test compositor now models both directions of clipboard data:
  compositor-owned `wl_data_offer` payloads for reads, and client-owned
  `wl_data_source` selection with offered MIME types and deterministic payload
  requests for writes.
- This slice still does not implement non-text formats, serial policy beyond
  the deterministic test path, clipboard manager persistence after process
  exit, or richer desktop edge cases. Step 200 moves to Wayland drag action
  negotiation rather than expanding clipboard formats.

## 2026-07-03 Text Soft Wrap Layout Records

- Step 198 adds `TextWrapLine`, `TextWrapLayout`, and
  `wrap_text_measurement(...)` in `include/cgpui/ui/text.hpp`. The current
  algorithm is deterministic greedy glyph-level wrapping over already-shaped
  fallback glyph advances; it does not split glyphs or introduce word/bidi
  paragraph layout.
- `PaintList::fill_text(...)` now derives wrap records from the measured text
  and authored paint width, expands text paint height when wrapped content
  needs more lines, and emits wrap-aware glyph origins. Existing single-line
  text remains one line with unchanged glyph positions when it fits the paint
  width.
- `TextPaint` and `TextDraw` carry the same line records, giving renderer
  tests and future Vulkan text work a stable view of wrapped byte ranges,
  glyph ranges, relative line origins, and line sizes.
- `TextElement` and `LabelElement` layout now size text through
  `wrap_text_measurement(...)` using the current max-width constraint. This is
  still a fallback-metric soft-wrap skeleton, not rich paragraph layout,
  platform shaping, bidi visual lines, selection geometry over wraps, or
  word-aware wrapping.

## 2026-07-03 Text Pointer Selection Geometry

- Step 197 adds deterministic single-line pointer hit geometry in
  `include/cgpui/ui/text.hpp`: `TextHitTestResult`,
  `hit_test_text_position(...)`, and `text_selection_range_from_points(...)`
  map measured glyph advances to byte offsets and normalized selection ranges.
- `WindowRuntime` now keeps a small text pointer drag state for text-input
  elements. Left pointer down focuses the input through the existing focus path
  and collapses the model selection to the hit offset; pointer move/up continue
  selecting against the original text-input element even when the pointer moves
  outside its bounds.
- The slice intentionally remains single-line and fallback-metric based. It
  does not add soft wrapping, bidi visual order, grapheme column accounting,
  paragraph layout, platform shaping, or selection handles. Step 198 now builds
  wrap records on top of the reusable measurement primitives while wrapped
  selection geometry remains future work.

## 2026-07-02 Linux AT-SPI Accessibility Adapter Skeleton

- Step 177 mirrors the Step 176 Win32 adapter boundary on Linux/Wayland. The
  new `WaylandAtspiAccessibilityAdapter` consumes
  `PlatformAccessibilityTreeUpdate` and tracks root element id, total nodes,
  focused nodes, and text-input nodes.
- The adapter is stored inside `WaylandWindow`, and
  `RegisteredWaylandWindow::update_accessibility_tree(...)` forwards platform
  updates into the underlying Wayland window. This matters because the runtime
  owns the registered wrapper, not the raw `WaylandWindow`.
- The slice intentionally avoids D-Bus, AT-SPI object paths, role mapping to
  AT-SPI enums, event emission, or desktop accessibility bus registration.
  Windows and Linux now have matching adapter skeletons, while production
  accessibility bridges remain future work.
- Step 178 can move to native additional-window creation without revisiting the
  public accessibility snapshot surface.

## 2026-07-02 Windows UIA Accessibility Adapter Skeleton

- Step 176 expands the platform accessibility update from a summary-only root
  id/node count into node records derived from `AccessibilityTreeSnapshot`.
  Each platform node carries element id, optional parent id, role, name, text,
  enabled/focusable/focused state, optional layout bounds, and child count.
- `WindowRuntime::handle_redraw()` now forwards the current accessibility
  snapshot to `PlatformWindow::update_accessibility_tree(...)` after layout,
  keeping platform adapters fed by the same tree used by public
  `accessibility_snapshot()` queries.
- The Win32 backend now has a `Win32UiaAccessibilityAdapter` skeleton that
  consumes the platform update and tracks root, total-node, focused-node, and
  text-input-node counts. This intentionally does not create COM objects,
  expose `IRawElementProvider*`, or claim production UI Automation parity yet.
- Step 177 should mirror this boundary on Linux with an AT-SPI adapter
  skeleton consuming the same platform update rather than adding
  Linux-specific public accessibility semantics.

## 2026-07-02 Glyph Bitmap And Fallback Rasterizer

- Step 169 adds CPU-side fallback raster data in `include/cgpui/ui/text.hpp`,
  not platform font rasterization or Vulkan texture upload. The new data model
  is `GlyphBitmap`, `GlyphRasterizerOptions`, and `RasterizedGlyph`.
- `rasterize_fallback_glyph(...)` consumes the existing `TextGlyphPaint`
  metadata from Step 151/152. It preserves the `GlyphAtlasKey`, uses
  `device_advance` for bitmap width, uses `key.device_font_size` for bitmap
  height, records the same advance/device font size, and derives a stable
  fallback baseline at 80% of device font size.
- The fallback bitmap is an alpha-only buffer with deterministic padding and
  foreground/background alpha options. This is enough for Step 170 to pack and
  upload bytes into atlas records without depending on DirectWrite,
  fontconfig, HarfBuzz, or real Vulkan images yet.
- Header cleanliness coverage now proves the glyph raster data model is usable
  through both UI text headers and renderer-facing includes. The renderer still
  stores placeholder atlas bounds until Step 170 teaches the glyph cache to
  allocate atlas slots from `RasterizedGlyph` bitmap data.

## 2026-07-02 Steps 169-178 Depth Pass Planning

- The next ten-step track should continue Windows/Linux depth work instead of
  pivoting to macOS. The current parity audit identifies Vulkan text rendering,
  Wayland payload/protocol handling, native accessibility adapters, and native
  multi-window creation as higher-leverage blockers for the existing target
  pair.
- Step 169 should start with CPU-side glyph bitmap and deterministic fallback
  rasterization in `include/cgpui/ui/text.hpp`. Existing Step 151/152 work
  already provides glyph keys, glyph paint metadata, and renderer cache
  lookup/store, but atlas entries still use placeholder bounds derived from
  glyph positions instead of actual raster data.
- Step 170 can then allocate atlas slots and upload records from rasterized
  glyphs without needing real Vulkan image upload yet. This keeps renderer
  state testable before GPU texture objects become necessary.
- Steps 173-175 should deepen Wayland using the existing test compositor. The
  current data-device and text-input code already routes skeleton events, so
  the next useful work is MIME offer tracking, text/URI payload extraction,
  and protocol-independent text-input state records.
- Steps 176-177 should consume the platform-neutral accessibility snapshot for
  UIA/AT-SPI adapter skeletons. They should not add separate public
  accessibility semantics until the adapter boundary proves a missing concept.

## 2026-07-02 GPUI-Core API Parity Audit

- Step 168 adds `docs/gpui-core-api-parity.md` as the Windows/Linux truth
  source for current GPUI-core-like parity. It deliberately separates
  Implemented, Partial, Missing, and Mac/Metal Deferred areas instead of
  claiming full upstream GPUI parity.
- Implemented areas now include the public prelude, `Context<T>`, `Entity<T>`,
  `WindowOptions`, entity/global/action/subscription/defer/timer/async
  runtime APIs, keyed element behavior, lifecycle/state storage, reusable
  widgets (`button`, `label`, `text_input`, `scrollable_list`), style/layout
  primitives, text/glyph metadata, renderer diagnostics, Win32/Wayland platform
  hooks, and Windows/Linux demo smoke coverage.
- Partial areas are explicit: Vulkan text still needs real glyph
  raster/upload/draw, accessibility is snapshot-level without UIA/AT-SPI,
  Wayland clipboard/drag/drop/IME are skeletons, Win32 drag/drop is not full
  OLE shell integration, multi-window is registry-level, frame timings are not
  a real profiler, and platform font discovery is still skeletal.
- Missing areas are now named for the next depth pass: full animation,
  theming, asset/image pipelines, rich text editing, native menus/dialogs,
  production accessibility bridges, full Vulkan drawing for remaining
  primitives, virtualization, and threaded async/cancellation.
- Mac remains a separate parity handoff. The audit keeps macOS/Cocoa + Metal
  outside the Windows/Linux completion claim and requires a Mac host plus Cocoa
  platform adapters and Metal renderer parity before any Mac parity claim.

## 2026-07-02 Accessibility Tree Skeleton

- Step 166 adds a platform-neutral accessibility snapshot, not Windows UIA or
  Linux AT-SPI adapters. `AccessibilityTreeSnapshot` is built from
  `ElementTree` preorder traversal and carries root id, per-node parent/child
  ids, role, name, text, enabled/focusable/focused flags, and optional layout
  bounds.
- `Element` now exposes default accessibility role/name/text hooks. Labels and
  text elements report their text as name/text, text inputs report the text
  role as `text_input`, and buttons report role `button` with accessible name
  derived from their child label/text before falling back to action name.
- `WindowRuntime::accessibility_snapshot()` and
  `WindowRuntimeContext::accessibility_snapshot()` reuse the installed
  runtime-owned element tree and mark the current keyboard-focus element owner.
  No installed tree returns an empty snapshot.
- The platform hook remains intentionally low-coupling:
  `PlatformAccessibilityTreeUpdate` carries only root element id and node
  count, while `PlatformWindow::update_accessibility_tree(...)` defaults to
  no-op. Real UIA/AT-SPI tree serialization remains future platform work.
- Step 167 can now focus on demo smoke coverage without needing accessibility
  OS adapter work; Step 168 should count accessibility as snapshot-level
  partial parity, not full native accessibility parity.

## 2026-07-02 Multi-Window Runtime Registry

- Step 159 introduces a platform-neutral ownership registry, not a full native
  multi-window event loop. `WindowRuntimeRecord` tracks runtime id, descriptor,
  root view id, live platform window/renderer pointers when active, and
  explicit `owns_window`, `owns_renderer`, and `owns_root_view` flags.
- The root window keeps the existing single-window run path. Its record is
  active only while `WindowRuntime::run(...)` owns a live platform window and
  borrowed renderer pointer, then clears the live pointers when the run returns.
  This preserves current callback/context behavior.
- App-opened windows now receive stable `WindowRuntimeId` values in
  `AppOpenedWindow::runtime_id`. Owned root views are still stored in the
  existing view registry, while the new runtime record declares future window
  and renderer ownership separately from actual platform creation.
- The new query surface is read-only:
  `root_window_runtime_id()`, `window_runtime_records()`, and
  `window_runtime_record(...)`. This gives Step 160 lifecycle events a concrete
  per-window record to update without forcing Step 159 to rewrite Win32 or
  Wayland event-loop behavior.

## 2026-07-02 Renderer Unsupported-Command Diagnostics

- Step 158 adds a renderer-facing diagnostic report rather than changing UI
  paint routing or pretending unsupported primitives are rendered. Supported
  Vulkan primitives remain `solid_rect` and `text`; rounded rects,
  text-selection metadata, and text-caret metadata are now nameable
  `RendererPrimitiveKind` values that can be reported explicitly when a
  renderer does not handle them yet.
- `RendererCommandReport` keeps supported command batching and unsupported
  diagnostics together. This gives later frame diagnostics and renderer-depth
  work one surface to inspect without breaking the existing
  `vulkan_build_renderer_command_batches(...)` helper used by prior tests.
- Unsupported command diagnostics carry the primitive kind, original command
  index, `unsupported_primitive` reason, and a human-readable message including
  the primitive name. The report preserves supported command batches even when
  unsupported commands are interleaved in the same stream.
- The post-merge Windows full debug run initially saw a transient
  `win32_text_input_test/default` failure. A targeted rerun passed 1/1 and the
  full Windows debug rerun passed 29/29, matching the feature-worktree Windows
  full debug result. WSL Arch Linux full debug passed 26/26.
- Step 159 should now move into Band H with multi-window runtime ownership.
  The renderer diagnostic surface is intentionally backend-facing and should
  not force platform window registry design to depend on Vulkan internals.

## 2026-07-01 Paint Command Snapshots

- Step 157 keeps paint snapshot serialization test-only in
  `tests/ui/paint_snapshot.hpp`; it does not add public API surface or change
  renderer/runtime ownership.
- The snapshot format captures command index, primitive kind, logical rects,
  colors, text content/font/device font size/glyph count, clip metadata,
  opacity, and affine transform. This gives later renderer changes a stable
  evidence trail for command order and metadata propagation.
- Widget coverage currently fixes a combined button/label/text-input paint
  stream, including rounded background, border order, text selection, text
  command, and caret command. `render_view_test` separately snapshots the
  renderer-submitted text command after `render_view(...)` translates
  `PaintList` output into `TextDraw`.
- The hello demo smoke marker is source-level and inert by default:
  `CGPUI_DEMO_PAINT_SNAPSHOT_SMOKE` requests a redraw when present, giving
  Step 167 a stable hook for future Windows/Linux demo smoke expansion without
  changing normal demo behavior.

## 2026-07-01 HiDPI Scale Propagation

- Step 156 makes the runtime store both framebuffer size and logical viewport
  size. Platform resize events still deliver framebuffer dimensions plus
  `DpiScale`; `WindowRuntime` derives `viewport_size = framebuffer / scale`
  for view context, layout, and paint.
- Authored sizes remain logical pixels. `LayoutInput::scale` and
  `to_logical_pixels(...)` / `to_device_pixels(...)` expose the conversion
  boundary without requiring existing element builders or style APIs to switch
  units.
- Text shaping now preserves logical `font_size`, `total_advance`, glyph
  origins, and glyph advances while also exposing device font size, device
  total advance, device origins, and device advances. Vulkan glyph cache
  entries consume the device-space glyph metadata so renderer resources can be
  scale-aware without changing text authoring APIs.
- Step 157 can use the new logical/device paint metadata to serialize stable
  paint command snapshots for widgets and the demo.

## 2026-07-01 Frame Statistics Diagnostics

- Step 155 keeps frame timing deterministic for tests: `FrameStatistics`
  exposes timing fields, but the current runtime-populated timings remain zero
  until a later real profiler/timer integration is added.
- The first useful statistics layer is structural rather than temporal:
  layout, paint, and render pass counts; emitted, submitted, and skipped
  command counts; primitive counts; and begin/clear/present counters are
  derived from the actual render record and command stream.
- `render_view(...)` accepts an optional statistics output pointer, preserving
  old call sites while letting `WindowRuntime::handle_redraw()` store the same
  frame record in `last_render_record_` and diagnostics. This keeps
  diagnostics observable without turning every render helper into a profiling
  API.
- Step 156 should thread HiDPI scale through layout, text metrics, and
  renderer resize metadata while keeping authored dimensions in logical pixels.

## 2026-07-01 Renderer Command Batching

- Step 154 keeps batching diagnostic-only. `RendererCommandBatchKey` is the
  shared public key shape over `RendererPrimitiveKind`, optional clip rect, and
  `PaintMetadata`; it does not describe a GPU pipeline state object yet.
- `vulkan_build_renderer_command_batches(...)` groups commands in stable
  submitted order, first solid rectangles and then text draws, and only merges
  adjacent commands with identical primitive kind, clip rect, opacity, and
  transform metadata. It deliberately does not reorder commands to chase larger
  batches because paint order is still authoritative.
- `VulkanRendererState::present_frame(...)` records the latest command batches
  after consuming text glyph cache metadata and before command-buffer recording.
  The current Vulkan path still only clears solid rectangles; text and batching
  remain diagnostic surfaces for later frame statistics and renderer-depth
  steps.
- Step 155 should extend diagnostics with frame timing and layout/paint/render
  counters using these command-batch records as renderer-facing evidence, while
  keeping real timing optional/deterministic for tests.

## 2026-07-01 Font Database Skeleton

- Step 149 introduces the font API as platform-neutral descriptors and an
  in-memory `FontDatabase`, not as real system font enumeration yet.
  `FontFaceDescriptor` currently carries `FontDescriptor`, PostScript name,
  source, and optional path.
- `FontDatabase::add_face(...)` ignores empty families and suppresses exact
  duplicate face descriptors. `resolve(...)` matches by family, while an empty
  request family soft-falls back to the first available face.
- `discover_test_fonts(...)` is the deterministic fake discovery route for
  tests and later shaping work. It preserves fixture order through the same
  database insertion rules used by platform discoveries.
- `PlatformApplication::discover_fonts()` is declared with only a forward
  declaration of `FontDatabase` in `platform.hpp`; the default empty
  implementation lives out-of-line in `src/platform/empty.cpp`. This avoids
  making every platform header consumer include UI text definitions.
- Win32 and Wayland now override `discover_fonts()` but intentionally return
  empty databases. Real DirectWrite/fontconfig-style discovery remains a later
  depth step; Step 150 can consume these descriptors for deterministic fallback
  shaping without depending on real platform font enumeration.

## 2026-07-01 Scrollable List Container

- Step 148 keeps the scrollable list as a public reusable widget built on
  existing primitives: `ScrollState`, `VerticalStackElement`, keyed elements,
  layout bounds, and paint clip metadata. It does not introduce a separate
  runtime scroll owner or virtualization system yet.
- `scrollable_list(state)` installs each `.item(key, child)` by setting the
  child element key and storing it in an internal vertical stack. This makes
  item identity visible to keyed reconciliation without inventing a second
  list-specific key map.
- Layout computes content size through the internal stack, writes viewport and
  content dimensions back to `ScrollState`, and applies the current scroll
  offset to child layout bounds so tests and paint commands observe scrolled
  geometry.
- Paint pushes the list viewport as a clip around direct item painting. The
  internal `VerticalStackElement` remains a layout container rather than a
  paint dispatcher, so the list paints its items explicitly.
- Step 149 should switch focus to text/font infrastructure: platform-neutral
  font descriptors and deterministic Win32/Linux discovery skeletons, while
  keeping renderer text work command-driven until later glyph cache slices.

## 2026-07-01 Text Input Widget Primitive

- Step 147 adds `TextInputElement` as a focusable widget backed by an existing
  `TextModel`; it reuses `TextElement` layout and caret/selection/text paint
  metadata instead of adding a second editable text rendering path.
- `text_input(model)` returns a fluent `TextInputBuilder` with text style,
  key, enabled, and disabled support. The widget is intentionally model-backed
  by reference, so ownership remains with the caller/runtime authoring code.
- `WindowRuntime::focused_text_model()` now preserves explicit
  `bind_text_model(...)` behavior first, then falls back to the focused
  installed `TextInputElement` model. This lets text input widgets receive text
  input, text edit bindings, clipboard copy/cut/paste, and IME geometry without
  requiring authors to manually bind the model to the element id.
- `TextInputElement::handle_event(...)` consumes text input, IME composition,
  and left pointer press events after runtime routing has applied the shared
  edit behavior. It does not introduce a private widget-specific editor loop.
- Step 148 should build the scrollable list container on top of the existing
  `ScrollState`, `ScrollElement`, keyed reconciliation, and viewport clip
  metadata rather than expanding text-input behavior further.

## 2026-07-01 Label Widget Primitive

- Step 146 keeps `LabelElement` as a read-only text widget over owned
  `std::string` content, not as a bound `TextModel` or editable text control.
- `label(...)` emits a single `PaintCommandKind::text` command when it has
  non-empty text and layout bounds. It deliberately omits
  `text_selection` and `text_caret` commands so label paint output cannot be
  confused with editable text input metadata.
- Label text styling uses the same `Style` fields as other text paths:
  foreground color, `FontDescriptor`, and font size. Missing foreground falls
  back to the current default label text color rather than adding theme or
  cascade lookup in this slice.
- The builder supports `.key(...)`, `.enabled(...)`, and `.disabled()` so label
  widgets fit keyed reconciliation and disabled subtree conventions, even
  though labels do not handle input directly.
- Step 147 should introduce an editable text input widget by composing the
  existing focus, text model, selection, clipboard, key-edit, and IME geometry
  surfaces. It should not retrofit editability into `LabelElement`.

## 2026-07-01 FocusHandle Primitive

- Step 144 keeps `FocusHandle` deliberately thin: it stores only an
  `ElementId` and forwards request/release to the existing
  `WindowRuntime::request_keyboard_focus(ElementId)` and
  `release_keyboard_focus(ElementId)` owner semantics.
- `FocusHandle::contains(...)` and `focused(...)` query the current
  `ViewInputState::keyboard_focus_element_owner`; for this primitive slice the
  two spellings are equivalent. The duplicate naming matches the GPUI-like API
  shape while leaving room for richer containment/focus scopes later.
- `WindowRuntime::input_state()` is now a public snapshot helper so handles and
  later widgets can query focus without requiring an event-time
  `WindowRuntimeContext`.
- Step 144 intentionally does not add focus rings, focus scopes, tab-order
  changes, platform focus adapters, or accessibility focus integration. Those
  remain later widget/platform/accessibility slices.

## 2026-07-01 Async Task Completion Skeleton Merged

- Step 136 is merged on `master` at
  `e957c6e feat: add async task completion skeleton` and post-merge verified
  on Windows and WSL Arch Linux.
- Step 137 should add runtime update batching on top of the existing
  model/global update and redraw scheduling paths without changing the Step
  136 deterministic completion queue or adding platform wakeups early.

## 2026-07-01 Async Task Completion Skeleton

- Step 136 keeps async work deterministic and runtime-local: `TaskId`,
  `TaskHandle`, `TaskCompletionCallback`, `spawn_task(...)`,
  `complete_task(...)`, and `drain_task_completions()` define the public
  skeleton without adding a thread pool, coroutine runtime, or platform wakeup
  integration yet.
- Completion injection is explicit in tests: `complete_task(id)` marks a task
  queued and inactive, but does not run the callback until
  `drain_task_completions()` executes on the runtime thread while the window
  and renderer are live.
- Completion callbacks drain FIFO, see a normal `WindowRuntimeContext`, and
  share the same redraw deferral discipline as deferred callbacks and timers.
  Duplicate completion of an already completed task soft-fails.
- Step 136 intentionally does not implement cancellation, background
  scheduling, thread safety, or platform event-loop wakeups. Those remain out
  of scope until later runtime/platform slices.

## 2026-07-01 Timer API

- Step 135 keeps timers deterministic and runtime-local: `TimerId`,
  `TimerCallback`, `schedule_timer(...)`, `schedule_repeating_timer(...)`,
  `cancel_timer(...)`, and `advance_time(...)` live on the shared runtime/
  context surface without adding real OS timer backends yet.
- Timer callbacks run while `WindowRuntime` still has a live window and
  renderer. Tests therefore drive `advance_time(...)` from the fake
  application's `run()` callback rather than after `WindowRuntime::run(...)`
  returns.
- One-shot timers are removed before their callback runs, repeating timers
  reschedule by their interval, and `request_render`/`request_layout`/
  `request_paint` inside timer callbacks defer a redraw until timer firing
  finishes. This mirrors Step 134's event/deferred-callback redraw discipline.
- Step 135 intentionally does not add platform event-loop wakeups, async task
  handles, batching, or diagnostics. Those remain Steps 136-138 and 165.

## 2026-07-01 Deferred Callback Queue Merged

- Step 134 is merged on `master` at
  `64f1614 feat: add deferred callback queue` and post-merge verified on
  Windows and WSL Arch Linux.
- Step 135 should build timer ids and deterministic ticking on top of the
  runtime loop without changing the Step 134 event-turn defer ordering or
  introducing platform wakeups early; platform wakeups remain Step 165.

## 2026-07-01 Deferred Callback Queue

- Step 134 keeps deferred work runtime-owned and deliberately small:
  `DeferredCallback` is a public `std::function<void(const
  WindowRuntimeContext&)>` spelling, `WindowRuntimeContext::defer(...)`
  forwards to `WindowRuntime`, and the runtime drains callbacks after
  `after_event_callback_` but before the deferred redraw request is flushed.
- Deferred callbacks are drained FIFO in batches. Callbacks queued by a
  deferred callback run in a later drain iteration before the runtime leaves
  the event turn, which keeps nested `cx.defer(...)` behavior deterministic
  without introducing timers or async task handles early.
- Redraw scheduling now treats `draining_deferred_callbacks_` like event
  dispatch: `request_render`, `request_layout`, and `request_paint` made from a
  deferred callback set the deferred redraw flag and request one redraw after
  the queue is drained.
- Step 134 intentionally does not add timer ids, async task handles, platform
  wakeups, batching, or diagnostics. Those remain Steps 135-138 and 165.

## 2026-07-01 Subscription Ownership Token Merged

- Step 133 is merged on `master` at
  `77293cb feat: add subscription ownership token` and post-merge verified on
  Windows and WSL Arch Linux.
- Step 134 should build deferred callbacks on top of the runtime/context
  capability surface without changing the new subscription token lifetime
  contract or legacy permanent observer behavior.

## 2026-07-01 Subscription Ownership Token

- Step 133 adds an explicit owned-observer path instead of changing the legacy
  `observe_model(...) -> bool` API. The legacy API remains a permanent
  observer registration so existing Step 102/132 behavior and demo authoring
  code do not silently disconnect at the end of an expression.
- `Subscription` is a move-only RAII token over `SubscriptionId`; dropping the
  token calls `release()`, and explicit `WindowRuntime::remove_subscription`
  clears the observer callback. Releasing/removing an already disconnected or
  unknown subscription soft-fails with `false`.
- Owned observer removal tombstones the callback rather than erasing the vector
  entry. This keeps notification iteration stable if a subscription is removed
  while callbacks are being processed.
- `subscriptions_for_view(...)` is intentionally unchanged. Step 133 owns
  observer lifetime tokens, not automatic view dependency tracking, reactive
  dependency inference, deferred callbacks, timers, or async completions.

## 2026-07-01 Scoped Action Registry Merged

- Step 132 is merged on `master` at
  `7de89c7 feat: add scoped action registry` and post-merge verified on
  Windows and WSL Arch Linux.
- Step 133 should add subscription ownership/disconnect semantics on top of the
  existing observer list without changing Step 132 action scope lookup, legacy
  action compatibility, or `ActionDispatchResult` metadata.

## 2026-07-01 Scoped Action Registry

- Step 132 keeps the old `register_action(name, handler)` spelling as the
  app/global action scope, so existing key bindings and demo authoring code stay
  source-compatible.
- Scoped dispatch now has explicit metadata through `ActionScope` and
  `ActionDispatchResult::scope`, `view_id`, and `element_id`. Missing actions
  still return `handled == false` with no scope or owner metadata.
- Lookup order is focused element, current/target view, window, then app. The
  view lookup uses the current event route when dispatch happens inside event
  handling and falls back to the root view outside an event route.
- `ViewContext` exposes explicit `register_app_action`,
  `register_window_action`, `register_view_action`, and
  `register_focused_element_action` helpers while preserving the older
  `register_action` helper as app/global scope forwarding.
- This step deliberately does not add subscription lifetime tokens, deferred
  callbacks, timers, async completions, or redraw batching; those remain Steps
  133-137.

## 2026-07-01 Global App State Registry Merged

- Step 131 is merged on `master` at
  `54bcec4 feat: add global app state registry` and post-merge verified on
  Windows and WSL Arch Linux.
- Step 132 should build scoped actions on top of the existing action dispatch
  path without changing the new global registry semantics; globals remain
  passive typed state until Step 137 adds update batching/redraw behavior.

## 2026-07-01 Global App State Registry

- Step 131 keeps global state runtime-owned and typed by `std::type_index`,
  mirroring the existing entity-store pattern without introducing a separate
  app object or ownership layer.
- `AppContext` and `ViewContext` expose the same `set_global`, `global`, and
  `update_global` spelling, so setup-time globals are visible to frame/event
  context code through one shared runtime store.
- Missing global reads return `nullptr`, and missing global updates return
  `false`. Global updates do not schedule redraw yet; Step 137 owns update
  batching and redraw coalescing for model/global changes.

## 2026-07-01 Entity Handle Convenience API Merged

- Step 130 is merged on `master` at
  `57e103a feat: add entity handle convenience` and post-merge verified on
  Windows and WSL Arch Linux.
- The next Step 131 global registry should reuse the existing context/runtime
  capability style: typed helpers over one runtime-owned store, soft-fail
  missing lookups, and no new app ownership layer.

## 2026-07-01 Entity Handle Convenience API

- Step 130 keeps `EntityHandle<T>` as a lightweight typed id wrapper, not a new
  ownership model. The runtime remains the source of truth for entity storage,
  liveness, notification, and invalidation.
- `EntityHandle<T>::read(cx)` delegates to the existing context entity read
  helper, while `update(cx, fn)` delegates to the existing notifying update
  path. This intentionally gives entity handles the same subscribed-view
  invalidation behavior already used by model updates.
- `downgrade()` returns the existing `WeakEntity<T>` shape, preserving the
  soft-fail upgrade semantics from Step 101 instead of introducing a second
  weak handle type.

## 2026-07-01 Context Authoring Alias Merged

- Step 129 is merged on `master` at
  `d1576fe feat: add context authoring alias` and post-merge verified on
  Windows and WSL Arch Linux.
- The public context authoring surface now has the GPUI-like spelling
  `Context<T>` while preserving the existing `ViewContext`/`WindowRuntimeContext`
  runtime behavior.
- Step 130 should add entity handle convenience methods on top of this alias
  and the existing entity/model helpers, without turning `Context<T>` into an
  owning context wrapper.

## 2026-07-01 Context Authoring Alias

- Step 129 keeps `Context<T>` intentionally as a public authoring alias over
  `ViewContext`; it does not add new context storage, model ownership, global
  state, async behavior, or runtime lifetime semantics.
- The alias is type-compatible with the existing `ViewContext` helper surface,
  so `Context<MyView>` can call model helpers, input snapshots, invalidation
  helpers, and other context APIs without changing `WindowRuntimeContext`.
- The template parameter is reserved for authoring readability and future
  typed-view ergonomics. Step 130 should build entity handle convenience on top
  of the existing context/model APIs rather than changing the `Context<T>`
  alias into an owning wrapper.

## 2026-07-01 Public Prelude Demo Rewrite Merged

- Step 128 is merged on `master` at
  `4026899 feat: rewrite demo with public prelude` and post-merge verified on
  Windows and WSL Arch Linux.
- The 128-step gate is complete: targeted architecture/prelude tests passed
  2/2, Windows full debug passed 29/29, and WSL Arch Linux full debug passed
  26/26 after the merge.
- Step 129 can now start from a fresh `context-authoring-alias` worktree. It
  should add the public `Context<T>` authoring alias over `ViewContext` without
  changing runtime ownership or starting the later entity handle/global-state
  steps early.

## 2026-07-01 Public Prelude Demo Rewrite

- Step 128 rewrites `examples/hello_window` around the public prelude and
  authoring APIs: `cgpui/cgpui.hpp`, `run_app`, `AppRunnerOptions`,
  `AppContext`, `View::render(ViewContext&)`, free factories, fluent builder
  shortcuts, and `ViewContext` model/text helpers.
- The demo no longer owns a manual `ElementTree` or constructs
  `WindowRuntime` directly. App-level lifecycle callbacks are still installed
  during `setup_context`, while author-facing text binding, focus, cursor,
  action, key binding, subscription, and model update calls go through
  `ViewContext`.
- Current rendered elements only give the runtime a stable root element id
  during `ElementTree::set_root(...)`. The demo therefore binds the text model
  to the rendered root id for this step and keeps true nested/keyed element
  identity deferred to the planned keyed reconciliation/widget work.
- Feature-worktree verification passed: targeted architecture/prelude tests
  2/2, Windows hello-window smoke tests 3/3, Windows full debug 29/29, and WSL
  Arch Linux full debug 26/26.

## 2026-07-01 IME Candidate Rectangle Data Merged

- Step 127 is merged on `master` at
  `80aadae feat: add focused text ime rect` and post-merge verified on Windows
  and WSL Arch Linux.
- The remaining pre-back-40 gate is Step 128 plus the post-Step-128 targeted,
  Windows full debug, and WSL Arch Linux full debug verification.
- Step 128 should rewrite the demo around the public prelude and new authoring
  APIs; it should not expand into Win32 IME placement or Wayland text-input
  protocol work now that focused-text IME geometry is available.

## 2026-07-01 IME Candidate Rectangle Data

- Step 127 exposes IME composition/candidate geometry as shared runtime data,
  not platform placement: `ImeCandidateRect`,
  `WindowRuntime::focused_text_ime_rect()`, and
  `WindowRuntimeContext::focused_text_ime_rect()` give later Win32 and Wayland
  IME code one geometry source to consume.
- The candidate rectangle intentionally mirrors Step 123 caret geometry:
  origin x is the focused `TextElement` layout x plus cursor byte offset times
  deterministic glyph width (`font_size * 0.5F`), origin y is the text element
  layout y, width is `1.0F`, and height is the text element font size.
- The runtime lookup soft-fails with `std::nullopt` when there is no focused
  text element, no bound `TextModel`, no routed `TextElement`, or no layout
  bounds. This keeps platform IME placement code from guessing stale geometry.
- Step 127 deliberately does not call Win32 IME APIs or Wayland text-input
  protocols. Those are later platform-depth slices; this step only pins the
  shared focused-text geometry contract.

## 2026-07-01 Wayland Clipboard Skeleton Merged

- Step 126 is merged on `master` at
  `ab464d5 feat: add wayland clipboard skeleton` and post-merge verified on
  Windows and WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 127-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch Linux full debug verification.
- Step 127 should expose focused text IME composition/candidate rectangle data
  without wiring platform IME placement yet.

## 2026-07-01 Wayland Clipboard Skeleton

- Step 126 keeps Wayland clipboard work deliberately skeletal: Linux now has a
  public `WaylandClipboard` implementation with observable `unsupported`,
  `no_seat`, and `available` support states, but it does not implement full
  Wayland data-device transfer yet.
- `WaylandClipboard` uses a `MemoryClipboard` fallback for all current support
  states, preserving runtime copy/cut/paste behavior when the compositor lacks
  data-device support or no seat is available.
- Linux `create_platform_clipboard()` now returns `WaylandClipboard` instead
  of `MemoryClipboard`, giving future data-device work a platform-specific
  backend boundary without changing the shared `Clipboard` interface.
- Step 126 intentionally does not wire `wl_data_device_manager` discovery into
  `WaylandApplication`; that belongs in later platform-depth work once the
  skeleton contract is pinned.

## 2026-07-01 Win32 System Clipboard Backend Merged

- Step 125 is merged on `master` at
  `389b9fb feat: add win32 system clipboard` and post-merge verified on
  Windows and WSL Arch Linux.
- The remaining pre-back-40 gate after Step 125 was Steps 126-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch Linux full debug verification.
- From the Step 125 merge snapshot, Step 126 should start from
  `.worktrees/wayland-clipboard-skeleton` on
  `codex/wayland-clipboard-skeleton` and add a Wayland clipboard backend
  skeleton with graceful unsupported/no-seat behavior while preserving the
  shared `Clipboard` interface.

## 2026-07-01 Win32 System Clipboard Backend

- Step 125 keeps `MemoryClipboard` as the deterministic test/runtime fixture
  while making the Windows platform clipboard real through
  `CF_UNICODETEXT`.
- `create_platform_clipboard()` now remains platform-neutral at the API
  boundary: Windows returns a UTF-8 wrapper over the system clipboard, while
  non-Windows keeps the existing memory fallback until the Wayland skeleton in
  Step 126.
- The RED test needs a forced target rebuild after editing
  `tests/platform/clipboard_test.cpp`; otherwise xmake may reuse the previous
  binary and falsely report the old clipboard contract as passing.
- The WSL distro name on this machine is `archlinux`, not `Arch`; use
  `wsl -d archlinux ...` for Linux verification.

## 2026-07-01 Platform Cursor Application Merged

- Step 124 is merged on `master` at
  `74ad787 feat: apply platform cursors` and post-merge verified on Windows and
  WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 125-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- Step 125 should add a Win32 system clipboard backend for UTF-8 text while
  keeping the existing memory clipboard path deterministic for runtime tests.

## 2026-07-01 Text Caret And Selection Paint Metadata Merged

- Step 123 is merged on `master` at
  `b0b9e00 feat: add text caret selection paint` and post-merge verified on
  Windows and WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 124-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- Step 124 should connect runtime cursor state to Win32 and Wayland platform
  hooks while keeping clipboard, IME geometry, and cursor theme depth out of
  scope.

## 2026-07-01 Text Caret And Selection Paint Metadata

- Step 123 is implemented in `.worktrees/text-caret-selection-paint` on
  `codex/text-caret-selection-paint` and feature-worktree verified on Windows
  and WSL Arch Linux.
- Caret and selection are paint-list metadata only at this stage:
  `PaintCommandKind::text_selection`, `PaintCommandKind::text_caret`,
  `TextSelectionPaint`, `TextCaretPaint`, and matching `PaintList` fill helpers
  expose intent without adding Vulkan text drawing.
- `TextElement` uses the existing Step 122 deterministic fallback metrics:
  glyph width is `font_size * 0.5F`, selection geometry is derived from sorted
  byte offsets, and caret geometry is one pixel wide at the model cursor.
- A bound empty `TextModel` still emits caret metadata. Text content commands
  are emitted only when text is non-empty.
- `render_view(...)` skips text, text selection, and text caret commands until
  later text/glyph renderer work consumes them directly.
- Step 123 intentionally does not add shaping, glyph cache ownership, platform
  IME placement, platform cursor/clipboard behavior, or Vulkan text drawing.

## 2026-06-30 Font Descriptor And Font Size Style Merged

- Step 122 is merged on `master` at
  `58561b1 feat: add font size style` and post-merge verified on Windows and
  WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 123-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- Step 123 should use the deterministic Step 122 `font_size` metrics and
  `TextPaint` font metadata to emit caret and selection paint metadata. Keep
  shaping, glyph cache ownership, platform IME placement, and Vulkan text
  drawing out of Step 123.

## 2026-06-30 Font Descriptor And Font Size Style

- Step 122 keeps font work as conservative public metadata: `FontDescriptor`
  currently carries a family string, `Style` and `StyleOverlay` carry font and
  `font_size`, and the element builder exposes `.font(...)` and
  `.font_size(...)` without adding platform font discovery yet.
- The default text metric contract is preserved at 16px height and 8px glyph
  width because `TextElement` now derives deterministic fallback metrics as
  glyph width `font_size * 0.5F` and height `font_size`.
- `TextPaint` now carries copied font metadata and font size alongside bounds,
  color, content, byte length, and clip metadata. This gives Step 123 a stable
  metric source for caret/selection paint metadata.
- Step 122 intentionally does not add shaping, glyph caches, font discovery,
  Vulkan text drawing, caret/selection commands, cursor, clipboard, or IME
  behavior. Those remain later planned slices.

## 2026-06-30 Text Paint Command Merged

- Step 121 is now merged at `cf180f4 feat: add text paint command` and
  post-merge verified on Windows and WSL Arch Linux.
- The remaining pre-back-40 gate is Steps 122-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- Step 122 should now add font descriptor and basic font-size style primitives
  on top of explicit text command metadata. Keep shaping, glyph caches, and
  Vulkan text drawing out of Step 122.

## 2026-06-30 Text Paint Command

- Step 121 separates UI text intent from rectangle painting by adding
  `PaintCommandKind::text`, `TextPaint`, and `PaintList::fill_text(...)`.
  Text payloads preserve bounds, color, copied UTF-8 content, byte length, and
  active clip metadata.
- `TextElement::paint(...)` now emits text commands instead of a placeholder
  solid rectangle. This keeps later font, caret, selection, glyph cache, and
  Vulkan text work attached to explicit text metadata instead of trying to
  infer text from rectangle fallbacks.
- `render_view(...)` intentionally skips text commands for now. That is better
  than converting text back to a solid rectangle because Step 152 will teach
  Vulkan to consume text/glyph metadata directly; until then text commands are
  observable in paint-list tests but not drawn by the simple rect renderer.
- Step 121 intentionally does not add font descriptors, font size style,
  caret/selection paint metadata, glyph caches, platform clipboard behavior, or
  real text shaping. Those remain Steps 122, 123, 125/126, and 149-152 work.
- Step 122 should build on this by adding font descriptor and font-size style
  metadata before caret/selection geometry tries to use text metrics.

## 2026-06-30 Back-40 Planning With Step 121 Active

- The后 40 步 remain Steps 129-168 and are still gated behind Steps 121-128
  plus the post-Step-128 Windows/WSL verification on `master`.
- The current planning anchor is the docs closeout commit
  `45a8dad docs: mark step 120 merged`; use
  `9aba0e6 feat: honor vulkan solid rect clips` as the Step 120 behavior
  commit, not the current `master` HEAD.
- Step 121 is already active in `.worktrees/text-paint-command` on
  `codex/text-paint-command`; future execution should finish and merge that
  branch before opening Step 122. Do not create a Step 129 worktree until the
  Step 128 exit contract passes.
- A useful后 40 步 plan needs more than branch names and target tests: each
  step now has an exit artifact and an explicit keep-out-of-scope note. This
  should reduce step creep in high-risk areas such as async, widgets, text
  shaping, Vulkan text drawing, multi-window lifecycle, accessibility, and the
  final parity audit.
- The distance estimate remains 8 implementation steps to Step 129 while Step
  121 is unmerged. After Step 121 merges and is post-merge verified, the
  distance becomes 7 implementation steps, Steps 122-128, plus the
  post-Step-128 verification gate.

## 2026-06-30 Back-40 Planning After Step 120 Merge

- The后 40 步 remain Steps 129-168, and they are now gated behind Steps
  121-128 plus the post-Step-128 Windows/WSL verification on `master`.
- Step 120 is no longer part of the active gate: it is merged at
  `9aba0e6 feat: honor vulkan solid rect clips` and post-merge verified on
  Windows and WSL Arch Linux.
- The next implementation slice is Step 121, text paint command metadata. It
  should replace text placeholder rectangles with command metadata without
  pulling in font descriptors, caret/selection rendering, or platform clipboard
  work early.
- The Step 168 outcome remains a practical Windows/Linux GPUI-core foundation
  plus a parity audit document. It is not full upstream GPUI parity, and
  macOS/Cocoa + Metal remains explicitly deferred.

## 2026-06-30 Vulkan Clip Rect Metadata

- Step 120 extends `SolidRect` with optional renderer-facing clip metadata so
  clipped paint commands can reach the backend without introducing a separate
  paint-command renderer API yet.
- `render_view(...)` must copy `PaintCommand::clip_rect` onto the `SolidRect`
  sent to `RenderFrame::draw_rect(...)`; otherwise the UI paint-list metadata
  added in Steps 53 and 119 stops before the renderer boundary.
- The current Vulkan solid-rect implementation uses `vkCmdClearAttachments`
  with a `VkClearRect`. Honoring clip metadata means computing the framebuffer-
  clamped intersection of `SolidRect::rect` and `SolidRect::clip_rect` before
  issuing the clear. This is equivalent to a scissor for the current clear-rect
  path and does not require a graphics pipeline yet.
- Win32 pixel sampling from an sRGB swapchain reports the clear color after
  conversion, so the outside-clip test should assert a dark clear-color range,
  not the raw linear `0.08/0.09/0.10` channel bytes.
- Step 121 can now focus on text paint commands. It should not revisit Vulkan
  clip handling or rounded-rect rasterization unless new text command tests
  force a shared command-vocabulary change.

## 2026-06-30 Back-40 Planning After Step 119 Merge

- The后 40 步 are still Steps 129-168, but the live entry gate has moved
  forward: Step 119 is merged and post-merge verified, so the remaining
  pre-back-40 work is Steps 120-128 plus post-Step-128 Windows/WSL
  verification.
- Step 120 should now be the next worktree. Its scope is Vulkan consumption of
  `PaintCommand::clip_rect` for solid rectangles; it should not expand into
  rounded-rect rasterization or text drawing.
- The Step 168 outcome remains a practical Windows/Linux GPUI-core foundation
  plus a parity audit document. It is not full upstream GPUI parity, and
  macOS/Cocoa + Metal remains explicitly deferred.

## 2026-06-30 Rounded-Rect Paint Command

- Step 119 keeps rounded rectangles as paint-list command metadata first. The
  Vulkan backend still receives the `solid_rect` fallback carried by each
  command; actual rounded drawing is intentionally left for later renderer
  work.
- Nonzero `Style::border_radius` on a styled background emits
  `PaintCommandKind::rounded_rect` with a `RoundedRect` payload preserving all
  four corner radii. Zero radius preserves the existing
  `PaintCommandKind::solid_rect` background behavior.
- `PaintList::fill_rounded_rect(...)` attaches the same active clip metadata as
  `fill_rect(...)`, so Step 120 can focus on Vulkan clip consumption without
  revisiting command storage.
- The next renderer slice should teach Vulkan to honor `PaintCommand::clip_rect`
  for solid rectangles. It should not implement real rounded-rect rasterization
  yet unless the plan is explicitly changed.

## 2026-06-30 Back-40 Planning After Step 118 Docs Closeout

- The后 40 步 are Steps 129-168 and remain gated behind completion of Steps
  119-128 plus post-Step-128 Windows and WSL verification.
- Current planning should anchor on the docs closeout commit
  `c0d38c7 docs: mark step 118 merged`, not only the Step 118 feature commit
  `9dfc2e7 feat: add layer elevation z order`.
- The intended Step 168 outcome is a practical Windows/Linux GPUI-core
  foundation plus `docs/gpui-core-api-parity.md`; it is not a claim of full
  upstream GPUI parity and it keeps macOS/Cocoa + Metal deferred.
- The immediate implementation action is still Step 119 in
  `.worktrees/rounded-rect-paint-command`, starting with RED tests for
  border-radius metadata in rounded-rect paint commands.

## 2026-06-30 Layer/Elevation Z Order Merged

- After Step 118 merged, the remaining pre-back-40 gate is Steps 119-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification.
- Step 119 should build on the current paint command stream by preserving
  border-radius metadata in a rounded-rect command shape first. Avoid changing
  Vulkan rounded drawing behavior until the command metadata is pinned by RED
  tests.

## 2026-06-30 Layer/Elevation Z Order

- Step 118 maps layer/elevation onto deterministic paint order by adding
  `Style::layer`, `StyleOverlay::layer`, builder `.layer(...)`, and
  `Element::layer()`/`z_order()`.
- Explicit nonzero `z_index` remains the compatibility override. The effective
  paint key is `z_index` when it is nonzero; otherwise it is `layer`.
- Layer and z-index metadata need to live on the final `Element`, not only on
  `Style`, because parent paint ordering sees the outer wrapper after click,
  pointer, key, and focusable wrappers are applied.
- Stable sort remains important: siblings with the same effective `z_order()`
  continue painting in authored insertion order, preserving the existing
  deterministic z-index contract.

## 2026-06-30 Back-40 Planning After Step 117 Docs Closeout

- The后 40 步 remain Steps 129-168 and should not preempt the active Step
  118-128 queue.
- The current planning anchor is `c3b6ecb docs: mark step 117 merged`; use
  `210c85d feat: add absolute positioning insets` only as the Step 117 feature
  commit reference.
- The remaining route to Step 129 is 11 implementation steps, Steps 118-128,
  plus post-Step-128 targeted verification, Windows full debug, WSL Arch full
  debug, and a clean tracked/staged worktree.
- The back-40 plan now explicitly records Steps 115 and 116 as gate-complete
  alongside Step 117. This matters because Steps 139-148 widget work and
  Steps 149-158 rendering work depend on the full flex/alignment/grow/shrink/
  absolute/layer layout substrate being complete before Step 129 starts.

## 2026-06-30 Absolute Positioning And Insets Merged

- After Step 117 merged, the remaining pre-back-40 gate is Steps 118-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification.
- Step 118 should build on Step 117's layout substrate by adding
  layer/elevation ordering as deterministic metadata first. Avoid changing
  absolute child sizing or adding right/bottom anchoring unless Step 118 tests
  explicitly require it.

## 2026-06-30 Absolute Positioning And Insets

- Position and inset need to live on the final `Element`, not only on `Style`,
  because parent stack/flex layout reads each built child after behavior
  wrappers like click, pointer, key, and focusability have been applied.
- Default `Position::relative` preserves all previous stack/flex layout.
  Absolute children are measured for their own size but excluded from normal
  flow size, gap spacing, flex grow/shrink allocation, justification, and
  alignment.
- Step 117 intentionally gives `left` and `top` placement semantics first.
  `right` and `bottom` are stored in `EdgeSizes` for API continuity and later
  anchoring work, but they should not drive layout until a later step adds
  explicit right/bottom behavior and tests.
- Stack and flex layout should place absolute children after the parent output
  size is known. This keeps normal-flow children deterministic and gives Step
  118 layer/elevation work a stable layout substrate.

## 2026-06-30 Back-40 Planning After Step 116 Docs Closeout

- The后 40 步 are still Steps 129-168, and the current planning anchor should
  be the docs closeout commit `1e542bd docs: mark step 116 merged`, not only
  the Step 116 feature commit `2806a4a feat: add flex grow shrink layout`.
- Step 117 already has an active worktree at
  `.worktrees/absolute-position-insets` on `codex/absolute-position-insets`.
  Future execution should continue that branch through verification and merge
  instead of creating a new Step 117 branch or starting Step 129.
- The useful post-Step-128 delivery target is four staged outcomes:
  context/entity/async ergonomics, keyed reusable widgets and style cascade,
  backend-ready text/render metadata, and Win32/Wayland platform closure plus a
  parity audit. Step 168 should remain the audit/handoff milestone, not a claim
  of full upstream GPUI parity.
- From current `master`, the back-40 entry is still gated by 12 implementation
  steps, Steps 117-128, plus post-Step-128 targeted, Windows full debug, and
  WSL Arch full debug verification.

## 2026-06-30 Flex Grow And Shrink Merged

- After Step 116 merged, the remaining pre-back-40 gate is Steps 117-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification.
- Step 117 should start from a fresh `.worktrees/absolute-position-insets`
  worktree and keep absolute positioning/insets as layout metadata first,
  before Step 118 layers/elevation and Step 119+ renderer command hardening.

## 2026-06-30 Flex Grow And Shrink

- Flex grow and shrink factors need to live on `Element`, not only on
  `Style`, because the parent `FlexElement` reads each final child element
  during layout.
- Builder-created behavior wrappers must copy the flex factors onto the
  outermost wrapper (`ClickElement`, `PointerElement`, `KeyElement`, and
  `FocusableElement`). Otherwise the parent flex container would see zero
  factors when a child has handlers or focusability.
- Default grow/shrink factors remain `0.0F`, preserving previous flex layout
  for existing children unless authors explicitly opt in.
- Shrink tests that constrain only height must still provide a nonzero max
  width when using aggregate `Size` initialization, because omitted width
  fields default to zero and can collapse output width before shrink behavior
  is evaluated.

## 2026-06-30 Back-40 Planning Hardening

- The后 40 步 should be treated as an execution-ready continuation of the
  current Step 128 gate, not as a separate roadmap. The useful completion
  target is a practical Windows/Linux GPUI-core foundation with public
  context/entity/global APIs, reusable keyed widgets, deterministic
  text/render metadata, platform hooks, and an explicit parity audit.
- Step 168 remains an audit milestone rather than a full GPUI parity claim.
  The audit should be the authoritative place for implemented, partial,
  missing, and Mac/Metal-deferred areas.
- The back-40 non-goals are important safety rails: do not begin macOS/Metal
  parity, do not replace Vulkan/xmake, do not require a full shaping stack
  before deterministic text contracts exist, do not create a separate widget
  framework, and do not combine small steps to reduce commit count.
- Step 129 should start from a verified `master` with a fresh
  `.worktrees/context-authoring-alias` worktree. Its first RED should be a
  public `Context<T>` authoring alias compile failure plus a runtime test that
  proves the alias preserves existing `ViewContext` helper behavior.

## 2026-06-30 Flex Alignment And Justification

- Flex alignment and justification can stay entirely in shared style/element
  code for Step 115. No runtime or platform-specific behavior is needed.
- `JustifyContent::start` preserves the old flex layout behavior. `center` and
  `end` use only positive constrained free space as initial main-axis offset,
  and `space_between` keeps the authored `gap` as the base gap while
  distributing extra free space between children.
- `AlignItems` uses the final constrained cross-axis size, so rows align child
  y origins and columns align child x origins. Cross-axis free space is clamped
  at zero, preserving previous overflow behavior for children larger than the
  constrained container.
- Measuring children before assigning final bounds is necessary because the
  final constrained flex size controls both main-axis justification and
  cross-axis alignment.

## 2026-06-30 Back-40 Planning After Step 114 Closeout

- The后 40 步 remain Steps 129-168 and should still be treated as a
  post-Step-128 execution queue, not the active branch queue while Steps
  115-128 are incomplete.
- Current `master` is anchored at `ac745f8 docs: mark step 114 merged`; the
  Step 114 behavior commit is `d78a017 feat: clip hidden overflow hit testing`.
- Step 115 already has a worktree at `.worktrees/flex-alignment-justification`
  on `codex/flex-alignment-justification`, and its baseline targeted tests have
  passed. The next practical action is to add Step 115 RED tests there, not to
  recreate the worktree or start Step 129.
- The route to Step 129 is 14 incomplete implementation steps, Steps 115-128,
  plus post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification. The route through Step 168 is 54 implementation steps plus
  checkpoint reviews after Steps 138, 148, 158, and 168.
- The back-40 order remains context/entity/global/async first, keyed
  reconciliation/widgets/style cascade second, text/font/renderer diagnostics
  third, and Windows/Wayland platform closure plus parity audit last.

## 2026-06-30 Hidden Overflow Hit Testing

- `StyledElement` needed its own `hit_test(...)` override because the inherited
  `Element::hit_test(...)` only checked the styled wrapper's layout bounds; it
  neither forwarded to the owned child nor applied `Overflow::hidden` clip
  semantics.
- Hidden overflow hit testing should mirror the existing paint clip contract:
  if `Style::clip_rect` is present, use it as the hit-test clip; otherwise use
  the styled element's layout bounds. Points outside that clip return no hit
  before checking child or self hits.
- Visible overflow must keep checking the child before falling back to the
  styled element itself. This preserves current authoring behavior where a
  child can be hit outside the wrapper's own bounds when overflow remains
  visible.
- Runtime coverage for an internally owned child is limited by
  `ElementTree::get(...)`: a child owned inside `StyledElement` is not a tree
  node, so runtime route assertions should verify root/no-target clipping
  behavior unless a later step promotes internal children into the tree.
- Keep `ScrollElement::hit_test(...)` wrapper-targeting unchanged. Step 113
  depends on scroll routing finding the scroll viewport wrapper so the runtime
  can locate the bound `ScrollState`.

## 2026-06-30 Back-40 Planning At Step 113 Docs Closeout

- The后 40 步 are still Steps 129-168, and the current anchor should be the
  docs closeout commit `2251d44 docs: mark step 113 merged`, not only the Step
  113 behavior commit `7a2ef39 feat: route scroll events to scroll state`.
- The gate into Step 129 is now exactly Steps 114-128 plus the post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification.
- The execution order should stay sequential and banded: Steps 129-138 for
  context/entity/global/async, Steps 139-148 for keyed reconciliation/widgets,
  Steps 149-158 for text/font/renderer diagnostics, and Steps 159-168 for
  Windows/Wayland platform closure plus the parity audit.
- The next practical implementation action remains Step 114 in a fresh
  `.worktrees/hidden-overflow-hit-testing` worktree, not Step 129.

## 2026-06-30 Scroll Routing

- `PointerScrolled` routing belongs after route target selection and ancestry
  refresh but before generic element dispatch/view fallback. That lets scroll
  state consume wheel/trackpad deltas without invoking the view when a bound
  `ScrollElement` owns the viewport under the pointer.
- `ScrollElement` should hit-test as the scroll viewport wrapper for now. Its
  child is owned internally rather than indexed in `ElementTree`, so returning
  the child id can leave `WindowRuntime::routed_element(...)` unable to find
  the element that actually owns the `ScrollState`.
- The scroll delta should be applied through `ScrollState::scroll_by(...)`
  instead of setting offsets directly so existing viewport/content-size clamp
  semantics remain the single source of truth.
- A no-scroll route remains a soft-fail path: if no routed `ScrollElement` is
  found, the runtime continues to normal element dispatch and view fallback.


## 2026-06-30 Back-40 Planning After Step 112 Closeout

- The后 40 步 are Steps 129-168 and remain a post-Step-128 follow-on queue, not
  the active branch queue while Step 113-128 are still incomplete.
- Current `master` is anchored at `ed948a7 docs: mark step 112 merged`; Step
  113 already has a worktree at `.worktrees/scroll-routing` on
  `codex/scroll-routing`, so the correct next action is to continue that
  worktree's RED/GREEN cycle.
- Step 129 is 16 implementation steps away: Steps 113-128 plus post-Step-128
  targeted, Windows full debug, and WSL Arch full debug verification. Step 168
  is 56 implementation steps away from current `master`, plus the four band
  checkpoint reviews.
- The back-40 band order remains context/entity/global/async first, keyed
  reconciliation/widgets/style cascade second, text/font/renderer diagnostics
  third, and Windows/Wayland platform closure plus the parity audit last.


## 2026-06-30 Focus Traversal

- Step 111 can reuse `ElementTree::enabled_preorder_ids()` as the traversal
  source. That keeps disabled descendants out of traversal before checking
  `Element::focusable()`, so disabled focusable-looking elements do not receive
  Tab focus.
- Focus traversal belongs in the keyboard-key pre-dispatch path: after a
  successful traversal, the current event route should be retargeted to
  `keyboard_focus_element_owner_` and ancestry refreshed so key bindings,
  routed element handlers, view fallback, and after-event callbacks all see the
  updated focused element.
- Shift+Tab can share the same traversal helper as Tab by passing a reverse
  flag. With no current focus, forward traversal chooses the first enabled
  focusable element and reverse traversal chooses the last one; with current
  focus, both directions wrap.
- The helper should ignore Tab variants with control, alt, or super modifiers
  so future app/window shortcuts can still bind those chords. Shift remains the
  reverse traversal selector.

## 2026-06-30 Back-40 Planning With Step 111 Active

- The后 40 步 still mean Steps 129-168, and they remain a post-Step-128
  follow-on queue. They should not preempt the active Step 111-128 gate.
- Current `master` is anchored at `d52ce80 docs: mark step 110 merged`; the
  Step 110 behavior commit is `10f2dd3 feat: add event propagation phases`.
- Step 111 has landed on `master`; planning should point future workers at
  Step 112, not at recreating `.worktrees/focus-traversal` or starting Step 129
  early.
- From current `master`, Step 129 is 17 implementation steps away: Steps
  112-128 plus post-Step-128 targeted, Windows full debug, and WSL Arch full
  debug verification.
- Step 168 is still 57 implementation steps away from current `master`: Steps
  112-168 plus the four band checkpoint reviews after Steps 138, 148, 158, and
  168.
- The back-40 band order remains context/entity/global/async first, keyed
  reconciliation/widgets/style cascade second, text/font/renderer diagnostics
  third, and Windows/Wayland platform closure plus parity audit last.

## 2026-06-30 Back-40 Planning After Step 110 Merge

- The后 40 步 remain Steps 129-168. They should still be treated as the
  post-Step-128 follow-on queue, not as the active implementation queue.
- Current `master` is anchored at `10f2dd3 feat: add event propagation phases`;
  Steps 89-110 are merged and post-merge verified on Windows and WSL Arch
  Linux.
- From this state, Step 129 is 18 implementation steps away: Steps 111-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification. Step 168 is 58 implementation steps away: Steps 111-168 plus
  the four band checkpoint reviews.
- The next active implementation should remain Step 111, focus traversal over
  enabled focusable elements with Tab and Shift+Tab actions. Starting Step 129
  before focus, scroll, layout depth, renderer/text metadata, cursor/clipboard,
  IME geometry, and the public-prelude demo rewrite land would destabilize the
  follow-on context/widget/platform plan.
- The back-40 band order should stay: context/entity/global/async, keyed
  reconciliation/widgets/style cascade, text/font/renderer diagnostics, then
  Windows/Wayland platform closure plus the parity audit.

## 2026-06-30 Event Propagation Phases

- Step 110 uses `EventRoute::element_ancestry` as the dispatch route. The order
  remains target-to-root: the target element handles first, then each ancestor
  can handle the same event until one returns consumed or cancelled.
- `ElementEventContext::target_element_id` intentionally remains the original
  route target for every bubbling phase. This lets ancestor handlers know which
  descendant was hit/focused without changing the public context shape yet.
- Disabled elements remain part of route ancestry for diagnostics, but their
  handlers are skipped during bubbling. Dispatch continues to later enabled
  ancestors and only falls back to the root view when every routed element
  returns unhandled or is skipped.
- A route without `element_ancestry` still falls back to the route target id,
  preserving compatibility for legacy routes while Step 109's ancestry refresh
  is now the normal path.

## 2026-06-30 Back-40 Planning After Step 109 Merge

- The post-Step-128 "back 40" remains Steps 129-168. It is an execution-ready
  follow-on queue, not the active branch queue while Step 110-128 are still
  incomplete.
- Current `master` is anchored at `08d0fef docs: mark step 109 merged`; the
  Step 109 feature commit is `74df1df feat: add event route ancestry`.
- From this state, Step 129 is 19 implementation steps away: Steps 110-128 plus
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification. Step 168 is 59 implementation steps away: Steps 110-168 plus
  the four band checkpoint reviews.
- The back-40 order should stay banded: context/entity/global/async first,
  keyed reconciliation/widgets/style cascade second, text/font/renderer
  diagnostics third, and Windows/Wayland platform closure plus parity audit
  last. This order avoids asking widgets or platform hooks to invent missing
  shared runtime contracts.

## 2026-06-30 Event Route Ancestry Metadata

- Step 109 keeps ancestry as a route snapshot, not a propagation behavior
  change. `EventRoute::element_ancestry` is ordered target-to-root so Step 110
  can handle the target first and then bubble through ancestors without
  recomputing parent links.
- `EventRoute::view_ancestry` is also ordered target-to-root. A normal root
  event records only `ViewId{1}`; a hit on a registered `ChildViewElement`
  records the child `ViewId` followed by the root view. The current event still
  dispatches through the existing root view fallback until Step 110 adds real
  propagation phases.
- Runtime route refresh must happen after the final target is selected,
  because keyboard focus, pointer capture, and hit testing can each choose a
  different element or view target. View-target pointer capture clears the
  element target before ancestry is recalculated so stale element metadata does
  not leak into view-routed events.
- Child-view route targeting only upgrades to the child view when the placeholder
  references a registered live view. Removed or missing child views still leave
  the event on the root route, matching the view registry soft-fail contract.

## 2026-06-30 Child-View Placeholder

- Step 108 is intentionally metadata-only. `ChildViewElement` stores a
  `ViewId`, lays out to a constrained placeholder size, participates in
  hit-testing as a normal element, and paints nothing for now.
- Real nested view rendering, parent/child view ancestry, and event propagation
  across view boundaries remain Step 109/110 work. Keeping Step 108 small gives
  those steps a concrete element marker without forcing their routing semantics
  early.
- `ViewId` belongs in `element.hpp` now because child-view placeholders are
  authorable through element builders and public prelude helpers. Keeping a
  duplicate `ViewId` in `ui.hpp` would either make `element.hpp` depend on the
  runtime header or split the public id type.
- The runtime integration test should only prove that a registered child
  `ViewId` can be referenced by an installed placeholder element while
  `WindowRuntime::find_view(...)` still resolves the view. It should not call
  the child view's `render(...)` yet.
- After the Step 108 merge, Step 109 is the right next slice because the
  placeholder now gives event routing a concrete element marker that can carry
  both `ElementId` ancestry and child `ViewId` metadata without needing nested
  rendering first.

## 2026-06-30 Back-40 Planning While Step 108 Active

- The後 40 步 are still Steps 129-168, but the active implementation path has
  not crossed the Step 128 gate yet. Step 108 already has a worktree, so the
  next engineering action should continue `.worktrees/child-view-placeholder`
  instead of creating a Step 129 branch.
- The post-Step-128 plan should be treated as an execution-ready queue with
  branch slugs, RED test intents, targeted commands, Windows full debug, and
  WSL Arch full debug for every step. It is not just a roadmap, but it is still
  blocked by Steps 108-128.
- The current pre-back-40 gate count is 21 implementation steps: Step 108
  through Step 128. The high-risk gate items are event ancestry/propagation,
  text paint metadata, Vulkan clip/text command handling, cursor/clipboard
  backends, IME geometry, and the public-prelude demo rewrite.
- Step 168 should close with a parity audit, not a claim of full upstream GPUI
  equivalence. The useful audit split is implemented, partial, missing, and
  Mac/Metal-deferred areas for the Windows/Linux track.

## 2026-06-30 View Registry Skeleton

- Step 107 is intentionally a registry/lifetime skeleton, not child-view
  rendering. It gives later child-view placeholders, ancestry routing, and
  multi-window work a single view lookup path before those behaviors exist.
- The root view is registered as borrowed and non-removable at `ViewId{1}`.
  Additional borrowed views can be registered by reference, and owned views can
  be registered through `std::unique_ptr<View>`.
- App-opened root views should use the general view registry instead of a
  separate root-view ownership map. This keeps `AppContext::open_window(...)`,
  `app_opened_window_root_view(...)`, `WeakView` upgrade, and future child-view
  lookup on the same storage contract.
- Removing a registered non-root view should make `find_view(...)`,
  `is_view_id_allocated(...)`, and `WeakView` upgrade soft-fail for that id.
  Plain ids from `allocate_view_id()` remain compatible with the older
  monotonic allocation contract until a later view lifecycle step replaces it
  with full generation/removal semantics.

## 2026-06-30 Back-40 Planning After Step 107

- The "后40步" plan is now best understood as a post-Step-128 execution plan,
  not an immediate branch queue. With Step 107 feature-worktree verified, the
  remaining gate into Step 129 is Steps 108-128 plus final Windows and WSL Arch
  Linux verification on `master`.
- Step 129 should start only after the child-view, ancestry, propagation,
  focus, scroll, layout-depth, render-command, cursor, clipboard, IME, and
  public-demo slices have landed. Starting it earlier would make the context
  and widget APIs depend on unstable or placeholder runtime surfaces.
- The four 10-step bands should remain ordered: context/entity/async first,
  keyed widgets and style cascade second, text/font/renderer diagnostics third,
  and Windows/Wayland platform closure plus parity audit last. This order gives
  each band a usable substrate instead of asking later platform work to invent
  missing shared contracts.
- Because Steps 129-168 all touch public headers, shared runtime behavior,
  renderer contracts, or platform-neutral event data, every step should keep
  WSL Arch full debug in its normal definition of done even when the visible
  feature name is Win32-specific.

## 2026-06-30 App-Opened Root View Lifecycle

- Step 106 should remain a storage/lifecycle slice, not real multi-window
  platform creation. `AppContext::open_window(WindowOptions,
  std::unique_ptr<View>)` records the window descriptor, allocates a distinct
  `ViewId`, and transfers root view ownership into `WindowRuntime`; `run_app`
  still creates only the one platform/native window from the primary
  descriptor.
- Keeping `open_window(WindowOptions)` source-compatible with an empty
  `root_view_id` preserves the Step 105 skeleton path while allowing callers
  that supply root view ownership to receive an allocated id and query the
  stored view during setup and frame callbacks.
- The runtime-owned root view container naturally destroys app-opened root
  views when `run_app` returns and the `WindowRuntime` is destroyed. No
  explicit cleanup at `WindowRuntime::run()` start or end is needed for this
  slice, and clearing there would risk erasing setup-time app-opened windows.
- The initial Step 106 GREEN failure was test-side: the after-frame callback
  captured a setup-local pointer variable by reference. Capturing the pointer
  value keeps the test checking runtime lifecycle behavior instead of a
  dangling test reference.

## 2026-06-30 Back-40 Planning After Step 105 Merge

- The "后40步" plan is now an execution-ready follow-on queue for Steps
  129-168, but the active implementation path is still Steps 106-128. Starting
  Step 129 early would skip root-view lifecycle storage, the view registry,
  child-view placeholders, route ancestry, propagation, focus/scroll/layout
  depth, render command hardening, cursor/clipboard/IME surfaces, and the
  public-prelude demo rewrite.
- The clean handoff into Step 129 requires 23 remaining implementation steps:
  Steps 106-128, then post-Step-128 targeted verification, Windows full debug,
  WSL Arch full debug, and a clean `master` except the known untracked
  `.vscode/`.
- The back-40 should be reviewed in four checkpoint bands: Step 138 for
  context/entity/global/async APIs, Step 148 for keyed widgets and style
  cascade, Step 158 for text/render diagnostics, and Step 168 for
  Windows/Wayland platform closure plus the GPUI-core parity audit.

## 2026-06-30 Render Invalidation Observability

- Render invalidation should be a strict superset of layout and paint
  invalidation: `request_render()` sets render/layout/paint and schedules a
  redraw, while existing `request_layout()` and `request_paint()` keep the new
  render bit false.
- The first after-render observation point belongs immediately after
  `View::render(...)` returns and any non-null root element is installed, before
  layout, paint, invalidation clearing, and frame index increment. This lets
  callers inspect the render sequence and installed root id while the frame is
  still in progress.
- `RenderRecord::root_element_id` should report the root installed by the
  current render pass. If a view returns an empty `AnyElement`, the record can
  still report sequence, view id, and viewport size with no installed root id.
- Resetting render sequence and last render record at `WindowRuntime::run()`
  keeps each run deterministic and matches the existing event/frame sequence
  reset behavior.

## 2026-06-30 Steps 129-168 Execution Matrix

- The follow-on 40 steps should start only after Step 128 has a clean
  post-merge Windows and WSL baseline; otherwise context/widget/platform work
  will sit on top of unstable render, model, routing, text, or demo surfaces.
- The useful next level of planning detail is operational rather than more
  conceptual: each step now has a branch slug, first RED test intent, and
  targeted command so the existing RED/GREEN/merge cadence can continue
  without re-deciding the shape of the step every turn.
- All Steps 129-168 should keep WSL Arch full debug in the per-step definition
  of done, including Win32-looking steps, because these slices almost always
  introduce shared public headers, runtime contracts, or platform-neutral event
  data consumed by the Linux/Wayland path.
- The dependency order should stay banded: context/entity/global/async before
  widgets, keyed/lifecycle/style/focus/widgets before renderer text depth,
  renderer/text/diagnostics before platform completion and the final parity
  audit.

## 2026-06-30 Runtime Render Pass

- The narrow Step 97 integration point is `WindowRuntime::handle_redraw()`:
  rendering before the existing owned-tree layout pass lets the runtime reuse
  current `ElementTree::layout_root`, hit testing, routed-element lookup, and
  invalidation cleanup behavior.
- `View::render(ViewContext&)` takes a non-const lvalue context, so the runtime
  must materialize `ViewContext render_context = context();` before calling the
  hook instead of passing the temporary result of `context()` directly.
- A non-null render result should replace the runtime owned tree through the
  same `set_element_tree(...)` path used by `ViewContext`; this keeps
  `element_root()` and pointer routing consistent with manually installed
  trees.
- Existing `paint(...)` is still invoked by `render_view(...)` after the render
  tree is installed, preserving the compatibility contract until later steps
  route paint command generation through the rendered element tree.

## 2026-06-30 View Render Hook Skeleton

- `ViewContext` must be available before `View` is declared because the new
  virtual render hook uses the GPUI-like context spelling directly in the base
  class.
- The first render hook should be additive: the default `View::render` returns
  an empty `AnyElement`, while the existing `paint(...)` method remains pure
  virtual and source-compatible for old runtime and demo paths.
- Step 96 deliberately does not install the rendered tree into the runtime.
  That behavior remains Step 97 so the API hook can be verified separately
  from redraw, layout, hit testing, and owned element tree replacement.
- Calling `request_paint()` or `request_layout()` from render already works
  through `ViewContext` because the alias still forwards to
  `WindowRuntimeContext`; Step 98 can add render-specific invalidation and
  observability without changing the Step 96 hook shape.

## 2026-06-30 Steps 129-168 Planning Refresh

- The forward queue should be treated as a post-Step-128 plan, not as the next
  active implementation queue: Step 96 still gates the current track because
  `View::render`, runtime render-tree installation, model/app lifecycle,
  bubbling/focus/scroll/layout depth, render commands, cursor/clipboard/IME
  surfaces, and the public-prelude demo all feed the APIs used by Steps
  129-168.
- The most useful planning boundary is an execution gate rather than a second
  competing active plan. This keeps the follow-on route visible while preserving
  the current Step 96-128 RED/GREEN workflow.
- After Step 128, the first 10 follow-on steps should bias toward context,
  entity, global, scoped action, subscription, deferred, timer, async, batching,
  and diagnostics APIs before widget or renderer expansion.

## 2026-06-30 Style-State Overlays

- A separate `StyleOverlay` is necessary because `Style` has non-optional
  scalar/POD fields where zero is a valid authored value; optional overlay
  fields preserve "unset" versus "override to zero" semantics.
- Step 95 should stay authoring/data-only: `resolved_style(...)` is
  deterministic and testable, but runtime hover/focus/disabled application can
  wait for later render/style resolution work.
- The merge order is base, hover, focus, disabled. This lets disabled state win
  over active interaction state while still inheriting focus/hover fields that
  disabled does not override.
- `StyledElement` can store a full `StyleState` while keeping its old
  `style()` API as a base-style accessor. That preserves existing layout,
  paint, z-index, and tests while making state overlays available to later
  runtime/style-cascade slices.

## 2026-06-30 Steps 129-168 Forward Plan

- Steps 129-168 should remain a follow-on queue until Steps 95-128 finish,
  because the later context/widget/platform work depends on `View::render`,
  model helpers, propagation, text paint commands, cursor/clipboard/IME
  surfaces, and the public-prelude demo rewrite.
- The highest-leverage next 40 steps after Step 128 are not more authoring
  sugar; they are API-shape completion and runtime maturity: typed contexts,
  entity handles, globals, scoped actions, subscription ownership, deferred
  work, timers, async completions, keyed reconciliation, style cascade,
  widgets, text/font renderer depth, multi-window storage, and Win32/Wayland
  lifecycle hooks.
- The riskiest region is Steps 149-168, where text/font rendering and platform
  hooks cross from deterministic metadata into Vulkan, Win32, and Wayland
  behavior. Keep each step command-driven, fakeable in tests, and backed by
  Windows plus WSL verification.

## 2026-06-30 Pointer Handler Shortcuts

- Pointer handler shortcuts fit the existing wrapper pattern: keep element
  layout, paint, and hit testing transparent while intercepting only matching
  `PointerButton` or `PointerMoved` events.
- `.on_click(...)` remains source-compatible and still means pointer press; the
  new `.on_pointer_down(...)`, `.on_pointer_up(...)`, and
  `.on_pointer_move(...)` handlers expose concrete event data for authoring
  paths that need button or position details.
- Pointer handlers should return their handler result only when consumed or
  cancelled; otherwise the wrapper can continue forwarding to its child, which
  keeps future bubbling/ancestor propagation work from being boxed in too early.

## 2026-06-30 Element Builder Fluent Style Shortcuts

- Builder-level style shortcuts should remain thin mutations of the builder's
  retained `Style`; this keeps `.style(Style)` source-compatible and avoids a
  second style storage path.
- `.size(Size)` and `.size(float, float)` need to update both
  `style_.preferred_size` and `size_` when the builder kind is `fixed_size`,
  because `FixedSizeElement` build output reads `size_` rather than
  `style_.preferred_size`.
- `.gap(float)` can share the retained style path because row, column, and
  vertical stack builders already copy `style_.gap` into their concrete element
  before appending children.

## 2026-06-30 Remaining Steps 93-128 Plan Shape

- The user-facing "后 40 步" still refers to the Step 89-128 target band, but
  Steps 89-92 are already complete on `master`; the active implementation
  queue is therefore Steps 93-128.
- The safest order is still mostly sequential because Step 93-98 authoring and
  render entry unlock model/app lifecycle work, Step 99-108 lifecycle unlocks
  child-view ancestry, and Step 109-118 interaction/layout metadata feeds the
  Step 119-128 renderer/platform/demo hardening work.
- Step 124-127 are the highest platform-risk region because they cross from
  pure command/runtime metadata into Win32 and Wayland backend behavior; keep
  memory clipboard and deterministic tests available while platform backends
  mature.

## 2026-06-30 Style Unit And Color Helpers

- `px(float)` can remain a direct `float` identity helper for now because every
  current style/layout field already stores pixel-like lengths as `float`.
- Free `edges(...)` helpers should forward to the existing `EdgeSizes::all`,
  `EdgeSizes::axes`, and `EdgeSizes::trbl` constructors so Step 93 builder
  shortcuts can use concise authoring names without adding another edge type.
- `rgb` and `rgba` normalize integer 0-255 color channels into the existing
  floating `Color` representation; alpha remains a `float` so authored opacity
  composes with the current `Color::a` field directly.

## 2026-06-30 Steps 89-128 Execution Planning

- The remaining Windows/Linux core API work is best sequenced in four bands:
  authoring/render entry first, then model/app/window lifecycle, then
  event/layout depth, then renderer/text/platform hardening and the public demo.
- Step 92 should stay deliberately small: inline `px`, `rgb`, `rgba`, and
  `edges(...)` helpers can layer over the existing `float`, `Color`, and
  `EdgeSizes` primitives without adding a new length type yet.
- Step 93 can then mutate the builder's retained `Style` directly, because
  `ElementBuilder` already owns `style_` and builds `StyledElement`, flex,
  stack, and text variants from that data.
- Steps 96-98 should preserve the existing `View::paint(...)` contract while
  adding `View::render(ViewContext&)`; this lets current tests and demos keep
  compiling while the GPUI-like render path becomes available.
- Before implementing Step 92, the Step 91 feature worktree still needs the
  post-merge WSL verification/cleanup on `master`.

## 2026-06-30 Element Child Overloads

- `AnyElement` is already `std::unique_ptr<Element>`, so the existing
  base-ownership overload is the public `AnyElement` path; Step 91 only needs
  extra overloads for builder materialization and typed derived ownership.
- `.child(ElementBuilder)` should call `into_element(...)` internally, keeping
  all builder-to-owned conversion behavior in the same public convention added
  in Step 89.
- The typed ownership template should exclude `Element` itself so normal
  `std::unique_ptr<Element>` and `AnyElement` calls continue using the existing
  non-template overload and overload resolution stays predictable.

## 2026-06-30 Element Authoring Factories

- Free factories should return `ElementBuilder`, not `AnyElement`, because the
  existing fluent builder methods still need to compose before ownership is
  materialized through `into_element(...)` or `build()`.
- `div()` maps directly to the current styled-box builder, `h_flex()` and
  `v_flex()` map to row/column flex builders, `v_stack()` maps to the stack
  builder, and `text(TextModel&)` preserves the existing non-owning text-model
  binding convention.
- Keeping these helpers in `element.hpp` makes them visible through both
  `cgpui/ui/element.hpp` and the public prelude without creating another
  authoring header before the child-conversion overloads land.

## 2026-06-30 AnyElement Authoring

- `AnyElement` can be a public alias for the existing
  `std::unique_ptr<Element>` ownership model, which keeps Step 89 source
  compatible with all current tree, builder, and runtime installation APIs.
- `into_element(ElementBuilder)` should build by value from the existing
  rvalue-qualified `ElementBuilder::build()` path; this establishes the common
  conversion convention without adding Step 90 factory helpers yet.
- Keeping `into_element(AnyElement)` as the owned-element identity overload
  gives later child overloads and free factories one public spell to target
  while avoiding reference-counted ownership or type erasure churn.

## 2026-06-30 128-Step Planning

- The next highest-leverage path toward a GPUI-like core API is authoring
  ergonomics first: `AnyElement`, free element factories, child conversion, and
  fluent style shortcuts make later demos and tests express the intended public
  surface instead of the current lower-level builder plumbing.
- View/model/app lifecycle should follow once authoring helpers exist, because
  `View::render(ViewContext&)`, model update invalidation, and app window
  opening all need a stable element-returning convention.
- Renderer and platform work should remain command- and abstraction-driven:
  add paint/text/clip/cursor commands at the UI/runtime boundary before
  teaching Vulkan, Win32, or Wayland to consume them.
- Full macOS parity is still not part of Steps 89-128; each new API should stay
  Mac-neutral and avoid introducing fresh Vulkan/Win32/Wayland assumptions into
  public core headers.

## 2026-06-30 Mac Readiness Audit

- The repository already has macOS source slots:
  `src/platform/macos/macos_application.mm` creates a Cocoa `NSWindow` backed
  by `CAMetalLayer`, and `src/renderer/metal/metal_renderer.mm` accepts
  `MetalSurfaceHandle`.
- `xmake.lua` already gates macOS platform and Metal renderer sources under
  `is_plat("macosx")`, so Step 88 does not need to move files or alter
  Windows/Linux build branches.
- The useful boundary improvement is a small public target map:
  Windows/Win32 -> Vulkan, Linux/Wayland -> Vulkan, and macOS/Cocoa -> Metal.
- Full Mac parity is still intentionally out of scope for this slice; the
  next Windows/Linux API steps should stay renderer-neutral above the existing
  `Renderer` and `NativeSurfaceHandle` abstractions.

## 2026-06-29 Baseline

- Current UI runtime has a single root `View`, a stable root `ViewId{1}`, input snapshots, keyboard focus owner, and pointer capture owner.
- `View::handle_event` currently returns `void`; changing this to an event result is the next routing prerequisite.
- Existing tests are concentrated in `tests/ui/window_runtime_test.cpp` and can verify runtime behavior without real platform windows.
- The main worktree has a pre-existing untracked `.vscode/` directory that should remain untouched.

## 2026-06-29 Event Dispatch Observability

- A dispatch observation can stay entirely in the UI runtime layer: no Win32,
  Wayland, or Vulkan platform changes are needed.
- Runtime lifecycle events handled internally by `WindowRuntime` should not
  produce view dispatch records.
- `WindowRuntimeContext::last_event_dispatch` intentionally reports the
  previous view-dispatched event while a view is handling the next event; the
  after-event callback sees the current record after it has been stored.

## 2026-06-29 Event Routing Shell

- A root-only router is enough for the current single-view runtime while
  establishing the API shape that later element/view routing can replace.
- `WindowRuntimeContext::event_route` is current during `View::handle_event`,
  while `last_event_dispatch` remains the previous completed dispatch until the
  view returns.
- `EventDispatchRecord` now duplicates `view_id`/`event_kind` as compatibility
  fields and stores the full `EventRoute` for future routing expansion.

## 2026-06-29 Entity Store Skeleton

- The entity store can live in `cgpui/core/entity.hpp` as a header-only
  template without adding a source file to `cgpui_core`.
- Monotonic, non-reused ids keep the first skeleton simple and avoid stale-id
  aliasing until a future generation/index scheme is justified.
- A typed `EntityId<T>` gives compile-time separation between stores while
  preserving a simple numeric `.value` for tests and diagnostics.

## 2026-06-30 Context Entity Access

- `WindowRuntimeContext` is acting as a capability handle: it can stay `const`
  while still forwarding controlled mutations into runtime-owned state.
- `WindowRuntime` can hold per-type entity stores behind
  `std::unordered_map<std::type_index, std::any>` without adding new source
  files or platform-specific code.
- Missing or removed entities should stay soft-fail APIs for now:
  read/mutate return `nullptr`, and remove returns `false`.

## 2026-06-30 View Identity Allocation

- Step 6 only needs identity allocation, not a view tree or reconcile pass.
- Keeping `ViewId{0}` invalid, root at `ViewId{1}`, and runtime allocation
  from `ViewId{2}` gives later element/view storage a simple stable id source.
- A runtime-local monotonic counter is enough until later steps add removal,
  reconciliation, or generation/index semantics.

## 2026-06-30 Element Base API

- `ElementId` should stay separate from `ViewId`; element tree ids and view
  routing ids are related later, but they are not the same type at this layer.
- `Element` should not allocate its own ids. The next tree/reconcile steps can
  assign ids through `assign_id()` and preserve them across rebuilds.
- Keeping the base class free of layout/event methods avoids coupling Step 7 to
  the later layout and routing slices.

## 2026-06-30 Element Tree Container

- `ElementTree` can remain header-only for now because it stores generic
  `Element` ownership and simple relationship vectors.
- Replacing the root clears existing node relationships but keeps monotonic id
  allocation moving forward, which avoids stale-id aliasing.
- The container intentionally rejects unknown parents softly with
  `ElementId{0}`; richer diagnostics can wait until tree construction APIs are
  more expressive.

## 2026-06-30 Element Reconcile Pass

- A parent-local index reconcile is enough for the first pass because Step 8
  already records ordered children.
- Reconcile should replace element instances while preserving their assigned
  ids; this gives later layout/routing steps stable ids across simple rebuilds.
- Keyed diffing, pruning, and type-aware compatibility checks are intentionally
  outside this slice.

## 2026-06-30 Layout Primitives

- Layout primitives can live in a separate `cgpui/ui/layout.hpp` header so the
  element tree does not grow layout responsibilities before Step 11.
- Using zero min constraints and infinite max constraints gives unconstrained
  layout callers a simple default while preserving explicit min/max tests.
- `constrain_size` is intentionally only a per-axis clamp helper; element layout
  methods, bounds storage, and traversal remain later slices.

## 2026-06-30 Fixed Size Layout

- Adding layout as a virtual method on `Element` gives later stack and routing
  steps a common leaf/container contract without adding tree traversal yet.
- The base element layout returns constrained zero size, which keeps existing
  test-only elements usable while giving constraints a visible effect.
- `FixedSizeElement` is deliberately leaf-only: it stores a preferred size and
  delegates all min/max behavior to `constrain_size`; bounds persistence and
  child layout remain future slices.

## 2026-06-30 Vertical Stack Layout

- A self-contained `VerticalStackElement` is enough for Step 12; it avoids
  coupling layout traversal to `ElementTree` before bounds and hit testing
  exist.
- The first stack pass can lay out children with default unconstrained
  `LayoutInput` and only clamp the final stack size; constraint propagation can
  become a later refinement when richer layout rules exist.
- The stack intentionally does not persist child origins or bounds yet, because
  Step 13 is the first slice that needs retained layout geometry.

## 2026-06-30 Hit Testing

- Retained bounds can stay on `Element` as mutable state updated by const
  `layout()` calls, matching the existing const layout API without adding a
  renderer or runtime dependency.
- `hit_test(Point)` should soft-fail with `ElementId{0}` before layout or
  outside the retained bounds, which keeps Step 14 pointer routing simple.
- `VerticalStackElement` can record direct child bounds while laying out owned
  children and use child-first hit testing; richer nested coordinate propagation
  can wait until element-tree traversal or more complete layout context exists.

## 2026-06-30 Pointer Hit Routing

- The existing event routing shell can carry an optional `ElementId` without
  changing root-view delivery yet.
- A non-owning runtime element root is enough for this slice because ownership,
  reconcile, and layout scheduling are still outside the runtime.
- Pointer capture should not be folded into hit routing yet; Step 15 can decide
  when capture should override a fresh hit-test result.

## 2026-06-30 Pointer Capture Routing

- Element-level capture can coexist with the older root-view capture state by
  storing a separate optional `ElementId` owner.
- Capture should override fresh hit testing only for pointer events, preserving
  non-pointer routes.
- Owner-matched release keeps the API soft-fail and mirrors the existing
  view-owner release behavior.

## 2026-06-30 Keyboard Focus Routing

- Element-level keyboard focus can reuse `EventRoute::target_element_id`, just
  like pointer routing, because the route already represents the event's
  intended element target.
- Keyboard and text-input events should use the focused element owner before
  any pointer-specific routing logic; pointer hit testing and pointer capture
  remain unaffected.
- Keeping view-level and element-level focus owners as separate optional fields
  preserves old root-view focus tests while exposing the element owner needed
  for later key binding and text-input slices.

## 2026-06-30 Action Command Primitive

- A runtime-local action handler table is enough for the first primitive and
  avoids a global registry before key binding or view trees exist.
- Reusing `EventResult` for action handlers keeps consumed/cancelled semantics
  aligned with event handling.
- Recording the last `ActionDispatchResult` gives Step 18 a simple observable
  target when key bindings start dispatching named actions.

## 2026-06-30 Key Binding Table

- A runtime-local vector of bindings is enough for exact key/modifier matching
  in the first slice; scoped binding tables and priorities can wait.
- Dispatching the matched action before root view event handling gives action
  handlers access to the current `EventRoute` while preserving existing view
  event delivery.
- Matching only `KeyboardKey` events keeps text input independent until the text
  model and focused-text routing slices land.

## 2026-06-30 Text Model

- A byte-offset cursor keeps the first text model small while still allowing
  UTF-8-safe codepoint navigation by skipping continuation bytes.
- The source test should stay ASCII on Windows; UTF-8 examples can be expressed
  with byte escapes to avoid MSVC code-page warnings.
- `backspace()` deletes the codepoint before the cursor; the test originally
  moved the cursor to offset 0 before expecting a successful second backspace,
  which was a test expectation bug rather than an implementation bug.

## 2026-06-30 Text Input Routing

- Binding text models by `ElementId` lets the Step 16 keyboard-focus route
  become actionable without introducing element event handlers yet.
- Runtime-owned text model pointers should remain non-owning in this slice; the
  owning element/view layer does not exist yet.
- The text-input routing test must fire the third key event before expecting
  the existing helper to release element keyboard focus; otherwise the second
  text input correctly still targets the model.

## 2026-06-30 40-Step Milestone

- Step 21 should introduce style as inert data first, not layout or painting
  behavior, so later builder, layout, and paint-tree steps have a stable shared
  vocabulary.
- Style primitives can stay header-only in `include/cgpui/ui/style.hpp` and
  reuse `Color` from `core/geometry.hpp` to avoid duplicating color types.
- A single `EdgeSizes` structure is enough for padding/border-width symmetry
  and keeps Step 24 padding layout and Step 25 border primitives aligned.

## 2026-06-30 Element Builder API

- `StyledElement` can be introduced as an inert wrapper first: it owns style
  data and an optional child, but does not paint backgrounds or apply padding
  until the dedicated Step 23 and Step 24 slices.
- Keeping `ElementBuilder::build()` returning `std::unique_ptr<Element>` matches
  the existing tree/container APIs and avoids introducing reference-counted
  element ownership prematurely.

## 2026-06-30 Styled Background Paint

- Element-level painting can reuse the existing `PaintList` command vocabulary
  without changing renderer or platform code.
- Keeping `Element::paint(PaintList&)` as a no-op default preserves existing
  test elements while letting styled/container elements opt into paint output.
- `StyledElement` should emit its background before painting its child, matching
  normal parent-background-then-content ordering and setting up Step 26 paint
  tree traversal.

## 2026-06-30 Styled Padding Layout

- Padding can be handled inside `StyledElement::layout` before a separate box
  model abstraction exists: content size comes from the child or preferred size,
  then top/right/bottom/left padding expands the outer size.
- Child bounds should be rewritten after the child layout pass so hit testing
  and later paint traversal observe the padded child origin.
- Existing layout constraints in this codebase clamp container output after
  child measurement, so this slice keeps the same simple constraint behavior.

## 2026-06-30 Border Style Primitives

- Border color and radii can be introduced as inert `Style` data without
  touching layout or paint yet; Step 26/27 can decide how much of the box model
  to materialize in paint commands.
- `BorderRadii` mirrors `EdgeSizes` as a compact POD helper, keeping top-left,
  top-right, bottom-right, and bottom-left explicit for later rounded-rect work.
- `Style::border_width` already existed from Step 21, so this slice only needed
  color and radius fields plus fluent setters.

## 2026-06-30 Element Paint Tree

- `ElementTree::paint(PaintList&)` can be a thin recursive traversal over the
  existing parent/child relationship data; no renderer or platform changes are
  needed for this slice.
- Painting an element before its children preserves the Step 23 parent
  background before content ordering.
- Sibling paint order follows the stored child id vector, matching append and
  reconcile order until a later z-order primitive changes that policy.

## 2026-06-30 Clip Overflow Primitives

- Clip and overflow can start as inert `Style` data; applying clip stacks to
  paint traversal is a later renderer/painter concern.
- `Overflow::visible` as the default preserves all existing layout, hit-test,
  and paint behavior until explicit hidden overflow is requested.
- Keeping `clip_rect` optional distinguishes "no authored clip" from an
  authored zero-size clip rectangle, which will matter when paint traversal
  starts honoring clipping.

## 2026-06-30 Z Order Paint Order

- `Style::z_index` is the smallest API surface for authored paint ordering
  because `StyledElement` already owns style data and `ElementTree::paint`
  already controls sibling traversal.
- Sorting only same-parent children keeps the existing parent-before-children
  invariant from Step 26 while making sibling paint order deterministic.
- `std::stable_sort` preserves insertion/reconcile order for equal z-index
  siblings, so existing append-order paint tests remain meaningful.

## 2026-06-30 Hover State Tracking

- Hover can be modeled as input state first: no hover enter/leave event types
  are needed until element handlers exist.
- Pointer capture should override event routing, not the physical hover target;
  the runtime therefore computes hover from the live pointer hit test before
  applying capture to `EventRoute`.
- Clearing hover on pointer-move misses keeps stale element ids from surviving
  after the pointer leaves the current element root.

## 2026-06-30 Cursor Shape State

- Cursor shape can start as runtime-visible input state before platform cursor
  handles are wired into Win32 and Wayland.
- Binding cursor shapes by `ElementId` composes directly with the Step 29 hover
  state and avoids inventing element event handlers in this slice.
- Restoring `CursorShape::default_arrow` on hover misses prevents stale cursor
  state from outliving the hovered element.

## 2026-06-30 Scroll State Model

- The first scroll primitive can remain UI-layer and header-only: it only needs
  viewport size, content size, and a clamped offset.
- Offsets should reclamp whenever viewport or content size changes so stale
  scroll positions cannot survive after content shrinks.
- Axis scrollability is derived from positive max offset, not from the current
  offset, which keeps `can_scroll_x`/`can_scroll_y` useful at both ends of a
  scroll range.

## 2026-06-30 Flex Layout Basics

- `FlexElement` can start as a sibling to `VerticalStackElement` rather than a
  replacement: it keeps row/column semantics explicit without disturbing the
  existing vertical-stack tests.
- The first flex slice should only measure natural child sizes and clamp the
  container output; grow, shrink, gap, align, and justify behavior can remain
  future work.
- Reusing the same child-first hit testing pattern as `VerticalStackElement`
  keeps routed pointer behavior consistent once flex elements are installed as
  runtime element roots.

## 2026-06-30 Runtime Invalidation

- Runtime invalidation can start as observable dirty state without scheduling
  redraws yet; Step 34 can connect dirty state to platform redraw requests.
- A layout invalidation should imply paint invalidation because a layout pass
  changes paint bounds, while a paint invalidation alone should not imply
  layout work.
- Clearing invalidation explicitly keeps tests and future frame processing able
  to model "dirty work consumed" separately from "new work requested".

## 2026-06-30 Update Scheduling

- Invalidation-triggered redraw should be deferred until after view event and
  after-event callbacks finish; the test fake window synchronously emits redraw
  events, so immediate requests would reenter the runtime too early.
- A single `redraw_scheduled_` flag is enough to coalesce layout and paint
  requests made during the same event into one platform redraw request.
- Successful redraw should clear both invalidation and scheduling state so
  later model/view changes can schedule the next frame cleanly.

## 2026-06-30 View Model Subscriptions

- A first subscription skeleton can be type-erased with `std::type_index` plus
  the typed `EntityId<T>::value`; this keeps the API typed while avoiding a
  full dependency graph.
- Querying subscriptions by `ViewId` is enough for tests and future debugging;
  notification can scan the small vector until a richer storage model is
  needed.
- Entity-change notification should request layout rather than paint only,
  because model changes may affect both layout and paint output.

## 2026-06-30 Text Selection Range

- Text selection can live entirely in `TextModel` as byte offsets for this
  slice, matching the existing byte-offset cursor while preserving UTF-8-safe
  movement helpers.
- Storing anchor and head separately gives Step 37 enough state for
  Shift+movement behavior, while `selection()` exposes a normalized start/end
  range for edit operations.
- Plain insertion should replace a non-collapsed selection and then collapse to
  the inserted text end, matching typical text editing behavior without adding
  platform clipboard or IME coupling yet.

## 2026-06-30 Text Edit Actions

- A small `TextEditAction` enum keeps keyboard editing semantics in the
  platform-neutral text model before binding them to Win32 or Wayland key
  events.
- Selection extension should preserve the original anchor and move only the
  head/cursor, which gives later Shift+Arrow routing the right primitive.
- `backspace()` and `delete_forward()` should erase a non-collapsed selection
  before falling back to adjacent-codepoint deletion, so direct text-model calls
  and edit-action dispatch behave consistently.

## 2026-06-30 Clipboard Abstraction

- The first clipboard slice can be a platform-layer text contract, independent
  from `PlatformApplication`, so the API is usable in tests and can later be
  backed by Win32 or Wayland system clipboard protocols.
- A shared `MemoryClipboard` implementation is enough for the Step 38 skeleton;
  platform-specific system integration can replace the factory implementation
  without changing callers.
- Keeping the factory in `cgpui_platform` instead of the Win32/Wayland leaf
  targets avoids duplicate definitions when examples link both shared platform
  code and a platform-specific implementation.

## 2026-06-30 IME Composition Skeleton

- IME composition should be a separate platform event from `TextInput` because
  update/preedit text is transient while commit text mutates the text buffer.
- Composition events should use the same keyboard-focus element route as key
  and text-input events; this lets Win32 and Wayland feed future IME messages
  into the focused text model without inventing a parallel routing path.
- The first text-model composition state can stay as one optional UTF-8 string:
  update replaces the preedit text, commit inserts it through normal text
  insertion, and cancel clears it without changing the underlying buffer.

## 2026-06-30 New API Demo

- The demo can exercise the new API surface without waiting for text shaping:
  it paints element rects, a cursor rect derived from `TextModel::cursor`, and a
  composition underline derived from the IME composition state.
- Runtime installation can stay non-owning for Step 40 by passing the demo's
  owned root element to `WindowRuntime::set_element_root`; later runtime-owned
  element-tree steps can replace that with a safer ownership model.
- `CGPUI_DEMO_INJECT_TEXT` gives the demo a scripted state-change path for
  smoke tests while keeping real interactive behavior available in normal runs.

## 2026-06-30 ElementTree Root Layout

- `ElementTree::layout_root` is intentionally a root helper, not a full tree
  traversal: existing container elements still own their child layout behavior.
- Empty-tree layout should return constrained zero size, matching base element
  layout semantics and giving callers a safe no-root path.

## 2026-06-30 ElementTree Root Hit Testing

- `ElementTree::hit_test_root` mirrors `layout_root`: it is a safe root-level
  convenience, while element/container implementations still define hit-test
  traversal and child priority.
- Empty-tree hit testing should return `ElementId{0}` so runtime routing can
  distinguish "no element root" from a valid target.

## 2026-06-30 Runtime-Owned Element Tree

- Runtime can support both the old non-owning `set_element_root` path and the
  new owning `set_element_tree` path by making them mutually exclusive; this
  avoids stale pointers when ownership changes.
- Pointer routing should use `ElementTree::hit_test_root` when an owned tree is
  installed, preserving the legacy `Element::hit_test` path for external roots.

## 2026-06-30 Runtime-Owned Tree Layout

- Redraw is the first safe place to auto-layout an owned element tree because
  the runtime has current framebuffer/viewport size and can update retained
  bounds before later pointer events rely on hit testing.
- Using viewport size as the max layout constraint preserves existing element
  min defaults while preventing oversized preferred roots from exceeding the
  drawable area.

## 2026-06-30 Element Event Hook

- `EventResult` belongs in the element-visible API once elements can handle
  events; keeping `ui.hpp` as a consumer avoids duplicating result semantics.
- The first `ElementEventContext` only needs the routed target element id. View
  runtime/application access can be layered later through a richer context.
- `StyledElement` forwarding to its child preserves wrapper composition until
  styled boxes grow their own interaction behavior.

## 2026-06-30 Runtime Element Event Dispatch

- Runtime event dispatch can treat element handling as the first phase and the
  existing root view as fallback, preserving old view behavior for unhandled
  element events.
- Owned `ElementTree` can resolve arbitrary routed ids with `get(id)`, while
  the legacy non-owning root can only safely dispatch to the root id itself.
- `EventDispatchRecord::result` should represent the final handler result,
  whether that result came from an element or from the fallback view.

## 2026-06-30 Element Builder Flex Helpers

- `ElementBuilder` can grow a small kind discriminator without changing the
  existing `box()` call sites; `box()` still builds a `StyledElement`.
- Row and column helpers should build concrete `FlexElement` containers so
  existing flex layout, hit testing, and future gap support remain centralized.
- Keeping box as a single-child wrapper preserves the earlier styled-box API
  while allowing row/column helpers to accept multiple chained children.

## 2026-06-30 Element Builder Stack Helper

- The same builder kind discriminator used for flex helpers can cover
  `v_stack()` without changing `VerticalStackElement` itself.
- Building a concrete `VerticalStackElement` keeps older stack-specific tests
  and future stack-only gap behavior independent from generic flex containers.
- The builder's accumulated child vector is now the shared path for multi-child
  container helpers.

## 2026-06-30 Element Builder Fixed-Size Helper

- `fixed_size(Size)` fits the existing builder kind model as a leaf builder,
  not a styled box variant.
- Building a concrete `FixedSizeElement` preserves the existing constraint and
  retained-bounds behavior without adding a second preferred-size path.
- The helper intentionally ignores accumulated children because fixed-size
  elements are leaf nodes in the current element model.

## 2026-06-30 Gap Style Layout

- `Style::gap` can stay as a single main-axis spacing primitive for now:
  vertical stack and flex column apply it on Y, while flex row applies it on X.
- Applying gap only when `child_index > 0` preserves existing zero-gap layout
  and avoids adding trailing space after the final child.
- The existing builder `style(...)` hook is enough to author container gap for
  `row`, `column`, and `v_stack` without wrapping those multi-child containers
  inside an extra `StyledElement`.

## 2026-06-30 Margin Style Layout

- Reusing `EdgeSizes` for `Style::margin` keeps the box-model vocabulary aligned
  with padding and border width without adding another edge type.
- Margin is modeled as outer size in `StyledElement::layout`; existing no-margin
  tests preserve prior behavior because the default edge values are zero.
- Child layout bounds should include the margin offset before padding so later
  hit testing and paint commands see content positioned inside the outer box.

## 2026-06-30 Border Paint Emission

- Border rendering can reuse the current `SolidRect` paint command vocabulary:
  styled borders are four filled rects emitted after background paint.
- Requiring `border_color` before emitting border rects preserves the previous
  inert behavior for authored widths that do not yet specify a color.
- Keeping side rects between top and bottom avoids double-painting corners while
  still giving Vulkan/Wayland and Win32 the same command stream shape.

## 2026-06-30 Overflow Clip Metadata

- Clip handling can start as paint-command metadata: renderers can continue
  drawing `SolidRect` unchanged while future renderer slices learn to honor the
  optional `PaintCommand::clip_rect`.
- A simple `PaintList` clip stack is enough for nested styled elements and keeps
  clip state out of individual element implementations beyond push/pop scope.
- Hidden overflow should use the element's retained layout bounds by default,
  while authored `Style::clip_rect` can override the metadata rect when present.

## 2026-06-30 Text Element Skeleton

- The first `TextElement` can stay in the element layer as a non-owning
  `TextModel*` binding; runtime-owned text-model routing already uses the same
  lifetime convention.
- A fixed 8x16 skeleton metric is enough to make layout, hit testing, and paint
  observable without introducing shaping, fonts, glyph atlases, or renderer text
  commands in this slice.
- Paint can emit a placeholder `SolidRect` through the existing paint path so
  Win32/Vulkan and Wayland/Vulkan remain unchanged until a later text renderer
  command is justified.

## 2026-06-30 Runtime Text Edit Actions

- Runtime text edit routing can mirror key bindings as a platform-neutral
  binding table from `KeyboardKey` data to `TextEditAction`, avoiding hard-coded
  Win32 or Wayland key assumptions in the text model.
- The existing element keyboard-focus owner is the right target selector:
  keyboard events already route to it, and text input/IME routing already look
  up text models by that focused `ElementId`.
- Matching edit bindings before view fallback keeps the model updated before
  observers inspect the event, while leaving unbound keys and missing text
  models as no-op cases for compatibility.

## 2026-06-30 Runtime Clipboard Paste

- Runtime clipboard paste can use the existing platform-neutral `Clipboard`
  interface without introducing Win32 or Wayland system clipboard details into
  the UI layer.
- Keeping the clipboard pointer non-owning matches the runtime's current
  non-owning `TextModel*` binding convention and lets tests inject
  `MemoryClipboard` directly.
- Paste should target the focused element's bound `TextModel`, so it composes
  with the same keyboard-focus route used by text input, IME, and edit actions.

## 2026-06-30 Runtime Clipboard Copy

- Copying selection text should live on top of `TextModel` selection state
  rather than duplicating byte-range normalization in the runtime.
- The runtime clipboard copy path can share the same non-owning `Clipboard*`
  and focused-element text-model lookup introduced for paste.
- Collapsed selections should not overwrite clipboard contents; returning
  `false` gives callers a simple observable no-op.

## 2026-06-30 Runtime Clipboard Cut

- Cut can compose copy plus the existing selection-aware
  `TextModel::delete_forward`, which avoids adding a second selected-range
  deletion primitive in the runtime.
- The operation should only mutate text after clipboard write succeeds, so
  failed copy/no selection/no focused model leaves the buffer untouched.
- After deletion, existing `TextModel` behavior collapses the cursor to the
  removed selection start, matching the text-edit action path.

## 2026-06-30 ViewContext Convenience

- A type alias keeps the public `ViewContext` spelling GPUI-like without
  duplicating runtime context storage or breaking existing
  `WindowRuntimeContext` callbacks.
- Thin context methods are enough for common authoring paths: they forward to
  the existing runtime APIs and preserve all focus, capture, clipboard, and
  invalidation behavior.
- Invalidation convenience tests should observe state during view event
  handling, not after-event callbacks, because deferred redraw can flush and
  clear invalidation before `after_event` returns.

## 2026-06-30 App Runner Skeleton

- The first public runner can sit above `WindowRuntime` rather than replacing
  it: the runner owns the renderer, while `WindowRuntime` still owns event
  dispatch and redraw behavior.
- A setup callback is enough for this slice to let callers bind elements,
  models, actions, cursors, and callbacks before the platform event loop starts.
- Accepting an injected `PlatformApplication&` and renderer factory keeps the
  runner deterministic in tests and leaves default platform/renderer creation
  for a later convenience overload.

## 2026-06-30 Text Element Builder

- `ElementBuilder::text(TextModel&)` should mirror `TextElement`'s non-owning
  binding instead of copying text, so runtime text-model bindings and builder
  authored text share the same lifetime convention.
- Building text as a direct leaf keeps the initial API small; styling can stay
  in wrapper elements until text-specific font, color, and shaping primitives
  exist.

## 2026-06-30 Focusable Element Hook

- Focus activation belongs first at the element boundary: runtime click routing
  can query `Element::focusable()` and call `Element::focus(...)` without
  baking widget-specific behavior into `WindowRuntime`.
- Keeping the default element non-focusable preserves existing hit-test and
  event-routing behavior for passive layout and paint elements.

## 2026-06-30 Runtime Click Focus

- Runtime focus activation can reuse the already-computed event route: after
  pointer capture/hit-test routing chooses a target element, a left-button press
  is enough to request element keyboard focus and call its focus hook.
- Calling `focus(...)` before normal element event dispatch lets the current
  pointer event and the following keyboard event observe the updated focus
  owner through the existing `WindowRuntimeContext` shape.

## 2026-06-30 Element Enabled State

- Enabled/disabled state fits best on `Element` rather than `Style`: it is an
  interaction primitive first, and Step 65 can use the same base API to skip
  dispatch without inventing style-aware event routing.
- Builder-level `enabled(bool)` should apply to every concrete element kind so
  disabled state stays orthogonal to layout choice, text leaves, and styled
  wrappers.

## 2026-06-30 Disabled Element Event Gate

- Disabled state is now a routing/interaction gate rather than a hit-test gate:
  route metadata can still identify the hit element, but the disabled element's
  handler is skipped.
- View fallback still observes the original event route after a disabled routed
  element is skipped, which preserves existing dispatch diagnostics and lets
  parent/view-level behavior decide whether to consume the event.
- `StyledElement` treats a disabled child the same as no interactive child for
  event forwarding, returning unhandled without mutating child event state.

## 2026-06-30 ElementTree Preorder Traversal

- `ElementTree::preorder_ids()` should be structural, root first and then each
  stored child subtree in child-list order; it intentionally does not reuse
  paint order, which may sort siblings by z-index.
- Returning ids rather than element pointers keeps the helper simple,
  copyable, and compatible with later APIs that can call `get(id)` or filter by
  id without exposing internal nodes.
- Empty-tree traversal returns an empty vector, matching the existing soft-fail
  behavior of root layout, hit testing, and paint traversal.

## 2026-06-30 ElementTree Typed Lookup

- `ElementTree::find_as<T>(ElementId)` is a convenience over the existing
  id-based `get` contract, not a new ownership path; it returns null for
  missing ids and type mismatches.
- Providing both const and mutable overloads lets tests and future ViewContext
  helpers preserve const-correctness while avoiding repeated `dynamic_cast`
  boilerplate at call sites.

## 2026-06-30 ViewContext Text Model Binding

- ViewContext text binding is just a capability-forwarding helper over runtime
  text-model binding; it should not introduce a second ownership or lookup path.
- Binding during an event should be immediately visible to subsequent text input
  routed to the focused element, matching the existing runtime text input path.

## 2026-06-30 ViewContext Element Tree Installation

- ViewContext element tree installation should forward ownership directly into
  `WindowRuntime::set_element_tree`; the context remains a capability wrapper,
  not a second tree store.
- Installing a tree during an event composes with `request_layout()`: the
  deferred redraw lays out the owned tree before later pointer routing uses it.

## 2026-06-30 ViewContext Action Helpers

- Action registration and dispatch can live on `ViewContext` as thin forwards
  over the runtime action table, preserving the existing handler storage and
  last-dispatch observability.
- `ActionHandler` can be declared before `WindowRuntimeContext` is defined
  because it only needs the context by reference; this lets context methods use
  the public handler alias without adding another callback type.

## 2026-06-30 ViewContext Key Binding Helper

- Key binding through `ViewContext` can reuse the runtime binding table without
  changing keyboard dispatch order; a binding registered during one event is
  available to subsequent keyboard events.
- Keeping action registration and key binding as separate context helpers
  mirrors the runtime API and lets later authoring helpers compose them without
  adding a combined shortcut abstraction too early.

## 2026-06-30 ViewContext Text Edit Binding Helper

- Text-edit bindings registered from `ViewContext` can stay as direct forwards
  to the runtime binding table; current-key dispatch still happens before the
  view handler, so tests should assert effects on later key events.
- `TextModel(std::string)` already places the cursor at the end, which is enough
  for text-edit binding tests without adding a cursor-positioning helper.

## 2026-06-30 ViewContext Cursor Binding Helper

- Cursor binding through `ViewContext` should stay a thin capability forwarder
  over `WindowRuntime::set_element_cursor`; the runtime remains the single
  storage owner for element-to-cursor mappings.
- A cursor binding registered during one event is visible to later pointer
  hover routing, matching the existing runtime cursor state path and preserving
  platform-neutral behavior for Windows and Wayland.

## 2026-06-30 ViewContext Element Focus Helpers

- Short `ViewContext::focus(ElementId)` and `blur(ElementId)` helpers can layer
  over the existing element keyboard-focus request/release APIs without changing
  routing semantics.
- Keeping the long `request_keyboard_focus(ElementId)` APIs alongside the short
  helpers preserves compatibility while giving author code a more GPUI-like
  spelling for common element focus flows.

## 2026-06-30 ViewContext Pointer Capture Element Helpers

- Element pointer capture should be authorable with an `ElementId` overload on
  `ViewContext`, while the runtime continues storing the existing
  `PointerCaptureOwner` tagged owner.
- The overloads are intentionally additive: existing `PointerCaptureOwner`
  calls remain available for view-level capture and explicit owner matching.

## 2026-06-30 Element Builder Click Handler

- Builder-level click handling can be implemented as an opt-in wrapper element,
  preserving existing dynamic-cast expectations for elements built without a
  click handler.
- The wrapper should preserve child layout, paint, and hit-test behavior, while
  consuming pointer-press handling through the attached handler before falling
  back to the child for other events.
- Wrapper elements created by builder helpers must mirror the builder's
  enabled state on the wrapper itself, not only on the wrapped child, because
  direct event dispatch sees the outer element first.

## 2026-06-30 Element Builder Focusable Helper

- Builder-level focusability can also be an opt-in wrapper, preserving existing
  concrete element types unless the helper is requested.
- Wrapper composition order matters: behavior wrappers like click handling need
  to stay inside capability wrappers like focusability so `focusable()` remains
  visible on the built element while event handling still reaches the click
  handler.

## 2026-06-30 Element Builder Key Handler

- Element-level key handling can use the same opt-in wrapper pattern as click
  handling, but it should accept the concrete `KeyboardKey` alongside the
  element event context so author code does not need to inspect the variant.
- Key and click handlers should remain behavior wrappers inside focusability,
  preserving `focusable()` visibility when helpers are composed.

## 2026-06-30 Element Builder Disabled Helper

- `ElementBuilder::disabled()` is pure authoring sugar over `enabled(false)`;
  keeping it as a flag flip lets the existing wrapper enabled propagation
  handle composed elements consistently.

## 2026-06-30 ElementTree Enabled Traversal

- `enabled_preorder_ids()` filters by each node's current `enabled()` state at
  push time but still traverses descendants, which makes it a general enabled
  query rather than a disabled-subtree pruning policy.

## 2026-06-30 Disabled Focus Activation

- Disabled elements remain routable and observable through hit testing, but
  runtime click-to-focus activation should use the same interaction gate as
  element event dispatch: the element must be both enabled and focusable.

## 2026-06-30 Disabled Hover Cursor

- Hover state should continue to describe the physical hit target, but cursor
  shape is an interaction affordance and should only be driven by enabled
  elements when the runtime can resolve the hovered id to a live element.
- Legacy non-owning roots can hit-test child ids that `routed_element()` cannot
  safely resolve, so cursor lookup keeps the old behavior for unresolved ids
  and only suppresses cursor bindings for resolved disabled elements.

## 2026-06-30 Focused Text Model Lookup

- The runtime already treats element keyboard focus as the owner selector for
  text input, IME, text edit actions, and clipboard operations, so a public
  `focused_text_model()` helper can be the single soft-fail lookup for that
  binding.
- Clipboard paste/copy/cut can reuse the helper directly, reducing duplicate
  `keyboard_focus_element_owner_` and `text_models_` lookup logic before
  ViewContext grows focused-text mutation helpers.

## 2026-06-30 ViewContext Focused Text Mutation

- A focused-text mutation helper fits the existing capability-forwarding
  `ViewContext` style: it looks up the runtime's focused text model, invokes an
  author callback when present, and returns false for no callback or no focused
  model.
- Tests for the helper should use key-only event sequences because generic text
  input routing tests intentionally insert text between keyboard events.

## 2026-06-30 Public Prelude Header

- `include/cgpui/cgpui.hpp` can stay as a simple aggregate header over the
  existing module headers for now; no new source target or platform-specific
  selection is needed for this authoring convenience.
- Header-cleanliness coverage should instantiate a concrete `View`, which means
  the test fixture must implement both `paint` and `handle_event` from the
  current public `View` contract.
- The prelude intentionally exposes the existing API surface without changing
  ownership or runtime behavior, keeping this slice an additive source
  compatibility improvement.

## 2026-06-30 ViewContext Event Route Helper

- `ViewContext::current_event_route()` should be a snapshot accessor over the
  existing `event_route` field, not a live recomputation from `WindowRuntime`.
- Keeping the public field and adding the helper preserves compatibility while
  giving author code a more intentional GPUI-like route lookup spelling.
- The helper is valid in both view event handlers and after-event callbacks
  because `WindowRuntime::context()` already snapshots `current_event_route_`
  into the context object for both paths.

## 2026-06-30 ViewContext Input State Helper

- `ViewContext::input_state()` should mirror the existing `input` snapshot
  field, preserving the event-time view of focus, pointer, hover, and cursor
  state instead of querying mutable runtime state later.
- Returning the snapshot by value keeps the helper simple and avoids exposing a
  mutable alias into the context object.
- The helper is intentionally additive: existing direct `context.input` access
  remains available while author code gains a clearer accessor spelling.

## 2026-06-30 Steps 129-168 Follow-On Planning

- Steps 129-168 should remain queued behind completion of Steps 99-128 unless
  the roadmap is explicitly reprioritized; Step 129 depends on the model,
  window, route, text, platform, and demo surfaces that Steps 99-128 still need
  to land.
- The post-Step-128 queue is best treated as four 10-step product bands:
  context/entity/global/async ergonomics, keyed reconciliation and reusable
  widgets, text/font/renderer maturity, then Windows/Wayland platform closure
  and parity documentation.
- Step 168 should not claim full GPUI parity. Its exit condition is a practical
  Windows/Linux GPUI-core foundation plus an explicit audit of implemented,
  partial, missing, and Mac/Metal-deferred areas.

## 2026-06-30 Model Entity Aliases

- `Model<T>` and `Entity<T>` should remain pure public authoring aliases over
  `EntityId<T>` at this stage; introducing wrapper objects before Step 100/101
  would create ownership and upgrade semantics before the plan has the model
  helpers and weak handles to support them.
- Header-cleanliness coverage belongs in both `core_header_cleanliness` and the
  public prelude because the aliases are part of the low-level entity API and
  also need to be available to normal GPUI-like authoring code through
  `cgpui/cgpui.hpp`.
- Reusing `EntityId<T>` preserves the existing typed-id separation between
  unrelated model/entity payload types and keeps `EntityStore<T>` monotonic id
  behavior unchanged.

## 2026-06-30 ViewContext Model Helpers

- `new_model`, `read_model`, `update_model`, and `remove_model` fit best as
  header-only `ViewContext` authoring helpers over the existing entity store;
  this keeps Step 100 additive and avoids adding a second model storage path.
- `update_model` should soft-fail when the model id is missing, and should only
  notify model changes after it finds the model and runs the author callback.
  That lets subscribed views receive layout/paint invalidation without making
  failed updates schedule redraws.
- `remove_model` can delegate to existing entity removal semantics, including
  notifying subscribed views only when an actual model is removed.

## 2026-06-30 Back-40 Plan Boundary

- The "后40步" plan is Steps 129-168, not a replacement for the active
  Steps 101-128 queue. Step 129 remains gated on Step 128 being merged and
  verified on both Windows and WSL Arch Linux.
- The post-Step-128 work should keep the current four-band shape:
  context/entity/global/async first, keyed reconciliation/widgets second,
  text/font/renderer maturity third, and Windows/Wayland platform closure plus
  parity audit last.
- The exit condition for Step 168 is a practical Windows/Linux GPUI-core
  foundation with an explicit audit of completed, partial, missing, and
  Mac/Metal-deferred areas; it should not claim full upstream GPUI parity.

## 2026-06-30 Weak Entity And View Handles

- `WeakEntity<T>` should remain a lightweight typed-id wrapper at this stage.
  Upgrade behavior belongs on `WindowRuntime`/`ViewContext`, where the runtime
  can check whether the model/entity store still contains the id.
- `WeakView` can mirror that shape for `ViewId`; the current liveness check is
  `is_view_id_allocated(...)`, which matches the existing monotonic view-id
  allocation model until Step 107 introduces a real multi-view registry.
- Step 101 intentionally does not add observer ownership, subscription tokens,
  or entity handle convenience methods. Those remain Step 102/133/130 work and
  would overbuild this slice.

## 2026-06-30 Back-40 Entry Contract Refresh

- The后 40 步 plan should stay an execution-ready queue for Steps 129-168, not
  an active implementation plan while Step 102-128 still have unresolved
  runtime, model, event, layout, renderer, platform, and demo work.
- The important boundary is the Step 128 handoff: Step 129 should start only
  after the public-prelude demo, model invalidation, view/app lifecycle,
  routing/focus/scroll/layout depth, text command metadata, cursor/clipboard,
  and IME geometry are all merged and verified on Windows and WSL Arch Linux.
- Since Step 102 already implements observation callbacks in its feature
  worktree, the next active implementation step is Step 103: make model
  updates/removals invalidate subscribed views automatically while preserving
  Step 102 observer callback semantics.

## 2026-06-30 Model Observe Helper

- Model observers should be public typed callbacks at the authoring boundary
  (`ModelObserver<T>`) and erased only inside the runtime storage layer, where
  `std::type_index` plus the raw entity id can match notifications without
  duplicating stores per model type.
- Observing a missing model should soft-fail and avoid registering the
  callback. That keeps stale ids from creating future callbacks that look
  alive but cannot read their model.
- Step 102 intentionally keeps automatic subscribed-view invalidation out of
  the observe helper. `notify_entity_changed(...)` still preserves the
  existing layout invalidation behavior, while Step 103 can formalize the
  model-driven subscribed-view invalidation contract without changing the
  observer callback shape.

## 2026-06-30 Model Update Invalidates Subscribed Views

- Once `View::render(ViewContext&)` is the GPUI-like element-tree authoring
  entry point, model/entity notifications for subscribed views need render
  invalidation, not only layout/paint invalidation. Otherwise a model update
  can schedule a frame without rebuilding the rendered element tree.
- `request_render()` is the narrowest existing runtime primitive for this
  contract because it already sets render, layout, and paint invalidation and
  schedules redraw. Reusing it keeps Step 103 small and avoids introducing a
  separate subscribed-view invalidation queue before the view registry lands.
- Observer callbacks remain part of `notify_entity_changed(...)`; Step 103
  changes the invalidation level after a match, while preserving Step 102's
  callback timing and missing-model soft-fail behavior.

## 2026-06-30 Back-40 Planning After Step 103

- The post-Step-128 "back 40" plan is Steps 129-168. It should remain a
  follow-on execution queue, not a replacement for the active Step 104-128
  work.
- Step 129 must stay gated on a clean Step 128 merge plus Windows and WSL Arch
  full debug verification, because the back-40 depends on the AppContext,
  WindowOptions, view registry, routing, layout, text command, cursor,
  clipboard, IME, and public-demo surfaces still scheduled in Steps 104-128.
- The current state for planning is Step 103 complete on `master` at
  `5949c84`, with Step 104 already opened in `.worktrees/app-context-wrapper`
  and baseline targeted tests passing.

## 2026-06-30 AppContext Setup Wrapper

- `AppContext` can start as a deliberately thin public setup wrapper containing
  `WindowRuntime& runtime`; this matches the current context-as-capability
  pattern without duplicating app/window ownership before Step 105.
- Keeping the existing `AppSetupCallback(WindowRuntime&)` field and adding an
  `AppContextSetupCallback` field preserves source compatibility for existing
  runner callers while exposing the GPUI-like setup spelling.
- Running the legacy runtime setup first and the new app-context setup second
  keeps existing behavior stable and gives callers a deterministic order when
  both callbacks are supplied.

## 2026-06-30 Back-40 Planning After Step 104

- The post-Step-128 "后40步" remains Steps 129-168, not a replacement for the
  active Step 105-128 execution queue. Starting Step 129 early would skip
  `WindowOptions`, root-view storage, view registry, child-view placeholders,
  routing ancestry, propagation, focus/scroll/layout depth, renderer text
  metadata, cursor/clipboard/IME surfaces, and the public demo rewrite.
- The next actionable implementation is already set up: continue
  `.worktrees/window-options-open-window` for Step 105 and add RED coverage for
  `WindowOptions` plus `AppContext::open_window(...)`. Recreating the Step 105
  worktree would be churn.
- The back-40 plan is useful as an execution profile now: 129-138 context and
  async, 139-148 keyed elements/widgets, 149-158 text/font/renderer maturity,
  and 159-168 Windows/Wayland closure plus the parity audit. Its entry gate
  stays Step 128 merged and Windows/WSL verified on a clean `master`.

## 2026-06-30 WindowOptions And Open Window Skeleton

- `WindowOptions` can start as a direct wrapper around `WindowDescriptor`.
  That keeps defaults (`CGPUI`, 1280x720) and platform-neutral descriptor
  mapping intact while adding the GPUI-like fluent authoring spelling needed by
  later app/window lifecycle steps.
- Step 105 should record app-opened window descriptors rather than create real
  platform windows. `run_app` still owns one `WindowRuntime`, one platform
  window, and one renderer; Step 106/159 can grow actual root-view and
  multi-window ownership without pretending this skeleton already owns extra
  native windows.
- The setup-time open-window record must remain visible after `run(...)`
  starts because `setup_context` runs before platform window creation. Clearing
  records at the start of `WindowRuntime::run(...)` would erase the only Step
  105 observable state and weaken the handoff to Step 106.

## 2026-06-30 Back-40 Planning After Step 112 GREEN

- The后 40 步 plan remains Steps 129-168 and should still wait behind the Step
  128 gate. Step 112 being GREEN in its feature worktree improves the distance
  estimate but does not open Step 129 yet because full verification, merge, and
  Steps 113-128 are still required.
- The immediate action is to finish Step 112 verification and merge from
  `.worktrees/scroll-element-binding`, then continue Step 113: wheel and
  trackpad scroll routing into bound scroll state.
- After Step 112 merges, the remaining route to the back-40 entry is 16
  implementation steps, Steps 113-128, plus post-Step-128 targeted, Windows
  full debug, and WSL Arch full debug verification on a clean `master`.

## 2026-06-30 Scroll Element Binding Merged

- `ScrollState` is intentionally an alias over `ScrollModel` at Step 112. That
  gives the public authoring API the GPUI-like name without duplicating scroll
  model storage or creating a second offset/update path before event routing
  lands in Step 113.
- `ScrollElement` binds a `ScrollState&` to one child, syncs viewport/content
  sizes during layout, and preserves child hit testing and event forwarding.
  Wheel/trackpad deltas are intentionally still Step 113 so the binding surface
  can be verified before runtime scroll routing mutates it.
- After Step 112 merged and post-merge verification passed, the active 128-step
  route is Step 113 through Step 128. The back-40 entry remains gated behind
  Step 128 and Windows/WSL full verification.

## 2026-06-30 Back-40 Planning After Step 115 Merge

- The后 40 步 plan remains Steps 129-168 and is still gated behind Step 128.
  Step 115 is now fully merged and post-merge verified, so the active
  pre-back-40 route is Steps 116-128 rather than Steps 115-128.
- From `master` at `c443592 feat: add flex alignment justification`, the
  effective distance to Step 129 is 13 implementation steps plus the
  post-Step-128 targeted, Windows full debug, and WSL Arch full debug
  verification. The distance through Step 168 is 53 implementation steps plus
  the Step 128 exit verification and four 10-step band checkpoint reviews.
- Step 116 should start from `.worktrees/flex-grow-shrink` on
  `codex/flex-grow-shrink` and should first add RED coverage for child flex
  grow/shrink factors affecting main-axis allocation while preserving default
  zero-grow behavior.

## 2026-06-30 Back-40 Planning After Step 121 Docs Closeout

- The后 40 步 plan remains Steps 129-168 and is still gated behind Step 128.
  Step 121 is now behavior-merged at `cf180f4 feat: add text paint command`
  and docs-closed at `dc010b6 docs: mark step 121 merged`, with post-merge
  targeted, Windows full debug, and WSL Arch Linux full debug verification
  already recorded.
- The active pre-back-40 route is exactly Steps 122-128 plus the
  post-Step-128 exit verification. Step 122 should continue the existing clean
  worktree `.worktrees/font-descriptor-font-size` on
  `codex/font-descriptor-font-size`; recreating that worktree would be churn.
- The后 40 步 planning target is four sequential bands: Steps 129-138 for
  context/entity/global/action/subscription/defer/timer/async/batching/
  diagnostics, Steps 139-148 for keyed reconciliation/widgets/style cascade,
  Steps 149-158 for text/font/renderer diagnostics, and Steps 159-168 for
  Win32/Wayland platform completion plus the parity audit.
- The effective distance to Step 129 is 7 implementation slices plus the
  Step 128 exit verification. The effective distance through Step 168 is 47
  implementation slices plus four band checkpoint reviews.

## 2026-07-01 Platform Cursor Application

- `CursorShape` needs to live below the UI layer once platform windows apply
  cursors directly. Moving it from `include/cgpui/ui/ui.hpp` to
  `include/cgpui/core/events.hpp` lets both `WindowRuntime` and
  `PlatformWindow` use the same platform-neutral enum without making platform
  depend on UI.
- The narrow runtime contract is to apply platform cursor changes after routed
  pointer hover recomputes `cursor_shape_`, and only when the shape changes.
  This keeps existing input snapshots intact while avoiding redundant platform
  calls for repeated pointer moves over the same cursor shape.
- Win32 can map the existing cursor shapes to system cursor resources with
  `LoadCursorW`, update the window class cursor through `SetClassLongPtrW`, and
  call `SetCursor` for immediate feedback.
- Wayland should stay a skeleton at Step 124: store the requested shape and
  issue a testable `wl_pointer.set_cursor` request when the pointer is over the
  window, but defer cursor theme loading and cursor surfaces to a later
  platform-depth step.
- The Wayland test compositor must count `set_cursor` requests, not merely
  record a boolean, because pointer enter can apply the default cursor before
  the explicit `PlatformWindow::set_cursor(text)` call.

## 2026-07-01 Runtime Update Batching

- Step 137 keeps update batching inside `WindowRuntime` rather than changing
  model observer ordering. `batch_updates(...)` increments a runtime-local
  depth counter and flushes deferred redraw requests only when the outermost
  batch exits.
- `schedule_redraw()` already had deferral hooks for event dispatch, deferred
  callbacks, timers, and task completions. Extending that single deferral path
  to `update_batch_depth_ > 0` preserves existing redraw semantics while
  allowing model/global changes inside a batch to coalesce.
- Typed global updates were previously state-only in runtime tests. Step 137
  makes `set_global(...)` and `update_global(...)` request render invalidation
  so batched global changes participate in the same redraw coalescing behavior
  as model updates.
- The public surface is deliberately narrow: `UpdateBatchCallback`,
  `WindowRuntimeContext::batch_updates(...)`, and
  `WindowRuntime::batch_updates(...)`. It does not introduce transaction
  rollback, observer reordering, or cross-thread mutation.

## 2026-07-01 Runtime Diagnostics Snapshot

- Step 138 should expose a read-only public snapshot from both
  `WindowRuntime` and `WindowRuntimeContext`, because app code should not need
  to stitch together raw runtime internals like `subscriptions_for_view(...)`,
  `invalidation_state()`, `last_render_record()`, and `frame_index`.
- Entity storage is type-erased in `WindowRuntime::entity_stores_`, so the
  narrow diagnostic route is to maintain a runtime-level `entity_count_` as
  entities are inserted/emplaced/removed. This avoids adding reflection or
  virtual diagnostics to `EntityStore<T>` during the context-side API band.
- Step 138 intentionally counts observer records separately from connected
  owned subscriptions. Permanent observers have a zero `SubscriptionId`, while
  owned subscriptions remain connected only while their callback is still
  present.
- The snapshot includes current invalidation and last render/frame state, but
  not timing, command counts, or profiling fields. Those belong to the later
  frame statistics diagnostics slice.

## 2026-07-01 Keyed Element Identity

- Step 139 keeps keyed identity parent-local and additive: `ElementKey`,
  `Element::key()`, and `ElementBuilder::key(...)` annotate elements without
  changing the existing `ElementId` allocation contract.
- `ElementTree::reconcile_children(...)` is the new batch path for keyed
  reorder/insert/remove. Existing `reconcile_child(parent, index, element)`
  remains index-based so older unkeyed tests and call sites keep their current
  behavior.
- Reused keyed nodes keep their `ElementId` and existing child subtrees while
  replacing the node's element payload. Removed direct children are pruned with
  their descendants, which gives Step 140 lifecycle hooks and Step 141 element
  state storage a clear mount/update/unmount boundary to build on.
- Builder keys are propagated to click, pointer, key, and focus wrappers so
  event/focus-decorated authored elements reconcile by the public key attached
  at the builder level, not by an inner implementation detail.
- The first Step 139 feature-worktree Windows full debug run saw the known
  transient `clipboard_test/default` failure; an immediate targeted rerun
  passed 1/1 and the Windows full debug rerun passed 29/29. No Step 139 code
  touches clipboard paths.

## 2026-07-01 Element Lifecycle Hooks

- Step 140 keeps lifecycle callbacks on the element reconciliation boundary:
  `on_mount(...)` fires when a fresh `ElementId` is allocated, `on_update(...)`
  fires when a new element payload reuses an existing `ElementId`, and
  `on_unmount(...)` fires before a node is pruned from the tree.
- Reused keyed children receive `on_update(...)` on the replacement element
  instead of unmounting the previous payload. This preserves the element-id
  continuity that Step 141 will use for per-element state storage.
- `set_root(...)` must unmount the previous root subtree before clearing
  `nodes_`; otherwise full root replacement skips lifecycle teardown for the
  old root and descendants.
- The first RED check initially appeared to pass because the RED patch was
  accidentally applied to the main worktree instead of the Step 140 feature
  worktree. The main worktree was restored to only the known untracked
  `.vscode/`, and the RED patch was reapplied to
  `.worktrees/element-lifecycle-hooks` before validating the expected missing
  API failure.

## 2026-07-01 Element State Storage

- Step 141 stores reusable-widget state on `ElementTree::Node`, keyed by
  `std::type_index` with `std::any` payloads. This makes state follow the
  stable `ElementId` rather than the replaceable `Element` payload used during
  keyed reconciliation.
- `ElementTree::state<T>(...)` soft-fails with `nullptr` for missing element
  ids, missing state, and wrong types. `state_or_init<T>(...)` creates state
  only for live nodes, while `emplace_state<T>(...)` explicitly replaces the
  state for that element/type pair.
- Removed element subtrees drop their state naturally because `remove_subtree`
  erases the owning node after lifecycle unmount. Reused keyed children retain
  their state across `reconcile_children(...)` updates and reorders.
- `WindowRuntime` and `WindowRuntimeContext` forward element-state access only
  when the runtime owns an installed `ElementTree`; legacy raw
  `set_element_root(...)` paths intentionally soft-fail until they migrate to
  tree ownership.

## 2026-07-01 Style Classes And Theme Tokens

- Step 142 keeps style vocabulary inert: `StyleClassId`, `StyleClasses`,
  `ThemeTokenId`, and `Theme` are public authoring data structures, but they do
  not affect `resolved_style(...)` or element rendering yet.
- `StyleClasses` preserves insertion order and de-duplicates repeated class
  ids. This gives Step 143 a deterministic class application order without
  introducing selectors or CSS-like matching in this slice.
- `Theme` has separate typed maps for color and spacing tokens. Missing token
  lookups soft-fail with `std::nullopt`, and replacing an existing token value
  is explicit through `set_color(...)` / `set_spacing(...)`.
- Cascade resolution, runtime theme switching, class-to-style rules, and
  inherited style behavior remain out of scope until Step 143 and later
  widget/style slices.

## 2026-07-01 Style Cascade Resolution

- Step 143 introduces a deterministic style-resolution primitive rather than a
  runtime-installed cascade. Callers pass a `StyleCascade`, local `StyleState`,
  ordered `StyleClasses`, an inline `StyleOverlay`, and state flags to
  `resolved_style(...)`.
- The merge order is local base, class rules in `StyleClasses` insertion order,
  local state overlays, then inline overlay. Missing class rules soft-fail so
  authors can share class lists across partial cascades.
- Class base styles are converted into sparse overlays by comparing against the
  default `Style`. This preserves local base fields that a class did not
  author, but it means Step 143 cannot intentionally reset a non-optional field
  back to its default value through a class base rule. A later richer rule type
  should make authored-default resets explicit if widgets need that behavior.
- `StyledElement` stores authored classes and inline styles, and the box builder
  exposes `.class_name(...)` plus `.inline_style(...)`. Normal layout and paint
  still use the local style state until a later slice supplies cascade context
  to runtime traversal.

## 2026-07-01 FocusHandle Primitive

- Step 144 deliberately keeps `FocusHandle` as a lightweight public handle over
  `ElementId`, not as a new focus graph or owner type. The existing
  keyboard-focus owner remains authoritative.
- `FocusHandle::request(...)` and `release(...)` forward through
  `WindowRuntime` / `WindowRuntimeContext`, so widget code can carry handles
  without reaching into runtime internals.
- `contains(...)` and `focused(...)` are snapshot-friendly queries over
  `ViewInputState`, `WindowRuntime`, and `WindowRuntimeContext`. This keeps
  focus inspection cheap for reusable widgets and tests.
- The new public `WindowRuntime::input_state()` snapshot mirrors the context
  access path and avoids duplicating focus/hover/pointer-capture state assembly.
- Step 145 should build the button primitive from these public pieces:
  focusable/disabled/click/action behavior, existing style-state overlays, and
  element builders. It should not introduce private runtime hooks unless a
  missing general primitive first becomes explicit in RED coverage.

## 2026-07-01 Button Widget Primitive

- Step 145 keeps the button primitive as a public reusable element/widget
  surface, not a new runtime event special case. `ButtonElement` is focusable,
  honors `enabled()`, runs an optional local click handler first, and then
  dispatches its action name through the general `ElementEventContext`
  `dispatch_action` callback wired to `WindowRuntime::dispatch_action(...)`.
- Button layout intentionally follows the same child-plus-padding/margin
  semantics as `StyledElement`: when a child is present, the child layout size
  becomes the content size. `preferred_size` remains the no-child base content
  size path rather than a forced outer-box override.
- Button paint now shares the styled-box background, border, border-radius, and
  overflow-clip command generation path with `StyledElement`, then paints its
  child afterward. This makes the button a visible widget primitive instead of
  only an event/action wrapper.
- The button stores hover/focus/disabled `StyleState` metadata and exposes it
  for authoring/tests, but normal paint still uses the local base style. A
  later slice that installs cascade/state flags into runtime traversal should
  teach widgets to resolve hovered/focused/disabled styles at layout/paint time.
- Step 146 can build `label(...)` as a thin text-widget helper over the
  existing `TextElement`, text style fields, and text paint command metadata;
  it should include header cleanliness coverage for the public widget helper.

## 2026-07-01 Text Shaping Run Abstraction

- Step 150 intentionally introduces deterministic fallback shaping, not full
  HarfBuzz/text-complex shaping. Each leading UTF-8 byte plus its continuation
  bytes becomes one `TextGlyphRun`, with byte offsets/lengths preserved for
  later glyph-atlas and caret/selection work.
- `shape_text(...)` stores the original text bytes, font descriptor, font size,
  total byte length, glyph advances, total advance, and line height. This gives
  Step 151 a stable bridge from UI text metadata to renderer-facing glyph cache
  keys without depending on platform font discovery yet.
- `TextElement` and `LabelElement` now derive layout width/height from
  `TextShapeRun::total_advance` and `line_height`. This fixes the old
  byte-count metric for UTF-8 text while preserving the existing ASCII fallback
  behavior of `font_size * 0.5F` per glyph.
- The public header-cleanliness test now directly exercises `TextShapeRun` and
  `shape_text(...)`; future text/glyph APIs should keep that coverage current
  so author-facing headers remain standalone.

## 2026-07-01 Glyph Atlas Cache Interface

- Step 151 keeps glyph caching renderer-facing but allocation-free:
  `GlyphCache` records lookup hits/misses and stores `GlyphAtlasEntry`
  metadata, but it does not allocate Vulkan images, upload glyph pixels, or
  depend on platform font discovery yet.
- `GlyphAtlasKey` is derived from the fallback shaping run: font family, font
  size, glyph index, byte offset, and byte length. This makes text paint
  metadata deterministic for ASCII and UTF-8 fallback glyph runs while leaving
  room for a later real shaper to provide stable glyph ids.
- `PaintList::fill_text(...)` now shapes text once and emits `TextPaint::glyphs`
  with per-glyph origin and advance metadata. Step 152 should consume this
  metadata in the Vulkan text path instead of reshaping raw text in the
  renderer.
- Header-cleanliness coverage now exercises the renderer-facing cache types and
  UI text glyph metadata. Future text-rendering slices should keep the public
  surface split: `ui/text.hpp` owns shaping/glyph paint metadata, while
  `renderer/renderer.hpp` owns atlas/cache lookup records.

## 2026-07-01 Vulkan Text Draw Path

- Step 152 makes text commands reach the renderer frame for the first time:
  `render_view(...)` now forwards `PaintCommandKind::text` as `TextDraw`
  instead of skipping it, while text selection and caret commands still remain
  metadata-only and skipped by the renderer bridge.
- `RenderFrame::draw_text(...)` intentionally has a default no-op body so
  existing fake frames and the deferred Metal skeleton remain source-compatible.
  Vulkan overrides it and stores `TextDraw` commands for frame presentation.
- The Vulkan path consumes `TextDraw::glyphs` through a persistent
  `GlyphCache` before recording the swapchain command buffers. This proves the
  renderer uses cache metadata, but it still does not rasterize glyph pixels,
  allocate atlas textures, bind pipelines, or emit textured quads.
- `vulkan_consume_text_draw(...)` stores deterministic atlas placeholder bounds
  from each glyph origin, advance, and font size. Step 153 can build opacity and
  transform command metadata on top of this command stream without depending on
  real font upload yet.

## 2026-07-01 Opacity and Transform Paint Metadata

- Step 153 keeps opacity and transform as deterministic command metadata, not a
  GPU pipeline or animation feature. `AffineTransform` lives in core geometry so
  style, UI paint, renderer commands, and future Metal/Vulkan paths can share
  the same public value type.
- `PaintList` owns metadata scoping: nested scopes multiply opacity and compose
  transforms parent-to-child. This keeps `StyledElement`, `ButtonElement`, text
  widgets, and scroll containers from needing renderer-specific knowledge while
  still producing stable command metadata.
- `render_view(...)` now forwards command metadata into both `SolidRect` and
  `TextDraw`. Text selection and caret remain metadata-only and skipped by the
  renderer bridge, matching the Step 152 behavior.
- Step 154 can use `PaintMetadata` directly when defining batching keys by clip,
  opacity, transform, and primitive kind; it should remain diagnostic grouping,
  not GPU optimization.

## 2026-07-02 Window Lifecycle Events

- Step 160 treats activation, minimize, restore, and close as lifecycle
  observability events on the existing root route. They produce
  `EventDispatchRecord` values and after-event callbacks, but they do not force
  view fallback handling the way input events do.
- `WindowFocused` intentionally stays on the existing view-dispatch path so
  prior focus behavior and view-level focus handling remain source-compatible.
- Close requests now record the close lifecycle event before invoking the close
  callback and `PlatformApplication::quit()`. This lets the close callback see
  `context.last_event_dispatch` for `window_close_requested` while preserving
  the existing quit semantics.
- Minimize/restore update the lightweight `ViewInputState::focused` snapshot
  only enough for lifecycle callbacks to observe focus loss. They do not yet
  model platform window state, visibility, occlusion, or renderer suspension;
  those should remain separate future platform/runtime decisions if needed.

## 2026-07-02 Win32 IME Placement

- Step 161 keeps UI-level `ImeCandidateRect` tied to `ElementId`, but the
  platform layer receives only `ImeTextInputPlacement` with a screen-space
  `Rect` and `byte_offset`. This keeps platform IME handling independent of
  element-tree ownership and focused-text lookup details.
- Runtime placement sync runs after focused text geometry can change: event
  dispatch, layout/redraw, focus request/release, and rendered-tree updates all
  converge through a cached platform placement update path so redundant native
  calls are avoided.
- Win32 stores the latest placement in `WindowState` and applies it to both IMM
  composition and candidate windows. Missing `HIMC` is treated as a soft skip,
  which keeps tests and non-IME scenarios deterministic.
- Wayland currently preserves the same placement state without protocol
  side effects. Step 162 should consume this state in an isolated text-input
  skeleton and report graceful unsupported behavior before attempting a full
  text-input-v3 implementation.

## 2026-07-02 Wayland IME Skeleton

- Step 162 makes IME capability explicit through `ImeTextInputSupport` on
  `WindowState`. The default remains `unsupported`; Win32 marks the existing
  IMM placement path as `available`, while Wayland reports `unsupported` until
  a text-input protocol global is actually bound.
- The Wayland `WaylandTextInput` skeleton is intentionally state-only: it
  accepts `ImeTextInputPlacement`, stores the latest placement, and exposes a
  support snapshot without sending protocol requests. This preserves focused
  text geometry through the platform boundary without pretending that
  text-input-v3 is implemented.
- `WaylandWindow::set_ime_text_input_placement(...)` now delegates to the
  skeleton and mirrors both support and placement into `WindowState`, giving
  runtime/tests a single platform-neutral observation point for supported and
  graceful-unsupported paths.
- Step 163 can now focus on Win32 drag/drop event shapes without reworking IME
  geometry. A future full Wayland text-input slice should replace only the
  internals of `WaylandTextInput` with protocol binding, enter/leave, content
  type, surrounding text, and cursor-rect requests.

## 2026-07-02 Win32 Drag-and-Drop Skeleton

- Step 163 defines platform-neutral drag/drop event shapes in the core event
  surface: `DragDropPayloadKind`, `DragDropPayload`, `DragEntered`,
  `DragUpdated`, `DragDropped`, and `DragExited`. Payloads are intentionally
  narrow for now: either text, files, or empty/none.
- Runtime routing treats drag/drop events as pointer-positioned events for
  hit-testing and `input.pointer_position` updates, but it does not update
  hover cursor state. This keeps DnD routing observable without merging drag
  state into the existing pointer hover state machine.
- Win32 currently uses deterministic `RegisterWindowMessageW` test hooks to
  translate UTF-16 text and file paths into public drag/drop events. This is a
  skeleton and does not yet claim real `IDropTarget`, shell/OLE data-object
  parsing, drag effects, or file-manager integration.
- Step 164 should reuse the public event shapes for Wayland data-device work
  and add Wayland-specific graceful no-data behavior without changing the
  Win32 test hook surface.

## 2026-07-02 Wayland Data-Device Drag-and-Drop Skeleton

- Step 164 reuses the existing public drag/drop event shapes rather than
  adding Wayland-specific public API: `DragEntered`, `DragUpdated`,
  `DragDropped`, and `DragExited` are emitted from Wayland data-device
  notifications.
- Production Wayland now binds `wl_data_device_manager`, creates a seat data
  device, tracks the active drag target surface through the existing
  window-lookup path, and maps data-device enter/motion/drop/leave into
  platform events. `DragExited` uses the last known drag position because
  Wayland leave has no coordinates.
- Payload handling intentionally soft-fails to `DragDropPayloadKind::none`.
  The slice does not implement MIME negotiation, `wl_data_offer_receive`, pipe
  reading, text extraction, URI-list parsing, file-manager integration, or drag
  effects.
- The Wayland test compositor now exposes a deterministic
  `wl_data_device_manager`/`wl_data_device` path and request helpers for
  drag-enter, drag-motion, drop, and leave. This keeps Step 164 testable
  without depending on an external compositor or desktop file manager.
- Step 165 should bridge runtime deferred callbacks, timers, and async
  completions to platform wakeups without changing the Step 164 DnD payload
  boundary.

## 2026-07-02 Platform Event Loop Wakeup

- Step 165 adds a platform-neutral wakeup event rather than overloading redraw:
  `WindowWakeupRequested` asks the runtime to drain queued non-render work,
  while `WindowRedrawRequested` remains the render path.
- Runtime queue producers now request a platform wakeup when the window is
  live: `defer(...)`, `schedule_timer(...)`,
  `schedule_repeating_timer(...)`, and `complete_task(...)`. Existing
  deterministic test hooks such as `advance_time(...)` remain available and
  keep their direct timer-drain behavior.
- Wakeup drain order is deterministic: task completions first, due timers
  second, deferred callbacks third. This prevents async completions from
  racing timer/defer callbacks inside the same wakeup turn.
- Wakeup handling is an outer drain batch. `handling_wakeup_` suppresses
  intermediate deferred-redraw flushes from task/timer/defer drains so one
  redraw is requested after all queued wakeup work drains. This fixed the
  initial Step 165 GREEN failure where a task completion flushed redraw before
  timers and deferred callbacks ran.
- Win32 implements the wakeup bridge by posting a private thread message to the
  event-loop thread and dispatching `WindowWakeupRequested` to tracked windows.
  Wayland implements the bridge with a nonblocking close-on-exec pipe polled
  alongside the display fd. The base/empty platform implementation stays a
  no-op for source compatibility.
- Step 166 can now focus on platform-neutral accessibility snapshots without
  needing to revisit runtime wakeup plumbing.

## 2026-07-02 Windows/Linux Demo Smoke Flows

- Step 167 keeps demo smoke coverage deterministic and bounded through
  environment variables rather than introducing broad desktop UI automation.
  `CGPUI_DEMO_SMOKE_FLOW` runs inside the existing hello-window runtime
  callbacks and exits by requesting close on the second frame.
- The full-flow smoke deliberately uses the public demo/runtime surface:
  `cgpui::MemoryClipboard`, a `cgpui::TextInput` event object, focused text
  mutation, runtime clipboard paste/copy helpers, `context.request_render()`,
  and `context.window.request_close()`. It does not add private platform hooks
  or a separate test-only demo binary.
- Existing demo smoke controls remain intact:
  `CGPUI_EXIT_AFTER_FIRST_FRAME`, `CGPUI_RESIZE_AFTER_FIRST_FRAME`,
  `CGPUI_CLOSE_AFTER_FIRST_FRAME`, `CGPUI_DEMO_INJECT_TEXT`, and
  `CGPUI_DEMO_PAINT_SNAPSHOT_SMOKE` still cover their narrower paths.
- The `hello_window` xmake target now has one additional platform smoke test on
  each active target: `windows_demo_smoke_flow` and `linux_demo_smoke_flow`.
  This raises full-suite counts to Windows 30/30 and WSL Arch Linux 27/27.

## 2026-07-02 Glyph Atlas Upload Records

- Step 170 keeps glyph upload preparation CPU-side and deterministic:
  `GlyphCache::allocate(...)` consumes `RasterizedGlyph`, creates fixed-size
  atlas pages as needed, row-packs glyph bitmaps, and records alpha upload
  bytes without creating Vulkan texture objects yet.
- `GlyphAtlasEntry` now carries `page_index`, so renderer-facing cache entries
  can distinguish atlas pages while preserving existing lookup and store
  behavior for manually supplied entries.
- Repeat allocation of an existing glyph is a cache hit for atlas storage:
  it returns the existing atlas bounds with `created == false` and does not
  append duplicate upload records.
- `vulkan_consume_text_draw(...)` now runs missing glyph metadata through
  `rasterize_fallback_glyph(...)` and atlas allocation, moving the text path
  beyond placeholder metadata toward uploadable bitmap data.
- Step 171 can build textured glyph quad records from these atlas entries and
  upload records; actual GPU texture creation remains a later renderer
  integration boundary.

## 2026-07-02 Textured Glyph Quad Records

- Step 171 adds a renderer-facing `TexturedGlyphQuad` data record rather than
  creating Vulkan image/sampler objects. Each quad carries the glyph key,
  atlas page index, device-space bounds, atlas pixel bounds, normalized atlas
  UV bounds, color, optional clip rect, and paint metadata.
- `vulkan_build_textured_glyph_quads(...)` is the bridge from `TextDraw` to
  render-preparable glyph geometry. It performs the existing glyph-cache lookup
  and fallback raster allocation path, then derives one stable quad per
  `TextGlyphPaint`.
- `vulkan_consume_text_draw(...)` now delegates to the textured-quad builder,
  so text preparation uses one path for lookup, rasterization, atlas
  allocation, and quad generation.
- Header-only/UI tests should instantiate `TexturedGlyphQuad` records directly
  and avoid calling Vulkan implementation functions unless their xmake target
  links `cgpui_renderer_vulkan`.
- Step 172 can add render-report counters over cache hits, rasterized glyphs,
  upload records, and emitted quads without yet requiring real GPU texture
  uploads.

## 2026-07-02 Vulkan Text Render Report Counters

- Step 172 adds `RendererTextRenderReport` inside `RendererCommandReport` so
  text commands can be reported as glyph-backed preparation instead of only
  generic metadata-backed command batches.
- The draw-data overload of `vulkan_build_renderer_command_report(...)`
  consumes solid rects, text draws, and a `GlyphCache`, then reports both the
  ordinary command batches and text-depth counters.
- Text-depth counters are delta-based for each report build: cache hits are
  counted from new lookup records, rasterized glyphs and upload records are
  counted from new upload records, and textured glyph quads are counted from
  the emitted quad vector.
- The parity audit now states the current renderer text status precisely:
  CPU fallback raster data, atlas allocation/upload records, textured glyph
  quads, and report counters exist; real Vulkan atlas textures, GPU upload,
  shader sampling, subpixel positioning, and full font fallback shaping remain
  incomplete.
- Step 173 can now pivot away from Vulkan text metadata and start the Wayland
  clipboard MIME payload extraction path.

## 2026-07-02 Wayland Clipboard MIME Payload Extraction

- Step 173 keeps `create_platform_clipboard()` conservative while adding an
  opt-in Wayland clipboard connection for tests and future integration:
  `WaylandClipboardOptions::connect_to_display` connects to the active
  Wayland display, binds `wl_data_device_manager`, `wl_seat`, and
  `wl_data_device`, and stores the current selection offer behind a PIMPL so
  public headers do not include Wayland protocol headers.
- Text MIME selection is deterministic: `text/plain;charset=utf-8` is chosen
  before `text/plain`, unsupported MIME types are ignored, and payload bytes
  are read through a pipe passed to `wl_data_offer_receive`.
- The Wayland test compositor now has a deterministic clipboard selection
  path: tests can set MIME/payload pairs, the compositor sends a
  `wl_data_offer`, writes the selected payload to the received fd, and records
  the requested MIME type for assertions.
- This is not full Wayland clipboard parity yet. Clipboard ownership/write
  offers, default platform-factory connection behavior, non-text MIME formats,
  and production desktop edge cases remain future work.
- Step 174 can reuse the same offer/MIME/payload direction for Wayland
  drag/drop, then add text and `text/uri-list` parsing through the drag
  data-device path.

## 2026-07-02 Wayland Drag/Drop MIME Payload Extraction

- Step 174 extends the Wayland data-device path from event routing to payload
  extraction. `WaylandDataDevice` now listens to `wl_data_offer.offer`, stores
  offered MIME types with the active drag offer, and reads payload bytes with
  `wl_data_offer_receive` through deterministic pipes.
- Drag payload preference is text first: `text/plain;charset=utf-8` then
  `text/plain`, followed by `text/uri-list` for files. Unsupported or empty
  offers continue to produce `DragDropPayloadKind::none`.
- URI-list parsing intentionally handles the practical local-file subset:
  comments and blank lines are ignored, `file:///...` and
  `file://localhost/...` are accepted, and percent escapes are decoded before
  exposing public file paths.
- The Wayland test compositor now uses a neutral `WaylandMimePayload` test
  type, retains the old clipboard payload alias, and can send drag offers as
  well as clipboard selection offers from the same deterministic data-offer
  receive path.
- Remaining drag/drop depth includes accept/finish/action negotiation,
  non-local URI policy, richer MIME formats, and production desktop-file
  manager edge cases.

## 2026-07-02 Wayland Text-Input State Machine

- Step 175 moves Wayland IME beyond placement-only storage. The Linux backend
  now has a minimal handwritten `zwp_text_input_v3` client binding and the
  test compositor exposes the matching server-side protocol surface for
  deterministic tests.
- `WaylandTextInputState` is protocol-independent window state: it tracks
  protocol availability, placement, enter/leave state, surrounding-text cursor
  records, content-type records, preedit text, and committed text.
- IME placement is now converted into protocol requests when text-input v3 is
  available: enable/disable, empty surrounding text with cursor/anchor byte
  offset, default content type, cursor rectangle, and protocol commit.
- Deterministic compositor enter/preedit/commit/leave events now route into
  public `ImeComposition` update/commit events through the platform callback,
  while the test compositor records the client-side surrounding/cursor/content
  commit state for assertions.
- This is still partial Wayland IME parity. Production input-method behavior,
  delete-surrounding editing, richer content hints/purposes, serial policy, and
  compositor-specific edge cases remain future work.

## 2026-07-02 Native Additional Window Creation Scaffold

- Step 178 turns app-opened child-window records from metadata-only registry
  entries into native `PlatformWindow` creation attempts owned by
  `WindowRuntime`.
- Additional windows are activated through the existing
  `PlatformApplication::create_window(...)` boundary, so the slice does not add
  Win32/Wayland-specific public APIs or fork the GPUI-like authoring surface.
- `WindowRuntimeRecord::native_window_error` preserves graceful failure
  semantics for child windows: a backend/test double can fail the child native
  window creation without failing the root `run_app(...)` path.
- Child platform-window callbacks currently update descriptor size on resize
  and mark the child record inactive on close request; shutdown clears child
  window/renderer pointers and releases the owned native-window vector.
- This remains partial multi-window parity. Child windows still do not own
  independent renderers, independent render loops, full event routing, or
  production activation/focus/lifecycle behavior.

## 2026-07-02 Steps 179-218 Production-Depth Track

- The next 40-step track should stay Windows/Linux first rather than starting
  macOS parity. The highest-value ordering is Vulkan text/rendering depth,
  text/font/editing depth, Win32/Wayland production adapter depth, then
  accessibility, multi-window, theme, asset, animation, and async depth.
- Step 179 should start with renderer-facing glyph atlas image descriptors and
  upload batches. This is the smallest next move from the current text path:
  glyph bitmap, atlas allocation, upload records, textured quads, and text
  render reports already exist, but there is no atlas image/upload planning
  surface yet.
- The 179-218 plan intentionally keeps real Vulkan image allocation, production
  UIA/AT-SPI providers, full child-window event loops, and macOS/Cocoa + Metal
  outside Step 179 so the next RED/GREEN slice stays small and verifiable.

## 2026-07-02 Vulkan Glyph Atlas Image Upload Planning

- Step 179 adds renderer-facing glyph atlas image descriptors and upload
  batches as the bridge between CPU-side `GlyphUploadRecord`s and later Vulkan
  texture resources.
- `vulkan_plan_glyph_atlas_uploads(...)` groups upload records by atlas page,
  keeps deterministic page image metadata, appends alpha payload bytes into a
  per-page upload buffer, and records each upload region's byte offset and
  size.
- This remains a planning slice: it does not allocate Vulkan images, create
  image views, bind device memory, issue transfer commands, or sample atlas
  textures. Step 180 should add texture resource lifetime records on top of
  these batches.

## 2026-07-02 Vulkan Glyph Atlas Texture Resource State

- Step 180 adds a deterministic atlas texture resource state object keyed by
  atlas page index. It turns Step 179 upload batches into create/reuse/drop
  diagnostics with stable generations.
- Repeated updates with the same page reuse the existing record and preserve
  its generation; updates without a previously live page emit a dropped record
  and remove it from the live resource span.
- This is still a resource-lifetime skeleton. It does not create real
  `VkImage`, `VkImageView`, device memory, staging buffers, descriptor sets, or
  transfer commands. Step 181 should build dirty-range upload tracking on top
  of this page-level resource state.

## 2026-07-02 Vulkan Glyph Atlas Dirty Upload Ranges

- Step 181 adds deterministic dirty upload ranges over Step 179 upload batches
  and Step 180 texture resource state. The range planner uses each live page's
  uploaded count to skip already-planned glyph regions.
- Dirty ranges are coalesced per atlas page into byte spans over the upload
  batch alpha buffer. Repeating the same batch emits no ranges; adding one
  glyph emits a range starting at that glyph's upload index and byte offset.
- This remains CPU-side upload planning. It does not allocate staging buffers,
  record Vulkan transfer commands, transition image layouts, or update
  descriptor sets.

## 2026-07-02 Vulkan Text Sampler Pipeline Readiness Report

- Step 182 adds a deterministic `TextSamplerPipelineDescriptor` to the renderer
  public surface and wires it into `RendererTextRenderReport`.
- Text render reports now distinguish glyph quad preparation from sampler
  pipeline readiness: non-empty textured glyph draws increment the sampler
  descriptor count and pending draw count while the descriptor reports shader
  modules, descriptor set layout, pipeline layout, and graphics pipeline as not
  ready.
- This is still a readiness-report slice. It does not create shader modules,
  descriptor set layouts, pipeline layouts, graphics pipelines, descriptor
  sets, or sampled atlas draw calls.

## 2026-07-02 Vulkan Rounded-Rect Tessellation Records

- Step 183 adds renderer-facing rounded-rect draw and tessellation records so
  rounded rectangles are no longer only unsupported diagnostics or solid-rect
  fallbacks.
- `render_view(...)` now forwards rounded-rect paint commands through
  `RenderFrame::draw_rounded_rect(...)`, preserving bounds, radius, color,
  clip, opacity, and transform metadata for Vulkan reporting.
- The Vulkan helper path can build deterministic rounded-rect tessellation
  records alongside solid rectangles and text draws, and frame statistics now
  count rounded-rect commands separately.
- This is still a geometry/reporting slice. It does not create a real Vulkan
  rounded-rect pipeline, shader path, anti-aliased edge generation, or GPU draw
  submission.

## 2026-07-02 Vulkan Text Selection And Caret Geometry Records

- Step 184 adds renderer-facing text selection and caret draw/geometry records
  so those text paint commands are no longer skipped or reported as unsupported.
- `render_view(...)` now forwards text selection and caret paint commands
  through `RenderFrame::draw_text_selection(...)` and
  `RenderFrame::draw_text_caret(...)`, preserving bounds, color, clip, opacity,
  and transform metadata for Vulkan reporting.
- Renderer command reports and frame statistics now count selection and caret
  commands separately from text glyph draws, matching the existing rounded-rect
  and text command depth.
- This is still a geometry/reporting slice. It does not create real Vulkan
  selection/caret draw pipelines, shader paths, or GPU submissions.

## 2026-07-02 Renderer Clip Stack Metadata

- Step 185 adds a bounded `RendererClipStackRecord` that preserves nested clip
  stack metadata while keeping the existing per-command `clip_rect` field as
  the current innermost clip.
- `PaintList` now records the full active clip stack on paint commands, and
  `render_view(...)` forwards that stack through solid rect, rounded rect,
  text, text selection, and caret draw records.
- Vulkan renderer command reports now include clip-stack counts, maximum stack
  depth, and batch keys that distinguish commands with the same current clip
  but different outer clip ancestors.
- This remains metadata/reporting work. It does not implement real Vulkan
  scissor-stack command emission or GPU clipping beyond the existing solid
  rectangle clip handling.

## 2026-07-03 Renderer Composition Stack Reports

- Step 186 adds a bounded `RendererCompositionStackRecord` that preserves
  nested opacity/transform metadata while keeping the existing per-command
  `metadata` field as the current composed metadata.
- `PaintList` now records the active metadata stack on paint commands, and
  `render_view(...)` forwards that stack through solid rect, rounded rect,
  text, text selection, and caret draw records.
- Vulkan renderer reports now count composition-stack records, track maximum
  stack depth, propagate the stack into rounded/text selection/caret geometry,
  and include the stack in batch keys so otherwise identical commands with
  different ancestors remain distinguishable.
- This remains diagnostics/reporting work. It does not implement real Vulkan
  transform stack emission, opacity compositing, or GPU pipeline state changes.

## 2026-07-03 Renderer Batch Submission Plan Records

- Step 187 adds renderer submission-plan records on `RendererCommandReport`
  that summarize command batches into deterministic submission groups.
- Submission plan keys preserve primitive kind, current clip, clip stack, and
  optional glyph atlas page index. Text submissions are split by atlas page and
  carry the current text sampler pipeline descriptor, while non-text
  submissions carry primitive-specific pipeline descriptors.
- Reports now expose submission record count, command count, and glyph quad
  count so later frame snapshots can summarize planned GPU work without
  re-walking raw command batches.
- This remains a deterministic planning/reporting slice. It does not allocate
  Vulkan command buffers, descriptor sets, render passes, pipeline objects, or
  submit work to a GPU queue.

## 2026-07-03 Renderer Frame Snapshot Report

- Step 188 adds `RendererFrameReport`, a frame-level aggregation over
  `RendererCommandReport` that preserves the underlying command report while
  summarizing supported/unsupported primitives, renderer batches, submission
  plans, glyph upload records, textured glyph quads, and renderer gaps.
- Frame gaps currently cover unsupported commands, pending text sampler
  pipeline readiness, metadata-only text draws, and missing submission plans.
  The gap list gives later diagnostics a single source for "what is still not
  real GPU work" in a frame.
- Vulkan exposes `vulkan_build_renderer_frame_report(...)` overloads that build
  the existing command report and wrap it into the new frame report. The parity
  audit now calls out frame-level text/render reports and planned submissions.
- This remains a reporting slice. It does not create real Vulkan command
  buffers, submit GPU work, or replace the existing deterministic report
  building path.

## 2026-07-03 Font Fallback Chain Resolution

- Step 189 adds a public `FontFallbackChain` over `FontDatabase` faces so text
  code can inspect ordered fallback candidates instead of only receiving the
  single face from `FontDatabase::resolve(...)`.
- `FontDatabase` now stores deterministic generic fallback families and
  resolves chains in requested-family, generic-fallback, first-available order
  with duplicate face pointers removed.
- Empty-family chain resolution intentionally preserves the existing
  `resolve({})` behavior by returning only the first registered face.
- This is still deterministic fallback planning. It does not perform
  DirectWrite/fontconfig discovery, per-codepoint coverage checks, shaping
  fallback splits, or platform font enumeration.

## 2026-07-03 Platform Font Discovery Records

- Step 190 adds `PlatformApplication::discover_font_records()` as the
  platform-facing record hook and keeps `discover_fonts()` as the database
  adapter over those records.
- Win32 now exposes a deterministic platform `FontFaceDescriptor` for
  `Segoe UI`; Wayland/Linux exposes a deterministic `sans-serif` fontconfig
  record. Both records keep `FontSource::platform`, family, postscript/name,
  and path-like metadata visible to tests without enumerating system fonts.
- Text helpers now include `font_database_from_discovered_faces(...)`, with
  `discover_test_fonts(...)` delegating to the same builder so platform and
  test records preserve source/path/name metadata consistently.
- This is still conservative discovery plumbing. It is not real DirectWrite or
  fontconfig enumeration, does not probe files, and does not verify glyph
  coverage.

## 2026-07-03 Grapheme-Aware Cursor Movement

- Step 191 moves `TextModel` cursor movement, selection extension, backspace,
  and delete from raw UTF-8 codepoint boundaries to deterministic grapheme
  boundaries.
- The helper intentionally covers the current editing-depth skeleton cases:
  ASCII, combining-mark clusters, variation selectors, regional indicator
  pairs, and emoji ZWJ sequences. This is enough to prevent common cursor and
  delete splits in those clusters without claiming full Unicode UAX #29
  segmentation.
- The implementation stays local to `TextModel` and does not introduce a
  shaping dependency. Later rich text work can replace or extend the boundary
  helper when full shaping and per-codepoint fallback coverage arrive.
- Step 192 should build word movement and word-selection actions on top of
  these grapheme-safe cursor primitives rather than reverting to byte or raw
  codepoint stepping.

## 2026-07-03 Text Word Navigation Actions

- Step 192 adds `TextEditAction` variants and public `TextModel` helpers for
  previous/next word movement plus previous/next word selection extension.
- Word movement reuses the Step 191 grapheme-boundary helpers. This keeps word
  navigation from splitting combining marks, regional indicator pairs, or ZWJ
  skeleton clusters while adding word-level jumps.
- The current word separator class is intentionally deterministic: ASCII
  whitespace, C1 next-line, no-break and Ogham spaces, Unicode U+2000 space
  range, line/paragraph separators, narrow no-break, medium mathematical, and
  ideographic space. Non-space punctuation remains part of the surrounding
  non-space run in this skeleton.
- Step 193 can add undo/redo over the expanded edit-action surface without
  revisiting word-boundary semantics.

## 2026-07-03 Text Undo And Redo Stack

- Step 193 adds undo/redo as `TextModel` behavior and `TextEditAction`
  variants, not as a separate widget-private editor layer. This keeps text
  input widgets, runtime edit actions, and direct text-model users on one edit
  history surface.
- History records snapshot text, cursor, selection anchor, and selection head
  before and after mutating edits. Insert, selection replacement, backspace,
  forward delete, and IME composition commit are recorded through the existing
  mutation paths.
- Undo restores the before snapshot and moves the record to redo; redo restores
  the after snapshot and returns the same record to the bounded undo stack.
  New edits clear redo, matching linear edit-history semantics.
- Restoring a history snapshot clears composition state. This avoids reviving a
  stale preedit session after undo/redo and keeps committed composition text as
  an ordinary edit record for the next IME slice.
- The history cap is currently a deterministic 100 undo records. This is a
  bounded model-level stack, not persistent document history, grouped typing,
  collaborative editing, or platform undo-manager integration.

## 2026-07-03 IME Delete-Surrounding Text Action

- Step 194 adds `ImeDeleteSurroundingText` as a platform event and
  `EventKind::ime_delete_surrounding_text` as the public runtime route kind, so
  IME delete-surrounding edits travel through the same keyboard-focused event
  path as preedit and commit.
- `TextModel::delete_surrounding_text(...)` interprets before/after lengths as
  byte lengths around the current cursor and clamps deletion to UTF-8 codepoint
  boundaries before recording the edit in the existing undo stack. This keeps
  Wayland text-input v3 byte-oriented semantics deterministic without claiming
  full grapheme-aware surrounding-text deletion yet.
- Runtime application is intentionally focused-text-model scoped: the event is
  still dispatched to the focused route, but model mutation only happens when
  the keyboard focus has an element owner with a bound or installed text model.
- The Wayland backend stores pending delete-surrounding protocol events until
  `done`, then emits them between pending preedit and commit events with the
  current keyboard modifiers. The test compositor mirrors that protocol shape
  by sending `delete_surrounding_text` followed by `done`.
- This slice does not implement Win32 TSF delete-surrounding integration,
  grapheme-length conversion, grouped IME history, or rich multiline
  surrounding-text context. Step 195 should build multiline navigation on top
  of the now-shared text mutation/history surface.

## 2026-07-03 Multiline Text Model And Line Navigation

- Step 195 adds deterministic hard-line helpers to `TextModel`:
  `line_count()`, `line_index_at(...)`, `line_start_offset(...)`, and
  `line_end_offset(...)`. Lines are currently delimited by LF bytes, and line
  ends exclude the newline byte.
- New `TextEditAction` variants cover line start/end movement, previous/next
  line movement, and matching selection extension. Vertical movement preserves
  the current byte column and clamps to the destination line end when the target
  line is shorter.
- This is intentionally model-level editing depth. It does not implement soft
  wraps, bidi visual lines, grapheme column accounting, pixel hit testing,
  paragraph layout, or a persistent preferred visual column. Step 196 can add
  measurement caching without treating these hard-line helpers as full text
  layout.

## 2026-07-03 Text Measurement Cache

- Step 196 adds deterministic text measurement primitives in `ui/text.hpp`:
  `measure_text(...)`, `TextMeasurementKey`, `TextMeasurement`,
  `TextMeasurementResult`, and `TextMeasurementCache`.
- Cache keys cover the current deterministic measurement tuple: text content,
  font descriptor, font size, and normalized DPI scale. Hits and misses are
  observable through `cache_hit`, `entry_count()`, `lookup_count()`,
  `hit_count()`, and `miss_count()`.
- `PaintList` and `render_view` now accept an optional measurement cache so
  renderer-facing tests can exercise repeated text paint measurement reuse
  without changing the default no-cache render path.
- This remains a deterministic fallback measurement cache over the existing
  `shape_text(...)` skeleton. It does not add platform shaping, glyph coverage
  fallback splits, soft wrapping, paragraph layout, cache eviction, or
  cross-frame runtime ownership yet. Step 197 can build pointer-selection
  geometry against these reusable measured glyph positions.

## 2026-07-03 Structural Optimization Task 1 Private Header Surgery

- The first aggressive optimization pass moves the largest private declaration
  clusters out of old internal aggregate headers. The remaining aggregates are
  intentionally compatibility entry points, not ownership locations:
  `wayland_application_internal.hpp` is 2 lines,
  `wayland_window_internal.hpp` is 94 lines, and `vulkan_internal.hpp` is 5
  lines.
- Because C++ class declarations are single continuous declarations, the
  Wayland application/window splits use class-body declaration slices for
  private member groups. The actual ownership boundaries are now visible in
  focused headers: application registry/input/cursor, registered window,
  window configure lifecycle, Vulkan platform helpers, Vulkan state, Vulkan
  swapchain resources, and Vulkan device queue search types.
- Task 1 stayed structure-only. No runtime behavior was intended to change.
  Windows focused verification passed 6/6 available targets, WSL Arch Linux
  focused verification passed 5/5 available targets, and `git diff --check`
  exited 0 with only existing CRLF normalization warnings.
- Task 2 should now attack the remaining long bridge implementation files:
  Win32 window proc dispatch, Wayland application window registry/creation,
  Vulkan swapchain creation, and Vulkan report submission.

## 2026-07-03 Structural Optimization Task 2 Bridge Implementation Split

- Remaining long bridge files can be split without changing public behavior by
  introducing focused helper implementation files and keeping the old files as
  dispatch-only compatibility points.
- Win32 `win32_window_proc.cpp` is now a 33-line dispatcher. Test drag/drop,
  lifecycle/paint/destroy, pointer, and keyboard/text messages live in their
  own `win32_window_proc_*` files.
- Wayland `wayland_application_windows.cpp` is now a 5-line placeholder entry.
  Registry lookup/global handling lives in
  `wayland_application_window_registry.cpp`; window creation lives in
  `wayland_application_window_creation.cpp`; cursor and IME placement setters
  moved back to cursor/input owners.
- Vulkan `vulkan_swapchain_create.cpp` now delegates surface querying and
  create-info construction to focused helpers. The create-info helper must keep
  `VkSurfaceTransformFlagBitsKHR pre_transform` in the plan so returned
  `VkSwapchainCreateInfoKHR` does not depend on discarded surface-capability
  storage.
- Vulkan report submission now keeps batch aggregation in
  `vulkan_report_submission.cpp`, text page expansion in
  `vulkan_report_text_submission.cpp`, and summary counters in
  `vulkan_report_submission_stats.cpp`.

## 2026-07-03 Structural Optimization Task 3 Public UI Header Surgery

- Public text APIs now have focused leaf headers. `text_layout.hpp` is only the
  compatibility aggregate over shape, glyph, measurement, wrapping, and
  hit-testing domains, while `TextEditAction` is no longer owned by
  `text_model.hpp`.
- `TextModel` is now closer to a normal public class declaration:
  construction, editing, history, navigation, and selection behavior live in
  focused `src/ui/text_model*.cpp` units. The header keeps private helper
  declarations because the class is still value-owned rather than PIMPL-backed.
- Runtime public types are no longer concentrated in `runtime_types.hpp`.
  The new runtime leaf headers map to callbacks, ids, handles, window options,
  app context, actions, events, diagnostics, input state, runtime context, and
  rendering/app-runner free functions.
- `WindowRuntime` private data and helper declarations are now isolated in
  `src/ui/window_runtime_internal.hpp` as a class-body declaration slice. This
  avoids a full storage/PIMPL migration while still removing the private bulk
  from the public facade header.
- Current Task 3 line counts are: `text_layout.hpp` 6,
  `text_model.hpp` 118, `runtime_types.hpp` 11, `window_runtime.hpp` 207,
  and `src/ui/window_runtime_internal.hpp` 235. The largest new text leaf is
  `text_glyphs.hpp` at 133 lines, and the largest new runtime leaf is
  `runtime_context.hpp` at 166 lines.
- Windows and WSL Arch Linux focused public-header verification both passed
  8/8, and `git diff --check` exited 0 with only existing CRLF normalization
  warnings.

## 2026-07-03 Structural Optimization Task 4 Public Element And Style Header Surgery

- The public element/style surface now uses thin compatibility aggregates plus
  focused leaves. The key old public headers are all below the 220-line guard:
  `element_tree.hpp` 104, `element_containers.hpp` 4,
  `element_interaction_nodes.hpp` 5, `element_builder.hpp` 3,
  `widget_builders.hpp` 6, and `style_core.hpp` 8.
- Moving only the public header declarations was not enough for the user's
  "thorough optimization" bar, because it created large replacement
  implementation files. The architecture test now also guards the new
  element/style implementation split: old implementation entry files stay
  1-line shells, `element_tree_reconcile.cpp` is 124 LF, and
  `element_flex_layout.cpp` is 144 LF after the second split.
- `Style` and `StyleOverlay` chain setters are no longer inline in the public
  headers. This intentionally trades some constexpr/header-only behavior for
  cleaner ABI and compile boundaries, matching the stated preference to
  optimize structure over compatibility conservatism.
- Windows and WSL Arch Linux focused verification for Task 4 both passed 6/6:
  `ui_source_structure_test`, `ui_header_cleanliness`, `element_test`,
  `style_test`, `render_view_test`, and `window_runtime_test`.
- `git diff --check` exited 0 with only existing CRLF normalization warnings
  and no whitespace errors after the Task 4 split.

## 2026-07-04 Structural Optimization Task 5 Renderer Platform Core Header Surgery

- Renderer, platform, and core public headers now follow the same aggregate plus
  leaf-header structure as the UI surface. The compatibility aggregates are
  intentionally tiny: `renderer.hpp` 7 lines, `glyph_atlas.hpp` 7,
  `renderer_reports.hpp` 7, `platform.hpp` 9, and `events.hpp` 8.
- The largest Task 5 renderer leaf headers are still comfortably under the
  220-line public-leaf target: `renderer_frame_reports.hpp` 184 and
  `glyph_cache.hpp` 180. Platform and core event leaves are all under 60 lines.
- Header-cleanliness verification caught two real split-boundary problems:
  `renderer_frame_reports.hpp` depended on `GlyphCache` without a direct
  include, and `glyph_cache.hpp` exposed command-facing signatures without
  directly including `renderer_commands.hpp`. The fix keeps leaf headers
  self-contained instead of relying on aggregate include order.
- The renderer/platform/core split stayed structure-only. Windows focused Task
  5 verification passed 8/8, WSL Arch Linux focused verification passed 5/5
  available targets, and Task 6 can now move to splitting the oversized
  `tests/ui/window_runtime_test.cpp` by runtime domain.

## 2026-07-04 Structural Optimization Task 6 Test Suite Structure Split

- `tests/ui/window_runtime_test.cpp` is no longer the runtime-test monolith. It
  is now a 41-line smoke target, while the behavioral coverage lives in focused
  binaries for input, focus, actions, text, rendering, scheduling, multiwindow,
  and theme domains.
- A shared `window_runtime_test_support.hpp` carries the common fake
  platform/renderer/view fixture. It is still the largest test-support file at
  1485 lines, but it replaces repeated fixture copies while the actual test
  ownership is now distributed across independently runnable targets.
- The split exposed two mechanical boundary hazards that are worth remembering:
  dispatch helper globals must move with their dispatch functions, and the last
  extracted range must not include the original file's anonymous-namespace
  closing brace when the new wrapper closes its own namespace.
- `window_runtime_focus_test.cpp` was added beyond the original seven-file
  plan because focus, tab traversal, keyboard-focus ownership, and scroll
  routing were large enough to deserve their own domain instead of inflating
  `window_runtime_input_test.cpp`.
- Windows and WSL Arch Linux focused Task 6 verification both passed 10/10,
  including `ui_source_structure_test` and every split runtime test target.
  Task 7 can now run final aggregation on Windows and WSL.

## 2026-07-04 Structural Optimization Task 7 Final Aggregation

- The earlier handoff-reported `vulkan_solid_rect_test/default` exit-5 failure
  was not reproducible in the final verification state. A focused rerun passed,
  and the full Windows aggregation later passed the same target as part of
  41/41 tests. No production Vulkan synchronization change was made without a
  reproducible root cause.
- Windows full debug verification passed 41/41 after `xmake f -c -m debug -P .`
  and `xmake test -P .`. WSL Arch Linux full debug verification passed 38/38
  after the corresponding `XMAKE_ROOT=y xmake f -y -c -m debug -P .` and
  `XMAKE_ROOT=y xmake test -y -P .` run.
- `git diff --check` exits 0 for the final structural branch. The only output
  is Git's expected LF-to-CRLF normalization warnings for touched text files.
- The aggressive structure pass leaves the old public and private aggregates
  thin while keeping leaf files under the encoded guards: public UI hot headers
  are now `text_model.hpp` 131, `runtime_types.hpp` 12,
  `text_layout.hpp` 7, `window_runtime.hpp` 219,
  `element_tree.hpp` 104, and `style_core.hpp` 8; renderer/platform/core
  aggregates are `renderer.hpp` 7, `platform.hpp` 10, and `events.hpp` 8.
- The main private/bridge hotspots are also reduced to small dispatch or
  declaration boundaries: `wayland_application_internal.hpp` 3,
  `wayland_window_internal.hpp` 107, `vulkan_internal.hpp` 6,
  `win32_window_proc.cpp` 33, `wayland_application_windows.cpp` 5,
  `vulkan_swapchain_create.cpp` 65, and
  `vulkan_report_submission.cpp` 100.
- The remaining large files are intentionally concentrated in test fixture
  support and split test domains rather than production API or implementation
  monoliths. `tests/ui/window_runtime_test.cpp` is now 41 lines, while
  `window_runtime_test_support.hpp` and the focused runtime test targets carry
  the detailed behavioral coverage.

## 2026-07-04 Structural Optimization Task 3 Follow-up Audit

- Task 3's public text header split was not complete until the text layout
  helper implementations also moved out of public leaf headers. The durable
  guard is now in `ui_source_structure_test`: it requires
  `src/ui/text_shape.cpp`, `src/ui/text_glyph_raster.cpp`,
  `src/ui/text_measurement.cpp`, `src/ui/text_wrapping.cpp`, and
  `src/ui/text_hit_testing.cpp`, and it rejects the `" inline "` token in the
  matching public text headers.
- The focused text implementation files intentionally live under `src/ui/` to
  match the Task 3 structure boundary, but they are compiled into the
  `cgpui_renderer` target. The Vulkan renderer consumes glyph rasterization and
  text paint metadata directly, so compiling them only into `cgpui_ui` would
  invert the dependency boundary.
- The follow-up verification passed Windows full debug 41/41 and WSL Arch Linux
  full debug 38/38 after the text implementation migration. `git diff --check`
  produced only expected LF-to-CRLF normalization warnings.

## 2026-07-04 GPUI Upstream Parity Ledger Phase A

- The concrete full-replication baseline is now pinned to upstream Zed/GPUI
  commit `5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0`, with recorded crate
  versions `gpui = 0.2.2` and `gpui_platform = 0.1.0`. This avoids chasing a
  moving pre-1.0 upstream target while implementing the remaining parity rows.
- The Phase A ledger establishes four status classes: `Required`, `Adapted`,
  `Deferred`, and `Non-goal`. Windows Win32 + Vulkan and Linux Wayland +
  Vulkan are required; macOS Cocoa + Metal is deferred to the later Mac phase;
  X11 remains deferred unless strict upstream Linux backend parity is
  explicitly accepted; wasm is a non-goal for the desktop target.
- The extractor is intentionally best-effort and deterministic. It fetches the
  pinned upstream crate root and Cargo.toml when network is available, but the
  example inventory falls back to the pinned default set if the network cannot
  be reached. On Windows, the fresh snapshot contained 55 public re-exports and
  20 examples.
- The first API parity gate is documentation/tooling/example oriented rather
  than a broad runtime behavior test. It proves the pin, ledger, JSON export,
  extractor, Phase A plan, xmake targets, and public hello-world parity example
  are present and wired through the build.
- Phase B should close ledger rows rather than inventing scope from memory.
  The immediate next implementation band is public `Application` / `App` /
  `Window` / `Context<T>` / entity / action / key-dispatch / test-context
  parity for Windows/Linux.

## 2026-07-04 Phase B Step 259 Application Facade

- `Application` belongs in a new app module, not in `ui.cpp`, runtime
  implementation files, or platform backends. The Step 259 structure is:
  `include/cgpui/app/application.hpp`, `include/cgpui/app/app.hpp`, and
  `src/app/application.cpp`, compiled by the new `cgpui_app` target.
- The facade intentionally wraps existing primitives instead of replacing them:
  `Application::create()` delegates to `create_platform_application()`, and
  `Application::run(...)` delegates to `run_app(...)`. This keeps existing
  low-level callers source-compatible while adding GPUI-shaped entry spelling.
- The WSL `application_facade_test/default` failure was a test lifetime bug,
  not a Linux platform failure. The test kept a raw pointer to a renderer owned
  by `run_app`'s local renderer vector and read it after `Application::run`
  returned. Direct execution happened to read stale memory that still looked
  valid; xmake's test runner exposed the use-after-free. The durable pattern is
  to record renderer observations through external counters, as
  `tests/ui/app_runner_test.cpp` already does.

## 2026-07-04 Phase B Step 260 App Window Context Facades

- `gpui::App` and `gpui::Window` parity should be layered as public app-module
  facades over the existing `WindowRuntime`, not by renaming `AppContext`,
  `WindowRuntimeContext`, or low-level runtime records. This keeps compatibility
  while moving the authoring surface closer to upstream GPUI spelling.
- The Step 260 ownership boundary is:
  `include/cgpui/app/app_facade.hpp`, `include/cgpui/app/window.hpp`,
  `src/app/app_facade.cpp`, and `src/app/app_context_facade.cpp`; after
  Step 279, `Window` implementation lives in `src/ui/window.cpp` so UI headers
  link through `cgpui_ui`. The app aggregate remains thin, and `cgpui_app`
  owns only the app-facade implementation.
- `AppContext::app()` and `WindowRuntimeContext::app()` intentionally return
  lightweight facade handles over the current runtime. `WindowRuntimeContext`
  also exposes `current_window()` so frame/update code can inspect the active
  window without reaching through the old raw `window` field spelling.
- `Window::viewport_size()` currently prefers the platform window state when
  a native window exists, then falls back to the descriptor size. This makes the
  facade useful for the root window and still deterministic for app-opened
  window records that do not yet have full production native redraw semantics.
- Structure tests should assert the real source boundary and forwarding shape,
  not an impossible static-qualified spelling. The failed check for
  `WindowRuntime::open_window` was fixed to require `runtime_->open_window`,
  which proves the facade still forwards through `WindowRuntime` while matching
  the implementation style.

## 2026-07-04 Phase B Step 261 Context Render Spelling

- `Context<T>`, `IntoElement`, and `Render<T>` belong in a focused public UI
  render leaf, not in `view.hpp` or a broad aggregate. `view.hpp` should remain
  the `View` base-class owner and consume `render.hpp`.
- The C++ adaptation for upstream `gpui::Render` is a concept:
  `Render<T>` requires `view.render(Context<T>&)` to return `IntoElement`.
  Because `Context<T>` is still an alias to `ViewContext`, existing
  `View::render(ViewContext&)` overrides remain source-compatible.
- `IntoElement` is intentionally an alias over `AnyElement` for this slice,
  paired with the existing `into_element(...)` builder functions. This gives
  public GPUI-shaped spelling without introducing a wrapper or changing
  element ownership semantics.
- The hello-world parity example is now the durable author-facing sample for
  this spelling: it returns `cgpui::IntoElement`, takes
  `cgpui::Context<HelloWorldView>&`, and asserts
  `cgpui::Render<HelloWorldView>`.

## 2026-07-04 Phase B Step 262 Context Capability Helpers

- `WindowRuntimeContext` already had the runtime machinery for app, current
  window, and entities, but the author-facing `Context<T>` spelling lacked
  GPUI-shaped helper names for `window()`, `entity(...)`, and
  `weak_entity(...)`.
- C++ cannot expose both a public data member named `window` and a member
  function named `window()`. The low-level platform window reference is now
  explicitly named `platform_window`, leaving `window()` free to return the
  public `cgpui::Window` facade while still preserving direct platform access
  for low-level tests and examples.
- Entity helpers intentionally wrap existing storage semantics:
  `entity(EntityId<T>)` returns `EntityHandle<T>` and
  `weak_entity(EntityId<T>)` returns `WeakEntity<T>`. This improves authoring
  shape without claiming the later lifecycle, transaction, and subscription
  work from Steps 265-282 is complete.

## 2026-07-04 Phase B Step 263 View Handles

- Typed view handle spelling belongs in a focused public leaf,
  `include/cgpui/ui/view_handle.hpp`, rather than in `ui.hpp`, `view.hpp`, or
  runtime input state. The compatibility aggregate only includes the leaf.
- `ViewHandle<T>` and `WeakViewHandle<T>` intentionally wrap the existing
  `ViewId` registry. They provide typed author-facing spelling, downgrade,
  typed upgrade, and read-only lookup, but they do not create a new view
  lifecycle or ownership model in this slice.
- Moving untyped `WeakView` out of `runtime_input_state.hpp` keeps pointer
  capture/input state focused while preserving the existing
  `WindowRuntimeContext::upgrade_view(WeakView)` compatibility path.

## 2026-07-04 Phase B Step 264 Public Authoring Surface

- The final Steps 259-264 public-name slice should be a prelude-level compile
  gate, not another low-level runtime helper. `cgpui/prelude.hpp` is the
  C++23 analogue to upstream `gpui::prelude`, while `cgpui/cgpui.hpp` remains
  a compatibility aggregate.
- The durable authoring guard is `public_authoring_surface_test`: it includes
  only `cgpui/prelude.hpp` and uses `Application`, `App`, `Window`,
  `Context<T>`, `Render`, `IntoElement`, typed view handles, and typed entity
  handles without spelling `WindowRuntimeContext`, `WindowRuntime`,
  `AppContext`, `ViewContext`, or `PlatformWindow`.
- This closes the public naming and C++ idiom band. Entity lifecycle,
  transactions, observations, deletion, and cross-context rules remain the
  next band rather than being smuggled into the prelude gate.

## 2026-07-04 Phase B Step 265 Entity Lifecycle Creation

- Step 265 should stay scoped to public entity creation spelling. The durable
  split is `Context<T>::new_entity<T>(...) -> EntityHandle<T>` and
  `insert_entity_handle(...)` over the existing runtime store; weak-handle
  semantics, observation, transactions, deletion, invalidation, and
  cross-context access rules remain future slices in Steps 266-270.
- The implementation belongs in the existing template context boundary:
  declarations in `include/cgpui/ui/runtime_context.hpp` and definitions in
  `include/cgpui/ui/runtime_templates.hpp`. No behavior needs to move into
  `ui.cpp`, `runtime_entities.cpp`, or broad runtime files for this slice.
- The RED test proved the intended public gap: a prelude-only authoring view
  could not call `context.new_entity<LifecycleState>(...)`. The GREEN path
  reuses `EntityHandle<T>::read(...)` and `EntityHandle<T>::update(...)`, so
  the slice demonstrates creation plus basic read/update handle ergonomics
  without claiming the rest of entity lifecycle parity.

## 2026-07-04 Phase B Step 266 Weak Entity Handles

- Step 266 should stay scoped to public weak-handle semantics. The durable
  shape is `WeakEntity<T>::upgrade(context) -> std::optional<EntityHandle<T>>`
  and `WeakEntity<T>::read(context) -> const T*`, layered over the existing
  `context.upgrade_entity(weak) -> std::optional<Model<T>>` runtime path.
- The implementation belongs in the focused public entity leaf
  `include/cgpui/core/entity.hpp`. The context/runtime compatibility methods
  already exist and do not need to move into `ui.cpp`, `runtime_entities.cpp`,
  or broader runtime files for this slice.
- The RED test proved the intended public gap: a prelude-only authoring view
  could downgrade an `EntityHandle<T>` but could not ask the `WeakEntity<T>` to
  upgrade or read itself. The GREEN path wraps the upgraded id back into an
  `EntityHandle<T>` and soft-fails with `std::nullopt` / `nullptr` when the
  entity is empty or no longer present.

## 2026-07-04 Phase B Step 267 Entity Observation

- Step 267 should stay scoped to public entity observation spelling. The
  durable shape is `EntityHandle<T>::observe(context, observer) -> bool`,
  `EntityHandle<T>::observe_subscription(context, observer) -> Subscription`,
  and matching `Context<T>::observe_entity(...)` /
  `observe_entity_subscription(...)` helpers.
- The implementation belongs in the existing public handle and context
  template boundaries: `include/cgpui/core/entity.hpp` owns the handle
  convenience methods, while `include/cgpui/ui/runtime_context.hpp` and
  `include/cgpui/ui/runtime_templates.hpp` own the context forwarding layer.
  No new observer storage, `ui.cpp` implementation, or broad runtime file
  change is needed for this slice.
- The GREEN path deliberately adapts existing `observe_model(...)` storage by
  wrapping callback ids back into `EntityHandle<T>`. Update transactions,
  deletion semantics, deterministic unsubscribe behavior, and cross-context
  access rules remain later slices.

## 2026-07-04 Phase B Step 268 Entity Update Transactions

- Step 268 should stay scoped to public entity update transaction spelling:
  `Context<T>::update_entity(handle, update)` and
  `EntityHandle<T>::update(context, update)` callbacks can receive both
  `T&` and `const Context<T>&`, and value-returning callbacks soft-fail with
  `std::optional<R>` when the entity is missing.
- Void update callbacks intentionally preserve the old `bool` result shape so
  existing `entity.update(context, [](T&) { ... })` code remains source
  compatible while routing through the new transaction helper.
- The implementation belongs in the existing template context boundary:
  `include/cgpui/core/entity.hpp` forwards handle updates, while
  `include/cgpui/ui/runtime_context.hpp` and
  `include/cgpui/ui/runtime_templates.hpp` own the transaction template.
  No new entity store, `ui.cpp` implementation, invalidation policy, deletion
  rule, or cross-context rule belongs in this slice.
- On Windows, xmake cannot reliably run a test executable whose target name
  contains `update`; `entity_update_transaction_test.exe` failed with
  `Unknown Error (740)` even though the binary exited 0 when run directly.
  Use `entity_transaction_test` as the target name while keeping the API under
  test as `update_entity(...)`.

## 2026-07-04 Phase B Step 269 Entity Invalidation

- Step 269 should stay scoped to explicit entity invalidation spelling:
  `EntityHandle<T>::invalidate(context) -> bool` and
  `Context<T>::invalidate_entity(handle) -> bool`. Empty and missing handles
  soft-fail with `false`; valid handles return `true`.
- Explicit invalidation is intentionally stronger than the old update return
  semantics: it notifies entity observers/subscribed views when present and
  still requests render invalidation when no observer was notified.
- Do not route `update_model(...)` or `update_entity(...)` through the explicit
  invalidation helper. Existing void update callers rely on the old `bool`
  result meaning "observer/subscriber was notified", so update helpers must
  keep calling `notify_entity_changed(...)`.
- The implementation belongs in the focused public entity/context template
  boundary: `include/cgpui/core/entity.hpp`,
  `include/cgpui/ui/runtime_context.hpp`,
  `include/cgpui/ui/window_runtime.hpp`, and
  `include/cgpui/ui/runtime_templates.hpp`. Deletion semantics and
  cross-context access rules remain future slices.

## 2026-07-04 Phase B Step 270 Entity Deletion Boundaries

- Step 270 closes the entity lifecycle band with public deletion spelling:
  `EntityHandle<T>::remove(context) -> bool` and
  `Context<T>::remove_entity(handle) -> bool`. Empty, missing, and already
  deleted handles soft-fail with `false`.
- Context-created `EntityHandle<T>` and `WeakEntity<T>` values now carry a
  runtime token. Handle reads, updates, invalidation, observation, removal, and
  weak upgrades reject mismatched tokens before touching the runtime store, so
  a handle from one `WindowRuntime` cannot affect another runtime's entity even
  when the type and numeric id match.
- Raw `EntityHandle<T>(EntityId<T>)` and `WeakEntity<T>(EntityId<T>)` remain
  unbound for low-level compatibility with existing runtime tests and helpers.
  The public context helpers bind new handles; compatibility ids still flow
  through the existing `EntityId<T>` methods.
- The implementation belongs in `include/cgpui/core/entity.hpp`,
  `include/cgpui/ui/runtime_context.hpp`, and
  `include/cgpui/ui/runtime_templates.hpp`. No new entity store or broad
  `ui.cpp`/`runtime_entities.cpp` rewrite is needed.

## 2026-07-04 Phase B Step 271 App Context Capability

- Step 271 starts the GPUI-like context-capability band with the app domain:
  `Context<T>::app_context() -> AppContext`. This reuses the existing
  `AppContext` facade rather than introducing a parallel capability type.
- The runtime implementation belongs beside the existing app/window facade
  bridge in `src/app/app_context_facade.cpp`; `runtime_context.hpp` only owns
  the declaration. This keeps the new capability out of `ui.cpp` and broad
  runtime implementation files.
- Runtime tests must avoid mutating globals from an after-frame callback in the
  fake platform window. `AppContext::set_global(...)` and
  `update_global(...)` request render, and the fake window dispatches redraw
  synchronously, so doing that inside after-frame recurses until stack
  overflow. Seed globals in setup, then read through `context.app_context()` in
  the frame callback.

## 2026-07-04 Phase B Step 272 View Context Capability

- Step 272 should stay scoped to public view-domain capability spelling:
  `Context<T>::view_context<T>() -> ViewContextCapability<T>`. The capability
  exposes the current `ViewId`, typed `ViewHandle<T>`, typed
  `WeakViewHandle<T>`, weak upgrade, read-only lookup, and current-view read.
- The implementation belongs in a focused public UI leaf,
  `include/cgpui/ui/view_context.hpp`, with template bodies in
  `include/cgpui/ui/runtime_templates.hpp`. It should not add a `.cpp`, grow
  `ui.cpp`, or introduce new runtime state for this slice.
- This is a facade over Step 263's typed view handle machinery. It does not
  claim view lifecycle, view observation, subscriptions, child-view ownership,
  or cross-runtime view-token behavior; those remain later Phase B slices.
- Parity guards that reject low-level authoring leaks must not reject the
  intentional public spelling `ViewContextCapability<T>`. Guard for
  `WindowRuntimeContext`, `WindowRuntime`, `AppContext`, and platform types
  instead.

## 2026-07-04 Phase B Step 273 Window Context Capability

- Step 273 should stay scoped to public window-domain capability spelling:
  `Context<T>::window_context() -> WindowContextCapability`. The capability
  groups the existing current-window facade queries and request helpers under
  one author-facing domain.
- The implementation belongs in the focused UI public leaf
  `include/cgpui/ui/window_context.hpp` and the focused UI source
  `src/ui/window_context.cpp`. It should not grow `ui.cpp`,
  `runtime_context.cpp`, or the broader app-context bridge file.
- This is a facade over the existing `Window` runtime facade, not a new native
  window lifecycle. Native child-window creation, event routing depth, and
  platform behavior remain later Phase B/F work.
- Header cleanliness coverage should prove the public type and template
  observation helpers are usable from UI leaf includes. Non-template
  `WindowContextCapability` methods now live in `cgpui_ui` so the UI target
  owns the symbols its public headers instantiate.

## 2026-07-04 Phase B Step 274 Element Context Capability

- Step 274 should stay scoped to public element-domain capability spelling:
  `Context<T>::element_context(ElementId) -> ElementContextCapability`. The
  capability groups existing element focus, keyboard focus, pointer capture,
  cursor, focus-handle, and typed element state helpers.
- The implementation belongs in a focused public UI leaf,
  `include/cgpui/ui/element_context.hpp`, with non-template forwarding in
  `src/ui/element_context.cpp` and template state helpers in
  `include/cgpui/ui/runtime_templates.hpp`. It should not grow `ui.cpp`,
  `runtime_context.cpp`, or broad runtime implementation files.
- The API is a facade over existing `WindowRuntimeContext` element helpers. It
  does not introduce new element tree ownership, event routing, view lifecycle,
  or entity context-token behavior; those remain separate slices.
- Keep tests aligned with the existing cursor vocabulary:
  `CursorShape::pointing_hand` is the current public spelling for pointer-like
  cursor behavior, so Step 274 does not add a `CursorShape::pointer` alias.

## 2026-07-04 Phase B Step 275 Async Context Capability

- Step 275 should stay scoped to public async-domain capability spelling:
  `Context<T>::async_context() -> AsyncContextCapability`. The capability
  groups existing runtime scheduling helpers: `defer`, one-shot/repeating
  timers, animation start/snapshot/cancel, foreground task completion,
  background task spawning, and update batches.
- The implementation belongs in a focused public UI leaf,
  `include/cgpui/ui/async_context.hpp`, with non-template forwarding in
  `src/ui/async_context.cpp`. It should not grow `ui.cpp`,
  `runtime_context.cpp`, or `runtime_context_scheduling.cpp`; those remain the
  lower-level runtime forwarding boundary.
- This is a facade over the existing manual/runtime task executor and
  deterministic timer/animation queues. It does not add task pools, priorities,
  structured task groups, async I/O hooks, or cross-thread entity access
  semantics; those stay in the later full async executor phase.
- The AsyncWindowContext ledger row can move to `Adapted` for this partial
  public capability, but its next step must still point to Phase G full async
  executor depth.

## 2026-07-04 Phase B Step 276 Test Context Capability

- Step 276 should stay scoped to public test-context capability spelling:
  `Context<T>::test_context() -> TestContextCapability`. The capability
  groups deterministic test-facing runtime observability and queue controls:
  runtime/view/frame ids, input/event/action snapshots, invalidation reset,
  diagnostics, timer advancement/cancellation, task completion, and task
  completion draining.
- The implementation belongs in a focused public UI leaf,
  `include/cgpui/ui/test_context.hpp`, with non-template forwarding in
  `src/ui/test_context.cpp`. It should not grow `ui.cpp`,
  `runtime_context.cpp`, or lower-level scheduling files.
- This is a facade over existing deterministic runtime hooks. It deliberately
  does not add simulated keyboard/pointer input, focus/clipboard helpers,
  redraw simulation, an upstream-style `gpui::test` macro, or a new harness;
  those remain in the later Step 295-300 / Phase G test behavior band.

## 2026-07-04 Phase B Step 277 Subscription Lifetime

- Step 277 should stay scoped to deterministic `Subscription` lifetime and
  unsubscribe behavior. It does not start entity-to-entity observation,
  window/view observation, action dispatch, key dispatch, or fuller test
  context simulation.
- The durable ownership boundary is `src/ui/subscription.cpp` for the public
  move-only RAII token and `src/ui/runtime_subscriptions.cpp` for runtime
  subscription lookup/removal. `src/ui/ui.cpp` should not own
  `Subscription::` methods, and diagnostics snapshot construction should not
  own subscription removal.
- Deterministic unsubscribe now means explicit `release()`, move-assignment
  release, and scoped destruction remove the observer record from
  `entity_observers_` once. Duplicate release, moved-from release, missing ids,
  and release after external removal soft-fail with `false`.
- Because `remove_subscription(...)` erases observer records, entity-change
  notification should not iterate by live vector reference while callbacks run.
  The implementation builds a pending callback list and rechecks subscribed
  observer ids before invocation, so callbacks can release subscriptions
  without invalidating the active traversal.

## 2026-07-04 Phase B Step 278 Entity-To-Entity Observation

- Step 278 should stay scoped to public entity-to-entity observation spelling:
  `EntityHandle<ObserverT>::observe_entity(context, observed, observer)`,
  `EntityHandle<ObserverT>::observe_entity_subscription(...)`, and matching
  three-argument `Context<T>::observe_entity(...)` /
  `observe_entity_subscription(...)` overloads.
- The implementation belongs in the existing public handle/context template
  boundary: `include/cgpui/core/entity.hpp`,
  `include/cgpui/ui/runtime_context.hpp`, and
  `include/cgpui/ui/runtime_templates.hpp`. It should not add a new runtime
  store or move observer behavior into `ui.cpp` or broad implementation files.
- The C++ adaptation of upstream GPUI observation callbacks passes mutable
  observer entity state, the observed `EntityHandle<ObservedT>`, and the
  current context. The callback mutates observer state directly but does not
  introduce a new observer-entity invalidation policy in this slice.
- Registration soft-fails when either handle is empty, belongs to another
  runtime token, or when the observer entity is missing. If the observer entity
  is removed after registration, callbacks skip it; deterministic unsubscribe
  remains owned by Step 277's `Subscription::release()` path.

## 2026-07-04 Phase B Step 279 Window/View Observation

- Step 279 should stay scoped to public window/view observation spelling:
  `Context<T>::observe_window(...)`, `Context<T>::observe_view(...)`,
  `WindowContextCapability::observe(...)`,
  `ViewContextCapability<T>::observe(...)`, and
  `ViewHandle<T>::observe(...)`, each with subscription-returning variants.
- Runtime observer storage belongs in the focused
  `src/ui/runtime_observations.cpp` implementation, with scheduling triggers
  in `src/ui/runtime_scheduling.cpp` and subscription erasure in
  `src/ui/runtime_subscriptions.cpp`. It should not grow `ui.cpp` or turn
  diagnostics snapshot construction into observer ownership code.
- `Window` and `WindowContextCapability` non-template implementation belongs
  in `cgpui_ui` (`src/ui/window.cpp` and `src/ui/window_context.cpp`) because
  the public UI templates instantiate those symbols. Keeping them in
  `cgpui_app` makes UI-header users depend on an app target just to link an
  observation callback.
- Window/view observers are tied to the existing invalidation/request flow:
  `request_render`, `request_layout`, and `request_paint` notify observers
  before scheduling redraw. `Subscription::release()` removes window and view
  observer records through the same deterministic token path as entity
  observers, and removed view records drop their view observers.

## 2026-07-04 Phase B Step 280 Observation Diagnostics

- Step 280 should stay scoped to making observation/subscription state
  observable through diagnostics. It should not introduce new observer storage,
  new callback ordering, action dispatch behavior, key dispatch behavior, or
  simulated test-context input.
- `RuntimeDiagnosticsSnapshot::entity_observer_count` remains entity-only for
  source compatibility. Add separate `window_observer_count` and
  `view_observer_count` fields instead of changing its meaning.
- The implementation belongs in the existing focused diagnostics boundary:
  `include/cgpui/ui/runtime_diagnostics.hpp` for public snapshot shape and
  `src/ui/runtime_diagnostic_snapshot.cpp` for count collection. The behavior
  test belongs in a focused `tests/ui/window_runtime_observation_diagnostics_test.cpp`
  target rather than further growing `window_runtime_actions_test.cpp`.

## 2026-07-04 Phase B Step 281 View Handle Runtime Token

- Step 281 should stay scoped to view-handle runtime-token boundaries in the
  observation/subscription band. It should not start child-view lifecycle
  ownership, action dispatch, key dispatch, or simulated test-context input.
- The gap was that `ViewHandle<T>` and `WeakViewHandle<T>` only carried a
  `ViewId`, so a handle captured from one `WindowRuntime` could read, observe,
  or weak-upgrade against another runtime when both runtimes had the same
  numeric root `ViewId`.
- Mirror the existing entity-handle token shape in the focused public leaf
  `include/cgpui/ui/view_handle.hpp`, bind handles in
  `include/cgpui/ui/runtime_templates.hpp`, and enforce weak upgrades in
  `src/ui/runtime_views.cpp`. Keep raw `ViewHandle<T>(ViewId)` and
  `WeakView(ViewId)` unbound for low-level compatibility.
- The behavior test belongs in a focused
  `tests/ui/window_runtime_view_handle_token_test.cpp` target rather than
  broadening `window_runtime_actions_test.cpp` or `window_runtime_multiwindow_test.cpp`.

## 2026-07-05 Phase B Step 282 Observation/Subscription Closure

- Step 282 should close the observation/subscription band before action/key
  dispatch begins. It should stay scoped to stale observer/subscription state,
  parity ledger evidence, and focused runtime guards.
- The concrete gap found in the closure audit was that `WindowRuntime::remove_view(...)`
  cleared view observers but left view-owned entity subscriptions in
  `entity_subscriptions_`, so `subscriptions_for_view(removed_view)` and
  `RuntimeDiagnosticsSnapshot::view_entity_subscription_count` could retain
  stale state after a view was removed.
- The implementation belongs in `src/ui/runtime_views.cpp` at the existing
  view-removal ownership boundary by calling `remove_subscriptions_for_view`
  before `remove_observers_for_view`. It should not move subscription storage
  out of `src/ui/runtime_subscriptions.cpp` or broaden observer storage.
- The behavior guard belongs in a focused
  `tests/ui/window_runtime_observation_closure_test.cpp` target so the
  runtime-level lifecycle cleanup is covered without growing API parity
  compile-only tests.

## 2026-07-05 Phase B Step 283 Typed Action Surface

- Step 283 should stay scoped to public typed-action authoring. The durable
  first surface is `Action<T>`, `action_name<T>()`, and `action_name_v<T>` for
  action structs that expose a static `std::string_view name`.
- The implementation belongs in the focused public UI leaf
  `include/cgpui/ui/action.hpp`, with `include/cgpui/ui/ui.hpp` and
  `include/cgpui/ui/runtime.hpp` acting only as thin aggregates. It should not
  add a `.cpp`, grow `ui.cpp`, or move string registry types out of
  `runtime_actions.hpp` in this slice.
- The public API parity guard belongs in
  `tests/api_parity/typed_action_surface_test.cpp` and should include only
  `cgpui/prelude.hpp`; it must not pull in `WindowRuntimeContext`,
  `WindowRuntime`, platform types, `KeyBinding`, or dispatch behavior.
- Keep typed registration/dispatch overloads, command metadata binding, key
  context grammar, key routing, and fuller simulated test-context input for the
  next action/key-dispatch slices.

## 2026-07-05 Phase B Step 284 Typed Action Registration/Dispatch

- Step 284 should stay scoped to typed action registration and dispatch
  overloads over the existing string-keyed action registry. It should not start
  key dispatch, key grammar, command metadata, enablement, bubbling, or fuller
  test-context input.
- Keep typed authoring helpers in `include/cgpui/ui/action.hpp`; add overload
  declarations at the existing runtime/context action ownership boundary and
  put the template definitions in a focused
  `include/cgpui/ui/runtime_action_templates.hpp` leaf instead of introducing
  another registry. Aggregate headers should remain thin.
- Behavior coverage should prove that an action struct with `static
  std::string_view name` can be registered and dispatched through the typed API,
  while preserving existing string action behavior and last-dispatch
  observability.

## 2026-07-05 Phase B Step 285 Action Scope Metadata

- Step 285 should stay scoped to recording action registration metadata over
  the existing scoped registry. It should not start command metadata binding,
  key dispatch, key grammar, enablement, bubbling, or fuller simulated
  test-context input.
- The important distinction is registration source vs dispatch scope:
  `register_action(...)` is a general/legacy registration source but still
  dispatches with `ActionScope::app` for source compatibility.
- The ownership boundary is `include/cgpui/ui/runtime_actions.hpp` for
  `ActionRegistrationScope` / `ActionRegistration`, existing runtime/context
  action declarations for read APIs, and `src/ui/runtime_action_metadata.cpp`
  for upsert/query behavior. `runtime_action_dispatch.cpp` should continue to
  own handler registration and dispatch, and command palette/key binding code
  should not grow this metadata behavior.

## 2026-07-05 Phase B Step 286 Typed Action Command Metadata

- Step 286 should stay scoped to binding typed action names into command
  palette metadata. It should not start key dispatch, key grammar, action
  enablement, bubbling, or fuller simulated test-context input.
- The correct module boundary is a focused template leaf:
  `include/cgpui/ui/runtime_command_palette_templates.hpp`. Keeping
  `CommandPaletteEntry` out of `runtime_action_templates.hpp` preserves the
  split between action registration/dispatch templates and command-palette
  metadata helpers.
- Typed command helpers fill `CommandPaletteEntry::action_name` from
  `action_name<T>()` and preserve the rest of the metadata payload: title,
  group, action scope, optional view id, optional element id, and enabled
  state. The existing string-based command registry and dispatch behavior stay
  source-compatible.
- Tests that call `Context<T>::app_context()` or `WindowRuntimeContext::app_context()`
  need to link `cgpui_app`; plain UI runtime tests that do not cross into the
  app facade can continue to link only `cgpui_ui`.

## 2026-07-05 Phase B Step 287 Action Enablement Metadata

- Step 287 should stay scoped to action registration enablement metadata and
  dispatch suppression. It should not start action payload macros, key routing,
  key grammar, focused-route bubbling, or fuller simulated test-context input.
- The ownership boundary is `include/cgpui/ui/runtime_actions.hpp` for
  `ActionRegistrationOptions` and `ActionRegistration::enabled`,
  `include/cgpui/ui/runtime_action_enablement_templates.hpp` for typed
  overloads that accept options, `src/ui/runtime_action_registration.cpp` for
  non-template registration, and `src/ui/runtime_action_metadata.cpp` for
  enabled-state queries and dispatch lookup.
- `src/ui/runtime_action_dispatch.cpp` should remain dispatch-only. It may ask
  metadata whether a candidate handler is enabled, but registration upsert,
  enabled-state filtering, command palette metadata, key bindings, and text
  action helpers belong elsewhere.
- Disabled registrations suppress handler invocation and produce an unhandled
  dispatch result at that scope. This is enablement metadata, not full GPUI
  action bubbling yet; Step 288 should decide how disabled focused/view/window
  handlers interact with route bubbling.
- Same-dispatch-key registrations must align enablement lookup with the
  existing handler-map behavior where the latest registration wins. The durable
  fix is for `upsert_action_registration(...)` to keep refreshed keys at the
  newest position and for `action_registration_enabled(...)` to search matching
  dispatch keys from newest to oldest.

## 2026-07-05 Phase B Step 288 Action Bubbling

- Step 288 should stay scoped to action bubbling through focused routes. It
  should not start key dispatch, key grammar, command-palette routing, action
  payload macros, or fuller simulated test-context input.
- The bubbling order remains focused element, view, window, then app. A
  disabled candidate registration is skipped and dispatch continues outward.
  A handler that returns `EventResult::unhandled()` also continues outward.
  A handler that returns consumed or cancelled stops bubbling and fills
  `ActionDispatchResult` with the stopping scope and optional view/element id.
- The implementation belongs in the focused dispatch boundary,
  `src/ui/runtime_action_dispatch.cpp`, while registration upsert, enabled
  filtering, command metadata, key binding grammar, and text edit bindings
  remain outside this file.
- The behavior guard belongs in `tests/ui/action_bubbling_test.cpp` so the
  runtime chain semantics stay separate from typed action surface,
  registration metadata, and key-dispatch tests.

## 2026-07-05 Phase B Step 289 Key Binding Grammar

- Step 289 should stay scoped to GPUI-style single-chord key binding grammar
  and author-facing string registration. It should not start platform
  modifier normalization, keymap contexts, partial matches, disabled scopes,
  command palette integration, action payload macros, or fuller test-context
  input simulation.
- The ownership boundary is `include/cgpui/ui/key_binding.hpp` for
  `KeyBinding`, `TextEditBinding`, and `parse_key_binding(...)`,
  `src/ui/runtime_key_binding_grammar.cpp` for grammar parsing, and the
  existing `src/ui/runtime_key_bindings.cpp` for pushing structured bindings.
  `runtime_actions.hpp` should remain action/command metadata only and include
  the key-binding leaf for source compatibility instead of defining key
  binding structs itself.
- `Context<T>::bind_key("ctrl-shift-s", "action")` parses and forwards to the
  existing structured `WindowRuntime::bind_key(KeyBinding)` path. Invalid
  grammar returns `false` and does not register a binding.

## 2026-07-05 Phase B Step 290 Platform Modifier Semantics

- Step 290 should stay scoped to platform modifier normalization for the
  existing single-chord grammar. It should not start keymap contexts, partial
  matches, disabled scopes, command palette integration, action payload macros,
  or fuller simulated test-context input.
- The ownership boundary is `src/ui/runtime_key_binding_modifiers.cpp` for
  platform alias semantics, `src/ui/key_binding_internal.hpp` for private
  parser sharing, `src/ui/runtime_key_binding_grammar.cpp` for tokenization,
  and `include/cgpui/ui/key_binding.hpp` for the public overload that accepts
  an explicit `DesktopPlatformTarget`.
- `secondary-*` is the cross-platform primary command modifier: Ctrl on
  Windows/Linux and Super on macOS. `platform-*`, `cmd-*`, `command-*`,
  `super-*`, `meta-*`, `win-*`, and `windows-*` bind the platform key in the
  current modifier model. Duplicate semantic modifiers should fail rather than
  silently collapse.
- `Context<T>::bind_key(...)` remains current-platform behavior by forwarding
  through `current_desktop_platform_target()`. Explicit platform-target parsing
  exists for deterministic parity tests and future macOS API correctness, not
  as a signal to begin macOS runtime/backend work in this slice.

## 2026-07-05 Phase B Step 291 Keymap Contexts

- Step 291 should stay scoped to context-aware key binding activation over the
  existing single-chord key binding table. It should not start partial matches,
  disabled key scopes, command palette integration, action payload macros, or
  fuller simulated test-context input.
- The public API boundary is `include/cgpui/ui/key_binding.hpp`:
  `KeyBindingContextKind`, `KeyBindingContext`, `KeyBinding::context`, and
  the context-aware `WindowRuntimeContext::bind_key(..., KeyBindingContext)`
  overload. Existing two-argument `bind_key(...)` remains app-context behavior
  for source compatibility.
- Runtime context filtering belongs in the focused
  `src/ui/runtime_key_binding_contexts.cpp` source. It selects the most
  specific active context for matching chords in focused-element, view,
  window, then app order, while `runtime_key_bindings.cpp` remains registration
  storage and `runtime_event_keyboard.cpp` remains a thin keyboard-event entry.
- `KeyBindingContext::view(ViewId)` matches the current routed view ancestry
  when an event route exists and falls back to the root view outside event
  routing. `KeyBindingContext::focused_element(ElementId)` matches the current
  keyboard focus element owner exactly.

## 2026-07-05 Phase B Step 292 Partial Key Matches

- Step 292 should stay scoped to multi-chord key binding grammar and pending
  partial-match dispatch. It should not start disabled key scopes, command
  palette integration, action payload macros, or fuller simulated test-context
  input.
- The public API boundary remains `include/cgpui/ui/key_binding.hpp`: add
  `KeyBindingChord` and `KeyBinding::sequence` while preserving legacy
  `KeyBinding::key_code` / `action` / `modifiers` fields for one-chord
  construction and source compatibility.
- Grammar parsing stays in `src/ui/runtime_key_binding_grammar.cpp`:
  whitespace separates chords, while existing `-` / `+` token splitting still
  separates modifiers inside each chord. `parse_key_binding("ctrl-k ctrl-s",
  ...)` records both chords and keeps the first chord mirrored in the legacy
  fields.
- Runtime partial-match state belongs in the focused
  `src/ui/runtime_key_binding_sequences.cpp` source. It owns pending sequence
  storage, prefix/exact matching, context-rank selection for completed
  sequences, and clearing pending state on mismatch. Context activation remains
  in `src/ui/runtime_key_binding_contexts.cpp`, registration storage remains
  in `src/ui/runtime_key_bindings.cpp`, and `runtime_event_keyboard.cpp`
  remains a thin keyboard-event entry.

## 2026-07-05 Phase B Step 293 Disabled Key Scopes

- Step 293 should stay scoped to disabled key binding scopes. It should not
  start command palette key integration, action payload macros, or fuller
  simulated test-context input.
- The public API boundary remains `include/cgpui/ui/key_binding.hpp`: add a
  boolean `KeyBindingContext::enabled` state plus `KeyBindingContext::disabled()`
  authoring helper. This keeps disabled scope metadata with the scope object
  rather than adding command-palette or action registration coupling.
- Runtime filtering belongs in `src/ui/runtime_key_binding_contexts.cpp` by
  making `WindowRuntime::key_binding_context_active(...)` return false for
  disabled contexts before app/window/view/focused-element matching.
- Sequence dispatch in `src/ui/runtime_key_binding_sequences.cpp` should keep
  asking context activation first, so disabled scopes are skipped for both
  exact matches and partial prefix matches. A disabled more-specific binding
  should not block an outer enabled binding or leave pending sequence state.

## 2026-07-05 Phase B Step 294 Command Palette Key Integration

- Step 294 should stay scoped to wiring command palette metadata to the
  existing key-binding dispatch path. It should not start action payload
  macros or Step 295+ simulated test-context input.
- Current `CommandPaletteEntry` records action name, title, group, scope,
  enabled state, and optional view/element ids. It does not carry a key grammar
  string and `WindowRuntime::register_command_palette_entry(...)` only stores
  valid entries.
- The smallest aligned API is to add a command-palette-owned key grammar field
  plus an optional explicit `KeyBindingContext`. Registration can parse the
  grammar and call the existing `WindowRuntime::bind_key(KeyBinding)` path
  without coupling command palette templates to key binding internals.
- Default key context should be derived from the entry scope: app, window,
  view when `view_id` is present, or focused-element when `element_id` is
  present. Disabled entries should not install key bindings, because direct
  key dispatch would otherwise bypass `CommandPaletteEntry::enabled`.
- Implementation ownership belongs in a focused
  `src/ui/runtime_command_palette_keys.cpp` source. The registry/query/dispatch
  file should call a narrow private `WindowRuntime::command_palette_key_binding`
  helper but should not own `parse_key_binding(...)` or `KeyBindingContext`
  derivation logic.

## 2026-07-05 Phase B Step 295 Test-Context Keystroke Simulation

- Step 295 should stay scoped to keyboard simulation on the public
  `TestContextCapability`. It should not start pointer simulation, focus/window
  activation simulation, clipboard helpers, timer/async advancement, redraw
  pumping, `gpui::test` macro equivalents, or action payload macro work.
- The correct runtime path is to parse GPUI-style key grammar with the existing
  `parse_key_binding(...)`, convert each parsed chord to `KeyboardKey`, and
  call `WindowRuntime::handle_event(PlatformEvent{key})`. This keeps simulated
  keystrokes on the same event/keymap/action path as native keyboard input and
  avoids a test-only shortcut into `dispatch_action(...)`.
- Implementation ownership belongs in the focused
  `src/ui/test_context_keystrokes.cpp` leaf. `src/ui/test_context.cpp` remains
  the observability/queue-control capability file, and
  `src/ui/window_runtime_internal.hpp` should not grow beyond the existing
  structure guard just to expose test-only hooks.
- The behavior guard belongs in
  `tests/ui/test_context_keystroke_simulation_test.cpp`: it should prove
  grammar-backed single-chord and multi-chord simulation, invalid grammar soft
  failure, direct `dispatch_keystroke(...)`, action dispatch observability, and
  real keyboard event delivery through the runtime.
- Step 296-300 should continue the same modular-first pattern: Step 296 pointer
  input simulation in a focused test-context pointer source, Step 297
  focus/window activation simulation in a focused focus/window source, Step 298
  clipboard helpers in a focused test-context clipboard source, Step 299
  timer/async advancement helpers in focused scheduling/test-context sources,
  and Step 300 redraw/frame pump simulation in a focused rendering/test-context
  source. Do not add these follow-on behaviors to `ui.cpp` or the broad
  `test_context.cpp` file after the fact.

## 2026-07-05 Phase B Step 296 Test-Context Pointer Simulation

- Step 296 stays scoped to pointer input simulation on the public
  `TestContextCapability`. It should not start focus/window activation,
  clipboard helpers, timer/async advancement, redraw pumping,
  `gpui::test` macro equivalents, or action payload macro work.
- The correct runtime path is direct event dispatch through
  `context_->runtime.handle_event(PlatformEvent{PointerMoved{...}})`,
  `PointerButton{...}`, and `PointerScrolled{...}`. This keeps simulated
  pointer input on the same input-state, hit-testing, route, view fallback, and
  after-event path as native pointer input.
- Implementation ownership belongs in the focused
  `src/ui/test_context_pointer.cpp` leaf. `src/ui/test_context.cpp` remains
  the observability/queue-control capability file, and
  `src/ui/test_context_keystrokes.cpp` remains keyboard-only.
- The behavior guard belongs in
  `tests/ui/test_context_pointer_simulation_test.cpp`: it should prove direct
  pointer move/button/scroll helpers update input state, emit the expected
  `EventKind` records, and preserve hit-test routing through the real runtime
  dispatch path.

## 2026-07-05 Phase B Step 297 Test-Context Focus Activation

- Step 297 stays scoped to focus/window activation simulation on the public
  `TestContextCapability`. It should not start clipboard helpers,
  timer/async advancement, redraw pumping, `gpui::test` macro equivalents, or
  action payload macro work.
- Window activation/focus simulation belongs in a focused
  `src/ui/test_context_focus.cpp` source and should dispatch
  `WindowActivated` / `WindowFocused` through
  `context_->runtime.handle_event(...)`, matching native window lifecycle and
  focus event paths.
- Element focus helpers should call the existing runtime focus APIs:
  `request_keyboard_focus(ElementId)` and `release_keyboard_focus(ElementId)`.
  They should not invent test-only focus storage.
- The concrete debugging finding was that `WindowRuntimeContext::input_state()`
  is an event-context snapshot, while `TestContextCapability::input_state()`
  needs to observe live runtime state after simulation helpers mutate focus or
  input. The durable fix is for the test-context capability to read
  `context_->runtime.input_state()`.
- The behavior guard belongs in
  `tests/ui/test_context_focus_activation_test.cpp`: it should prove window
  activation/focus dispatch produces runtime event records, element focus
  routes keyboard events to the focused element, and release clears keyboard
  focus.

## 2026-07-05 Phase B Step 298 Test-Context Clipboard Helpers

- Step 298 stays scoped to test-context clipboard helpers over the current
  text-only `Clipboard` contract. It should not introduce upstream
  `ClipboardItem`, test macro equivalents, timer/async advancement, redraw
  pumping, or broader platform clipboard production behavior.
- The public API should keep upstream-adjacent names for direct test-context
  clipboard access: `set_clipboard(Clipboard*)`, `write_to_clipboard(...)`,
  and `read_from_clipboard()`. Existing runtime editing operations should stay
  visible as `paste_clipboard_text()`, `copy_selection_to_clipboard()`, and
  `cut_selection_to_clipboard()`.
- Implementation ownership belongs in the focused
  `src/ui/test_context_clipboard.cpp` leaf, forwarding through real
  `WindowRuntime` clipboard APIs. Runtime text read/write helpers belong in
  `src/ui/runtime_clipboard.cpp` next to copy/cut/paste diagnostics.
- The structure guard belongs in `tests/architecture/ui_source_structure_test.cpp`
  so future test-context clipboard behavior does not drift into broad
  `src/ui/test_context.cpp` or unrelated capability files.
- The behavior guard belongs in `tests/ui/test_context_clipboard_test.cpp`: it
  should prove direct read/write through an injected clipboard, copy/cut/paste
  forwarding through focused text state, and soft failure when no clipboard is
  installed.

## 2026-07-05 Phase B Step 299 Test-Context Timer Async Advancement

- Step 299 stays scoped to deterministic timer/async advancement on the public
  `TestContextCapability`. It should not start redraw/frame pump simulation,
  upstream `gpui::test` macro equivalents, `ClipboardItem` payload parity, or
  action macro payload work.
- The upstream-adjacent API is `run_until_parked()`. In CGPUI it should drain
  currently runnable work through the real runtime wakeup order: queued task
  completions, timers already due at the current deterministic clock, deferred
  callbacks, and deferred redraw flushing. It must not fast-forward future
  timers.
- `advance_time_until_parked(delta_ms)` is the deterministic convenience
  helper: advance the existing runtime clock by `delta_ms`, then run until
  parked. Existing `advance_time(...)`, `complete_task(...)`,
  `cancel_timer(...)`, and `drain_task_completions()` remain available as
  lower-level controls.
- Implementation ownership belongs in focused
  `src/ui/test_context_scheduling.cpp`. `src/ui/test_context.cpp` should keep
  only observability and `WindowRuntimeContext::test_context()` construction,
  and future redraw/frame pumping should go in a separate rendering/test
  context source.
- The behavior guard belongs in `tests/ui/test_context_time_async_test.cpp`:
  it should prove nested ready work drains to parked, future timers stay
  pending until time is advanced, and time advancement then parks again.

## 2026-07-05 Phase B Step 300 Test-Context Frame Pump

- Step 300 stays scoped to redraw/frame pumping on the public
  `TestContextCapability`. It should not start upstream `gpui::test` macro
  equivalents, `ClipboardItem` payload parity, action macro payload work, or
  broader visual element drawing helpers.
- The upstream-adjacent behavior is a window-bound test draw path: CGPUI keeps
  this as `request_redraw()` for scheduling through the runtime redraw
  coalescing path and `draw_frame()` for directly simulating a platform
  `WindowRedrawRequested` frame. Both helpers pump the real runtime renderer,
  after-frame callback, diagnostics, and frame-index path.
- Implementation ownership belongs in focused
  `src/ui/test_context_rendering.cpp`. `src/ui/test_context.cpp` remains the
  observability/construction file, `src/ui/test_context_scheduling.cpp`
  remains timer/async-only, and renderer/runtime internals should not grow
  test-only shortcuts.
- The behavior guard belongs in `tests/ui/test_context_frame_pump_test.cpp`:
  it should prove `request_redraw()` goes through platform redraw scheduling,
  `draw_frame()` can pump a frame without issuing another platform redraw
  request, and runtime diagnostics track the pumped frames.

## 2026-07-05 Phase B Step 301 Public Result Conventions

- Step 301 stays scoped to public window-opening result conventions. It should
  not start platform service result conversion, async-spawn result conversion,
  renderer factory public API redesign, or broad exception/error rewrites.
- The public API shape is `try_open_window(...) -> Result<AppOpenedWindow>` on
  `WindowRuntime`, `App`, and `AppContext`, while existing
  `open_window(...) -> AppOpenedWindow` remains source-compatible.
- `try_open_window(...)` should not simply wrap `open_window(...)`, because
  doing so publishes failed app-opened windows before returning `Error`. The
  Result path needs its own focused creation flow that publishes records only
  after platform-window and renderer creation both succeed.
- Implementation ownership belongs in `src/ui/runtime_window_results.cpp`.
  App facade forwarding stays in `src/app/app_facade.cpp`; app-context
  service forwarding stays in `src/ui/app_context_services.cpp`; the original
  compatibility record path stays in `src/ui/runtime_windows.cpp`.
- The behavior guard belongs in
  `tests/api_parity/public_result_conventions_test.cpp`, covering API spelling,
  platform-window failure, renderer failure, and facade/context forwarding.

## 2026-07-05 Phase B Step 301 Merge

- Step 301 is merged on `master` at `53e625a feat: add public window result
  conventions` and post-merge verified on Windows full debug 87/87 and WSL
  Arch Linux full debug 84/84.
- The durable ownership boundary for public window Result conventions is now
  `src/ui/runtime_window_results.cpp`. Keep the compatibility
  `open_window(...)` record-publishing path in `src/ui/runtime_windows.cpp`.
- Step 302 should start platform service result conventions without folding
  async-spawn result conversion, renderer factory public API redesign, or
  broader exception/error rewrites into the same slice.

## 2026-07-05 Phase B Step 302 Platform Service Result Conventions

- Step 302 stays scoped to native menu and native file-dialog Result
  conventions. It should not start async-spawn result conversion, renderer
  factory public API redesign, `ClipboardItem` payload parity, upstream
  `gpui::test` macro equivalents, or production-depth native service work.
- The public API shape is `try_install_native_menu(...) ->
  Result<NativeMenuInstallation>` and `try_show_native_file_dialog(...) ->
  Result<NativeFileDialogResult>` on `WindowRuntime`, `AppContext`, and
  `ViewContext`/`WindowRuntimeContext`. Existing `install_native_menu(...)` and
  `show_native_file_dialog(...)` compatibility methods remain source-compatible.
- Unsupported platform services should return `ErrorCode::unsupported_platform`
  and must not overwrite the last successful runtime service state. Supported
  file-dialog cancellation (`accepted == false`) remains a successful value,
  because user cancellation is not a platform failure.
- Implementation ownership belongs in focused
  `src/ui/runtime_platform_service_results.cpp`; compatibility menu/dialog
  storage remains in `src/ui/runtime_platform_services.cpp`, and app/context
  files only forward.
- The behavior and API guard belongs in
  `tests/api_parity/platform_service_result_conventions_test.cpp`; the
  structure guard belongs in `tests/architecture/ui_source_structure_test.cpp`.

## 2026-07-05 Phase B Step 302 Merge

- Step 302 is merged on `master` at
  `8998ec2 feat: add platform service result conventions` and post-merge
  verified on Windows full debug 88/88 and WSL Arch Linux full debug 85/85.
- The durable ownership boundary for platform service Result conventions is
  `src/ui/runtime_platform_service_results.cpp`; keep compatibility
  menu/dialog state storage in `src/ui/runtime_platform_services.cpp`.
- Step 303 should start async-spawn result conventions without folding
  renderer factory public API redesign, `ClipboardItem` payload parity,
  upstream `gpui::test` macro equivalents, or action macro payloads into the
  same slice.

## 2026-07-05 Phase B Step 303 Async-Spawn Result Conventions

- Step 303 stays scoped to foreground/background task spawn Result conventions.
  It should not start renderer-creation result redesign, `ClipboardItem`
  payload parity, upstream `gpui::test` macro equivalents, action macro
  payloads, task priorities, structured task groups, or broader async runtime
  production depth.
- The public API shape is `try_spawn_task(...) -> Result<TaskHandle>` and
  `try_spawn_background_task(...) -> Result<TaskHandle>` on `WindowRuntime`,
  `WindowRuntimeContext`, and `AsyncContextCapability`. Existing
  `spawn_task(...)` and `spawn_background_task(...)` compatibility methods
  remain source-compatible and keep returning an empty `TaskHandle` for invalid
  callbacks.
- Runtime Result validation belongs in focused
  `src/ui/runtime_task_results.cpp`. Existing task record creation,
  completion draining, and worker-thread ownership remain in
  `src/ui/runtime_tasks.cpp`; task state/cancellation queries remain in
  `src/ui/runtime_task_state.cpp`.
- Invalid foreground completion, background work, and background completion
  callbacks return `ErrorCode::invalid_argument` with specific messages before
  task records are allocated. The structure guard should require
  `runtime_task_results.cpp` and reject `Result<TaskHandle>` drift back into
  `runtime_tasks.cpp`.

## 2026-07-05 Phase B Step 303 Merge

- Step 303 is merged on `master` at
  `085cd30 feat: add async spawn result conventions` and post-merge verified
  on Windows full debug 89/89 and WSL Arch Linux full debug 86/86.
- The durable ownership boundary for async-spawn Result conventions is
  `src/ui/runtime_task_results.cpp`; keep task record creation and completion
  draining in `src/ui/runtime_tasks.cpp`, and keep task state/cancellation
  queries in `src/ui/runtime_task_state.cpp`.
- Step 304 should start renderer-creation result conventions without folding
  `ClipboardItem` payload parity, upstream `gpui::test` macro equivalents,
  action macro payloads, task priorities, structured task groups, or broader
  async runtime production depth into the same slice.

## 2026-07-05 Phase B Step 304 Renderer-Creation Result Conventions

- Phase B has three remaining groups after Step 303: Steps 304-306 renderer
  creation Result conventions, Steps 307-312 public API compatibility
  examples, and Steps 313-318 final Windows/WSL verification plus authoring
  vocabulary freeze before Phase C.
- `WindowRuntime::try_open_window(...)` already propagates renderer factory
  errors without publishing an app-opened window record through
  `src/ui/runtime_window_results.cpp`, but Step 304 should expose and guard a
  clearer renderer-creation Result boundary instead of relying only on the
  window-opening flow.
- `ui_source_structure_test` failure code 100 after the first GREEN was a
  structure-budget issue, not a behavior failure. The public
  `window_runtime.hpp` stayed at its 240-line budget, while
  `window_runtime_internal.hpp` reached 262 against a 260-line cap after the
  internal child-renderer overload was added. Keeping that internal declaration
  on one line restored the boundary without broadening the guard.
- Step 304 ownership should remain: renderer Result creation in
  `src/ui/runtime_renderer_results.cpp`; root `run(...)` in
  `src/ui/runtime_run.cpp`; child-window activation in
  `src/ui/runtime_window_activation.cpp`; window-opening publication in
  `src/ui/runtime_window_results.cpp`.

## 2026-07-05 Phase B Step 304 Merge

- Step 304 is merged on `master` at
  `5f09830 feat: add renderer result conventions` and post-merge verified on
  Windows full debug 90/90 and WSL Arch Linux full debug 87/87.
- The durable ownership boundary for renderer-creation Result conventions is
  `src/ui/runtime_renderer_results.cpp`; keep root runtime startup in
  `src/ui/runtime_run.cpp`, child-window activation in
  `src/ui/runtime_window_activation.cpp`, and window publication in
  `src/ui/runtime_window_results.cpp`.
- Step 305 should continue renderer Result follow-on work without folding
  `ClipboardItem` payload parity, upstream `gpui::test` macro equivalents,
  action macro payloads, task priorities, structured task groups, or broader
  async runtime production depth into the same slice.

## 2026-07-05 Phase B Step 305 Renderer-Resize Result Conventions

- Step 305 stays scoped to renderer surface-resize Result conventions. It
  should not start renderer frame/redraw Result conversion, asset payload
  parity, upstream `gpui::test` macro equivalents, action macro payloads, task
  priorities, structured task groups, or broader async runtime production depth.
- The public API shape is `try_resize_surface(Size, DpiScale) -> Result<void>`
  on `WindowRuntime`. Existing `resize_surface(...)` remains source-compatible
  and still calls `fail_and_quit(...)` on renderer resize failure.
- Runtime Result validation belongs in focused
  `src/ui/runtime_renderer_resize_results.cpp`. Existing redraw orchestration
  stays in `src/ui/runtime_rendering.cpp` and renderer creation stays in
  `src/ui/runtime_renderer_results.cpp`.

## 2026-07-05 Phase B Step 306 Renderer Frame Result Conventions

- Step 306 stays scoped to frame/redraw Result conventions. It should not
  start `ClipboardItem` payload parity, upstream `gpui::test` macro
  equivalents, action macro payloads, task priorities, structured task groups,
  or broader async/runtime production depth.
- The public API shape is `WindowRuntime::try_draw_frame() -> Result<void>`
  plus `TestContextCapability::try_draw_frame() -> Result<void>`. Existing
  redraw events and `TestContextCapability::draw_frame()` remain
  source-compatible fail-and-quit wrappers.
- Runtime frame Result ownership belongs in focused
  `src/ui/runtime_renderer_frame_results.cpp`; `src/ui/runtime_rendering.cpp`
  and `src/ui/runtime_window_rendering.cpp` should stay thin compatibility
  wrappers, and `src/ui/test_context_rendering.cpp` should only forward the
  test-context Result helper.
- The behavior/API guard is
  `tests/api_parity/renderer_frame_result_conventions_test.cpp`, covering API
  spelling, begin-frame errors, present errors, compatibility fail-and-quit,
  and test-context Result forwarding. The structure guard is
  `tests/architecture/ui_source_structure_test.cpp`.

## 2026-07-05 Phase B Step 306 Merge

- Step 306 is merged on `master` at
  `4649f71 feat: add renderer frame result conventions` and post-merge
  verified on Windows full debug 92/92 and WSL Arch Linux full debug 89/89.
- The durable ownership boundary for renderer frame/redraw Result conventions
  is `src/ui/runtime_renderer_frame_results.cpp`; keep compatibility
  fail-and-quit wrappers in `src/ui/runtime_rendering.cpp` and
  `src/ui/runtime_window_rendering.cpp`.
- Step 307 should start public API compatibility examples without folding
  `ClipboardItem` payload parity, upstream `gpui::test` macro equivalents,
  action macro payloads, task priorities, or structured task groups into the
  same slice.

## 2026-07-05 Phase B Step 307 Public API Compatibility Examples

- Step 307 stays scoped to public API compatibility examples. It should not
  add new runtime behavior, `ClipboardItem` payload parity, upstream
  `gpui::test` macro equivalents, action macro payloads, task priorities, or
  structured task groups.
- The durable public example is
  `examples/api_parity/public_api_compatibility/main.cpp`. It must compile
  using only `cgpui/prelude.hpp` and should stay written from an application
  author's point of view rather than a runtime/internal-test point of view.
- The structure/API guard is
  `tests/api_parity/public_api_compatibility_examples_test.cpp`. It requires
  the example target and fails if the example includes private headers or names
  `WindowRuntime` internals directly.

## 2026-07-05 Phase B Step 307 Merge

- Step 307 is merged on `master` at
  `8ffc1bd feat: add public API compatibility example` and post-merge verified
  on Windows full-debug 93/93, WSL Arch Linux public API example build, and WSL
  Arch Linux full-debug 90/90.
- The durable boundary is now the prelude-only example plus
  `tests/api_parity/public_api_compatibility_examples_test.cpp`. Step 308
  should freeze and expand public authoring vocabulary without adding runtime
  behavior or moving compatibility checks into broad implementation files.

## 2026-07-05 Phase B Step 308 Public Authoring Vocabulary Freeze

- Step 308 stays scoped to documentation plus structure/API guards. It should
  not add runtime behavior, `ClipboardItem` payload parity, upstream
  `gpui::test` macro equivalents, action macro payloads, task priorities, or
  structured task groups.
- The durable ownership boundary is `docs/gpui-public-authoring-vocabulary.md`
  plus `tests/api_parity/public_authoring_vocabulary_freeze_test.cpp`. The
  guard ties the frozen vocabulary to `cgpui/prelude.hpp`,
  `include/cgpui/app/app.hpp`, `include/cgpui/ui/ui.hpp`, and
  `examples/api_parity/public_api_compatibility/main.cpp`.
- Step 309 should expand examples against the frozen vocabulary without moving
  compatibility checks into broad implementation files or changing low-level
  runtime behavior.

## 2026-07-05 Phase B Step 308 Merge

- Step 308 is merged on `master` at
  `83bbe62 docs: freeze public authoring vocabulary` and post-merge verified
  on Windows full-debug 94/94, WSL Arch Linux public API example build, and WSL
  Arch Linux full-debug 91/91.
- The durable freeze boundary is `docs/gpui-public-authoring-vocabulary.md`
  plus `tests/api_parity/public_authoring_vocabulary_freeze_test.cpp`; keep
  future example-expansion slices prelude-only and author-facing.
- Step 309 should expand public API compatibility examples against the frozen
  vocabulary without adding `ClipboardItem` payload parity, upstream
  `gpui::test` macro equivalents, action macro payloads, task priorities, or
  structured task groups.

## 2026-07-05 Phase B Step 309 Public API Example Expansion

- Step 309 stays scoped to example and compatibility coverage against the
  frozen public authoring vocabulary. It should not add new runtime behavior,
  `ClipboardItem` payload parity, upstream `gpui::test` macro equivalents,
  action macro payloads, task priorities, or structured task groups.
- The durable example boundary is
  `examples/api_parity/public_authoring_workflow/main.cpp`. It must compile
  using only `cgpui/prelude.hpp`, remain author-facing, and avoid private
  headers or direct `WindowRuntime` internals.
- The structure/API guard is
  `tests/api_parity/public_api_example_expansion_test.cpp`, with
  `tests/api_parity/gpui_parity_ledger_test.cpp` also requiring the new
  example file, test file, and xmake targets so future ledger drift is caught.
- A transient `xmake` package filelock error can occur if example build and
  tests run concurrently against the Vulkan SDK package cache; rerunning the
  build alone succeeded. Keep the full verification commands sequential when
  touching build targets in this worktree.

## 2026-07-05 Phase B Step 309 Merge

- Step 309 is merged on `master` at
  `2cfb166 docs: expand public API authoring examples` and post-merge verified
  on Windows full-debug 95/95, WSL Arch Linux public API example builds, and
  WSL Arch Linux full-debug 92/92.
- The durable Step 309 boundary is the prelude-only
  `examples/api_parity/public_authoring_workflow/main.cpp` example plus
  `tests/api_parity/public_api_example_expansion_test.cpp`; keep subsequent
  example expansion slices author-facing and out of runtime internals.
- Step 310 should continue the public API compatibility example expansion
  queue without adding runtime behavior or opening the deferred Phase B
  exclusions.

## 2026-07-05 Phase B Step 310 Public Context Capability Example

- Step 301 is already complete on `master`; the active unfinished Phase B
  slice is Step 310.
- Step 310 stays scoped to another public API compatibility example against
  the frozen public authoring vocabulary. It should not add runtime behavior,
  `ClipboardItem` payload parity, upstream `gpui::test` macro equivalents,
  action macro payloads, task priorities, or structured task groups.
- The durable example boundary is
  `examples/api_parity/public_context_capabilities/main.cpp`. It must compile
  using only `cgpui/prelude.hpp`, remain author-facing, and avoid private
  headers or direct `WindowRuntime` internals.
- The structure/API guard is
  `tests/api_parity/public_context_capability_example_test.cpp`, with
  `tests/api_parity/gpui_parity_ledger_test.cpp` also requiring the new
  example file, test file, and xmake targets so future ledger drift is caught.
- A transient `xmake` package filelock error recurred when the new example
  build ran in parallel with the new test. Keep Step 310 verification commands
  sequential around example builds.

## 2026-07-05 Phase B Step 310 Merge

- Step 310 is merged on `master` at
  `7f7838b docs: add public context capability example` and post-merge
  verified on Windows full-debug 96/96 and WSL Arch Linux full-debug 93/93.
- The durable Step 310 boundary is the prelude-only
  `examples/api_parity/public_context_capabilities/main.cpp` example plus
  `tests/api_parity/public_context_capability_example_test.cpp`; keep
  subsequent example expansion slices author-facing and out of runtime
  internals.
- Step 311 should continue the public API compatibility example expansion
  queue without adding runtime behavior or opening the deferred Phase B
  exclusions.

## 2026-07-05 Phase B Step 311 Public Async/Test Workflow Example

- Step 301 is already complete on `master`; the active unfinished Phase B
  slice is Step 311, not a redo of Step 301.
- Step 311 stays scoped to another public API compatibility example against
  the frozen public authoring vocabulary. It should not add runtime behavior,
  `ClipboardItem` payload parity, upstream `gpui::test` macro equivalents,
  action macro payloads, task priorities, or structured task groups.
- The durable example boundary is
  `examples/api_parity/public_async_test_workflow/main.cpp`. It must compile
  using only `cgpui/prelude.hpp`, remain author-facing, and avoid private
  headers or direct `WindowRuntime` internals.
- The structure/API guard is
  `tests/api_parity/public_async_test_workflow_example_test.cpp`, with
  `tests/api_parity/gpui_parity_ledger_test.cpp` also requiring the new
  example file, test file, xmake targets, Markdown ledger entries, and JSON
  ledger entries so future drift is caught.
- Public async/test examples can use generic lambdas for scheduling/task
  callbacks so the example does not need to spell internal runtime context
  types while still compiling against the public `AsyncContextCapability` and
  `TestContextCapability` APIs.

## 2026-07-05 Phase B Step 311 Merge

- Step 311 is merged on `master` at
  `2e7a669 docs: add public async test workflow example` and post-merge
  verified on Windows full-debug 97/97 and WSL Arch Linux full-debug 94/94.
- The durable Step 311 boundary is the prelude-only
  `examples/api_parity/public_async_test_workflow/main.cpp` example plus
  `tests/api_parity/public_async_test_workflow_example_test.cpp`; keep final
  public example expansion work author-facing and out of runtime internals.
- Step 312 should finish the public API compatibility example expansion queue
  without adding runtime behavior or opening deferred Phase B exclusions.

## 2026-07-05 Phase B Step 312 Public Surface Closure Example

- Step 301 is already complete on `master`; the active unfinished Phase B
  slice is Step 312, not a redo of Step 301.
- Step 312 stays scoped to public example/test/docs/xmake coverage. It should
  not add runtime behavior, `ClipboardItem` payload parity, upstream
  `gpui::test` macro equivalents, action macro payloads, task priorities, or
  structured task groups.
- The durable example boundary is
  `examples/api_parity/public_phase_b_surface_closure/main.cpp`. It must
  compile using only `cgpui/prelude.hpp`, remain author-facing, and avoid
  private headers or direct `WindowRuntime` internals.
- The structure/API guard is
  `tests/api_parity/public_phase_b_surface_closure_example_test.cpp`, with
  `tests/api_parity/gpui_parity_ledger_test.cpp` requiring the new example,
  new test, xmake targets, Markdown ledger entries, and JSON ledger entries so
  future drift is caught.
- The closure example intentionally covers frozen Phase B vocabulary in one
  prelude-only place: `Application`, `App`, `Window`, `AppContext`,
  `Context<T>`, context capabilities, entity/view handles, typed actions,
  command palette entries, key binding contexts, `Result<T>`, `ErrorCode`,
  async/test capabilities, native menu/file dialog Result APIs, task Result
  APIs, frame Result APIs, and pointer capture/release through public element
  context helpers.
- WSL example builds should stay sequential. The first Linux public example
  build may cold-build static libraries and take around two minutes, while
  subsequent public example builds are much faster.

## 2026-07-05 Phase B Step 312 Merge

- Step 312 is merged on `master` at
  `a9ad4dc docs: add public phase b surface closure example` and post-merge
  verified on Windows full-debug 98/98 and WSL Arch Linux full-debug 95/95.
- The durable Step 312 boundary is the prelude-only
  `examples/api_parity/public_phase_b_surface_closure/main.cpp` example plus
  `tests/api_parity/public_phase_b_surface_closure_example_test.cpp`; keep the
  final Phase B closure examples author-facing and out of runtime internals.
- Steps 313-318 should now run the final Phase B Windows/WSL verification and
  public authoring vocabulary freeze audit before Phase C starts.

## 2026-07-05 Phase B Steps 313-318 Final Freeze Audit

- Step 301 is already complete on `master`; do not redo it. The current Phase
  B closeout band is Steps 313-318.
- The durable final-freeze guard is
  `tests/api_parity/public_phase_b_completion_audit_test.cpp`. It reads the
  roadmap, public vocabulary document, Markdown/JSON parity ledger, xmake, and
  all public API parity examples. It fails if Phase B is not marked closed
  before Phase C, if the new audit is not ledger-tracked, if any public example
  stops being prelude-only, or if deferred Phase B exclusions leak into public
  examples.
- The final freeze keeps `ClipboardItem` payload parity, upstream
  `gpui::test` macro equivalents, action macro payloads, task priorities, and
  structured task groups deferred. Phase C should start with elements, style,
  layout, widgets, and uniform-list work from this frozen public authoring
  boundary.
- After merge, `master` post-merge verification passed Windows full debug
  99/99 and WSL Arch Linux full debug 96/96 with JSON validation, focused
  public/freeze gates 8/8, and all five public API examples built on both
  hosts.

## 2026-07-05 Phase C Step 319 Div Child List

- Step 319 owns `div` child-list behavior in the existing focused style-node
  and builder modules, not in aggregate headers: `StyledElement` keeps a
  vector of children, `child()` remains a first-child compatibility view,
  `StyledElement::children()` exposes the list for focused tests, repeated
  `.child(...)` retains every child, and
  `ElementBuilder::children(std::vector<AnyElement>)` appends an owned
  collection.
- The new default multi-child `div` layout stacks children vertically using
  `Style::gap` plus existing padding/margin behavior; `h_flex()`, `v_flex()`,
  and `v_stack()` keep their existing dedicated layout nodes.
- The durable behavior guard is `tests/ui/element_test.cpp`; the parity guard
  is `tests/api_parity/gpui_parity_ledger_test.cpp`. Keep Step 320 focused on
  the next `div` style-vocabulary helper instead of broad widget or layout
  rewrites.

## 2026-07-05 Phase C Step 319 Merge

- Step 319 is merged on `master` at `14aaff0` and post-merge verified with
  Windows JSON validation, Windows full debug 99/99, and WSL Arch Linux full
  debug 96/96.
- Step 320 should continue the same focused Phase C band by adding the next
  `div` style-vocabulary helper, while keeping widget, uniform-list, and broad
  layout rewrites out of the slice.

## 2026-07-05 Phase C Step 320 Div Flex Vocabulary Helpers

- Step 320 stays scoped to authoring vocabulary over existing flex style
  values. It adds `items_start()`, `items_center()`, `items_end()`,
  `justify_start()`, `justify_center()`, `justify_end()`,
  `justify_between()`, and `flex_1()` on `ElementBuilder`, without adding flex
  wrap, flex direction style, widget behavior, or a new layout engine.
- The durable implementation boundary is
  `src/ui/element_builder_layout.cpp`, alongside the existing
  `gap(...)`, `align_items(...)`, `justify_content(...)`, `flex_grow(...)`,
  and `flex_shrink(...)` setters. Keep these aliases out of
  `element_builder_style.cpp` and aggregate headers.
- The behavior guard is `tests/ui/element_test.cpp`, and the structure guard
  is `tests/architecture/ui_source_structure_test.cpp`.

## 2026-07-05 Phase C Step 321 Merge

- Step 321 is merged on `master` at
  `694d64e feat: add div sizing color border helpers` and post-merge verified
  with Windows JSON validation, Windows full debug 99/99, and WSL Arch Linux
  full debug 96/96.
- Step 322 should continue the same focused Phase C band with the remaining
  `div` style vocabulary, while keeping widget, uniform-list, and broad layout
  rewrites out of the slice.

## 2026-07-05 Phase C Step 320 Merge

- Step 320 is merged on `master` at `5040365` and post-merge verified with
  Windows JSON validation, Windows full debug 99/99, and WSL Arch Linux full
  debug 96/96.
- Step 321 should continue the same focused Phase C band with the next
  `div` sizing/color/border helper slice, while keeping widget,
  uniform-list, and broad layout rewrites out of the slice.

## 2026-07-05 Phase C Step 321 Div Sizing Color Border Helpers

- Step 321 stays scoped to upstream-style authoring vocabulary over existing
  style storage. It adds `w(...)`, `h(...)`, `bg(...)`, `text_color(...)`,
  `border_1()`, and `rounded(...)` on `ElementBuilder`, without adding new
  layout behavior, widget behavior, uniform-list behavior, or a broad style
  system rewrite.
- The durable implementation boundary is split by ownership:
  `w(...)`, `h(...)`, and `border_1()` live in
  `src/ui/element_builder_layout.cpp`; `bg(...)`, `text_color(...)`, and
  `rounded(...)` live in `src/ui/element_builder_style.cpp`.
- The behavior guard is `tests/ui/element_test.cpp`, and the structure guard
  is `tests/architecture/ui_source_structure_test.cpp`.

## 2026-07-05 Phase C Step 322 Div Overflow Opacity Position Helpers

- Step 322 stays scoped to authoring aliases over existing style storage. It
  adds `overflow(...)`, `overflow_hidden()`, `overflow_visible()`,
  `opacity(...)`, `z_index(...)`, `relative()`, `top(...)`, `right(...)`,
  `bottom(...)`, and `left(...)` on `ElementBuilder` without inventing shadow
  storage, text-style behavior, widget behavior, uniform-list behavior, flex
  wrap, or a layout-engine rewrite.
- The durable implementation boundary is split by ownership:
  `overflow(...)`, `overflow_hidden()`, `overflow_visible()`, and
  `opacity(...)` live in `src/ui/element_builder_style.cpp`; `z_index(...)`,
  `relative()`, `top(...)`, `right(...)`, `bottom(...)`, and `left(...)` live
  in `src/ui/element_builder_layout.cpp`.
- The single-edge inset helpers preserve the other authored inset edges by
  copying `style_state_.base.inset`, changing one field, and forwarding through
  `inset(...)`.
- The behavior guard is `tests/ui/element_test.cpp`, the structure guard is
  `tests/architecture/ui_source_structure_test.cpp`, and
  `tests/api_parity/gpui_parity_ledger_test.cpp` keeps the ledger vocabulary
  in sync.

## 2026-07-05 Phase C Step 322 Merge

- Step 322 is merged on `master` at
  `f4f2fc2 feat: add div overflow position helpers` and post-merge verified
  with Windows JSON validation, Windows full debug 99/99, and WSL Arch Linux
  full debug 96/96.
- Step 323 should continue the same focused Phase C `div` style-vocabulary
  band while keeping shadow storage, text-style behavior, widget behavior,
  uniform-list behavior, flex wrap, broad layout rewrites, and the Phase B
  closeout exclusions out of the slice.

## 2026-07-05 Phase C Step 323 Div Text Style Helpers

- Step 323 stays scoped to text-style aliases over existing style storage. It
  adds `ElementBuilder::text_size(...)` as an alias for `font_size(...)` and
  `ElementBuilder::font_family(...)` as an alias for `font(FontDescriptor{...})`
  without adding font-weight, line-height, inherited text-style cascade, shadow
  storage, widget behavior, uniform-list behavior, or broad style rewrites.
- The durable implementation boundary is `src/ui/element_builder_style.cpp`,
  alongside `font(...)`, `font_size(...)`, `text_color(...)`, overflow, and
  opacity helpers. These aliases should stay out of
  `src/ui/element_builder_layout.cpp`.
- Step 324 should handle the remaining `div` shadow vocabulary/storage as its
  own slice so shadow state, paint metadata, and structure tests can be shaped
  deliberately.

## 2026-07-05 Phase C Step 323 Merge

- Step 323 is merged on `master` at
  `78f2f05 feat: add div text style helpers` and post-merge verified with
  Windows JSON validation, Windows full debug 99/99, and WSL Arch Linux full
  debug 96/96.
- Step 324 should now add the remaining `div` shadow vocabulary/storage while
  keeping font-weight, line-height, inherited text-style cascade, widget
  behavior, uniform-list behavior, broad style rewrites, and the Phase B
  closeout exclusions out of the slice.

## 2026-07-05 Phase C Step 324 Div Shadow Vocabulary Storage

- Step 324 stays scoped to `div` shadow authoring and deterministic style
  storage. It adds `BoxShadow`, `Style::box_shadow`,
  `StyleOverlay::box_shadow`, `ElementBuilder::shadow(...)`, and
  `ElementBuilder::shadow_sm()` without adding broad renderer shadow geometry,
  widget behavior, uniform-list behavior, inherited text-style cascade, or
  Phase B closeout exclusions.
- Paint observability is intentionally narrow: `StyledElement::paint` records a
  `PaintCommandKind::box_shadow` command before background, border, and child
  painting, and `src/ui/render_view_commands.cpp` treats that command as a
  renderer no-op until a later renderer-geometry slice.
- Durable ownership is split by module: value/storage declarations in
  `include/cgpui/ui/style_values.hpp`, `style_box.hpp`, and
  `style_overlay.hpp`; non-template bodies in `src/ui/style_box.cpp`,
  `src/ui/style_overlay.cpp`, `src/ui/element_builder_style.cpp`, and
  `src/ui/paint_shadow.cpp`; cascade propagation in
  `include/cgpui/ui/style_cascade.hpp`; behavior guards in
  `tests/ui/style_test.cpp` and `tests/ui/element_test.cpp`; and structure
  guards in `tests/architecture/ui_source_structure_test.cpp`.

## 2026-07-07 Phase C Step 324 Merge

- Step 324 is merged on `master` at
  `ef306c0 feat: add div shadow vocabulary` and post-merge verified with
  Windows JSON validation, Windows full debug 99/99, and WSL Arch Linux full
  debug 96/96.
- Step 325 should continue Phase C into layout constraints while keeping the
  Step 324 renderer-geometry omission deliberate: shadow paint commands remain
  observable metadata until a later renderer slice owns actual shadow geometry.

## 2026-07-07 Phase C Step 325 Layout Constraints

- Step 325 stays scoped to public min/max layout constraints over the existing
  `LayoutConstraints` clamp path. It adds style storage and authoring helpers,
  not percentage sizing, flex-wrap, absolute/fixed-positioning rewrites,
  widget behavior, uniform-list behavior, or Phase B closeout exclusions.
- Durable ownership is split by module: `Style::min_size` /
  `Style::max_size` and mutators live in `include/cgpui/ui/style_box.hpp` and
  `src/ui/style_box.cpp`; overlay storage/mutators live in
  `include/cgpui/ui/style_overlay.hpp` and `src/ui/style_overlay.cpp`;
  cascade propagation stays in `include/cgpui/ui/style_cascade.hpp`;
  `ElementBuilder::min_size(...)`, `max_size(...)`, `min_w(...)`, `min_h(...)`,
  `max_w(...)`, and `max_h(...)` live in `src/ui/element_builder_layout.cpp`;
  and `StyledElement::layout` owns the authored/external constraint merge.
- The behavior guards are `tests/ui/style_test.cpp` and
  `tests/ui/element_test.cpp`; the structure guard is
  `tests/architecture/ui_source_structure_test.cpp`, and the parity ledger
  guard keeps the Step 326 next-slice marker in sync.

## 2026-07-07 Phase C Step 325 Merge

- Step 325 is merged on `master` at
  `a1c945e feat: add div layout constraints` and post-merge verified with
  Windows JSON validation, Windows full debug 99/99, and WSL Arch Linux full
  debug 96/96.
- Step 326 should continue Phase C into percentage-like sizing while keeping
  Step 325's min/max constraint layer as public vocabulary over the existing
  `LayoutConstraints` clamp path, not a broad layout rewrite.

## 2026-07-07 Phase C Step 326 Percentage-Like Sizing

- Step 326 stays scoped to percentage-like content sizing for `div` /
  `StyledElement`. It adds `PercentageSize`, `Style::percentage_size`,
  `StyleOverlay::percentage_size`, `ElementBuilder::size_pct(...)`,
  `ElementBuilder::w_pct(...)`, and `ElementBuilder::h_pct(...)`, but not
  percentage margins/padding/gaps, widgets, uniform-list behavior, flex-wrap,
  fixed/absolute positioning rewrites, or Phase B closeout exclusions.
- Percentage axes resolve only against finite external
  `LayoutInput.constraints.max_size`; unconstrained axes fall back to existing
  content/preferred sizing, and negative resolved values clamp to zero.
- Durable ownership is split by module: value/storage declarations live in
  `include/cgpui/ui/style_values.hpp`, `style_box.hpp`, and
  `style_overlay.hpp`; non-template bodies live in `src/ui/style_box.cpp`,
  `src/ui/style_overlay.cpp`, and `src/ui/element_builder_layout.cpp`;
  cascade propagation stays in `include/cgpui/ui/style_cascade.hpp`; and
  `src/ui/element_style_nodes.cpp` owns percentage resolution before the
  existing min/max clamp path.
- The behavior guards are `tests/ui/style_test.cpp` and
  `tests/ui/element_test.cpp`; the structure guard is
  `tests/architecture/ui_source_structure_test.cpp`, and the parity ledger
  guard keeps the Step 327 next-slice marker in sync.

## 2026-07-07 Phase C Step 326 Merge

- Step 326 is merged on `master` at
  `7b748b7 feat: add div percentage sizing` and post-merge verified with
  Windows JSON validation, Windows full debug 99/99, and WSL Arch Linux full
  debug 96/96.
- Step 327 should continue Phase C with focused margins, padding, and gaps
  behavior. Keep percentage sizing complete as a finite-parent-size layer, and
  keep percentage margins/padding/gaps, widgets, uniform-list behavior,
  flex-wrap, fixed/absolute positioning rewrites, and Phase B closeout
  exclusions out of Step 327 unless that slice explicitly owns them.

## 2026-07-07 Phase C Step 327 Margin Padding Gap Shorthands

- Step 327 stays scoped to authoring shorthand over existing
  `Style::padding`, `Style::margin`, and `StyledElement::layout` gap
  composition. It adds `ElementBuilder::p(...)`, `px(...)`, `py(...)`,
  `pt(...)`, `pr(...)`, `pb(...)`, `pl(...)`, `m(...)`, `mx(...)`,
  `my(...)`, `mt(...)`, `mr(...)`, `mb(...)`, and `ml(...)` without adding
  percentage margins/padding/gaps, flex-wrap, widgets, uniform-list behavior,
  fixed/absolute positioning rewrites, or Phase B closeout exclusions.
- Durable ownership is `include/cgpui/ui/element_builder_core.hpp` for public
  declarations and `src/ui/element_builder_layout.cpp` for non-template
  bodies. The helpers preserve opposite-axis and opposite-edge values by
  copying current edge storage before forwarding through `padding(...)` or
  `margin(...)`.
- The behavior guard is `tests/ui/element_test.cpp`; the structure guard is
  `tests/architecture/ui_source_structure_test.cpp`, and the parity ledger
  guard keeps the Step 328 next-slice marker in sync.

## 2026-07-07 Phase C Step 327 Merge

- Step 327 is merged on `master` at
  `32ec5e1 feat: add div spacing shorthands` and post-merge verified with
  Windows JSON validation, Windows debug config, Windows full debug 99/99,
  WSL Arch Linux debug config, and WSL Arch Linux full debug 96/96.
- Step 328 should continue Phase C with focused absolute/fixed positioning
  behavior while keeping percentage margins/padding/gaps, flex-wrap, widgets,
  uniform-list behavior, broad layout rewrites, and Phase B closeout exclusions
  out of the slice unless that slice explicitly owns them.

## 2026-07-07 Phase C Step 328 Absolute Fixed Positioning

- Step 328 stays scoped to positioning vocabulary and out-of-flow layout for
  existing element containers. It adds `Position::fixed` and
  `ElementBuilder::fixed()` without adding overlay layers, nested scroll
  clipping, percentage margins/padding/gaps, flex-wrap, widgets,
  uniform-list behavior, or Phase B closeout exclusions.
- Durable ownership is `include/cgpui/ui/style_values.hpp` for the enum value,
  `include/cgpui/ui/element_builder_core.hpp` for the public declaration,
  `src/ui/element_builder_layout.cpp` for the non-template builder body, and
  `src/ui/element_style_nodes.cpp` / `src/ui/element_flex_layout.cpp` for the
  out-of-flow child layout behavior.
- The behavior guard is `tests/ui/element_test.cpp`: `.fixed()` must set
  `Position::fixed`, and absolute/fixed children must not contribute to
  `StyledElement` vertical flow, gap composition, or content size. The
  structure guard is `tests/architecture/ui_source_structure_test.cpp`, and
  the parity ledger guard keeps the Step 329 overlay-layer next-slice marker
  in sync.

## 2026-07-07 Phase C Step 328 Merge

- Step 328 is merged on `master` at
  `38574a2 feat: add div fixed positioning` and post-merge verified with
  Windows JSON validation, Windows debug config, Windows full debug 99/99,
  WSL Arch Linux debug config, and WSL Arch Linux full debug 96/96.
- Step 329 should continue Phase C with the focused overlay-layer slice while
  keeping nested scroll clipping, widgets, uniform-list behavior, broad layout
  rewrites, and Phase B closeout exclusions out of the slice unless that slice
  explicitly owns them.

## 2026-07-07 Phase C Step 329 Overlay Layers

- Step 329 stays scoped to direct overlay ordering for existing element
  containers. It makes direct `StyledElement`, flex, and vertical-stack
  children respect `z_order()` for paint order, hit-test order, and event
  dispatch order without adding nested scroll clipping, widgets,
  uniform-list behavior, broad layout rewrites, renderer z-buffer behavior, or
  Phase B closeout exclusions.
- Durable ownership is the private `src/ui/element_layer_ordering.hpp` /
  `src/ui/element_layer_ordering.cpp` helper pair. Container implementations
  in `src/ui/element_style_paint.cpp`, `src/ui/element_style_nodes.cpp`,
  `src/ui/element_flex_node.cpp`, and
  `src/ui/element_vertical_stack_node.cpp` call that focused helper instead of
  open-coding child ordering.
- Paint ordering is stable low-to-high `z_order()` with authored-order
  tie-breaking; hit testing and event dispatch traverse high-to-low
  `z_order()` with reverse-authored-order tie-breaking so the visually topmost
  direct child receives input first.
- The behavior guard is `tests/ui/element_test.cpp`; the structure guard is
  `tests/architecture/ui_source_structure_test.cpp`; and the parity ledger
  guard keeps the Step 330 nested scroll clipping next-slice marker in sync.

## 2026-07-07 Phase C Step 329 Merge

- Step 329 is merged on `master` at
  `6c1bfe4 feat: add direct overlay layer ordering` and post-merge verified
  with Windows JSON validation, Windows debug config, Windows full debug
  99/99, WSL Arch Linux debug config, and WSL Arch Linux full debug 96/96.
- Step 330 should continue Phase C with the focused nested scroll clipping
  slice while keeping widgets, uniform-list behavior, broad layout rewrites,
  renderer z-buffer behavior, and Phase B closeout exclusions out of the slice
  unless that slice explicitly owns them.

## 2026-07-07 Phase C Step 330 Nested Scroll Clipping

- Step 330 stays scoped to paint clip propagation for nested overflow and
  scroll containers. It makes `PaintList::push_clip(...)` store the effective
  intersection with the current clip stack so nested hidden-overflow and
  scrollable-list clips clamp `PaintCommand::clip_rect` together, including
  zero-size metadata for disjoint clips.
- Durable ownership is the private `src/ui/paint_clip.hpp` /
  `src/ui/paint_clip.cpp` helper pair. `src/ui/paint.cpp` keeps the public
  `PaintList` method bodies and delegates clip intersection to that focused
  helper instead of open-coding geometry in element paint paths.
- Behavior coverage lives in `tests/ui/element_test.cpp`: direct nested
  `PaintList` clips preserve effective clip-stack metadata, and a hidden
  overflow `div` containing a `scrollable_list(...)` paints child commands with
  the outer/inner clip intersection. Structure coverage lives in
  `tests/architecture/ui_source_structure_test.cpp`, and the parity ledger
  guard moves the next slice to Step 331 style cascade depth.
- Step 330 does not add widget behavior, uniform-list virtualization, broad
  layout rewrites, renderer z-buffer behavior, or the Phase B closeout
  exclusions.

## 2026-07-07 Phase C Step 330 Merge

- Step 330 is merged on `master` at
  `266bc9f feat: add nested paint clipping` and post-merge verified with
  Windows JSON validation, Windows debug config, Windows full debug 99/99,
  WSL Arch Linux debug config, and WSL Arch Linux full debug 96/96.
- Step 331 should start the Phase C style cascade depth band while keeping
  widgets, uniform-list behavior, broad layout rewrites, renderer z-buffer
  behavior, and Phase B closeout exclusions out of the slice unless that slice
  explicitly owns them.

## 2026-07-07 Phase C Step 331 Style Cascade Active State

- Step 331 stays scoped to active-state style cascade storage and resolution.
  It adds `StyleState::active`, `StyleStateFlags::active`,
  `ElementBuilder::active_style(...)`, and `ButtonBuilder::active_style(...)`
  without adding real pointer-pressed active semantics, widget interaction
  changes, uniform-list behavior, broad layout rewrites, renderer z-buffer
  behavior, or Phase B closeout exclusions.
- The active overlay resolves after hover/focus and before disabled for both
  local style state and class style rules. Inline overlays still apply last.
  This gives active selectors a deterministic authoring surface while leaving
  event-driven active state to the later focusable/interactable band.
- Durable ownership is data-only `include/cgpui/ui/style_state.hpp`, cascade
  resolution in `include/cgpui/ui/style_cascade.hpp`, public declarations in
  `include/cgpui/ui/element_builder_core.hpp` and
  `include/cgpui/ui/button_builder.hpp`, and non-template builder bodies in
  `src/ui/element_builder_style.cpp` and `src/ui/button_builder.cpp`.
- Behavior coverage lives in `tests/ui/style_test.cpp` and
  `tests/ui/element_test.cpp`; structure coverage lives in
  `tests/architecture/ui_source_structure_test.cpp`; and the parity ledger
  guard moves the next slice to Step 332 class style reuse depth.

## 2026-07-07 WSL Object Cache Contamination Recovery

- A post-merge Step 331 WSL full-debug failure with
  `file format not recognized` was traced to non-ELF/corrupt object-cache
  artifacts in the root worktree, not to a code regression. `file` reported
  the bad `command_palette_key_integration_test.cpp.o` as `data`.
- If another WSL link failure reports `file format not recognized` for an
  object under `build/.objs/.../linux/x86_64/debug`, avoid repeating a normal
  `xmake test` loop. Use WSL-side `xmake clean -a`, reconfigure with
  `--ccache=n`, and rerun the full WSL suite from the clean Linux build.

## 2026-07-07 Phase C Step 332 Class Style Reuse Depth

- Step 332 stays scoped to reusable class-style rules. It adds
  `StyleClassRule`, `StyleCascade::set_class_rule(...)`, and
  `StyleCascade::class_rule(...)` without adding theme token fallback,
  inherited text style, dynamic invalidation, or pointer-active semantics.
- Reused classes resolve depth-first before the owning class's base/state
  overlays. Cycles are skipped through a local visiting stack so class rules can
  be authored defensively without hanging style resolution.
- Durable ownership is `include/cgpui/ui/style_cascade.hpp` for declarations
  and `src/ui/style_cascade.cpp` for non-template bodies and recursion helpers.
  Behavior coverage lives in `tests/ui/style_test.cpp` and
  `tests/ui/element_test.cpp`; structure coverage lives in
  `tests/architecture/ui_source_structure_test.cpp`.

## 2026-07-07 Phase C Step 333 Theme Token Fallback

- Step 333 stays scoped to theme token fallback during style resolution. It
  adds token references for existing color and spacing-like style fields,
  theme-aware `resolved_style(...)` overloads, and `StyledElement` forwarding
  without adding runtime theme switching, inherited text style, dynamic
  invalidation, widget behavior, or pointer-active semantics.
- Missing tokens should soft-fail: a token reference that is not present in the
  provided `Theme` must leave the existing concrete style value intact instead
  of clearing or defaulting it. Concrete no-theme resolution remains compatible
  and ignores token references.
- Durable ownership keeps token-reference data in public leaf style headers,
  non-template setter bodies in `src/ui/style_box.cpp` and
  `src/ui/style_overlay.cpp`, actual `Theme::color(...)` /
  `Theme::spacing(...)` lookup in focused private
  `src/ui/style_theme_tokens.cpp`, shared base-overlay diffing in
  `src/ui/style_cascade_overlays.cpp`, and theme-aware style-cascade ordering
  in `src/ui/style_theme_cascade.cpp`.

## 2026-07-07 Phase C Step 334 Inherited Text Style

- Step 334 should stay scoped to inherited text style flowing from styled
  containers into descendant text nodes. It should not add runtime theme
  switching, dynamic invalidation, widget behavior, pointer-active semantics,
  flex-wrap, or a broad cascade/layout rewrite.
- The likely durable ownership boundary is a focused private
  `src/ui/text_style_inheritance.hpp` / `.cpp` helper pair that merges
  inheritable text fields. Container paint/layout code should delegate to that
  helper instead of open-coding inheritance in broad element files.
- Inheritable fields for this slice are the existing text-facing style values:
  foreground color, font descriptor/family, and font size. Explicit child text
  style should win over inherited parent text style, and nested styled
  containers should merge parent inherited values with their local text style
  before forwarding to text descendants.
- `font_size` needed a narrow explicit-authoring marker because it is a plain
  `float` with default value `16.0F`. `StyleAuthoredTextFields` records
  `with_font(...)` and `with_font_size(...)` calls while preserving the public
  `Style::font_size` type and existing default-style behavior.
- Flex and vertical stack containers currently do not paint their children, so
  inherited text style through those containers should be observed through
  layout/effective text style in Step 334. Direct styled-element-to-label
  inheritance is covered through both layout and paint commands.

## 2026-07-07 Phase C Step 334 Merge

- Step 334 is merged on `master` at
  `a472317 feat: add inherited text style` and post-merge verified with JSON
  validation, Windows debug config, Windows full debug 99/99, WSL Arch Linux
  debug config, and WSL Arch Linux full debug 96/96.
- Step 335 should continue the style cascade depth band with dynamic
  invalidation when style-affecting state changes. Keep runtime theme
  switching, widget behavior, pointer-active semantics, and broad cascade or
  layout rewrites out unless Step 335 explicitly owns them.

## 2026-07-07 Phase C Step 335 Dynamic Style Invalidation

- Step 335 stays scoped to invalidating existing render/layout/paint state
  when runtime style-affecting element state changes. It does not add runtime
  theme switching, real pointer-active semantics, widget behavior, or resolved
  hover/focus style application to layout/paint.
- Hover state changes are observed through redraw scheduling because the test
  fake window processes redraw requests synchronously, so transient
  invalidation bits are cleared before the pointer event reaches the view.
  Repeated pointer movement within the same hovered element must not schedule
  another redraw.
- Keyboard focus element changes can be tested directly through
  `WindowRuntime::invalidation_state()` because the runtime can request focus
  before a platform window is running. Re-requesting the same focus element,
  releasing the wrong element, and empty focus requests must remain no-ops.
- Durable ownership is `src/ui/runtime_style_invalidation.cpp` for the
  transition comparison and `request_render()` delegation. Input and focus
  runtime files call the focused helper instead of directly requesting render
  invalidation.
- After Step 335 lands, Step 336 should be a style-cascade depth closeout and
  evidence audit for Steps 331-335. Do not start focusable/interactable
  semantics, pointer-active behavior, focus rings, or widget behavior until the
  Step 337 band opens.
- Tests that observe render/accessibility counts must account for new
  style-state invalidation. Hovering into an element can add one redraw/render
  pass, focus requests can send a focus accessibility live update before a
  later value/text update, and focus release can leave layout/paint invalidated
  before a subsequent explicit paint request.

## 2026-07-07 Phase C Step 335 Merge

- Step 335 is merged on `master` at
  `e88bd78 feat: add dynamic style invalidation` and post-merge verified with
  JSON validation, Windows debug config, Windows full debug 99/99, WSL Arch
  Linux debug config, and WSL Arch Linux full debug 96/96.
- Step 336 should close the Steps 331-335 style-cascade evidence band. Keep
  focusable/interactable semantics, pointer-active behavior, focus rings,
  widget behavior, and broad resolved-style layout/paint rewrites out until the
  next band opens.

## 2026-07-07 Phase C Step 336 Style Cascade Closeout

- Step 336 is a closeout/audit slice, not a runtime behavior slice. The durable
  boundary is `tests/api_parity/phase_c_style_cascade_depth_audit_test.cpp`,
  which guards the Steps 331-335 style-cascade evidence and the ledger handoff
  to Phase C Step 337 focusable/interactable semantics.
- The closeout keeps runtime theme switching, real pointer-active semantics,
  widget behavior, focusable/interactable semantics, and broad resolved-style
  layout/paint rewrites out of Step 336. Those remain future focused slices.

## 2026-07-07 Phase C Step 336 Merge

- Step 336 is merged on `master` at
  `f794b22 test: close phase c style cascade band` and post-merge verified with
  WSL Arch Linux debug config plus WSL Arch Linux full debug 97/97.
- Step 337 should start the focusable/interactable semantics band. Keep broad
  widget behavior, runtime theme switching, and broad resolved-style
  layout/paint rewrites out unless that slice explicitly owns them.

## 2026-07-07 Phase C Step 337 Pointer Active Semantics

- Step 337 stays scoped to real pointer-active input state for routed elements.
  It adds `ViewInputState::active_element_id`,
  `WindowRuntime::update_active_state_for_event(...)`, and focused
  `src/ui/runtime_active_state.cpp` ownership without adding tab-order depth,
  focus ring metadata, click/drag gesture synthesis, keyboard activation,
  broad widget behavior, runtime theme switching, or broad resolved-style
  layout/paint rewrites.
- The active state machine handles only left `PointerButton` events in this
  slice. Press sets active state to the enabled routed target; release clears
  it; disabled and no-hit targets leave active state empty.
- Active-state transitions should call
  `WindowRuntime::request_style_state_invalidation(previous, next)` so active
  styles invalidate through the same narrow helper added in Step 335. Repeated
  no-op active transitions should not request redraws.
- Structure coverage should keep active-state ownership out of
  `src/ui/runtime_event_input.cpp` and preserve the existing line-count guards
  for `tests/ui/window_runtime_test_support.hpp` and
  `src/ui/window_runtime_internal.hpp`. Focused tests should assert
  `context.input.active_element_id` directly rather than growing shared
  `RecordingView` comparison state.
- Step 338 should continue the focusable/interactable band with
  tab-order/focus-ring metadata unless a narrower roadmap slice supersedes it.

## 2026-07-07 Phase C Step 337 Merge

- Step 337 is merged on `master` at
  `6f60668 feat: add pointer active state` and post-merge verified with JSON
  validation, Windows debug config, Windows full debug 100/100, WSL Arch Linux
  debug config, and WSL Arch Linux full debug 97/97.
- Step 338 should stay scoped to tab-order/focus-ring metadata in the
  focusable/interactable band. Keep click/drag gesture synthesis, keyboard
  activation, broad widget behavior, runtime theme switching, and broad
  resolved-style layout/paint rewrites out unless that slice explicitly owns
  them.

## 2026-07-07 Phase C Step 338 Tab Order Focus Ring Metadata

- Step 338 stays scoped to tab-order and focus-ring metadata. It adds
  `FocusMetadata`, `FocusRingVisibility`, `ElementBuilder::tab_index(...)`,
  `ElementBuilder::focus_ring(...)`, accessibility reporting, and runtime Tab
  ordering without adding click/drag gesture synthesis, keyboard activation,
  broad widget behavior, runtime theme switching, or broad resolved-style
  layout/paint rewrites.
- Durable ownership is split across a focused public leaf
  `include/cgpui/ui/focus_metadata.hpp`, non-template Element accessors in
  `src/ui/element_focus_metadata.cpp`, builder setter bodies in
  `src/ui/element_builder_interaction.cpp`, build propagation in
  `src/ui/element_builder_finish.cpp`, accessibility copy-out in
  `src/ui/element_tree_accessibility.cpp`, and runtime focus ordering in
  `src/ui/runtime_focus_order.cpp`. `runtime_event_focus_routes.cpp` should
  remain a small traversal route delegator.
- Runtime Tab ordering treats positive `tab_index` values as an explicit
  ordered prefix before default tree order. Equal positive indices and all
  default/zero entries keep tree-order stability. Negative `tab_index` values
  remain focusable metadata but are skipped by Tab traversal.
- Structure coverage should keep `FocusRingVisibility` out of
  `element_core.hpp` as an enum definition, keep `Element` method bodies out
  of the public header, keep focus-order sorting out of
  `runtime_event_focus_routes.cpp`, and preserve the private runtime header
  line-count guard.
- Step 339 should continue the focusable/interactable band with click/drag
  gesture synthesis while keeping keyboard activation and broad widget
  behavior out unless that slice explicitly owns them.

## 2026-07-07 Phase C Step 338 Merge

- Step 338 is merged on `master` at
  `5290912 feat: add focus traversal metadata` and post-merge verified with
  JSON validation, Windows debug config, Windows full debug 100/100, WSL Arch
  Linux debug config, and WSL Arch Linux full debug 97/97.
- Step 339 should continue the focusable/interactable band with focused
  click/drag gesture synthesis. Keep keyboard activation, broad widget
  behavior, runtime theme switching, and broad resolved-style layout/paint
  rewrites out unless that slice explicitly owns them.

## 2026-07-07 Phase C Step 339 Click Drag Gesture Synthesis

- Step 339 should stay scoped to mouse gesture synthesis for existing
  element handlers. It should make `on_click(...)` run from a synthesized
  click after left press/release over the same enabled routed element, not
  from raw pointer-down alone.
- Drag gesture state can be represented as runtime input metadata while the
  left press is active: a pointer move after a valid press marks the pressed
  element as dragging and suppresses the later click. This slice should not
  add OS drag/drop payload behavior, widget-specific drag APIs, keyboard
  activation, or broad widget behavior.
- Durable ownership should be a focused private
  `src/ui/runtime_gesture_synthesis.hpp` / `.cpp` helper pair. The broad
  input router should keep pointer-position, hover, cursor, and route
  resolution only; route dispatch should call the helper instead of owning
  gesture-state details.
- Final Step 339 ownership keeps the public event signal minimal:
  `ElementGestureKind::click` plus `ElementEventContext::gesture` identify
  synthesized clicks, while `ViewInputState` exposes pointer-down, clicked,
  dragging element id, and dragging boolean metadata for runtime/test
  observability. `runtime_active_state.cpp` computes the enabled routed target
  and delegates gesture-state transitions to `runtime_gesture_synthesis.cpp`.
- Runtime route dispatch should only synthesize click after raw routed element
  handlers decline a left-button release. A pointer move while the left press
  is active marks dragging and suppresses the later click. Raw pointer press
  and release alone should not invoke `on_click(...)`.
- Step 340 should continue the focusable/interactable band with keyboard
  activation semantics. Keep OS drag/drop payload behavior, widget-specific
  drag APIs, broad widget behavior, runtime theme switching, and broad
  resolved-style layout/paint rewrites out unless that future slice owns them.

## 2026-07-07 Phase C Step 339 Merge

- Step 339 is merged on `master` at
  `66125aa feat: synthesize click drag gestures` and post-merge verified with
  JSON validation, Windows debug config, Windows full debug 100/100, WSL Arch
  Linux debug config, and WSL Arch Linux full debug 97/97.
- Step 340 should continue the focusable/interactable band with keyboard
  activation semantics. Keep OS drag/drop payload behavior, widget-specific
  drag APIs, broad widget behavior, runtime theme switching, and broad
  resolved-style layout/paint rewrites out unless that slice explicitly owns
  them.

## 2026-07-07 Phase C Step 340 Keyboard Activation

- Step 340 should stay scoped to keyboard activation for the already-focused
  enabled element route. Enter (`13`) and Space (`32`) pressed on the focused
  route should synthesize the same `ElementGestureKind::click` context added in
  Step 339.
- Raw routed key handlers must get first refusal. If a focused element's
  `on_key(...)` handler consumes or cancels the Enter/Space press, keyboard
  activation should not synthesize the click gesture.
- Durable ownership should remain in focused gesture synthesis helpers:
  `src/ui/runtime_gesture_synthesis.hpp` / `.cpp` can own the keyboard
  activation predicate and synthesized dispatch, while
  `src/ui/runtime_event_route_dispatch.cpp` should only delegate after raw
  element dispatch declines the key. Avoid growing
  `src/ui/runtime_event_keyboard.cpp` or broad widget files.
- `WindowRuntime::run(...)` resets keyboard focus before creating the platform
  window, so runtime keyboard-activation tests that need an initially focused
  element must request focus during the run callback, after the reset. A
  pre-run `request_keyboard_focus(element_id)` is intentionally wiped.
- `KeyElement` must treat synthesized click gestures as child delegation, not
  as another raw `KeyboardKey` callback. Otherwise an unhandled Enter key would
  call `on_key(...)` twice: once before synthesis and once during synthesized
  click dispatch.
- After Step 340, the next focused Phase C focusable/interactable gap should
  be Step 341 disabled interaction semantics, leaving broad widget behavior,
  runtime theme switching, and broad resolved-style layout/paint rewrites out
  unless that slice explicitly owns them.

## 2026-07-07 Phase C Step 340 Merge

- Step 340 is merged on `master` at
  `2edda1a feat: add keyboard activation semantics` and post-merge verified
  with JSON validation, Windows debug config, Windows full debug 100/100, WSL
  Arch Linux debug config, and WSL Arch Linux full debug 97/97.
- Step 341 should stay focused on disabled interaction semantics in the
  focusable/interactable band. Keep broad widget behavior, runtime theme
  switching, and broad resolved-style layout/paint rewrites out unless that
  slice explicitly owns them.

## 2026-07-07 Phase C Step 341 Disabled Interaction Semantics

- Step 341 stays scoped to clearing stale runtime interaction state when an
  element already participating in input state becomes disabled or disappears.
  It should not add broad widget behavior, runtime theme switching, or broad
  resolved-style layout/paint rewrites.
- Durable ownership is focused `src/ui/runtime_disabled_interaction.cpp` via
  `WindowRuntime::refresh_disabled_interaction_state()`. `runtime_events.cpp`
  delegates to the helper after active-state updates and before focus/key/text
  dispatch so the next event observes cleaned state.
- Disabled and missing elements are treated the same for element-owned hover,
  active, keyboard focus, pointer capture, pointer-down, clicked, and dragging
  state. View-owned pointer capture remains untouched.
- The helper must preserve legacy `set_element_root(...)` behavior by finding
  child elements in unowned static element hierarchies before deciding an id
  is missing. Otherwise existing hover-state tests that use a stack root with
  assigned child ids lose valid hover state.
- Step 342 should close the focusable/interactable band before Phase C moves
  into built-in widget expansion.

## 2026-07-07 Phase C Step 341 Merge

- Step 341 is merged on `master` at
  `7bd5ad5 feat: clear disabled interaction state` and post-merge verified
  with JSON validation, Windows debug config, Windows full debug 100/100, WSL
  Arch Linux debug config, and WSL Arch Linux full debug 97/97.
- Step 342 should be a focused focusable/interactable band closeout before
  Phase C starts built-in widget expansion.

## 2026-07-07 Phase C Step 342 Focusable Interactable Closeout

- Step 342 is an audit-only closeout slice. Durable ownership is
  `tests/api_parity/phase_c_focusable_interactable_audit_test.cpp`, which
  guards the Steps 337-341 focusable/interactable evidence, the roadmap
  checkbox for Steps 337-342, the ledger evidence, and the handoff to Phase C
  Step 343 built-in widget expansion.
- The closeout does not add runtime or widget behavior. Keep broad widget
  behavior, runtime theme switching, broad resolved-style layout/paint
  rewrites, `ClipboardItem`, upstream `gpui::test` macro equivalents, action
  macro payloads, task priorities, and structured task groups out of Step 342.
- Step 343 should start built-in widget expansion from the frozen
  focusable/interactable boundary, with button/label/text-input and adjacent
  widget primitive gaps as the next Phase C surface.

## 2026-07-07 Phase C Step 342 Merge

- Step 342 is merged on `master` at
  `7ad6133 test: close phase c focusable band` and post-merge verified with
  JSON validation, diff hygiene, Windows debug config, Windows full debug
  101/101, WSL Arch Linux debug config, and WSL Arch Linux full debug 98/98.
- Step 343 should start built-in widget expansion from this closed
  focusable/interactable boundary. Do not start it in this pause point.

## 2026-07-07 Phase C Step 343 Built-In Widget Expansion

- Step 343 should stay focused on button/label/text-input widget module
  ownership and authoring ergonomics. It adds `ButtonBuilder::label(...)` and
  moves existing builder implementation bodies under `src/ui/widgets/*` without
  changing the lower-level element node ownership.
- Durable ownership for this slice is public leaf headers
  `include/cgpui/ui/button_builder.hpp`, `label_builder.hpp`, and
  `text_input_builder.hpp`, implementation files under `src/ui/widgets/`, and
  focused coverage in `tests/ui/builtin_widget_test.cpp` plus
  `tests/architecture/widget_source_structure_test.cpp`.
- Step 344 should continue the built-in widget band with checkbox/radio/switch
  widgets. Keep slider, list item, menu item, icon/image, container primitive
  expansion, runtime theme switching, broad resolved-style layout/paint
  rewrites, `ClipboardItem`, upstream `gpui::test` macros, action macro
  payloads, task priorities, and structured task groups out unless a later
  slice explicitly owns them.
- After Step 343 lands, closeout/audit tests that previously expected
  `next_step` to be Step 343 must move their handoff expectation to Step 344
  while still requiring the Step 343 evidence. This avoids turning a completed
  handoff into stale audit drift.

## 2026-07-07 Phase C Step 343 Merge

- Step 343 is merged on `master` at
  `10907e2 feat: add built-in widget module boundary` and post-merge verified
  with JSON validation, diff hygiene, Windows debug config, Windows full debug
  103/103, WSL Arch Linux debug config, and WSL Arch Linux full debug 100/100.
- Step 344 should continue the built-in widget band with checkbox/radio/switch
  widgets, reusing the focused `src/ui/widgets/*` ownership rather than putting
  widget implementation bodies back into broad `src/ui/*` files.

## 2026-07-07 Phase C Step 344 Checkbox Radio Switch Widgets

- Step 344 stays scoped to checkbox/radio/switch widget authoring and state
  metadata. It should not add slider, list item, menu item, icon/image,
  container primitives, runtime theme switching, broad resolved-style
  layout/paint rewrites, `ClipboardItem`, upstream `gpui::test` macros, action
  macro payloads, task priorities, or structured task groups.
- Durable public widget ownership is `include/cgpui/ui/toggle_builder.hpp` plus
  `src/ui/widgets/toggle_builder.cpp`, matching the Step 343
  `src/ui/widgets/*` builder boundary. The aggregate headers should stay thin
  and only include the new leaf.
- Durable element ownership is a focused choice element leaf:
  `include/cgpui/ui/element_choice_nodes.hpp`, with behavior in
  `src/ui/element_choice_nodes.cpp`, layout in
  `src/ui/element_choice_layout.cpp`, and paint in
  `src/ui/element_choice_paint.cpp`. Splitting layout keeps the behavior file
  under the structure-test cap and avoids growing broad element files.
- The smallest accessibility extension for this slice is role plus string
  value metadata: checkbox/radio report checked or unchecked, switch reports on
  or off, and text input preserves its existing platform value behavior through
  `TextInputElement::accessibility_value()`. This avoids adding a broader
  accessibility state model before a later accessibility parity slice owns it.
- Step 345 should continue the built-in widget band with a focused slider
  widget, reusing the same public leaf plus focused implementation/source
  structure pattern.

## 2026-07-07 Phase C Step 344 Merge

- Step 344 is merged on `master` at
  `8c6f788 feat: add checkbox radio switch widgets` and post-merge verified
  with JSON validation, diff hygiene, Windows debug config, Windows full debug
  103/103, WSL Arch Linux debug config, and WSL Arch Linux full debug 100/100.
- Step 345 should stay scoped to a focused slider widget in the built-in
  widget band. Keep list item, menu item, icon/image, container primitives,
  runtime theme switching, broad resolved-style layout/paint rewrites,
  `ClipboardItem`, upstream `gpui::test` macros, action macro payloads, task
  priorities, and structured task groups out unless a later slice explicitly
  owns them.

## 2026-07-07 Phase C Step 345 Slider Widget

- Step 345 stays scoped to a focused slider widget in the built-in widget band.
  It should not add list item, menu item, icon/image, container primitives,
  runtime theme switching, broad resolved-style layout/paint rewrites,
  `ClipboardItem`, upstream `gpui::test` macros, action macro payloads, task
  priorities, or structured task groups.
- Durable public widget ownership is `include/cgpui/ui/slider_builder.hpp`
  plus `src/ui/widgets/slider_builder.cpp`, matching the Step 343/344
  `src/ui/widgets/*` builder boundary. Aggregate headers stay thin and only
  include the new leaf.
- Durable element ownership is a focused slider element leaf:
  `include/cgpui/ui/element_slider_nodes.hpp`, with behavior in
  `src/ui/element_slider_nodes.cpp`, layout in
  `src/ui/element_slider_layout.cpp`, and paint in
  `src/ui/element_slider_paint.cpp`. The structure test keeps these source
  files inside local line-count caps and prevents broad `src/ui/*` builder
  dumping.
- The smallest accessibility extension for this slice is slider role plus
  string value metadata: `AccessibilityRole::slider`,
  `PlatformAccessibilityRole::slider`, and `SliderElement::accessibility_value()`
  report the current clamped/stepped value. A broader accessibility state model
  remains later work.
- Existing `ElementGestureKind` only carries `none` and `click`, so this slice
  implements click-to-value slider updates rather than introducing a broader
  drag gesture/event surface. Drag/capture-depth slider behavior should be a
  later focused input slice if needed.
- Step 346 should continue the built-in widget band with list item and menu
  item widgets before icon/image and container primitive gaps.

## 2026-07-08 Phase C Step 345 Merge

- Step 345 is merged on `master` at
  `ff12a4c feat: add slider widget` and post-merge verified with JSON
  validation, diff hygiene, Windows debug config, final Windows full debug
  103/103, WSL Arch Linux debug config, and WSL Arch Linux full debug 100/100.
- The first two Windows full debug attempts failed only
  `clipboard_test/default` with xmake exit code 14 and no assertion output.
  Focused `clipboard_test/default`, the shortened reproduction sequence, and a
  later full Windows suite all passed, so this was recorded as transient
  validation noise rather than a Step 345 regression.

## 2026-07-08 Phase C Step 346 List Menu Widgets

- Step 346 stays scoped to list item and menu item widgets in the built-in
  widget band. It should not add icon/image widgets, container primitives,
  runtime theme switching, broad resolved-style layout/paint rewrites,
  `ClipboardItem`, upstream `gpui::test` macros, action macro payloads, task
  priorities, or structured task groups.
- Durable public widget ownership is `include/cgpui/ui/item_builder.hpp` plus
  `src/ui/widgets/item_builder.cpp`, matching the Step 343-345
  `src/ui/widgets/*` builder boundary. Aggregate headers stay thin and only
  include the new leaf.
- Durable element ownership is a focused item element leaf:
  `include/cgpui/ui/element_item_nodes.hpp`, with behavior in
  `src/ui/element_item_nodes.cpp`, layout in
  `src/ui/element_item_layout.cpp`, and paint in
  `src/ui/element_item_paint.cpp`. The structure test keeps these files inside
  local line-count caps and prevents broad `src/ui/*` widget dumping.
- The smallest accessibility extension for this slice is list/menu role plus
  string value metadata: `AccessibilityRole::list_item`,
  `AccessibilityRole::menu_item`, `PlatformAccessibilityRole::list_item`, and
  `PlatformAccessibilityRole::menu_item`. List items report selected or
  unselected values; menu items are action-only and report no selection value.
- Step 347 should continue the built-in widget band with icon/image widgets
  before container primitive gaps.

## 2026-07-08 Phase C Step 347 Icon Image Widgets

- Step 347 stays scoped to image and icon widget authoring in the built-in
  widget band. It should not add container primitives, SVG/image decoders,
  runtime theme switching, broad resolved-style layout/paint rewrites,
  `ClipboardItem`, upstream `gpui::test` macros, action macro payloads, task
  priorities, or structured task groups.
- Durable public widget ownership is `include/cgpui/ui/image_builder.hpp` plus
  `src/ui/widgets/image_builder.cpp`, matching the Step 343-346
  `src/ui/widgets/*` builder boundary. Aggregate headers stay thin and only
  include the new leaf.
- Durable element ownership is a focused image element leaf:
  `include/cgpui/ui/element_image_nodes.hpp`, with behavior in
  `src/ui/element_image_nodes.cpp`, layout in
  `src/ui/element_image_layout.cpp`, and paint in
  `src/ui/element_image_paint.cpp`. The structure test keeps these files
  inside local line-count caps and prevents broad `src/ui/*` widget dumping.
- The smallest accessibility extension for this slice is image role plus alt
  text: `AccessibilityRole::image` and `PlatformAccessibilityRole::image`.
  Icon-specific behavior is represented as square sizing and optional tint
  metadata carried by `ImagePaint` and `ImageDraw`; renderer shader/decode
  depth remains later work.
- Step 348 should continue the built-in widget band with container primitive
  follow-up.

## 2026-07-08 Phase C Step 347 Merge

- Step 347 is merged on `master` at
  `c4c0c85 feat: add image icon widgets` and post-merge verified with JSON
  validation, diff hygiene, Windows debug config, Windows full debug 103/103,
  WSL Arch Linux debug config, and WSL Arch Linux full debug 100/100.
- Step 348 should stay scoped to container primitives and should not start
  until the user resumes after this pause.

## 2026-07-08 Phase C Step 348 Container Primitives

- Step 348 stays scoped to container primitive public authoring in the built-in
  widget band. It should not add uniform list behavior, new layout engines,
  runtime theme switching, broad resolved-style layout/paint rewrites,
  `ClipboardItem`, upstream `gpui::test` macros, action macro payloads, task
  priorities, or structured task groups.
- Durable public widget ownership is `include/cgpui/ui/container_builder.hpp`
  plus `src/ui/widgets/container_builder.cpp`, matching the Step 343-347
  `src/ui/widgets/*` builder boundary. Aggregate headers stay thin and only
  include the new leaf.
- The smallest useful container addition is `h_stack()`, implemented as the
  horizontal row/flex-row counterpart to the existing `v_stack()` public
  spelling. `div()`, `h_flex()`, `v_flex()`, and `v_stack()` keep their
  existing semantics while their public free-function bodies move out of the
  broad element-builder factory file.
- `src/ui/element_builder_factories.cpp` should keep the low-level
  `ElementBuilder::box()`, `row()`, `column()`, and `v_stack()` factories
  because those are lower-level builder construction helpers. The structure
  guard should only forbid public container free-function bodies there.
- Step 349 should move Phase C into uniform list parity, starting with stable
  item identity and virtualized range calculation before scroll anchoring,
  measurement-cache, large-list recycling, and keyboard/pointer selection
  depth.

## 2026-07-08 Phase C Step 349 Uniform List Identity

- Step 349 stays scoped to uniform-list identity/range observability on the
  existing `scrollable_list(...)` element. It should not add scroll anchoring,
  measurement-cache invalidation, large-list item recycling, selection, or new
  layout engines.
- Durable public ownership is `include/cgpui/ui/uniform_list.hpp`; aggregate
  exposure is through the existing thin `include/cgpui/ui/ui.hpp` include
  edge. Non-template behavior lives in `src/ui/uniform_list.cpp`.
- Durable scroll element ownership is `src/ui/element_scroll_layout.cpp` for
  `ScrollableListElement::layout(...)` and
  `ScrollableListElement::layout_snapshot()`. The broad
  `element_scroll_nodes.hpp` header keeps declarations and storage only.
- The snapshot records content-local bounds before scroll offsets are applied
  to child layout bounds. This preserves current paint/hit-test behavior while
  giving future anchoring/recycling slices stable keyed identity and visible
  range data to consume.
- Missing child keys use deterministic index-string fallback keys for the
  snapshot only; this does not replace keyed reconciliation or create a broader
  item recycling model.
- The ledger `status_counts` field is not a simple count of `rows.status`
  values, so Step 349 leaves it unchanged while adding the new required
  `gpui uniform_list` row and explicit Step 350 handoff.
- Step 350 should continue with focused scroll anchoring over the Step 349
  snapshot boundary. Keep measurement caches, item recycling, and selection
  behavior for later slices unless Step 350 explicitly owns them.

## 2026-07-08 Phase C Step 350 Scroll Anchoring

- Step 350 stays scoped to keyed scroll anchoring over the Step 349
  `UniformListLayoutSnapshot` boundary. It should not add measurement-cache
  invalidation, large-list item recycling, selection behavior, or a new layout
  engine.
- Durable public ownership is still `include/cgpui/ui/uniform_list.hpp`, with
  non-template behavior in `src/ui/uniform_list.cpp`. The new surface is
  `UniformListScrollAnchor`, `capture_uniform_list_scroll_anchor(...)`, and
  `apply_uniform_list_scroll_anchor(...)`.
- Durable scroll element ownership remains
  `src/ui/element_scroll_layout.cpp`: `ScrollableListElement::layout(...)`
  captures the previous keyed anchor from `layout_snapshot_`, relayouts the
  content, applies the anchor against the new snapshot, and then recalculates
  visible range and shifted child bounds from the adjusted `ScrollState`.
- The anchor records key, index, element id, and viewport offset. Application
  uses the stable key and preserves horizontal scroll offset; if the key is no
  longer present, the previous offset is left unchanged.
- Windows `rg.exe` is still brittle in this environment: the WinGet link failed
  to start with "no application is associated" during Step 350 docs lookup, so
  `Select-String` was used for the text search.
- Step 351 should continue uniform list parity with item measurement cache.
  Keep recycling and keyboard/pointer selection for later slices unless that
  slice explicitly owns them.

## 2026-07-08 Phase C Step 351 Item Measurement Cache

- Step 351 stays scoped to keyed item measurement caching over the existing
  `UniformListLayoutSnapshot` boundary. It should not add large-list item
  recycling, keyboard/pointer selection, cache eviction, or a new layout
  engine.
- Durable public ownership remains `include/cgpui/ui/uniform_list.hpp`; the
  focused non-template cache implementation lives in
  `src/ui/uniform_list_measurement.cpp` rather than the existing range/anchor
  source file.
- `UniformListItemMeasurementCache::measure(...)` treats a stable key as a
  cache hit, updates the stored size from the latest layout bounds, and tracks
  entry, lookup, hit, and miss counts. This gives later recycling work a
  reusable measurement boundary while preserving current full child layout.
- `ScrollableListElement::layout(...)` records
  `UniformListLayoutSnapshot::measurements` before applying the scroll-anchor
  offset and exposes `ScrollableListElement::measurement_cache()` for tests and
  diagnostics.
- Step 352 should continue uniform list parity with large-list recycling over
  this measurement cache. Keep keyboard/pointer selection for the later
  selection slice unless Step 352 explicitly owns it.

## 2026-07-08 Phase C Step 352 Large-List Recycling

- Step 352 stays scoped to a retained/recycled large-list window over the
  existing `UniformListLayoutSnapshot` and Step 351 measurement cache boundary.
  It should not add keyboard/pointer selection, cache eviction, or a new layout
  engine.
- Durable public ownership remains `include/cgpui/ui/uniform_list.hpp`; the
  focused non-template recycling implementation lives in
  `src/ui/uniform_list_recycling.cpp` rather than the existing range/anchor or
  measurement source files.
- `calculate_uniform_list_recycling_window(...)` expands the visible range by
  a small overscan count, records retained count plus recycled before/after
  counts and measured extents, and exposes `retains(...)` / `recycles(...)`
  helpers for scroll integration.
- `ScrollableListElement::layout(...)` stores
  `UniformListLayoutSnapshot::recycling_window` after measurement and marks
  each `UniformListItemIdentity::recycled`; `ScrollableListElement::paint(...)`
  skips recycled children so large-list recycling is observable without
  replacing the existing full child layout pass in this slice.
- The first focused RED run failed exactly on unresolved
  `UniformListRecyclingWindow` methods and
  `calculate_uniform_list_recycling_window(...)`, confirming the tests were
  aimed at the new Step 352 boundary.
- Step 353 should continue uniform list parity with keyboard/pointer selection
  over the Step 349-352 snapshot, anchor, measurement, and recycling state.

## 2026-07-08 Phase C Step 353 Keyboard Pointer Selection

- Step 353 stays scoped to single-selection state over the existing uniform-list
  snapshot boundary. It should not add multi-select/range selection, broad text
  selection/caret behavior, cache eviction, a new layout engine,
  `ClipboardItem`, upstream `gpui::test` macros, action macro payloads, task
  priorities, or structured task groups.
- Durable public ownership is `include/cgpui/ui/uniform_list_selection.hpp`.
  `include/cgpui/ui/uniform_list.hpp` only gains the snapshot `selected` flag,
  preserving the existing uniform-list leaf line cap.
- Durable implementation ownership is split between
  `src/ui/uniform_list_selection.cpp` for state/hit/movement helpers and
  `src/ui/element_scroll_events.cpp` for `ScrollableListElement::handle_event(...)`.
  This keeps event logic out of `element_scroll_nodes.hpp` and layout logic out
  of the selection helper source.
- Pointer selection uses content-local snapshot bounds, so the scroll element
  converts viewport pointer positions by adding the current `ScrollState`
  offset before calling `select_uniform_list_item_at_point(...)`.
- Keyboard movement only handles pressed ArrowUp, ArrowDown, Home, and End.
  Unhandled or non-moving key events continue to the list content.
- Step 354 should close the uniform-list band with audit/docs evidence before
  Phase C moves to the Steps 355-360 window/examples widget band.

## 2026-07-08 Phase C Step 354 Uniform List Closeout

- Step 354 is an audit-only closeout over the existing Step 349-353
  uniform-list work. It should not add runtime behavior, widget behavior,
  multi-select/range selection, cache eviction, new layout engines,
  `ClipboardItem`, upstream `gpui::test` macros, action macro payloads, task
  priorities, or structured task groups.
- Durable coverage for the closeout is the new
  `tests/api_parity/phase_c_uniform_list_audit_test.cpp` target, registered in
  `xmake.lua`. It guards the roadmap checkbox, the Markdown/JSON parity
  ledger evidence, the public authoring vocabulary freeze, and the Step 355
  window/examples handoff.
- The `gpui uniform_list` ledger row can move from Required to Adapted because
  the focused Phase C uniform-list band now has stable identity/range,
  anchoring, measurement cache, recycling, keyboard/pointer selection, and an
  audit gate. Future production-depth list work can still be tracked as new
  rows or later phase depth, but it should not keep this Phase C band open.
- Step 355 should start the window/examples widget band for menu demos,
  shadow, window positioning, window shadow, and input examples using public
  APIs.

## 2026-07-08 Phase C Step 355 Window Examples

- Step 355 starts the window/examples widget band with a public example target,
  not new runtime/window production behavior.
- Durable example ownership is
  `examples/api_parity/public_window_examples/main.cpp`; it includes only
  `cgpui/prelude.hpp` and uses `WindowOptions`, `NativeMenuModel`,
  `CommandPaletteEntry`, `text_input(...)`, `menu_item(...)`, `button(...)`,
  `label(...)`, `BoxShadow`, `shadow(...)`, `shadow_sm()`, and fixed-position
  element APIs.
- Durable guard ownership is
  `tests/api_parity/phase_c_window_examples_public_api_test.cpp`; it checks
  the example target, source fragments, docs/ledger/vocabulary sync, and
  forbidden private/runtime headers.
- Step 356 should deepen the same band without adding `ClipboardItem`, upstream
  `gpui::test` macros, action macro payloads, task priorities, structured task
  groups, or direct `WindowRuntime`/private include usage.

## 2026-07-08 Phase C Step 356 Window Examples Workflow

- Step 356 stays in the window/examples band and deepens Step 355 through a
  public workflow example, not new production window behavior.
- Durable example ownership is
  `examples/api_parity/public_window_examples_workflow/main.cpp`; it includes
  only `cgpui/prelude.hpp` and uses `TestContextCapability` to exercise public
  window activation/focus, redraw/frame-pump, key binding simulation, and
  pointer dispatch alongside menu installation, text input, shadow, and
  fixed-positioning authoring.
- Durable guard ownership is
  `tests/api_parity/phase_c_window_examples_workflow_test.cpp`, registered in
  `xmake.lua`; it freezes the example target/source, the prelude-only boundary,
  the roadmap/ledger/vocabulary handoff, and the Step 357 next slice.
- The older Step 355 public-example guard needed its handoff assertion updated
  from Step 356 to Step 357 once Step 356 evidence landed. Frozen phrases in
  roadmap/docs should stay contiguous because these source guards use simple
  substring checks.
- Step 357 should continue the window/examples widget band without adding
  `ClipboardItem`, upstream `gpui::test` macros, action macro payloads, task
  priorities, structured task groups, or private runtime headers.

## 2026-07-08 Phase C Step 357 Window Examples Widget Catalog

- Step 357 stays in the window/examples band and adds a public widget catalog
  example, not new production widget or runtime behavior.
- Durable example ownership is
  `examples/api_parity/public_window_examples_widget_catalog/main.cpp`; it
  includes only `cgpui/prelude.hpp` and uses public checkbox/radio/switch,
  slider, list/menu, image/icon, h_stack/v_stack container widgets, text input,
  and window options.
- Durable guard ownership is
  `tests/api_parity/phase_c_window_examples_widget_catalog_test.cpp`,
  registered in `xmake.lua`; it freezes the example target/source, the
  prelude-only boundary, the roadmap/ledger/vocabulary handoff, and the Step
  358 next slice.
- Step 358 should continue the window/examples widget band without adding
  `ClipboardItem`, upstream `gpui::test` macros, action macro payloads, task
  priorities, structured task groups, or private runtime headers.

## 2026-07-08 Phase C Step 358 Window Examples Interaction States

- Step 358 stays in the window/examples band and adds a public
  interaction-state example, not new production window/runtime behavior.
- Durable example ownership is
  `examples/api_parity/public_window_examples_interaction_states/main.cpp`; it
  includes only `cgpui/prelude.hpp` and uses public hover/focus/active/disabled
  style overlays, focus ring and tab-index metadata, keyed controls, click
  handlers, command-palette entries, key bindings, text input, and window
  options.
- Durable guard ownership is
  `tests/api_parity/phase_c_window_examples_interaction_states_test.cpp`,
  registered in `xmake.lua`; it freezes the example target/source, the
  prelude-only boundary, the roadmap/ledger/vocabulary handoff, and the Step
  359 next slice.
- Future WSL verification for this and subsequent Phase C slices should use a
  D-drive build output under `/mnt/d/Dev/Projects/cgpui/.build-wsl/...`, not a
  WSL-home build directory.

## 2026-07-08 Phase C Step 359 Window Examples Service Matrix

- Step 359 stays in the window/examples band and adds a public service-matrix
  example, not new production window/runtime behavior.
- Durable example ownership is
  `examples/api_parity/public_window_examples_service_matrix/main.cpp`; it
  includes only `cgpui/prelude.hpp` and uses public menu accelerators,
  command-palette entries, key bindings, test-context window hooks, window
  options, shadow/fixed positioning, text input, buttons, and menu items.
- Durable guard ownership is
  `tests/api_parity/phase_c_window_examples_service_matrix_test.cpp`,
  registered in `xmake.lua`; it freezes the example target/source, the
  prelude-only boundary, the roadmap/ledger/vocabulary handoff, and the Step
  360 closeout handoff.
- Step 360 should close the window/examples widget band without adding
  `ClipboardItem`, upstream `gpui::test` macros, action macro payloads, task
  priorities, structured task groups, or private runtime headers.

## 2026-07-08 Phase C Step 360 Window Examples Closeout

- Step 360 is an audit-only closeout over the existing Steps 355-359 public
  window/example band. It should not add production runtime/window behavior.
- Durable guard ownership is
  `tests/api_parity/phase_c_window_examples_closeout_test.cpp`, registered in
  `xmake.lua`; it freezes the five public example targets/sources, the
  prelude-only boundary, the roadmap checkbox, the Markdown/JSON ledger,
  public vocabulary evidence, and the handoff to Phase C Step 361 SVG/image
  element front-end APIs.
- The closeout keeps `ClipboardItem`, upstream `gpui::test` macros, action
  macro payloads, task priorities, structured task groups, private runtime
  headers, and direct `WindowRuntime` use out of the window/examples band.

## 2026-07-08 Phase C Step 361 SVG Image Front End

- Step 361 stays scoped to author-facing image source metadata. It should not
  add SVG decoding, asset registries, renderer upload behavior, GPU texture
  lifetime, `ClipboardItem`, upstream `gpui::test` macros, action macro
  payloads, task priorities, structured task groups, private runtime headers,
  or direct `WindowRuntime` use.
- Durable public ownership is `include/cgpui/ui/image_source.hpp`, with
  non-template bodies in `src/ui/image_source.cpp`. `image_builder.hpp` remains
  the widget authoring leaf and `element_image_nodes.hpp` stores the source
  metadata without growing renderer-specific upload/report declarations.
- `ImageSource` is the front-end bridge: raster sources wrap an existing
  `ImageAssetDescriptor`, while SVG sources preserve the SVG string and expose
  descriptor metadata so the existing paint and render paths keep receiving an
  `ImageAssetDescriptor`.
- `svg(...)` is intentionally front-end only in this slice. It builds an
  `ImageElementKind::svg` element and preserves SVG metadata for later asset
  registration/decoding work, but paint still emits the existing image paint
  command.
- Step 362 should continue with SVG/image asset registration examples over
  this source boundary.

## 2026-07-08 Phase C Step 362 SVG Image Asset Registration

- Step 362 stays scoped to deterministic registration of raster image assets
  and SVG source strings over the existing `ImageSource` boundary. It should
  not add SVG decoding, PNG/JPEG loading, renderer upload behavior, GPU
  texture lifetime, private runtime headers, or direct `WindowRuntime` use.
- Durable public ownership is `include/cgpui/ui/image_asset_registry.hpp`,
  with non-template bodies in `src/ui/image_asset_registry.cpp`.
  `ImageAssetRegistry` allocates ids for id-less raster assets and SVG sources,
  stores `RegisteredImageAsset` records, exposes registered raster assets for
  later upload planning, and returns `ImageSource` values that feed
  `image(...)`/`svg(...)`.
- Verification passed JSON validation, diff hygiene, focused Step 362 and
  adjacent guard tests 7/7, Windows debug full 112/112, and WSL Arch Linux
  debug full 109/109. WSL output stayed under
  `.build-wsl/phase-c-svg-image-asset-registration` on D:, while
  `/root/.xmake` remained absent and `/tmp` remained empty.
- Step 363 should continue the SVG/image band with public example coverage over
  registered image and SVG sources.

## 2026-07-08 Phase C Step 363 SVG/Image Public Example Coverage

- Step 363 stays in the public example/documentation layer. It adds
  `examples/api_parity/public_svg_image_sources/main.cpp`,
  `api_parity_public_svg_image_sources`, and
  `tests/api_parity/phase_c_svg_image_public_examples_test.cpp` to show
  registered raster and SVG sources through the prelude-only authoring
  boundary.
- The example constructs `ImageAssetRegistry`, registers an id-less in-memory
  raster `ImageAsset` and an SVG source string, checks `ImageSourceKind`, uses
  `registry.raster_assets()` for upload-planning observability, and feeds
  `RegisteredImageAsset::source()` into `image(...)` and `svg(...)`.
- This slice intentionally does not add SVG decoding, PNG/JPEG loading,
  renderer upload behavior, GPU texture lifetime, private runtime headers, or
  direct `WindowRuntime` use. Step 364 should close the SVG/image band.

## 2026-07-08 Phase C Step 364 SVG/Image Band Closeout

- Step 364 is an audit-only closeout over the Step 361-363 SVG/image
  front-end authoring band. It should not add SVG decoding, PNG/JPEG loading,
  renderer upload behavior, GPU texture lifetime, private runtime headers, or
  direct `WindowRuntime` use.
- Durable closeout ownership is
  `tests/api_parity/phase_c_svg_image_closeout_test.cpp`, registered in
  `xmake.lua`; it guards the Step 361 source APIs, Step 362 asset registry,
  Step 363 prelude-only registered-source example, roadmap checkbox, ledger
  JSON handoff, public vocabulary evidence, and Step 367 next slice.
- The older Step 363 public-example guard needed its global `next_step`
  assertion moved off Step 364 after the closeout advanced the global handoff
  to Step 367. It now checks the SVG/image band handoff evidence instead of
  pinning the global queue.
- Step 367 should start widget family structure tests requiring widget
  families to keep public leaf headers, focused source files, and focused
  behavior tests.

## 2026-07-08 Phase C Step 367 Widget Family Structure Tests

- Step 367 stays structure-focused. It should not add new widget behavior,
  renderer behavior, SVG decoding, PNG/JPEG loading, GPU texture lifetime,
  private runtime headers, or direct `WindowRuntime` use.
- Durable structure ownership is
  `tests/architecture/widget_source_structure_test.cpp`, which now uses a
  `WidgetFamilyBoundary` table. Each widget family must name its public leaf
  header, focused source file, and focused behavior tests.
- The covered families are label, button, text input, toggle controls, slider,
  list/menu items, image/icon/SVG, container primitives, and scrollable list.
  Scrollable list keeps its focused source at `src/ui/scrollable_list_builder.cpp`
  because that boundary predates the `src/ui/widgets/*.cpp` builder folder but
  remains a dedicated source file.
- The API parity guard for the docs/ledger handoff is
  `tests/api_parity/phase_c_widget_family_structure_test.cpp`. Step 373 should
  run the final element/style/widget ledger audit and decide which remaining
  Phase C rows are complete or explicitly deferred.

## 2026-07-08 Phase C Step 373 final ledger audit

- Step 373 is an audit-only closeout: it should not add runtime behavior,
  widget behavior, renderer behavior, SVG decoding, PNG/JPEG loading, GPU
  texture lifetime, private runtime headers, direct `WindowRuntime`,
  `ClipboardItem`, upstream `gpui::test` macros, action macro payloads, task
  priorities, or structured task groups.
- The stale closeout gap before this slice was `gpui::div` still handing off
  to an older Phase C widget/examples step, while roadmap Steps 373-378 and
  the global `phase_c_widget_evidence.next_step` still pointed at the Step 373
  audit itself.
- The final audit evidence should tie together
  `phase_c_style_cascade_depth_audit_test.cpp`,
  `phase_c_focusable_interactable_audit_test.cpp`,
  `phase_c_uniform_list_audit_test.cpp`,
  `phase_c_window_examples_closeout_test.cpp`,
  `phase_c_svg_image_closeout_test.cpp`,
  `phase_c_widget_family_structure_test.cpp`, and the new
  `phase_c_final_ledger_audit_test.cpp`. The next non-Phase-C handoff is
  Phase D Step 379 text/font shaping.

## 2026-07-08 Phase C Step 373 post-merge closeout

- Root `master` post-merge WSL verification passed 113/113 using
  `.build-wsl/master` on D: for xmake global, package, and build output, with
  `/dev/shm/cgpui` only as transient temp.
- Disk placement after the WSL run confirmed `/tmp` was 0, `/root/.xmake` was
  absent, `/dev/shm/cgpui` was absent after the run, `.build-wsl/master` was
  5.09GB, total `.build-wsl` was 18.15GB, `.worktrees` was 4.75GB, root
  `build` was 4.88GB, C: had 25.86GB free, and D: had 2.28GB free.
- On this Windows environment, `rg --files docs | Select-String ...` can fail
  at process startup with an `rg.exe` access-denied error from the WinGet link.
  Use `Get-ChildItem -LiteralPath docs -File -Recurse` as the local fallback.
- No disk cleanup was performed in this closeout continuation; keep
  `.build-wsl`, worktrees, WSL VHDs, Codex logs, Git loose objects, and caches
  untouched unless the user explicitly approves removal.
- Completion audit must check the actual Phase C checkbox ranges, not only the
  per-step prose: the `Steps 319-324` range was stale as unchecked while its
  embedded evidence already recorded all six slices as merged and verified.

## 2026-07-08 Post-Phase-C Scope Decision

- User accepted deferring game-engine-specific integration, engine runtime
  embedding/sharing, Android, iOS, and X11.
- User did not accept deferring the other C++23 GPUI platform goals. Future
  slices should treat Vulkan production depth, declarative widgets, reactive
  state/subscription depth, low-allocation runtime structures, static and
  dynamic widget support, editor/AI-IDE-class text/tool UI primitives,
  Win32/Wayland production behavior, and later macOS Cocoa + Metal parity as
  required work.
- DirectX/WebGPU/other backend names should remain clean extension boundaries
  rather than accidental implementation promises until the user opens those
  tracks explicitly.

## 2026-07-08 Zero-Cost Abstraction Principle

- User confirmed zero-cost abstraction should be achievable and should be
  recorded so future implementation slices do not forget it.
- Treat zero-cost abstraction as a measurable engineering constraint: hot paths
  for layout, reconcile/diff, render command generation, event routing,
  reactive invalidation, and renderer submission must not gain hidden heap
  allocation, broad type erasure, avoidable virtual dispatch, repeated handle
  wrapping, or tree-wide per-frame scans.
- Keep static widget composition and dynamic widget/plugin/editor escape
  hatches as separate paths. Dynamic behavior is allowed, but its cost must be
  explicit and must not become the default for static UI composition.
- Future slices that add abstraction boundaries should add behavior, structure,
  or micro-benchmark style guards when the boundary could affect allocation,
  dispatch, or per-frame work.

## 2026-07-08 Zero-Cost Static Element Fast Path

- The existing UI authoring path still defaults to dynamic ownership:
  `AnyElement = std::unique_ptr<Element>`, virtual layout/paint/event hooks,
  `std::function` handlers, and `ElementTree` state backed by `std::any` and
  `dynamic_cast`. That path should remain as the explicit dynamic escape hatch
  for plugins/editor runtime cases, not the long-term default for static
  widget composition.
- The first code slice should create a separate compact static element fast
  path rather than broad-rewriting `Element`. Durable ownership should be a
  focused public leaf over dense `ElementId` records and `std::span` views, so
  traversal and hit testing do not need heap allocation, broad type erasure,
  virtual dispatch, or tree-wide id scans.
- `ElementId`, `ViewId`, and `ElementKey` should move to a small public id
  leaf so static UI code can reference ids without including the dynamic
  `Element`/handler surface.
- Implemented boundary: `include/cgpui/ui/element_ids.hpp` owns ids,
  `include/cgpui/ui/static_element_tree.hpp` owns the compact public records,
  and `src/ui/static_element_tree.cpp` owns non-template lookup, validation,
  and hit-testing behavior.
- Static element records are dense-id indexed: `ElementId{1}` maps to
  `nodes()[0]`, so hot lookup avoids tree-wide id scans. Child relationships
  are explicit `std::span<const ElementId>` ranges rather than per-node
  heap-owned child vectors.
- The dynamic path stays visible through `IntoElement = AnyElement` and
  `ElementTree`; `StaticIntoElement`/`StaticRender` are separate spellings so
  later declarative templates can choose the static route without hiding the
  cost of dynamic widget/plugin UI.

## 2026-07-09 Zero-Cost Runtime Static Render Path

- The static authoring concept is now connected to the runtime frame path:
  `View::supports_static_render()` gates an opt-in `render_static(...)` hook,
  and `WindowRuntime::try_draw_frame()` calls the static path before dynamic
  `render(...)`.
- Static render installation is isolated in
  `src/ui/runtime_static_rendering.cpp` and private declarations are split
  into `src/ui/runtime_static_rendering_internal.hpp`; structure tests guard
  this path from using `AnyElement`, `std::function`, `std::any`,
  `dynamic_cast`, `std::unique_ptr`, or unordered-map storage.
- The runtime now retains the last installed `StaticElementTreeView`, exposes
  it through `WindowRuntime::static_element_tree()`, records
  `RenderTreeKind::static_element_tree` and `static_node_count`, and clears
  the static tree when a dynamic `ElementTree` or manual element root is
  installed.
- Pointer hit testing, route ancestry, hover/active/focus metadata, focus
  ordering, disabled-state cleanup, and accessibility snapshots now read the
  static tree when it is installed. The dynamic `AnyElement`/`ElementTree`
  path remains the explicit fallback and preserves existing manual
  `set_element_root(...)` cursor behavior.
- Remaining zero-cost work after this slice is deeper than the runtime root
  frame path: static widget builders still need lowering into static records,
  event-route storage still uses public vector-backed route records, and text
  input/scroll/action capability dispatch still has explicit dynamic escape
  paths for editor/plugin/runtime UI.

## 2026-07-09 Phase D Step 379 Text Shaping Backend Boundary

- Phase D covers Steps 379-458: text shaping, font fallback, measurement,
  wrapping, selection/caret behavior, edit history, IME, rich text, examples,
  and final text ledger closure.
- The Step 379 entry slice should not claim HarfBuzz shaping is complete on
  this machine: Windows has no visible `pkg-config`, WSL Arch reports no
  HarfBuzz `pkg-config`, and a D-drive header search found no `hb.h`.
- Implemented boundary: `include/cgpui/ui/text_shaping_backend.hpp` and
  `src/ui/text_shaping_backend.cpp` expose backend selection, HarfBuzz
  capability reporting, and explicit fallback reasons. `shape_text(...)`
  now requests HarfBuzz by default, records the selected backend on
  `TextShapeRun`, and uses deterministic fallback when HarfBuzz is not
  available.
- The capability flag is intentionally tied to
  `CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND`, not a raw dependency/header define, so
  future dependency probing cannot make the deterministic implementation claim
  it used HarfBuzz before the backend exists.
- `TextGlyphRun::glyph_id` and `GlyphAtlasKey::glyph_id` are now present so a
  later HarfBuzz backend can pass stable shaped glyph ids into paint/atlas
  metadata without changing text authoring APIs.
- Steps 379-386 remain open until the real HarfBuzz dependency/backend is
  linked on Windows/Linux and verified; this slice is the backend boundary and
  fallback observability needed before that work.

## 2026-07-09 Phase D Step 380 Text Shaping Dispatch Split

- Step 380 keeps the public `shape_text(...)` API stable and moves the real
  glyph-run construction out of `src/ui/text_shape.cpp`.
- Durable private ownership is `src/ui/text_shaping_internal.hpp`,
  `src/ui/text_shaping_dispatch.cpp`, and `src/ui/text_shaping_fallback.cpp`.
  `TextShapingRequest` is implementation-only, while public callers keep using
  `TextShapingOptions` and `TextShapeRun`.
- `shape_text_with_selected_backend(...)` is now the future HarfBuzz insertion
  point. It currently routes to `shape_text_with_deterministic_fallback(...)`,
  preserving Step 379 fallback metadata and glyph ids.
- Structure coverage now fails if `text_shape.cpp` grows the fallback glyph
  loop again, which protects the backend boundary before the real HarfBuzz
  implementation lands.

## 2026-07-09 Phase D Step 381 Wrapped Glyph Id Propagation

- Step 381 fixes the wrapped text paint path so
  `text_glyph_paint_metadata(run, lines, origin)` copies each
  `TextGlyphRun::glyph_id` into `GlyphAtlasKey::glyph_id`.
- The unwrapped glyph paint path already preserved glyph ids after Step 379;
  the wrapped overload was still defaulting every atlas key to glyph id 0.
- This keeps atlas identity stable after soft wrapping and avoids forcing a
  later HarfBuzz backend to special-case wrapped lines.

## 2026-07-09 Phase D Step 382 Shaped Glyph Offsets

- Step 382 adds `TextGlyphRun::offset` as the public record needed for
  HarfBuzz-style glyph positioning.
- Deterministic fallback leaves offsets at zero, preserving existing advances
  and measurements.
- Both `text_glyph_paint_metadata(...)` overloads add the shaped offset to the
  logical glyph origin before computing device origin, so wrapped and
  unwrapped paint paths share the same positioning behavior.

## 2026-07-09 Phase D Step 383 Shaping Metadata

- Step 383 adds `TextShapingDirection`, `TextShapingScript`, and language
  metadata to `TextShapingOptions` and `TextShapeRun`.
- Auto direction and auto script resolve deterministically to left-to-right and
  common script for the fallback path; explicit direction/script/language are
  preserved on the shape run.
- This is HarfBuzz input plumbing, not script detection or bidirectional text
  layout yet.

## 2026-07-09 Phase D Step 384 HarfBuzz Backend Boundary

- Step 384 adds the future HarfBuzz backend insertion point without claiming
  that production HarfBuzz shaping is available on this machine.
- Durable ownership is `src/ui/text_shaping_harfbuzz.cpp`, routed from
  `src/ui/text_shaping_dispatch.cpp` through
  `shape_text_with_harfbuzz(...)` in `src/ui/text_shaping_internal.hpp`.
- Disabled builds rewrite the internal request to deterministic fallback with
  `TextShapingFallbackReason::backend_unavailable`; defining
  `CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND` before the real implementation lands is
  intentionally a build-time error instead of a fake successful backend.

## 2026-07-09 Phase D Step 385 Shaping Diagnostics

- Step 385 keeps shaping diagnostics allocation-free by adding capability
  snapshots to `TextShapingBackendSelection` and `TextShapeRun` instead of
  introducing diagnostic strings.
- `TextShapeRun::used_fallback()` mirrors backend requested-vs-used state, and
  `TextShapeRun::backend_selection()` reconstructs the public selection record
  from run metadata so measurement/wrapping callers can inspect the same
  diagnostic shape without reaching into private shaping dispatch.

## 2026-07-09 Phase D Step 386 Text Shaping Readiness Audit

- Step 386 is a readiness audit rather than a false HarfBuzz completion.
- Durable guard ownership is
  `tests/api_parity/phase_d_text_shaping_audit_test.cpp`; it verifies the
  Step 379-385 shaping boundary evidence, xmake target registration, the
  guarded HarfBuzz source insertion point, fallback/capability diagnostics,
  and the roadmap/ledger wording that production HarfBuzz shaping remains
  incomplete.
- The audit lets subsequent Phase D text/font work proceed without losing the
  real dependency-backed HarfBuzz task: `CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND`
  must still require a real implementation and verification on Windows/Linux.

## 2026-07-09 Phase D Step 387 Font Database Boundary

- Step 387 starts the font discovery/fallback band with a module-boundary
  cleanup before native backend work.
- Durable ownership is `src/ui/text_font.cpp`; public `text_font.hpp` now keeps
  declarations for `FontDatabase`, `FontFallbackChain`,
  `font_database_from_discovered_faces(...)`, and `discover_test_fonts(...)`
  while non-template bodies live in the implementation file.
- Build ownership is `cgpui_platform`, with `cgpui_ui` removing
  `src/ui/text_font.cpp` from its wildcard source set, because
  `PlatformApplication::discover_fonts()` calls the discovered-face helper
  before UI targets are necessarily linked.
- This does not add DirectWrite, fontconfig, FreeType, or CoreText discovery
  yet. It creates the focused UI font boundary those platform adapters should
  feed in subsequent slices.

## 2026-07-09 Phase D Step 388 Platform Font Discovery Diagnostics

- Step 388 adds a platform-facing result and diagnostics boundary rather than
  real native enumeration. `PlatformFontDiscoveryResult` carries discovered
  font records plus allocation-light backend/status/count booleans, and
  `PlatformApplication::discover_font_discovery()` now feeds the older
  `discover_font_records()` and `discover_fonts()` adapters.
- Durable ownership is `include/cgpui/platform/platform_font_discovery.hpp`
  and `src/platform/platform_font_discovery.cpp`; this keeps the platform font
  discovery API out of broad application files and removes the base font
  adapter bodies from `src/platform/empty.cpp`.
- Win32 and Wayland no longer keep static font records in the application or
  services files. The current deterministic platform fallback records live in
  `src/platform/win32/win32_font_discovery.cpp` and
  `src/platform/linux/wayland_font_discovery.cpp` with diagnostics reporting
  DirectWrite/fontconfig as deterministic fallback, not native availability.
- Real DirectWrite/fontconfig enumeration, FreeType face metadata, and native
  coverage checks remain later steps over this boundary.

## 2026-07-09 Phase D Step 389 Win32 DirectWrite Font Discovery

- Step 389 promotes only the Win32 side from deterministic platform fallback
  to real DirectWrite system font-family enumeration. `win32_discover_fonts()`
  creates a shared DirectWrite factory, reads the system font collection, emits
  one platform `FontFaceDescriptor` per family, and reports
  `PlatformFontDiscoveryStatus::native_available` when records are found.
- The deterministic Segoe UI fallback remains inside
  `src/platform/win32/win32_font_discovery.cpp` and is used only if
  `DWriteCreateFactory`, `GetSystemFontCollection`, or record construction
  fails.
- The local SDK exposes PostScript informational strings through
  `IDWriteFont::GetInformationalStrings(...)`, not
  `IDWriteFontFace::GetInformationalStrings(...)`; the first GREEN compile
  caught this and the implementation now queries the representative font
  directly.
- Linux fontconfig/FreeType enumeration, font file paths, coverage checks, and
  richer face metadata remain later Phase D work.

## 2026-07-09 Phase D Step 390 Linux Fontconfig Backend Boundary

- The current WSL Arch host exposes `pkg-config` itself but no visible
  fontconfig/FreeType pkg-config entries, headers, CLI, or runtime libraries;
  native Linux font discovery cannot honestly be marked complete on this
  machine without adding dependencies.
- Step 390 therefore adds the guarded Linux backend insertion point rather
  than a false native implementation. `WaylandApplication::discover_font_discovery()`
  now routes through `wayland_discover_fonts_with_fontconfig()`, with private
  ownership in `src/platform/linux/wayland_font_discovery_internal.hpp` and
  `src/platform/linux/wayland_fontconfig_discovery.cpp`.
- Default builds continue to return the deterministic `sans-serif`
  fontconfig fallback. Defining `CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND`
  enables the future Fontconfig C API path that enumerates `FC_FAMILY`,
  `FC_STYLE`, `FC_FILE`, and PostScript metadata into platform font records.
- `tests/platform/wayland_font_discovery_test.cpp` accepts both the current
  deterministic fallback and future native-available diagnostics, so the test
  will keep guarding the bridge when the dependency-backed path becomes
  available.

## 2026-07-09 Phase D Step 391 Coverage-Aware Font Fallback

- Step 391 adds coverage metadata to font records without introducing a
  shaping dependency or platform coverage probe. `FontUnicodeRange` records
  inclusive Unicode ranges on `FontFaceDescriptor`, and the non-template
  coverage helpers live in `src/ui/text_font.cpp`.
- `font_face_covers_codepoint(...)` treats missing coverage metadata as
  unknown-but-usable for legacy ordered fallback behavior, while
  `FontDatabase::resolve_chain_for_codepoint(...)` first prefers faces that
  explicitly declare coverage for the requested codepoint.
- If no declared coverage face matches, the codepoint-specific resolver falls
  back to the existing `resolve_chain(...)` order. This keeps current native
  records, which do not yet expose coverage, from producing an empty font
  chain.
- This is the data-model and fallback selection step only. Real coverage
  extraction from DirectWrite/fontconfig/FreeType, per-script shaping splits,
  emoji/color glyph planning, and missing-glyph diagnostics remain later Phase
  D work.

## 2026-07-09 Phase D Step 392 Explicit Fallback-Chain Shaping

- Step 392 connects the Step 391 coverage-aware chain to shaping through an
  explicit overload: `shape_text(text, FontFallbackChain, ...)`.
- The overload copies the caller-provided chain into
  `TextShapeRun::font_fallback_faces` and uses the first face as the run's
  primary `FontDescriptor`. Empty chains preserve the existing empty-font
  behavior.
- This deliberately avoids hidden global `FontDatabase` lookup inside
  `shape_text(...)`. Callers that want coverage-aware fallback must resolve the
  chain explicitly, which keeps the cost visible and compatible with the
  zero-cost abstraction rule.
- Real per-glyph fallback splitting is still future work: the deterministic
  fallback shaper records the chain on the run but does not yet split one run
  into multiple font-specific glyph spans.

## 2026-07-09 Phase D Step 393 Glyph Fallback Face Indices

- Step 393 adds `TextGlyphRun::font_fallback_face_index`, giving each shaped
  glyph a stable index into `TextShapeRun::font_fallback_faces`.
- The deterministic fallback shaper now decodes the UTF-8 codepoint for each
  glyph and selects the first provided fallback face whose coverage metadata
  includes that codepoint. With no fallback chain or no matching coverage, the
  index remains 0.
- This is still diagnostic/planning metadata, not full multi-font shaping.
  Glyph advances and deterministic glyph ids remain unchanged; later steps
  should use the face index to split font runs and record missing-glyph cases.

## 2026-07-09 Phase D Step 394 Font Fallback Band Audit

- Step 394 is an audit-only closeout for the Step 387-393 font discovery and
  fallback band.
- Durable guard ownership is
  `tests/api_parity/phase_d_font_fallback_audit_test.cpp`; it checks xmake
  registration, roadmap/ledger/task-plan/finding evidence, Win32 DirectWrite
  enumeration, guarded Linux fontconfig insertion, coverage-aware fallback
  chain APIs, explicit fallback-chain shaping, and glyph-level fallback face
  indices.
- This closeout does not claim dependency-backed Linux fontconfig/FreeType
  enumeration or production HarfBuzz shaping is complete. It hands Phase D to
  the next fallback-splitting, emoji/color glyph, and missing-glyph diagnostic
  band.

## 2026-07-09 Phase D Step 395 Contiguous Font Fallback Runs

- Step 395 adds deterministic per-codepoint fallback splitting metadata without
  adding renderer-side font switching yet.
- `TextFontFallbackRun` lives in the public `text_shape.hpp` leaf beside
  `TextGlyphRun`, and `TextShapeRun::font_runs` records contiguous spans by
  fallback face index with glyph range, byte range, logical advance, and device
  advance.
- The coalescing implementation stays in
  `src/ui/text_shaping_fallback.cpp` via `append_font_fallback_run_span(...)`;
  it is driven only by the explicit `FontFallbackChain` passed to
  `shape_text(...)`, so Step 395 keeps the zero-hidden-global-lookup rule.

## 2026-07-09 Phase D Step 396 Missing Glyph Diagnostics

- Step 396 adds missing-glyph metadata rather than renderer-side replacement
  glyph rendering.
- `TextMissingGlyphDiagnostic` lives in the public `text_shape.hpp` leaf and
  `TextShapeRun::missing_glyphs` records codepoint, glyph index, byte range,
  and fallback face index for known coverage misses.
- The fallback shaper records a miss only when the explicit fallback chain is
  non-empty, every face declares coverage, and no face covers the codepoint.
  Faces with unknown coverage keep the existing unknown-but-usable behavior so
  platform records do not produce false missing-glyph diagnostics before native
  coverage extraction lands.

## 2026-07-09 Phase D Step 397 Color Glyph Planning

- Step 397 adds planning metadata for default emoji-plane color glyphs, not
  real native color font rendering.
- `TextColorGlyphPlan` lives in the public `text_shape.hpp` leaf and
  `TextShapeRun::color_glyphs` records codepoint, glyph index, byte range,
  fallback face index, and requested native-color format.
- The deterministic fallback shaper appends plans through
  `append_color_glyph_plan(...)` when `codepoint_prefers_color_glyph(...)`
  detects U+1F000..U+1FAFF. Step 398 follows up on variation-selector-specific
  emoji presentation; full Unicode emoji data, COLR/CBDT/SBIX/SVG font format
  selection, and renderer-side color glyph drawing remain later Phase D/G work.

## 2026-07-09 Phase D Step 398 Emoji Presentation Selector Planning

- Step 398 adds deterministic planning for `U+FE0F` emoji presentation
  selectors without claiming real HarfBuzz variation shaping.
- The fallback shaper keeps the selector as a deterministic glyph record but
  associates its color-glyph request with the immediately preceding
  emoji-capable base glyph through
  `append_emoji_presentation_color_glyph_plan(...)`.
- `is_emoji_presentation_selector(...)` suppresses false missing-glyph
  diagnostics for the selector itself, so explicit fallback chains that cover
  the base symbol but not `U+FE0F` do not report a missing glyph for the
  selector.
- The implementation remains in `src/ui/text_shaping_fallback.cpp`; behavior
  coverage lives in `test_shape_text_records_color_glyph_plans`, and structure
  coverage guards the new helper names in `ui_source_structure_test`.

## 2026-07-09 Phase D Step 399 Emoji Selector Span Metadata

- Step 399 keeps Step 398's deterministic fallback behavior and adds the
  selector span to `TextColorGlyphPlan`.
- `TextColorGlyphPlan::has_emoji_presentation_selector`,
  `emoji_presentation_selector_byte_offset`, and
  `emoji_presentation_selector_byte_length` record whether `U+FE0F` authored
  the color presentation request and where that selector lives in the original
  UTF-8 text.
- `mark_emoji_presentation_selector_span(...)` updates an existing default
  emoji-plane plan when a selector follows a color-default codepoint, or marks
  the newly appended BMP-symbol emoji plan when the base needed the selector to
  request color presentation.
- This is still metadata for future native shaping/rendering. It does not add
  full Unicode emoji data, HarfBuzz variation shaping, COLR/CBDT/SBIX/SVG font
  selection, or renderer-side color glyph drawing.

## 2026-07-09 Phase D Step 400 Emoji ZWJ Missing-Glyph Suppression

- Step 400 prevents deterministic fallback shaping from reporting `U+200D` as
  a missing glyph when it is serving as an emoji ZWJ sequence joiner.
- The suppression is deliberately narrow: `is_emoji_sequence_joiner_between_emoji(...)`
  requires a previous emoji-capable codepoint and a next emoji-capable
  codepoint before the missing-glyph diagnostic is skipped.
- `next_utf8_codepoint_after(...)` keeps the lookahead local to
  `src/ui/text_shaping_fallback.cpp`; it does not introduce Unicode
  segmentation tables or a HarfBuzz dependency.
- `U+200D` still remains a deterministic glyph record, and each emoji
  codepoint in the sequence still gets its own `TextColorGlyphPlan`. Native
  ZWJ ligature shaping and single-glyph emoji sequence rendering remain future
  HarfBuzz/color-font work.

## 2026-07-09 Phase D Step 401 Script Run Metadata

- Step 401 adds lightweight script-run metadata to deterministic fallback
  shaping rather than full Unicode itemization.
- `TextScriptRun` lives in the public `text_shape.hpp` leaf, and
  `TextShapeRun::script_runs` records contiguous glyph/byte/advance spans by
  fallback-classified script.
- `classify_text_shaping_script(...)` and `append_script_run_span(...)` stay in
  `src/ui/text_shaping_fallback.cpp`, keeping the helper local to the fallback
  implementation and avoiding a new broad script module before HarfBuzz lands.
- The classifier deliberately covers only deterministic ranges needed for the
  current metadata path: latin, han, hiragana, katakana, hangul, hebrew,
  arabic, devanagari, emoji-plane, and common fallback. Full Unicode script
  data, bidirectional shaping, HarfBuzz script itemization, and real native
  script segmentation remain future Phase D work.

## 2026-07-09 Phase D Step 402 Fallback Metadata Band Audit

- Step 402 is an audit-only closeout for the Step 395-401 fallback metadata
  band; it does not add runtime shaping, rendering, or font discovery behavior.
- `tests/api_parity/phase_d_fallback_splitting_audit_test.cpp` freezes the
  public `TextFontFallbackRun`, `TextMissingGlyphDiagnostic`,
  `TextColorGlyphPlan`, and `TextScriptRun` evidence plus the fallback-source
  helper ownership for font spans, missing-glyph diagnostics, color glyph
  planning, emoji presentation selectors, emoji ZWJ suppression, and
  deterministic script-run spans.
- The closeout keeps the remaining gaps explicit before Step 403 starts text
  measurement and wrapping: production HarfBuzz shaping, full Unicode script
  data, bidirectional text layout, real color glyph rendering, native ZWJ
  ligature shaping, and dependency-backed native Linux fontconfig/FreeType
  enumeration remain incomplete.

## 2026-07-09 Phase D Step 403 Grapheme Column Measurement

- Step 403 adds grapheme column metadata to `TextMeasurement` rather than
  changing shaping, wrapping, or cursor movement behavior.
- `TextGraphemeColumn` records column range, byte range, glyph range, logical
  advance, and device advance. `TextMeasurement::grapheme_columns` is filled
  during `measure_text(...)` from the shaped fallback glyph records.
- The implementation stays in `src/ui/text_measurement_grapheme.cpp` through
  `build_text_grapheme_columns(...)` and
  `text_grapheme_column_includes_codepoint(...)`, keeping measurement-specific
  span construction out of broad UI/runtime files.
- The deterministic segmentation covers the current skeleton cases already
  expected by the text model: combining marks, variation selectors,
  zero-width-joiner continuations, and regional-indicator pairs. It is not full
  Unicode grapheme breaking, bidirectional text layout, paragraph shaping, or
  production HarfBuzz itemization.

## 2026-07-09 Phase D Step 404 Grapheme-Aware Soft Wrap

- Step 404 changes soft wrapping to consume
  `TextMeasurement::grapheme_columns` instead of splitting directly on each
  glyph advance when column metadata is available.
- `TextWrapLine` now records `column_start` and `column_end` alongside the
  existing byte/glyph ranges, so paint metadata can keep using glyph spans
  while text layout callers can reason about measured grapheme columns.
- `text_wrap_line_for_column_range(...)` is the focused helper that maps a
  column span back to byte and glyph ranges. The older
  `text_wrap_line_for_range(...)` remains as a compatibility fallback for
  measurements without column metadata.
- This preserves combining-mark, variation-selector, and regional-indicator
  columns as atomic soft-wrap units. It does not implement hard wraps,
  Unicode line-break classes, bidirectional layout, paragraph shaping, or a
  production HarfBuzz itemization path.

## 2026-07-09 Phase D Step 405 Hard-Wrap Line Records

- Step 405 adds explicit hard-wrap metadata for newline columns without
  changing shaping or measurement. Newlines remain shaped glyph records, but
  wrapped line ranges exclude the newline glyph from paint metadata.
- `TextWrapBreakKind` distinguishes no break, soft width break, and hard
  newline break; `TextWrapLine::break_kind` carries that result beside
  byte/glyph/column ranges.
- `text_wrap_column_is_hard_break(...)` keeps newline detection local to
  `src/ui/text_wrapping.cpp`, and `wrap_text_measurement(...)` still uses
  measured grapheme columns as the primary wrapping unit.
- This is explicit `\n` handling only. CRLF normalization, Unicode line-break
  classes, bidirectional paragraph layout, and production shaping itemization
  remain later Phase D work.

## 2026-07-09 Phase D Step 406 Bidirectional Planning Metadata

- Step 406 adds deterministic bidi planning metadata over measured grapheme
  columns rather than full Unicode visual reordering.
- `TextBidiRun`, `TextMeasurement::base_direction`, and
  `TextMeasurement::bidi_runs` record contiguous LTR/RTL spans with
  byte/glyph/column ranges plus logical/device advances.
- `src/ui/text_measurement_bidi.cpp` owns the fallback planner through
  `build_text_bidi_runs(...)` and `classify_text_bidi_direction(...)`,
  keeping bidi span construction out of broad measurement or wrapping files.
- `TextWrapLine::bidi_run_start`, `TextWrapLine::bidi_run_end`, and
  `TextWrapLayout::base_direction` carry the measurement bidi plan into wrap
  records so later paragraph layout can map lines back to directional runs.
- The classifier is intentionally small and deterministic: Hebrew/Arabic
  ranges become RTL, whitespace/newline follows the base direction, and all
  other codepoints stay LTR. Full Unicode bidi data, visual run placement,
  paragraph shaping, and HarfBuzz itemization remain future Phase D work.

## 2026-07-09 Phase D Step 407 Line Metrics And Line Boxes

- Step 407 adds explicit deterministic line metrics rather than deriving line
  box values ad hoc from `TextShapeRun::line_height` in wrapping code.
- `TextLineMetrics` records logical and device ascent, descent, leading, line
  height, and baseline; `TextMeasurement::line_metrics` stores the measured
  metrics beside grapheme and bidi metadata.
- `src/ui/text_line_metrics.cpp` owns `text_line_metrics_for_shape_run(...)`
  and the deterministic fallback baseline helper. The current fallback keeps
  ascent/baseline at 80 percent of line height, descent at the remainder, and
  leading at zero.
- `TextWrapLine::metrics` carries those values into each wrapped line, so line
  records now expose the line box and baseline inputs that later paragraph
  layout, hit testing, selection painting, and native text rendering will need.
- This is not platform-derived font metrics. DirectWrite/fontconfig/FreeType
  ascent/descent/leading extraction, paragraph line boxes, and native shaping
  itemization remain future work.

## 2026-07-09 Phase D Step 408 Paragraph Layout Cache

- Step 408 adds an explicit paragraph layout cache over measurement plus
  wrapping, not a hidden global text-layout cache.
- `TextParagraphLayout` carries a `TextMeasurement` and the corresponding
  `TextWrapLayout`; `TextParagraphLayoutResult` mirrors the existing cache-hit
  result style used by measurement and uniform-list caches.
- `TextParagraphLayoutCache` keys entries by text, font descriptor, font size,
  normalized scale, and max width, so changing the wrap width produces a new
  paragraph layout entry.
- The implementation lives in `src/ui/text_paragraph_layout.cpp`; the public
  leaf is `include/cgpui/ui/text_paragraph_layout.hpp`, and
  `include/cgpui/ui/text_layout.hpp` stays a thin aggregate over the focused
  leaves.
- This is deterministic reuse infrastructure. It does not add eviction policy,
  incremental paragraph invalidation, multi-paragraph shaping, platform font
  metric extraction, or production HarfBuzz paragraph itemization.

## 2026-07-09 Phase D Step 409 CRLF Hard-Wrap Normalization

- Step 409 normalizes `\r\n` hard-wrap handling at the measured grapheme
  column boundary rather than adding a separate paragraph pass.
- `text_measurement_is_crlf_pair(...)` keeps adjacent CR and LF glyphs in one
  `TextGraphemeColumn`, preserving their byte/glyph span while preventing the
  CR glyph from becoming visible text on the previous wrapped line.
- `text_wrap_column_is_hard_break(...)` now treats columns starting with
  either `\n` or `\r` as hard breaks, so CRLF and lone CR/LF all use the same
  hard-wrap path.
- Wrapped glyph paint metadata naturally skips the CRLF glyphs because the
  hard-wrap line range ends before the break column and the next line starts
  after it.
- Full Unicode line-break classes, paragraph shaping, and native shaping
  itemization remain future Phase D work.

## 2026-07-09 Phase D Step 410 Text Measurement/Wrapping Band Audit

- Step 410 is an audit-only closeout for the Step 403-409 text measurement and
  wrapping band.
- `tests/api_parity/phase_d_text_measurement_wrapping_audit_test.cpp` freezes
  the public measurement, wrapping, paragraph-layout, and documentation
  evidence for grapheme columns, grapheme-aware soft wrap, hard-wrap records,
  bidi planning, deterministic line metrics, explicit paragraph layout/cache
  records, and CRLF hard-wrap normalization.
- The closeout marks roadmap Steps 403-410 complete and leaves the next
  handoff at Steps 411-418 selection and caret behavior.
- Remaining text gaps are still explicit rather than hidden by the audit:
  production HarfBuzz shaping, real color glyph rendering, full Unicode bidi
  reordering, full Unicode line-break classes, paragraph shaping, cache
  eviction policy, platform-derived font metrics, and dependency-backed native
  Linux fontconfig/FreeType enumeration remain future Phase D work.

## 2026-07-09 Phase D Step 411 Text Selection Drag Records

- Step 411 starts the selection/caret band with explicit drag-selection
  records in the focused `text_hit_testing` leaf.
- `TextSelectionDragDirection` distinguishes collapsed, forward, and backward
  drags; `TextSelectionDrag` preserves anchor/head offsets beside the normalized
  `TextSelectionRange`.
- `text_selection_drag_from_offsets(...)` and
  `text_selection_drag_from_points(...)` are zero-allocation helpers that reuse
  the existing text hit-testing path and keep anchor/head direction visible to
  future double/triple click and word/line selection work.
- `WindowRuntime::apply_text_pointer_selection(...)` now routes pointer down,
  move, and release selection updates through the same helper instead of
  open-coding anchor/head model updates.

## 2026-07-09 Phase D Step 412 Word Selection Range Helpers

- Step 412 keeps word-selection range computation in `TextModel`, where the
  existing grapheme and word-boundary helpers already live.
- `TextModel::word_selection_range_at(...)` returns a `TextSelectionRange`
  without allocating, selecting the word containing the requested byte offset
  and returning a collapsed range for separators or end offsets.
- Continuation-byte offsets are normalized back to the owning UTF-8 codepoint
  boundary before separator/word checks, so future hit-testing and
  double-click paths can reuse the helper without exposing invalid byte ranges.
- Runtime double-click/triple-click gesture policy is intentionally not added
  in this slice; this only exposes the model-level range primitive.

## 2026-07-09 Phase D Step 413 Line Selection Range Helpers

- Step 413 adds the line-selection counterpart to the Step 412 word range
  helper inside `TextModel`.
- `TextModel::line_selection_range_at(...)` returns the current logical line
  range without allocating, using existing `line_start_for_offset(...)` and
  `line_end_for_offset(...)` navigation.
- The helper trims a trailing `\r` before `\n`, so CRLF lines select the text
  content while keeping newline bytes out of the returned range.
- Empty lines return collapsed ranges, while an end-of-buffer offset still
  selects the final non-empty line when one exists.

## 2026-07-09 Phase D Step 414 Multi-Click Selection Granularity

- Step 414 adds click-count plumbing without changing native platform
  synthesis policy yet.
- `PointerButton::click_count` defaults to `1`, so existing constructed and
  platform-produced pointer button events keep single-click behavior.
- `TextSelectionGranularity` lives in the focused text hit-testing leaf and
  maps click counts to caret, word, and line selection through
  `text_selection_granularity_for_click_count(...)`.
- The mapping is explicit and zero-allocation: `0` and `1` select caret,
  `2` selects word, and `3+` selects line.
- Expanded WSL verification exposed that renderer-owned shaping objects call
  `FontFallbackChain`/coverage helpers from `src/ui/text_font.cpp`; the file is
  therefore explicitly compiled into both `cgpui_platform` for platform font
  discovery and `cgpui_renderer` for shaping/link-order independence.

## 2026-07-09 Phase D Step 415 Runtime Double-Click Word Selection

- Step 415 wires the Step 414 click granularity plumbing into runtime text
  input selection for double-clicks.
- `WindowRuntime::apply_text_pointer_selection(...)` still keeps single-click
  caret placement and drag selection on the existing `TextSelectionDrag` path.
- For `TextSelectionGranularity::word`, the runtime computes the hit offset,
  calls `TextModel::word_selection_range_at(...)`, and sets the range directly
  without storing `text_pointer_selection_drag_`, so the following pointer-up
  event preserves the selected word instead of collapsing it.
- Platform-native double-click synthesis remains future work; this slice
  consumes the explicit `PointerButton::click_count` field when callers or
  tests provide it.

## 2026-07-09 Phase D Step 416 Runtime Triple-Click Line Selection

- Step 416 adds the line-selection counterpart to Step 415 inside the same
  focused runtime pointer-selection boundary.
- `WindowRuntime::apply_text_pointer_selection(...)` now consumes
  `TextSelectionGranularity::line` from `click_count >= 3`, calls
  `TextModel::line_selection_range_at(...)`, and sets the selected line
  directly without arming `text_pointer_selection_drag_`.
- The runtime keeps single-click caret/drag and double-click word selection
  separate, so each granularity remains explicit and testable without hidden
  allocation or platform gesture synthesis.

## 2026-07-09 Phase D Step 417 Multiline Selection And Caret Paint Geometry

- Step 417 moves text selection/caret rectangle calculation out of
  `TextElement::paint(...)` and into focused internal
  `src/ui/text_selection_paint_geometry.cpp` / `.hpp` helpers.
- The helper measures and wraps the same text/font/scale/max-width inputs used
  by text paint, then maps selection byte ranges onto `TextWrapLine` metrics so
  multiline selections emit one `TextSelectionPaint` command per painted line.
- Caret geometry now uses the matching wrapped line and line metrics, so a
  cursor on a later hard-wrapped line paints at that line's x/y instead of
  using first-line byte-offset geometry.
- This is still deterministic fallback geometry. It does not claim full bidi
  visual selection, paragraph shaping, scroll-to-caret, preferred-column
  navigation, or native text renderer selection quads.

## 2026-07-09 Phase D Step 418 Selection/Caret Band Closeout

- Step 418 closes the selection/caret band with explicit, zero-allocation
  preferred-column and scroll-to-caret primitives rather than hidden runtime
  scans, covering the Step 411-418 evidence.
- `TextModel` stores `preferred_line_column_` only while vertical cursor or
  selection movement is active, so short-line clamping does not destroy the
  original target column and non-vertical edits/movement reset the state.
- `ScrollModel::scroll_rect_into_view(...)` moves the reusable scroll math out
  of the public header into `src/ui/scroll.cpp`, keeping the public leaf thin.
- `TextElement::caret_rect(...)` and
  `TextElement::scroll_caret_into_view(...)` reuse the same measured hard-wrap
  caret geometry as paint, and runtime IME placement now consumes that shared
  `text_caret_rect(...)` path instead of byte-offset glyph-width math.
- Runtime scroll routing now recognizes `ScrollableListElement` as a scroll
  state owner alongside `ScrollElement`; focused text clipboard copy/cut/paste
  coverage remains part of the frozen selection/caret evidence.
- The closeout audit is
  `tests/api_parity/phase_d_selection_caret_audit_test.cpp`; remaining Phase D
  text work starts at Step 419 edit history and continues through IME, rich
  text, examples, and final verification.

## 2026-07-09 Phase D Step 419 Adjacent Typing History

- Step 419 starts the edit-history band by coalescing uninterrupted typing into
  one undo/redo record instead of pushing one snapshot per `insert_text(...)`.
- `TextInsertHistoryPolicy::merge_adjacent_typing` is the default public insert
  policy for text-input typing. `TextInsertHistoryPolicy::separate_edit` keeps
  paste and composition commits independent so later IME grouping can be added
  explicitly.
- The grouping gate is owned by `src/ui/text_model_history.cpp`: it only merges
  when the previous record was typing, grouping is still open, and the new
  before snapshot exactly equals the previous after snapshot.
- Navigation, selection, delete, undo/redo, composition updates/cancel, and
  selection replacement call or imply `clear_edit_history_grouping()`, so
  grouping does not silently cross user-visible operation boundaries.

## 2026-07-09 Phase D Step 420 IME Composition History Grouping

- IME delete-surrounding edits belong to the active composition transaction at
  the model history layer. Capturing the pre-composition snapshot once lets
  commit and cancel create one undo record without teaching platform event
  routing about undo grouping.
- This closes the model-level grouped-commit behavior only. Deeper Win32
  TSF/IMM and Wayland text-input v3 surrounding-text protocol behavior remains
  in the later IME platform band.

## 2026-07-09 Phase D Step 421 Undo Manager Integration Points

- External editor shells need more than `can_undo()` / `can_redo()` booleans:
  they need stack depth, clean state, and a cheap revision value for command
  enablement and document-dirty UI. Returning a small
  `TextEditHistoryStatus` by value keeps this zero-allocation and leaves stack
  ownership inside `TextModel`.
- `mark_edit_history_clean()` must close the active adjacent-typing group.
  Otherwise saving after the first typed character and then continuing to type
  would merge across the save point and erase the undo-visible clean boundary.

## 2026-07-09 Phase D Step 422 Redo Invalidation Diagnostics

- Redo invalidation is different from redo consumption. A normal `redo()` pops
  one redo record and should not report data loss, while a branch edit after
  undo clears the remaining redo stack and needs a diagnostic.
- `TextEditHistoryStatus::last_redo_invalidation` records the last branch
  invalidation with a small enum, cleared redo depth, and revision. This gives
  editor shells an observable signal without exposing the stacks or allocating a
  separate undo manager object.

## 2026-07-09 Phase D Step 423 Edit Transaction Diagnostics

- The edit-history status surface now reports the last transaction separately
  from redo invalidation. This keeps normal commits, adjacent typing merges,
  undo, redo, and clean marks observable without forcing callers to infer them
  from stack-depth deltas.
- `TextEditHistoryTransactionDiagnostic` only carries enum/policy/depth/revision
  scalars. It deliberately does not copy `TextHistorySnapshot` text or expose
  undo/redo vectors, preserving the zero-cost/low-allocation direction for
  editor command state and diagnostics.

## 2026-07-09 Phase D Step 424 Edit-History Band Closeout

- Step 424 is an audit-only closeout for the edit-history band. It freezes the
  Step 419-423 evidence: adjacent typing grouping, composition commit grouping,
  undo-manager status, redo invalidation diagnostics, and transaction
  diagnostics.
- The closeout keeps the model/history layer as the owner of undo/redo stacks.
  Active-target IME platform behavior remains the next Phase D band rather than
  being smuggled into edit-history diagnostics.

## 2026-07-09 Phase D Step 427 IME Surrounding Text Placement

- `ImeTextInputPlacement` is now the explicit cross-platform carrier for
  surrounding text, selection anchor, and content hint/purpose metadata. The
  fields default to empty/zero so existing placement callers keep their old
  behavior unless they opt into richer IME context.
- Runtime focused-text placement fills the new fields from the focused
  `TextModel` only when applying IME placement to the platform window. This
  keeps the allocation visible at the platform-boundary update point instead
  of hiding a text copy in layout, paint, or per-frame traversal.
- Wayland text-input v3 now submits the caller-provided surrounding text,
  cursor, anchor, content hint, and content purpose through the focused
  `src/platform/linux/wayland_text_input_requests.cpp` boundary. Win32 IMM
  keeps candidate/composition rectangle placement but preserves the same public
  placement state.
- The first Windows focused rerun failed in `window_runtime_text_test` with
  return code 320 because the new assertion expected `selection_anchor == 1`
  while the platform-placement test still used a collapsed `set_selection(3, 3)`.
  Changing that test fixture to `set_selection(1, 3)` kept the cursor at byte 3
  and made the anchor assertion meaningful.

## 2026-07-09 Phase D Step 428 Wayland IME Serial Propagation

- Wayland text-input v3 `done(serial)` is now preserved on the emitted IME
  events instead of being discarded in the protocol listener. The serial is
  forwarded only at the Wayland text-input/window bridge boundary and stored as
  a scalar on `ImeComposition` / `ImeDeleteSurroundingText`, so the slice does
  not add hidden allocation, broad policy state, or per-frame work.
- This closes serial transport, not the full IME policy story. Richer Wayland
  serial policy, preedit styling, candidate placement, and Win32 TSF depth
  remain explicit Phase D gaps.

## 2026-07-09 Pre-Phase-D Scope And Zero-Cost Guard

- The roadmap already records the post-Phase-C user scope decision: defer
  game-engine-specific integration, engine runtime embedding, Android, iOS,
  and X11, while keeping the desktop C++23 GPUI runtime requirements active.
- The roadmap also records zero-cost abstraction as a hard engineering
  constraint: static fast paths first, no hidden hot-path allocation, no broad
  type erasure or avoidable virtual dispatch on hot paths, explicit dynamic
  escape hatches, and tests for abstraction boundaries.
- `tests/api_parity/phase_c_final_ledger_audit_test.cpp` now freezes both
  roadmap sections so the Phase C closeout cannot silently lose the binding
  scope decision or zero-cost principle before future Phase D work continues.
- The previous guard was correct but implicit: it lived inside the Phase C
  final ledger audit. A dedicated pre-Phase-D entry gate makes the user
  requirement visible as its own failing target and records the required
  focused gates for both Windows and WSL:
  `pre_phase_d_entry_gate_test/default`,
  `phase_c_final_ledger_audit_test/default`,
  `static_render_runtime_test/default`, `ui_source_structure_test/default`,
  and `gpui_parity_ledger_test/default`.

## 2026-07-09 Phase D Step 429 Wayland Preedit Cursor Metadata

- Wayland text-input v3 `preedit_string` sends `cursor_begin` and `cursor_end`
  alongside the preedit text. Step 429 keeps that protocol metadata instead of
  discarding it, storing the values in `WaylandTextInput::PendingPreedit` until
  `done(serial)` emits the IME update.
- `ImeComposition` now carries `preedit_cursor_begin` and
  `preedit_cursor_end` as scalar metadata. The runtime still consumes the
  preedit text normally; this slice does not add styling spans, candidate
  placement, or a broader Wayland serial policy.
- The test compositor now has an overload for explicit preedit cursor ranges,
  and `wayland_keyboard_test` asserts `1..3` for the `"draft"` preedit update.

## 2026-07-09 Phase D Step 430 Preedit Style Metadata

- Step 430 adds platform-neutral IME preedit styling as fixed-capacity metadata
  on `ImeComposition`, not as a dynamic allocation-heavy style vector.
- `ImePreeditStyleSpan`, `kImePreeditStyleSpanCapacity`,
  `append_ime_preedit_style(...)`, and
  `append_ime_default_preedit_style(...)` live at the core text event boundary,
  with non-inline helper bodies in `src/core/event_text.cpp`.
- Wayland text-input v3 does not provide rich style spans, so the Wayland
  window preedit path attaches one default underline span covering the preedit
  text. This preserves the future rendering/candidate-placement input without
  changing `TextModel` composition behavior.

## 2026-07-09 Phase D Step 431 IME Candidate Placement Metadata

- `ImeTextInputPlacement::rect` was doing double duty for composition/cursor
  compatibility and candidate-window placement. Step 431 keeps that compatible
  field but adds `candidate_rect` as an explicit optional candidate geometry.
- Runtime focused text placement now copies the `ImeCandidateRect` into both
  `rect` and `candidate_rect`, making the current behavior explicit while
  preserving the low-allocation boundary at platform placement time.
- Wayland text-input v3 now prefers `candidate_rect` for
  `set_cursor_rectangle`; Win32 IMM keeps `COMPOSITIONFORM` on `rect` and uses
  `candidate_rect` for `CANDIDATEFORM`.
- This is metadata/routing depth only. Production candidate UI policy remains
  a later Phase D gap.

## 2026-07-09 Phase D Step 432 Win32 IMM Composition Strings

- Step 432 keeps Win32 IME message dispatch thin: `WM_IME_COMPOSITION` and
  `WM_IME_ENDCOMPOSITION` are forwarded from
  `src/platform/win32/win32_window_proc_lifecycle.cpp` into the focused
  `src/platform/win32/win32_window_ime.cpp` module.
- `win32_window_ime.cpp` now reads `GCS_COMPSTR` and `GCS_RESULTSTR` through
  `ImmGetCompositionStringW`, converts UTF-16 data through the existing
  `utf8_from_utf16(...)` helper, and emits platform-neutral
  `ImeCompositionPhase::update` / `commit` events.
- `WM_IME_ENDCOMPOSITION` emits an explicit cancel event so runtime text models
  can clear unfinished composition without Win32-specific state.
- The current Windows host does not make synthetic `ImmSetCompositionStringW`
  data readable through `GCS_COMPSTR`, so behavior coverage stays on the stable
  end-composition cancel message and structure coverage freezes the production
  `GCS_*` parsing boundary.

## 2026-07-09 Phase D Step 433 Wayland Stale Serial Policy

- Wayland text-input v3 `done(serial)` is now policy-bearing rather than only
  forwarded metadata. `WaylandTextInput` stores the last accepted serial and
  treats non-increasing serials as stale.
- Stale or inactive `done(serial)` calls clear pending preedit, delete
  surrounding, and commit state instead of emitting partially queued IME events
  later.
- `reset_text_input()` also clears pending state and resets the serial baseline,
  keeping serial lifetime tied to the text-input object.
- The Wayland test compositor can now attach request-specific serials to
  preedit and commit events. `wayland_keyboard_test` sends serial 5 preedit,
  serial 4 stale commit, then normal delete/commit events that continue at
  serials 6 and 7, verifying stale commit text never reaches the callback.

## 2026-07-09 Phase D Step 434 IME Platform Band Closeout

- Step 434 is an audit-only closeout for the active-target IME platform band.
- `tests/api_parity/phase_d_ime_platform_audit_test.cpp` freezes the Step
  427-433 evidence for surrounding text/content hints, Wayland serial
  propagation and stale serial policy, preedit cursor/style metadata,
  candidate-placement metadata, Win32 IMM composition/result string ingestion,
  and the focused platform source boundaries.
- The closeout moves the Phase D handoff to Step 435 rich text runs.
- Remaining gaps stay explicit: production candidate UI policy, deeper Win32
  TSF integration, and richer platform-specific composition styling are later
  work rather than hidden by the audit.

## 2026-07-09 Phase D Step 435 Rich Text Run Core

- Step 435 should start the rich-text band with a focused run-core leaf rather
  than adding rich-text state to `TextModel` or renderer files.
- `RichTextSpan`, `RichTextRun`, `RichTextAttributes`, `RichTextDecoration`,
  `RichTextRunBuildScratch`, `RichTextLinkId`, and
  `build_rich_text_runs(...)` form the first public surface. The implementation
  clips spans to the text length, drops empty spans, merges overlapping
  attributes in caller order, coalesces adjacent equal runs, and exposes a
  caller-owned output/scratch overload so callers can reuse storage.
- This closes rich-text run normalization only. Inline images, painting,
  syntax-theme integration, link activation, and run-aware hit testing remain
  explicit later Phase D work.

## 2026-07-09 Phase D Step 436 Rich Text Byte Hit Metadata

- Step 436 should keep rich-text interaction metadata in the focused
  `text_rich_text` leaf rather than routing through `WindowRuntime` or renderer
  hit-test state.
- `RichTextRunHit`, `RichTextLinkHit`,
  `rich_text_run_at_byte_offset(...)`, and
  `rich_text_link_at_byte_offset(...)` expose small optional records for
  half-open byte ranges. Link hits reuse numeric `RichTextLinkId`, avoiding
  string link targets or hidden allocation in the hit path.
- This closes byte-offset run/link lookup only. Point-based rich-text hit
  testing, click activation wiring, inline image spans, and paint integration
  remain explicit later Phase D work.

## 2026-07-09 Phase D Step 437 Rich Text Point Hit Metadata

- Step 437 should live in a separate focused `text_rich_text_hit_testing`
  public leaf because it composes rich-text run metadata with text geometry
  hit-testing instead of extending run normalization.
- `RichTextRunPointHit`, `RichTextLinkPointHit`,
  `rich_text_run_at_point(...)`, and `rich_text_link_at_point(...)` reuse
  `hit_test_text_position(...)` to get the byte offset, then call the Step 436
  byte-range lookup helpers. This keeps allocation and state ownership visible:
  no runtime state, renderer state, or string link targets are introduced.
- This closes point-based run/link metadata only. Click activation wiring,
  inline images, syntax-theme integration, and rich-text paint integration
  remain explicit later Phase D work.

## 2026-07-09 Phase D Step 438 Rich Text Inline Image Metadata

- Step 438 keeps inline image handling as metadata, not paint or asset loading:
  it lives in a focused `text_rich_text_inline_image` public leaf and reuses the
  existing numeric `ImageAssetId` surface instead of string paths or URLs.
- `RichTextInlineImageSpan`, `RichTextInlineImageRun`, and
  `build_rich_text_inline_image_runs(...)` normalize caller-provided inline
  image anchors by filtering invalid ids/sizes/out-of-range spans, clipping
  byte ranges to the text length, preserving zero-length insertion anchors, and
  sorting deterministically into caller-owned output storage.
- This closes inline image metadata only. Rich-text image paint integration,
  image loading/registration policy, click activation wiring, and syntax-theme
  integration remain explicit later Phase D work.

## 2026-07-09 Phase D Step 439 Rich Text Syntax Theme Metadata

- Step 439 keeps syntax highlighting as a caller-owned token-to-span adapter,
  not a parser, editor integration, or renderer concern. The focused
  `text_rich_text_syntax` leaf owns the public enum/theme/span builder surface.
- `RichTextSyntaxRole`, `RichTextSyntaxToken`, `RichTextSyntaxTheme`,
  `rich_text_syntax_attributes_for_role(...)`, and
  `build_rich_text_syntax_spans(...)` convert fixed role tokens into
  `RichTextSpan` records with clipping, empty-style filtering, and deterministic
  sorting. The design avoids maps, reflection, runtime state, renderer state,
  and hidden parser ownership.
- This closes syntax-theme metadata only. Syntax parsing, editor token source
  integration, rich-text paint integration, click activation wiring, and inline
  image paint integration remain explicit later Phase D work.

## 2026-07-09 Phase D Step 440 Rich Text Paint Metadata Integration

- Rich-text paint integration should start as metadata transport, not renderer
  behavior. `PaintList::fill_rich_text(...)` can reuse the existing plain text
  measurement/wrapping/glyph metadata path while preserving caller-provided
  `RichTextRun` and `RichTextInlineImageRun` records for later renderer and
  interaction work.
- The focused ownership boundary is `src/ui/paint_rich_text.cpp` plus the
  existing paint command and renderer command records. This keeps rich-text
  metadata out of broad `paint.cpp`, runtime state, Vulkan drawing code, and
  string link targets.
- This closes paint-command/render-frame metadata retention only. Actual
  multi-color glyph painting, inline image drawing/loading, click activation,
  syntax parsing, and editor token source integration remain explicit Phase D
  gaps.

## 2026-07-09 Phase D Step 441 Rich Text Link Activation Metadata

- Link activation can stay as a focused metadata helper over existing
  point-hit records. `rich_text_link_activation_at_point(...)` should accept a
  `PointerButton`, require a primary single-button release, require an
  in-bounds text hit, and return only scalar activation metadata plus the
  existing numeric `RichTextLinkId`.
- This boundary deliberately avoids runtime-owned rich-text state, renderer
  state, broad event dispatch, string link targets, and hidden allocation. It
  gives a future rich-text element or editor surface a small record to consume.
- This closes activation-record creation only. Runtime element dispatch, link
  command handling, inline image drawing/loading, syntax parsing, and editor
  token source integration remain explicit Phase D gaps.

## 2026-07-09 Phase D Step 442 Rich Text Metadata Band Closeout

- Step 442 is an audit-only closeout for Steps 435-441, not a renderer/runtime
  feature slice. It should freeze the already-landed public leaves for
  rich-text runs, byte/point hit metadata, inline image metadata, syntax-theme
  metadata, paint metadata transport, and link activation metadata.
- The audit should update the main text ledger row so rich-text metadata is no
  longer listed as wholly missing, while keeping the remaining gaps explicit:
  runtime rich-text element dispatch, actual link command handling, inline image
  drawing/loading, syntax parsing/editor token source integration, and actual
  multi-color glyph painting.
- This keeps the next Phase D handoff pointed at text/input public examples
  rather than hiding unfinished renderer, parser, or editor integration work
  under the rich-text metadata band.

## 2026-07-09 Phase D Step 443 Text Input Public Examples

- Phase D Step 443 text input public examples should stay prelude-only and
  avoid private runtime hooks. Step 443 starts the text-input public examples band with `examples/api_parity/public_text_input_examples/main.cpp`,
  `api_parity_public_text_input_examples`, and
  `tests/api_parity/phase_d_text_input_public_examples_test.cpp`, not a new
  text engine feature.
- The example should demonstrate TextModel public editing, selection ranges,
  edit-history status, text-input builder use, IME placement and composition metadata,
  delete-surrounding metadata, command palette entries, and key
  bindings from public headers.
- Text wrapper examples, richer official input workflows, rich-text examples,
  and final Phase D verification remain separate later steps.

## 2026-07-09 Phase D Step 444 Text Wrapper Public Examples

- Phase D Step 444 text wrapper public examples should remain an examples/API
  compatibility slice, not a new text engine feature.
- Step 444 extends the text-input examples band with
  `examples/api_parity/public_text_wrapper_examples/main.cpp`,
  `api_parity_public_text_wrapper_examples`, and
  `tests/api_parity/phase_d_text_wrapper_public_examples_test.cpp`,
  demonstrating label and TextElement wrappers, TextModel-backed public text
  elements, measurement/wrapping/glyph paint metadata, TextMeasurementCache
  use, layout sizing, and accessibility text through public headers.
- Richer official input workflows, rich-text examples, and final Phase D
  verification remain separate later steps.

## 2026-07-09 Phase D Step 445 Text Input Workflow Public Example

- Phase D Step 445 text input workflow public example should remain a public
  workflow example, not a new text model feature or a private history hook.
- Step 445 extends the text-input examples band with
  `examples/api_parity/public_text_input_workflow/main.cpp`,
  `api_parity_public_text_input_workflow`, and
  `tests/api_parity/phase_d_text_input_workflow_examples_test.cpp`,
  demonstrating undo/redo and redo invalidation, edit-history clean markers,
  line/word navigation and composition cancellation, selection text,
  backspace/delete-forward, TextEditAction dispatch, and history diagnostics
  through public headers.
- Rich-text examples, final examples closeout, and final Phase D verification
  remain separate later steps.

## 2026-07-09 Phase D Step 446 Rich Text Public Example

- Phase D Step 446 rich text public example should demonstrate the public
  metadata surface only. It must not imply that the renderer already performs
  multi-color glyph drawing or inline image drawing.
- Step 446 extends the text-input examples band with
  `examples/api_parity/public_rich_text_examples/main.cpp`,
  `api_parity_public_rich_text_examples`, and
  `tests/api_parity/phase_d_rich_text_public_examples_test.cpp`,
  demonstrating rich-text run, syntax, inline-image, hit, activation, and paint metadata
  through public headers.
- renderer glyph coloring and inline image drawing remain later work, alongside
  final examples closeout and final Phase D verification.

## 2026-07-09 Phase D Guarded HarfBuzz Backend

- Phase D guarded HarfBuzz backend should be a real guarded implementation, not
  the old `#error` insertion point. The default build still has no new
  dependency requirement and preserves deterministic fallback.
- The guarded HarfBuzz backend now shapes through hb_shape in
  `src/ui/text_shaping_harfbuzz.cpp` when
  `CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND` is enabled, using file-backed font faces when available and deterministic fallback on shaping failure.
  `tests/api_parity/phase_d_harfbuzz_backend_audit_test.cpp` freezes this
  guarded source boundary.
- DirectWrite font-file extraction remains later work, along with native ZWJ
  ligature shaping depth, full Unicode script data, bidirectional shaping, and
  paragraph shaping.

## 2026-07-09 Phase D Fontconfig Package Wiring

- Phase D font discovery now has system-optional fontconfig package wiring in
  `xmake.lua`. On Linux, the Wayland platform target defines
  `CGPUI_HAS_FONTCONFIG_DISCOVERY_BACKEND` and links `fontconfig` only when the
  system package is available.
- This enables the existing `FcFontList` native enumeration path without
  turning missing development headers into a hard build failure.
- FreeType metrics extraction and richer per-face coverage remain later work.

## 2026-07-09 Phase D Text Examples Closeout

- Phase D text examples closeout should freeze examples, not add new runtime
  text behavior.
- Step 450 closes the text examples band with
  `tests/api_parity/phase_d_text_examples_closeout_test.cpp`, covering the
  public-only prelude examples `api_parity_public_text_input_examples`,
  `api_parity_public_text_wrapper_examples`,
  `api_parity_public_text_input_workflow`, and
  `api_parity_public_rich_text_examples`.
- renderer glyph coloring and inline image drawing remain later work.

## 2026-07-09 Phase D Final Closeout

- Phase D final closeout should freeze verified text/IME/rich-text/example
  evidence and hand the roadmap to Phase E without hiding remaining production
  text gaps.
- Steps 451-458 close with
  `tests/api_parity/phase_d_final_closeout_test.cpp`, Windows full debug suite
  passed 139/139, and WSL Arch Linux full debug suite passed 136/136 using
  D-drive WSL build/cache output plus `/dev/shm/cgpui` transient temp.
- Remaining Phase D text gaps stay explicit for later work: DirectWrite
  font-file extraction, real color glyph rendering, native ZWJ ligature shaping depth,
  full Unicode script data, full Unicode bidirectional
  shaping/reordering, paragraph shaping, cache eviction policy,
  platform-derived font metrics, Unicode line-break classes, FreeType metrics
  and richer per-face coverage, platform-specific preedit styling, production
  candidate UI policy, runtime rich-text element dispatch, actual link command
  handling, inline image drawing/loading, syntax parsing/editor token source
  integration, and actual multi-color glyph painting.
- Phase E Step 459 Vulkan glyph atlas production follows this closeout.

## 2026-07-10 Phase E Step 459 Vulkan Glyph Atlas Production

- Phase E Step 459 Vulkan glyph atlas production should start with a focused
  renderer resource-planning leaf, not by putting Vulkan handle ownership into
  the public aggregate or shader pipeline work.
- `include/cgpui/renderer/glyph_atlas_production.hpp` exposes
  `GlyphAtlasProductionResourceState`, resource records, upload commands, and
  `GlyphAtlasProductionPlan` without exposing Vk handles.
- `src/renderer/vulkan/vulkan_glyph_atlas_production.cpp` implements
  `vulkan_plan_glyph_atlas_production_resources(...)`, converting glyph atlas
  upload batches into alpha8 atlas page image readiness, memory allocation and bind readiness, image-view and sampler readiness, and dirty upload command path readiness.
- descriptor set binding remains Step 460, along with private Vulkan
  renderer-state handle ownership and command-buffer recording.
## 2026-07-10 Phase E Step 512 Resume

- The guessed `src/platform/window_runtime_scheduling.cpp` and
  `tests/platform/window_runtime_scheduling_test.cpp` paths do not exist. The
  existing scheduling coverage is
  `tests/ui/window_runtime_scheduling_test.cpp`; follow its production symbols
  before deciding whether Step 512 belongs to swapchain present policy or the
  window scheduler.
- `session-catchup.py` cannot be launched directly on this Windows host. It
  must run through the configured Python interpreter.
- The production frame path is `WindowRuntime::schedule_redraw()` -> platform
  `request_redraw()` -> `WindowRuntime::try_draw_frame()` -> `render_view()` ->
  `VulkanRendererState::present_frame()`.
- Window scheduling currently coalesces invalidation into one outstanding
  redraw. Vulkan pacing is implicit: prefer MAILBOX, fall back to FIFO, request
  `minImageCount + 1` images within the surface maximum, and wait on one global
  in-flight fence before every frame.
- Split the combined Steps 512-514 band by ownership: Step 512 freezes a
  focused Vulkan present-mode/image-count/in-flight pacing policy; Step 513
  integrates next-frame scheduling for invalidation raised while rendering;
  Step 514 closes the batching/scheduling band with structure and end-to-end
  evidence.
- Keep Step 512 private to `src/renderer/vulkan`: a
  `VulkanPresentPacingPlan` should centralize MAILBOX/FIFO selection,
  saturation-safe `minImageCount + 1` depth clamped by `maxImageCount`, one CPU
  frame in flight, and the fence/acquire wait timeouts. Swapchain creation and
  `present_frame()` should consume that plan instead of duplicating constants.
- The current parity documents still explicitly say production frame pacing is
  incomplete, so Step 512 must update the roadmap and Markdown/JSON ledger but
  should not claim the full gap closed until Step 514 integration closeout.
- Phase E Step 512 adds a focused Vulkan present pacing policy: MAILBOX with FIFO fallback, saturation-safe swapchain image depth, and one CPU frame in flight via shared fence/acquire waits. Step 513 next-frame scheduling is next.
- The pure policy leaf contains no Vulkan operations; the focused
  `vulkan_present_pacing_wait.cpp` execution leaf owns `vkWaitForFences`, keeping
  `vulkan_presentation.cpp` below its existing orchestration limit.
## 2026-07-10 Phase E Step 513 Next-Frame Scheduling

- `WindowRuntime::schedule_redraw()` currently returns immediately while
  `redraw_scheduled_` is true. During `try_draw_frame()`, that flag stays true
  until after presentation, so invalidation requested by render callbacks is
  silently dropped and the later `clear_invalidation()` erases its flags.
- The focused ownership boundary is a new private
  `src/ui/runtime_frame_scheduling.cpp`, with begin/complete/abort helpers and
  two runtime booleans for active rendering and one coalesced next-frame request.
  `runtime_scheduling.cpp` only marks the next frame while rendering, and
  `runtime_renderer_frame_results.cpp` only brackets frame work with the helper.
- Complete scheduling after the `after_frame` callback so render,
  after-render, and after-frame invalidation all survive the current frame and
  coalesce into one platform redraw. The fake platform dispatches redraws
  synchronously, making the focused test an end-to-end two-frame gate.
- Phase E Step 513 adds next-frame scheduling: render-time invalidation survives frame completion and repeated requests are coalesced into exactly one platform redraw. Step 514 batching and scheduling closeout is next.
## 2026-07-10 Phase E Step 514 Batching/Scheduling Closeout

- Step 514 should be audit-only. Steps 507-513 already own the production
  behavior for reusable geometry buffers, exact command reuse, adjacent pipeline
  batching, upload barrier waves, swapchain recovery, Vulkan present pacing,
  and root-window next-frame scheduling.
- The closeout target should freeze each focused module and behavior target,
  require the existing renderer/UI structure gates, and move the roadmap to
  Step 515 renderer diagnostics without adding another runtime abstraction.
- Phase E Step 514 closes the batching and frame scheduling integration closeout for Steps 507-513, freezing reusable geometry buffers through next-frame scheduling. Step 515 renderer diagnostics is next.

## 2026-07-10 Phase E Step 515 Renderer Diagnostics Resume

- The roadmap defines one eight-step renderer diagnostics band, Steps 515-522,
  comparing planned work with submitted GPU work and explicitly naming upload
  bytes, draw counts, dropped resources, and frame timing.
- Existing public ownership is already split across
  `renderer_submission_reports.hpp`, `renderer_frame_reports.hpp`, and focused
  text/image/geometry report leaves. Step 515 should extend a focused report
  boundary instead of adding diagnostics bodies to `renderer.hpp`,
  `renderer_frame.hpp`, or a broad Vulkan renderer entry file.
- Existing Vulkan report implementation is similarly modular under
  `vulkan_report_*.cpp`; the next source pass must determine which planned and
  submitted values already exist and which frame-state snapshot owns the
  comparison.
- `RendererFrameReport` is a CPU planning/reporting surface: it derives command,
  batch, glyph-upload, image-upload, and gap counts before GPU submission. It
  does not represent queue-submit results.
- `VulkanRendererState` currently retains resource objects and
  `last_command_batches_`, but exposes no submitted-frame diagnostic snapshot.
  A new comparison model should remain independent from the existing planning
  report so later steps can populate actual submission counters incrementally.
- Step 515 boundary decision: add a focused public renderer diagnostics leaf
  with plain planned/submitted work records plus a non-template comparison
  function in a focused `.cpp`. Do not add live Vulkan state wiring, byte
  accounting, draw accounting, dropped-resource accounting, timing, or runtime
  propagation until their assigned Steps 516-521.
- `RenderFrame` currently owns recording plus `present()`, while `Renderer`
  owns resize and `begin_frame()` only. Neither interface exposes diagnostics;
  adding a virtual retrieval method in Step 515 would force every renderer and
  test double to change before the Vulkan snapshot exists.
- UI `FrameStatistics` and `RuntimeDiagnostics` already own runtime-facing
  frame counters, but they are a downstream consumer rather than the renderer
  comparison model. Their timing fields are still initialized without a real
  renderer timing source, which belongs to Step 519/521.
- The new public leaf should be included by thin `renderer_reports.hpp` so
  `renderer.hpp` remains only an aggregate. The non-template comparison body
  belongs in `src/renderer/renderer_frame_diagnostics.cpp`.
- Step 515 baseline data will compare command and batch counts. The result must
  report pending planned work and unexpected submitted work separately with
  saturation-safe subtraction, so under-submission and over-submission are both
  observable without signed-count conversions.
- Behavior coverage belongs in a platform-neutral
  `tests/renderer/renderer_frame_diagnostics_test.cpp` target depending on
  `cgpui_renderer`. Structure coverage will require the new leaf/source,
  aggregate inclusion, non-inline implementation ownership, and focused line
  limits in `renderer_source_structure_test`.
- The existing UI runtime diagnostics path confirms the later Step 521 adapter
  can add renderer diagnostics to `FrameStatistics`/snapshot propagation after
  `render_view(...)`; Step 515 does not change that path.
- The Step 515 implementation is behavior-green before documentation: exact,
  under-submitted, over-submitted, and empty work comparisons pass, and the
  architecture gate accepts the focused header/source ownership. The remaining
  test exit is the intentional five-document gate.
- Phase E Step 515 adds `RendererFrameWork` and `RendererFrameDiagnostics` to
  compare planned and submitted renderer work across command and batch counts
  with saturation-safe pending and unexpected counts.
  Step 516 upload-byte accounting is next.
- Step 515 final Windows verification is 196/196 after a complete debug build.
  WSL still has no installed distribution, so the platform-neutral API is
  compile-verified only on Windows in this slice and remains covered by the
  final Phase E Linux gate.

## 2026-07-10 Phase E Step 516 Upload-Byte Accounting

- Glyph and image upload batches already carry explicit per-region
  `byte_size`. Those region sizes represent planned buffer-to-image payload;
  glyph staging's 4-byte buffer-offset padding is allocation detail and should
  not be counted as submitted upload content.
- Step 516 will add `upload_byte_count` to `RendererFrameWork`, matching
  pending/unexpected byte deltas to `RendererFrameDiagnostics`, and a focused
  `RendererUploadByteCounts` accumulator split into glyph-atlas, image, and
  total bytes.
- The accumulator must use saturation-safe `size_t` addition and expose whether
  saturation occurred. Counting region metadata permits deterministic overflow
  tests without allocating impossible payload vectors.
- Implementation ownership stays in a new focused
  `src/renderer/renderer_frame_upload_diagnostics.cpp`; the existing comparison
  source only gains the new delta dimension. Vulkan state integration remains
  Step 520.
- Phase E Step 516 adds `RendererUploadByteCounts` for
  glyph and image upload payload bytes, saturation-safe upload-byte accounting, and
  pending/unexpected upload-byte comparison.
  Step 517 draw-count accounting is next.
- Step 516 final Windows verification is 197/197 after a complete debug build.
  The public leaf remains backend-neutral; WSL/Linux verification remains in
  the final Phase E gate because no distribution is installed.

## 2026-07-10 Phase E Step 517 Draw-Count Accounting

- Vulkan frame recording emits one draw call per resolved frame resource, not
  per renderer batch. Solid/rounded/image resources each resolve once, while a
  text command can resolve multiple `VulkanTextDrawCommand` page runs and emit
  multiple draw calls.
- Step 517 must therefore keep `draw_count` separate from command and batch
  counts in `RendererFrameWork` and expose primitive-aware counters rather than
  deriving draw count from existing report totals.
- The backend-neutral model should cover every public `RendererPrimitiveKind`
  and use an explicit saturation-safe add function. Step 520 can call it while
  producing the live Vulkan snapshot from resolved submissions.
- Implementation ownership will use a focused
  `src/renderer/renderer_frame_draw_diagnostics.cpp`; Step 517 will not modify
  the private Vulkan recorder yet.
- `VulkanFrame` records all six public primitive kinds in authored draw order,
  but the current resolved draw cursor submits only solid, rounded, text, and
  image resources. Selection/caret therefore remain valid planned draw kinds;
  Step 518 can classify their missing submitted resources explicitly rather
  than hiding them from Step 517 counters.
- Phase E Step 517 adds `RendererDrawCounts` for primitive-aware GPU draw counts,
  saturation-safe draw-count accounting, and pending/unexpected draw
  comparison. Step 518 dropped-resource accounting is next.
- Step 517 final Windows verification is 198/198 after a complete debug build.
  The public diagnostics boundary remains backend-neutral; WSL/Linux
  verification remains in the final Phase E gate because no distribution is
  installed.

## 2026-07-10 Phase E Step 518 Dropped-Resource Accounting

- Dropped-resource identity must distinguish authored command index from the
  resolved resource index because one text command can expand into multiple
  page-run draw resources.
- The comparison contract should consume ordered planned and submitted
  `RendererFrameResource` spans. Submitted work is an ordered subsequence of
  planned work; unexpected submitted records may appear before a later exact
  match without hiding that match.
- Backend support is explicit input rather than a hard-coded Vulkan policy.
  Missing unsupported kinds classify as `unsupported_primitive`; missing
  supported kinds classify as `missing_submission_resource`.
- Ownership belongs in a new focused public leaf and `.cpp`, with the existing
  `renderer_reports.hpp` remaining a thin compatibility aggregate. Live Vulkan
  snapshot population remains Step 520.
- Phase E Step 518 adds `RendererDroppedResourceDiagnostics` for ordered planned-resource submission gaps, classifying unsupported and missing submission resources while preserving command/resource identity. Step 519 frame-timing diagnostics is next.
- Focused Step 518 validation confirms exact resources stay clean, unsupported
  selection/caret resources remain visible, missing supported text page runs
  preserve their resource identity, and unexpected submitted records do not
  hide later stable-order matches.
- Step 518 final Windows verification is 199/199 after a complete debug build.
  WSL/Linux verification remains in the final Phase E gate because no
  distribution is installed.

## 2026-07-10 Phase E Step 519 Frame-Timing Diagnostics

- The production Vulkan CPU path exposes six stable timing boundaries: pacing
  wait, resource preparation, image acquisition, command recording, queue
  submission, and presentation.
- The public diagnostics helper should accept already-measured nanoseconds and
  never query a clock. This keeps timing deterministic in tests and leaves
  backend instrumentation ownership to Step 520.
- Fixed-width `std::uint64_t` nanoseconds plus saturation-safe stage/total
  accumulation avoid duration-type conversions and signed overflow in the
  diagnostics surface.
- Frame-budget comparison needs independent remaining and over-budget deltas;
  equality is within budget with both deltas zero.
- Phase E Step 519 adds `RendererFrameTimingDiagnostics` for explicit CPU frame-stage nanoseconds, saturation-safe timing accumulation, and frame-budget comparison. Step 520 live Vulkan diagnostic snapshots are next.
- Focused Step 519 validation confirms all six stage counters contribute to
  the total, zero additions are no-ops, overflow saturates both the stage and
  total, and budget equality remains within budget.
- Step 519 final Windows verification is 200/200 after a complete debug build.
  WSL/Linux verification remains in the final Phase E gate because no
  distribution is installed.

## 2026-07-10 Phase E Step 520 Live Vulkan Diagnostic Snapshots

- A successful command recording already proves solid/rounded buffer ranges,
  glyph page bindings, and image draw resources are usable. The diagnostic
  resource builder can consume those prepared records after recording without
  duplicating Vulkan command planning.
- Planned text resources are page runs, while selection/caret each remain one
  authored resource. The current submitted set intentionally omits
  selection/caret, so the live snapshot exposes those drops instead of hiding
  them in command totals.
- `Renderer::last_frame_diagnostic_snapshot()` returns a state-owned const
  pointer. This avoids copying the dropped-resource vector and lets Step 521
  read the result immediately after a successful present.
- Queue submission moved into `vulkan_frame_submission.cpp`, keeping
  `vulkan_presentation.cpp` under its existing structure limit while the timer
  records all six Step 519 stages.
- Phase E Step 520 adds `RendererFrameDiagnosticSnapshot` for live Vulkan planned and submitted work, including upload bytes, draw counts, dropped selection and caret resources, and CPU stage timings. Step 521 runtime diagnostic propagation is next.
- Focused validation confirms six authored commands with two text page runs
  produce seven planned resources, five submitted resources, four submitted
  commands/batches, and two explicit selection/caret drops. A real Win32
  present exposes the same state-owned snapshot contract through `Renderer`.
- The Step 462 glyph-atlas lifecycle structure test encoded fence reset and
  queue submission as presentation-file responsibilities. Step 520's focused
  submission module requires that contract to check the public call order in
  presentation and the Vulkan operation order inside the submission leaf.
- Step 520 final Windows verification is 201/201 after a complete debug build.
  WSL/Linux verification remains in the final Phase E gate because no
  distribution is installed.
- The compatible base-renderer diagnostic default is a public non-template
  implementation and therefore belongs in a focused `.cpp`, not inline in
  `renderer_frame.hpp`. The existing `renderer.hpp` compatibility aggregate is
  required when compiling that implementation because the leaf still relies on
  report and geometry declarations supplied earlier by the aggregate.

## 2026-07-10 Phase E Step 521 Runtime Renderer Diagnostics

- Storing the complete renderer snapshot directly in `FrameStatistics` would
  copy the dropped-resource vector into both the render record and the runtime's
  last-frame statistics on every frame. `RendererFrameStatistics` therefore
  keeps only fixed-size work, upload, draw, drop-count, and timing summaries.
- `WindowRuntime` retains one complete `RendererFrameDiagnosticSnapshot` after
  a successful frame so `RuntimeDiagnosticsSnapshot` still preserves dropped
  resource identity and reason metadata on demand.
- `render_view(...)` applies renderer statistics only after `present()`
  succeeds. The total renderer CPU nanoseconds populate `frame_time_ms`; the
  existing render/layout/paint timing fields remain zero until their own CPU
  instrumentation exists.
- The UI path consumes only `Renderer::last_frame_diagnostic_snapshot()` and
  has structure guards against `dynamic_cast` and Vulkan renderer type names.
- The runtime diagnostic state lives in a two-line private class fragment, so
  `window_runtime_internal.hpp` remains at its 260-line limit.
- Phase E Step 521 adds `RendererFrameStatistics` to propagate fixed-size renderer work, upload, draw, dropped-resource, and timing summaries into `FrameStatistics` after successful renderer presentation, while `RuntimeDiagnosticsSnapshot` preserves the full renderer snapshot without Vulkan downcasts. Step 522 renderer diagnostics closeout is next.

## 2026-07-10 Phase E Step 522 Renderer Diagnostics Closeout

- Step 522 is an audit-only integration closeout over Steps 515-521. It should
  add no runtime implementation and should follow the existing
  `phase_e_*_integration_closeout_test.cpp` ownership pattern.
- The focused closeout target should freeze planned/submitted work comparison,
  upload byte accounting, primitive draw accounting, dropped-resource identity
  and reason metadata, frame-stage timing, live renderer snapshots, and runtime
  propagation after successful presentation without Vulkan downcasts.
- The closeout must also lock the seven focused behavior targets plus renderer
  and UI structure coverage before handing Phase E to Step 523 pixel/screenshot
  testing.
- Phase E Step 522 closes the renderer diagnostics integration closeout for Steps 515-521, freezing work through runtime propagation evidence across planned/submitted work, upload bytes, draw counts, dropped resources, frame timing, live snapshots, and runtime summaries. Step 523 pixel/screenshot testing is next.

## 2026-07-10 Phase E Step 523 Vulkan Frame Capture Foundation

- The repository has no existing framebuffer readback, screenshot, golden-image,
  or pixel-comparison facility. Current Vulkan tests stop at command/resource
  recording and live presentation state.
- The eight-step pixel band maps cleanly to one capture foundation followed by
  text, rounded rectangle, image, clip, transform, opacity, and resize output
  cases in Steps 524-530.
- Step 523 must capture pixels produced by the actual renderer path. A separate
  CPU reference rasterizer or duplicated test-only pipeline would not prove the
  production Vulkan output.
- The capture boundary should remain backend-neutral at the public renderer
  surface while Vulkan owns swapchain transfer-source support, image layout
  transitions, staging/readback memory, and row-normalized RGBA8 output.
- Normal frames should pay no readback synchronization cost. Capture is an
  explicit per-frame request; only that frame allocates/ensures a host-visible
  transfer-destination buffer, records the post-render-pass copy, waits for the
  submitted fence, and maps the completed bytes.
- The swapchain image must advertise `VK_IMAGE_USAGE_TRANSFER_SRC_BIT` when the
  surface capability supports it. Capture requests must report unsupported
  capability explicitly instead of silently returning blank pixels.
- The command buffer must transition the acquired image from
  `VK_IMAGE_LAYOUT_PRESENT_SRC_KHR` to transfer source after the render pass,
  copy tightly packed pixels, and restore present layout before submission.
- Swapchain formats may be BGRA or RGBA. The public snapshot should normalize
  channel order to tightly packed RGBA8 while recording whether the bytes use
  sRGB or linear encoding.
- The public request belongs to `RenderFrame`, not the renderer-wide next-frame
  state: `request_pixel_capture()` scopes intent to exactly the frame that will
  be presented. `Renderer::last_frame_pixels()` exposes the latest successful
  capture afterward.
- Both virtual additions need compatible out-of-line defaults. Non-Vulkan test
  renderers should compile unchanged; the default frame request returns
  `unsupported_platform`, and the default renderer snapshot pointer is null.
- `RendererFramePixels` should expose width, height, linear/sRGB encoding,
  tightly packed RGBA8 storage, validity, row byte count, and bounds-checked
  pixel lookup. Its non-template bodies belong in a focused `.cpp`.
- `vulkan_presentation.cpp` is 164 lines under a 165-line structure limit.
  Capture resource preparation, command emission, fence completion, mapping,
  and channel normalization must live in focused modules; presentation should
  remain orchestration-only.
- A requested capture must force command recording even when the ordinary frame
  signature matches a reusable command buffer. Reusing a buffer recorded before
  capture would omit the image-to-buffer copy.
- Step 523 needs two coverage layers: a platform-neutral public snapshot/default
  behavior test and a real Win32 Vulkan frame-capture test. Later pixel steps can
  reuse the live capture fixture while Linux/Wayland receives its active-host
  counterpart in this band.
- Phase E Step 523 adds `RendererFramePixels` and explicit per-frame capture with optional Vulkan swapchain transfer-source readback, present-layout restoration, and normalized RGBA8 output. Step 524 text pixel coverage is next.

## 2026-07-11 Phase E Steps 524-530 Pixel Output Coverage

- Deterministic fallback glyph rasterization produces a solid coverage rectangle
  with one-pixel padding, so text output can assert both foreground pixels and
  untouched clear pixels without native font variability.
- Rounded rectangles, nearest-sampled 2x2 images, explicit clip rectangles,
  translation metadata, precomposed opacity, and post-resize clear frames each
  provide stable sample coordinates over the shared capture fixture.
- The current Wayland Vulkan smoke target requires a real `WAYLAND_DISPLAY` and
  explicitly skips otherwise. The pixel band should add an equivalent Wayland
  capture target using the same public API; the final Linux gate must run it
  when a distribution/display is available rather than treating a skip as proof.
- Seven focused pixel files are preferable to one broad scenario file. They can
  share window/capture helpers while keeping each Step 524-530 output contract
  independently named and diagnosable.
- `Renderer::last_frame_pixels()` exposes renderer-owned state. Multi-frame
  tests that need to compare captures must copy the first snapshot before the
  next successful capture replaces that state.
- The live image quadrants proved the renderer-wide Vulkan Y mapping was
  inverted: each vertex shader emitted OpenGL-style `1 - normalized.y * 2`
  while draw recording installed a positive-height Vulkan viewport. Correct
  top-left coordinates require `normalized.y * 2 - 1` for all three pipelines.
- The shared Windows helper crossed its existing structure cap once resize and
  message-pump behavior landed. Window lifecycle/resize belongs in
  `vulkan_pixel_test_window.hpp`; capture and pixel comparison remain in the
  thinner `vulkan_pixel_test_support.hpp` compatibility helper.
- Phase E Step 524 adds real Vulkan text pixel coverage for deterministic fallback glyph foreground and clear-background output in authored top-left coordinates. Step 525 rounded rectangle pixel coverage is next.
- Phase E Step 525 adds real Vulkan rounded rectangle pixel coverage for filled centers and anti-aliased rounded corners. Step 526 image pixel coverage is next.
- Phase E Step 526 adds real Vulkan nearest-image pixel coverage for top-left RGBA quadrant orientation and corrects all text, rounded, and image vertex shaders to positive-viewport Vulkan top-left Y mapping with validated embedded SPIR-V. Step 527 clip pixel coverage is next.
- Phase E Step 527 adds real Vulkan clip pixel coverage for inside, horizontal-outside, and vertical-outside samples. Step 528 transform pixel coverage is next.
- Phase E Step 528 adds real Vulkan transform pixel coverage for translated output and untouched original/distant coordinates. Step 529 opacity pixel coverage is next.
- Phase E Step 529 adds real Vulkan opacity pixel coverage for encoding-aware half-red composition over opaque black. Step 530 resize pixel coverage is next.
- Phase E Step 530 adds persistent-renderer resize pixel coverage across exact 64x64 and 96x48 Win32 client extents, plus an active-display Wayland capture target using the same public API. Step 531 pixel-band closeout is next.
- Phase E Step 531 closes the pixel-output integration band for Steps 523-530, freezing backend-neutral capture, real Win32 Vulkan text/rounded/image/clip/transform/opacity/resize pixels, corrected top-left shader coordinates, and the active-display Wayland capture target. Step 532 Windows full verification is next.
- Phase E Step 532 confirms the committed pixel-output closeout on Windows: debug configuration/build succeeds and the complete suite passes 213/213, including all real Vulkan pixel targets. Step 533 WSL full verification is next.
- Phase E Step 533 confirms the committed pixel-output closeout on WSL Arch Linux: debug configuration/build succeeds and the complete suite passes 203/203, including a real `wayland_frame_pixel_capture_test/default` run on `WAYLAND_DISPLAY=wayland-0`. Step 534 production-path audit is next.
- Phase E Step 534 audits the production Vulkan path for required glyph, text, rounded-rectangle, clip/composition, image, SVG, batching/scheduling, diagnostics, and pixel-output modules; focused ownership remains intact, presentation/command-recording caps remain 165/180, and GCC 16 image row-length narrowing is resolved explicitly. Step 535 final closeout guard is next.
- Phase E Step 535 adds `phase_e_final_closeout_test` as the audit-only guard for Steps 459-534, freezing the required production modules, all Phase E integration closeouts, cross-platform verification evidence, structure caps, and the Phase F handoff. Step 536 ledger closeout is next.
- Phase E Step 536 closes the renderer parity ledger for required Phase E primitives and moves the active handoff to Phase F platform production depth without claiming later optional renderer refinements. Step 537 final Windows/WSL gate is next.
- Phase E Step 537 passes the final Windows and WSL gates: Windows full debug passes 214/214 and WSL Arch Linux passes 204/204, including active-display Wayland frame pixel capture, with JSON, structure, SPIR-V, line-count, phrase, and diff hygiene audits green. Step 538 Phase E final closeout is next.
- Phase E final closeout: Steps 531-538 close with `tests/api_parity/phase_e_final_closeout_test.cpp`; Windows full debug suite passes 214/214 and WSL Arch Linux full debug suite passes 204/204, including active-display Wayland pixel capture on `WAYLAND_DISPLAY=wayland-0`, using D-drive WSL build/cache output plus `/dev/shm/cgpui` transient temp. The required Windows/Linux Vulkan renderer production path is complete for Phase E primitives, and Phase F Step 539 window lifecycle production depth is next.

## 2026-07-11 Phase E Final Closeout

- WSLg exposes `WAYLAND_DISPLAY=wayland-0` with a live socket at
  `/run/user/0/wayland-0`; the Wayland pixel target therefore exercises real
  window, renderer, capture, present, and RGBA validation paths.
- GCC 16 diagnosed `VkBufferImageCopy::bufferRowLength` initialization from a
  `size_t` conditional expression. The validated upload stride is already
  bounded by `uint32_t`; an explicit conversion documents that invariant and
  removes the cross-compiler narrowing warning.
- Final Phase E ownership remains distributed across eight focused integration
  closeouts. The final guard audits them and the two broad Vulkan orchestration
  caps instead of moving implementation back into entry files.

## 2026-07-11 Phase F Entry Audit

- The authoritative Phase F scope is Steps 539-618 in ten eight-step bands:
  lifecycle, Win32 input, Wayland input, clipboard, drag/drop, native menus,
  dialogs/services, multi-window event loops, diagnostics/stress, and final
  Windows/WSL closeout.
- The platform layer already has focused Win32 window procedure, size, event,
  input, IME, and OLE drag/drop modules plus focused Wayland application,
  registry, configure, input, text-input, data-device, and window-state modules.
  Phase F should deepen these ownership boundaries rather than create a second
  platform stack or move behavior into broad entry files.
- Existing platform tests cover Win32 focus, DPI scale, pointer/keyboard/text
  input and Wayland compositor close, resize, keyboard, pointer button, and
  scroll behavior. Phase F completion needs production-state tests for the
  roadmap behaviors that these focused tests do not yet prove.
- The public platform surface remains thin: `PlatformWindow` currently exposes
  redraw, title, close, scale, and logical size, while window lifecycle depth is
  mostly private to the platform backends. Step 539 must first freeze a focused
  production lifecycle contract and its ownership before broadening behavior.
- The working tree entered Phase F on `master` at `ac814b5a` with only the
  pre-existing untracked `.vscode/` directory.
- Core event vocabulary already includes activated, focused, minimized, and
  restored events. Wayland also parses activated/maximized/fullscreen xdg
  states, while Win32 handles focus, size, DPI, and close messages. The missing
  cross-platform contract is an observable production lifecycle snapshot.
- Many tests and the macOS backend derive from `PlatformWindow`; a new pure
  virtual would create unrelated churn. Step 539 should add an out-of-line
  compatible default and override it only in the active Win32 and Wayland
  backends.
- Step 539 ownership is now frozen: public lifecycle data in
  `include/cgpui/platform/platform_window_lifecycle.hpp`, compatibility default
  in a focused `src/platform/platform_window_lifecycle.cpp`, and backend reads
  in new `win32_window_lifecycle.cpp` and `wayland_window_lifecycle.cpp` files.
  Later Steps 540-545 can deepen the same snapshot without relocating code.
- The lifecycle snapshot should carry native-created, initial-configured,
  active, focused, close-requested, and display-state fields. Step 539 proves
  creation/configure/close on real backends; later lifecycle steps own active,
  focus, minimize/maximize/fullscreen, position, chrome, and parent behavior.
- Real Wayland verification exposed an important ownership layer: callers hold
  `RegisteredWaylandWindow`, not the underlying `WaylandWindow`. New platform
  queries must be forwarded by that wrapper or they silently use compatible
  base defaults even when the underlying backend has production state.
- Phase F Step 539 adds a public window lifecycle snapshot with compatible defaults and real Win32/Wayland native-created, initial-configure, close-requested, and display-state reporting, including registered Wayland wrapper forwarding. Step 540 activation and focus production behavior is next.

## 2026-07-11 Phase F Step 540 Activation And Focus Audit

- Win32 currently emits `WindowFocused` for `WM_SETFOCUS`/`WM_KILLFOCUS` but
  does not handle `WM_ACTIVATE`, and its Step 539 snapshot queries Win32 global
  focus/activation functions. Synthetic lifecycle messages therefore cannot
  prove state-before-event ordering. Focused backend-owned booleans are needed.
- Wayland xdg configure already stores the current activated state before
  emitting `WindowActivated`. Keyboard enter/leave calls
  `WaylandWindow::focus_changed`, but that method only emits an event; adding a
  focused member closes the lifecycle snapshot gap without changing input
  ownership.
- Step 540 tests should read `lifecycle_state()` inside activation/focus event
  callbacks. This proves the backend updates observable state before dispatch,
  not merely that an event spelling exists.
- The existing Wayland test compositor can drive activated true/false configure
  states and keyboard enter/leave without adding new helper APIs.
- Phase F Step 540 makes activation and focus state-before-event observable on Win32 through `WM_ACTIVATE`/`WM_SETFOCUS`/`WM_KILLFOCUS` and on Wayland through xdg activated configures plus keyboard enter/leave, with lifecycle snapshots matching callback state. Step 541 resize and scale-change production behavior is next.

## 2026-07-11 Phase F Step 541 Resize And Scale Audit

- Win32 already updates `WindowState` before emitting `WindowResized` for both
  `WM_SIZE` and `WM_DPICHANGED`, but the existing DPI test only compares the
  event value after dispatch. Step 541 needs a callback-time state-order test
  covering both size and DPI transitions.
- Wayland xdg configure already acknowledges a real resize and stores the new
  size before `WindowResized`, but the backend fixes scale at `1.0F` and has no
  `wl_output` binding, `wl_surface.enter/leave` listener, output-scale update,
  or `wl_surface_set_buffer_scale` path.
- Wayland xdg toplevel sizes are logical surface dimensions. Production scale
  handling must retain logical size separately, derive framebuffer pixels as
  logical size times the active integer output scale, and emit resize state
  only after both values are updated.
- The modular ownership boundary is a private output-scale registry owned by
  `WaylandApplication`, a focused window surface-scale module owned by
  `WaylandWindow`, and thin registry/window-creation wiring. The broad
  application and window entry files should not absorb the new behavior.
- Multi-output behavior should choose the maximum scale of entered outputs,
  apply it with `wl_surface_set_buffer_scale`, and react to dynamic
  `wl_output.scale` changes through an application-to-window notification.
- Phase F Step 541 makes resize and scale state-before-event observable on Win32, and adds production Wayland `wl_output` scale tracking, surface enter/leave handling, buffer-scale updates, logical-to-framebuffer conversion, and real dynamic scale/resize coverage. Step 542 close policy production behavior is next.

## 2026-07-11 Phase F Step 542 Close Policy Audit

- Win32 `WM_CLOSE`, Win32 programmatic `request_close()`, Wayland xdg close,
  and Wayland programmatic `request_close()` currently collapse into an empty
  `WindowCloseRequested{}` event. Callers cannot distinguish the source.
- Both backends set `WindowState::close_requested` before the callback, but the
  state cannot be cancelled or explicitly accepted and repeated requests are
  not coalesced through a shared policy contract.
- The root runtime invokes its existing close callback and then unconditionally
  quits. Additional windows are immediately cleaned up. A usable close policy
  needs context-level accept/cancel resolution while preserving default accept
  behavior for existing callbacks and applications.
- The ownership boundary is a thin public close snapshot/resolution leaf, a
  private shared close controller compiled in `cgpui_platform`, focused Win32
  and Wayland close implementation files, and a focused runtime context bridge.
  Broad platform entry files and event-routing files should only orchestrate.
- Phase F Step 542 adds source-aware, state-before-event close requests with shared pending/accept/cancel/coalescing policy on Win32 and Wayland, plus runtime callback cancellation with compatible default acceptance. Step 543 fullscreen and minimize/maximize production behavior is next.
- The expanded WSL gate reaches `ui_source_structure_test` return code 8, the
  required-source read loop. Windows-side enumeration confirms every listed
  path exists and is non-empty, including the new
  `src/ui/runtime_context_window_close.cpp`; the remaining diagnosis is the
  WSL test process source-root/copy view, not a missing repository file.

## 2026-07-11 Phase F Step 543 Display State Audit

- `PlatformWindowLifecycleState` already exposes normal, minimized, maximized,
  and fullscreen snapshots, but `PlatformWindow` has no command API for any of
  those transitions.
- Win32 currently derives minimized/maximized state from `IsIconic` and
  `IsZoomed`; Wayland parses maximized/fullscreen xdg configure states. Both are
  read-only observation paths today.
- The modular Step 543 boundary should add a compatible public display-state
  command leaf/default plus focused Win32 and Wayland command implementations.
  Existing window entry files should only forward or own compact state.
- Win32 fullscreen must not alias maximize. The production path needs to save
  windowed style/extended-style/placement, apply a monitor-sized borderless
  frame, expose fullscreen before the resulting resize callback, and restore
  the saved frame before a later normal/minimized/maximized request.
- The existing hand-written xdg-toplevel interface stops at opcode 2
  (`set_title`). Standard display requests require the complete request table
  through opcode 13 plus focused wrappers for set/unset maximized,
  set/unset fullscreen, and set minimized.
- Wayland has no minimized configure state. Tests should verify that minimize
  reaches the compositor, while maximized/fullscreen/normal lifecycle state is
  only asserted after matching compositor configures are acknowledged.
- Phase F Step 543 adds real Win32 minimize/maximize/restore and reversible borderless fullscreen, plus Wayland xdg-toplevel display requests with compositor-confirmed lifecycle state. Step 544 window positioning production behavior is next.

## 2026-07-11 Phase F Step 544 Window Positioning Audit

- No public window position query/request or initial descriptor position exists.
  Win32 creation always uses `CW_USEDEFAULT`; later `SetWindowPos` calls are
  currently limited to DPI, chrome, and fullscreen internals.
- Win32 can provide real top-level desktop positioning through
  `GetWindowRect`, `SetWindowPos`, and `WM_MOVE`.
- Wayland xdg-toplevel deliberately does not expose absolute desktop
  positioning. `xdg_positioner` is popup-only and xdg surface window geometry
  is surface-local, so neither may be presented as top-level placement.
- The public boundary must expose capability/optional position explicitly.
  Wayland should return unsupported/false, while Win32 should support initial
  descriptor placement plus later query/request behavior.
- Phase F Step 544 adds capability-aware top-level positioning with initial/query/request support and WindowMoved delivery on Win32, while Wayland explicitly reports absolute positioning unsupported. Step 545 transparent and decorated window production behavior is next.

## 2026-07-11 Phase F Step 545 Chrome Production Audit

- Step 206 deliberately left `xdg-decoration` negotiation, transparent
  swapchain/compositor behavior, frameless interaction, and native diagnostics
  as future production work.
- Win32 already applies `WS_OVERLAPPEDWINDOW` versus `WS_POPUP`, resizable
  style bits, and `WS_EX_LAYERED`, but has no focused real-backend behavior
  test for creation-time and live chrome transitions.
- Wayland currently reports the entire request unsupported and applies an
  opaque decorated default because the application does not bind
  `zxdg_decoration_manager_v1` or create a toplevel-decoration object.
- Vulkan swapchain selection always prefers `VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR`.
  A transparent window request must travel through `RenderSurfaceDescriptor`
  and prefer a supported non-opaque composite-alpha mode before opaque
  fallback; this belongs in the renderer surface/swapchain boundary, not the
  Wayland window implementation.
- The base `PlatformWindow::apply_window_chrome(...)` body still lives in the
  broad `src/platform/empty.cpp`; Step 545 should move it to a focused
  `src/platform/platform_window_chrome.cpp` implementation file.
- Phase F Step 545 adds real Win32 decorated/frameless/layered chrome transitions, Wayland xdg-decoration server/client-side negotiation with non-resizable size constraints, and transparent-aware Vulkan composite-alpha selection. Step 546 child-window ownership production behavior is next.

## 2026-07-11 Phase F Step 546 Child Window Ownership Audit

- App setup runs before `WindowRuntime::run(...)`, so app-opened windows cannot
  receive a real root native owner on the old immediate-creation path. Their
  runtime records must remain pending until root platform-window and renderer
  activation completes.
- `PlatformApplication::create_child_window(...)` is the compatibility
  boundary: generic/test applications may fall back to `create_window(...)`,
  while production backends own native parent semantics.
- Win32 top-level child windows must remain overlapped/popup windows and pass
  the root `HWND` as the `CreateWindowExW` owner, rather than using `WS_CHILD`.
- Wayland already declares the xdg-shell `set_parent` request in its interface
  table, but lacked the client request wrapper and parent toplevel plumbing.
  The request belongs before the child's initial `wl_surface_commit`.
- Phase F Step 546 adds root-owned runtime child windows with deferred activation after root creation, real Win32 owner HWNDs, and Wayland xdg-toplevel parent requests before first commit. Step 547 Win32 pointer input production behavior is next.

## 2026-07-11 Phase F Step 547 Win32 Pointer Button Audit

- The old Win32 pointer procedure only handled left/right/middle down and up,
  did not register the class for double-click messages, and discarded the
  public `PointerButton::click_count` added for text selection behavior.
- `WM_XBUTTON*` requires returning `TRUE` after handling and derives the
  back/forward identity from the high word of `wParam`; that message decoding
  belongs in a focused Win32 pointer-button module rather than expanding the
  window procedure.
- Phase F Step 547 adds real Win32 double-click counts and back/forward XBUTTON mapping through a focused pointer-button decoder. Step 548 Win32 wheel and high-precision scroll production behavior is next.

## 2026-07-11 Phase F Step 548 Win32 Pointer Scroll Audit

- The old Win32 path handled only `WM_MOUSEWHEEL`; horizontal wheel messages
  fell through to the default window procedure.
- Floating-point division already avoided integer truncation, but callers had
  no way to distinguish full `WHEEL_DELTA` notches from fractional deltas
  emitted by high-resolution devices.
- The modular boundary is a focused decoder that owns message/axis/raw-delta
  interpretation. The window event module retains HWND-specific screen-to-
  client coordinate conversion and public event publication.
- Phase F Step 548 adds real Win32 vertical and horizontal wheel routing with fractional high-precision deltas and explicit precision metadata through a focused pointer-scroll decoder. Step 549 Win32 keyboard production behavior is next.

## 2026-07-11 Phase F Step 549 Win32 Keyboard Key Audit

- The old procedure handled only `WM_KEYDOWN` and `WM_KEYUP`; system-key
  messages fell through and native scan/repeat/extended/context bits were lost.
- `KeyboardKey` can preserve this metadata with defaulted scalar fields without
  changing existing key-binding consumers that depend only on key code,
  action, and modifiers.
- The decoder owns native message interpretation and modifier snapshots. The
  main window procedure must carry `lParam`, while `Win32Window` only publishes
  the already-decoded public event.
- System-key events must still reach `DefWindowProc` after publication so
  native behaviors such as Alt+F4 remain available despite the new metadata.
- Phase F Step 549 adds real Win32 key and system-key decoding with scan-code, repeat, extended-key, and system-message metadata through a focused keyboard-key decoder. Step 550 Win32 dead-key production behavior is next.

## 2026-07-11 Phase F Step 550 Win32 Dead-Key Audit

- `WM_DEADCHAR` and `WM_SYSDEADCHAR` must update pending composition state
  without publishing a committed `TextInput` event.
- `WM_SYSDEADCHAR` still falls through to `DefWindowProc` after pending state is
  captured, preserving native system-character behavior.
- The next committed `WM_CHAR` can expose that it resulted from pending dead-key
  composition through compatible defaulted `TextInput::composed` metadata.
- Pending dead-key state belongs to the window text boundary and must be cleared
  on focus loss so a later unrelated character is not mislabeled as composed.
- The first Windows behavior verification passes 3/3 for dead-key behavior,
  existing Win32 text input, and ordinary/system keyboard-key regression.
- Phase F Step 550 adds explicit Win32 dead/system-dead character suppression with pending composition state, composed TextInput metadata, and focus-loss reset through focused dead-key and window-text modules. Step 551 Win32 text-input production behavior is next.

## 2026-07-11 Phase F Step 551 Win32 Text-Input Audit

- The current `WM_CHAR` path converts one UTF-16 code unit at a time, so it
  cannot preserve supplementary-plane characters delivered as surrogate pairs.
- Win32 production text input also needs explicit `WM_UNICHAR` support: return
  `TRUE` for `UNICODE_NOCHAR` capability probes and commit valid UTF-32 values.
- `WM_SYSCHAR` must not become ordinary text input, but it must clear pending
  dead-key and surrogate state before continuing to `DefWindowProc`.
- Partial surrogate state must be per-window, allocation-free, and reset on
  focus loss alongside pending dead-key composition state.
- Runtime `TextInput` routing inserts the payload without filtering, so Win32
  C0/DEL control `WM_CHAR` values must remain keyboard events rather than being
  published a second time as insertable text.
- Phase F Step 551 adds production Win32 text input with UTF-16 surrogate pairing, WM_UNICHAR negotiation and codepoint delivery, system-character suppression, and focus-loss state reset through focused text-input and window-procedure modules. Step 552 Win32 cursor theme and system cursor production behavior is next.

## 2026-07-11 Phase F Step 552 Win32 Cursor Audit

- The current Win32 helper maps only seven cursor shapes and keeps cursor
  resource ownership inside the mixed keyboard/cursor input-helper file.
- Standard system coverage is missing diagonal resize, all-direction resize,
  wait, app-starting/progress, help, and up-arrow cursors.
- Updating only the class cursor and current thread cursor is insufficient:
  client `WM_SETCURSOR` must reapply the active shape after Windows changes it.
- System cursor/theme changes must reload the active shared system cursor and
  update both the class cursor and current client cursor.
- Phase F Step 552 adds production Win32 system cursors with expanded public shapes, focused system-resource mapping, client WM_SETCURSOR reapplication, and settings/theme refresh. Step 553 Win32 pointer capture and drag production behavior is next.

## 2026-07-11 Phase F Step 553 Win32 Pointer-Capture Audit

- Runtime pointer capture currently changes only `pointer_capture_owner_`; it
  never asks the active `PlatformWindow` to capture native pointer messages.
- `PlatformWindow` has no pointer-capture contract, and the Win32 procedure has
  no `SetCapture`, owner-matched `ReleaseCapture`, `WM_CAPTURECHANGED`, or
  `WM_CANCELMODE` handling.
- Step 553's drag scope is in-window press/move/release continuity through
  native capture. OLE external drag/drop remains owned by Steps 571-578.
- The public platform boundary should use a focused leaf with compatible
  defaults so existing fake and fallback windows keep their logical behavior.
- Native capture loss needs an explicit platform event. Runtime must clear the
  logical capture owner and cancel pointer-down/drag state before consumers
  observe that event.
- Win32 capture ownership belongs in focused pointer-capture helpers and a
  dedicated window-procedure module, not the broad pointer or window sources.
- Intentional runtime release must suppress its synchronous
  `WM_CAPTURECHANGED` callback to avoid nested event dispatch; external owner
  replacement and `WM_CANCELMODE` remain observable loss events.
- Phase F Step 553 adds native Win32 pointer capture with compatible platform control, owner-matched release, capture-loss cancellation, and continuous outside-window drag movement. Step 554 Win32 input DPI-change production behavior is next.

## 2026-07-11 Phase F Step 554 Win32 Input DPI Audit

- Win32 `WM_DPICHANGED` already updates `WindowState::scale`, and runtime viewport
  dimensions are derived in logical pixels from framebuffer size and scale.
- Pointer move/button handlers still publish `GET_X/Y_LPARAM` client device
  pixels directly; wheel and OLE drag paths convert screen to client pixels but
  likewise omit division by the current scale.
- After a live DPI change, those physical coordinates no longer align with
  runtime layout, hit testing, pointer capture routing, or drag positions.
- Native client and screen positions should share one focused, allocation-free
  physical-to-logical Win32 helper. Scroll deltas remain dimensionless wheel
  units and must not be scaled.
- Synthetic drag test-hook payloads already express public logical positions
  and should not be transformed a second time.
- A focused `win32_input_coordinates` module can serve client LPARAM, screen
  LPARAM, and OLE POINTL inputs without making the general event or drag/drop
  files own DPI math.
- Moving pointer move/button/wheel publication into
  `win32_window_pointer_events.cpp` keeps the aggregate events file focused on
  keyboard, activation/focus, and synthetic drag orchestration.
- The live-DPI behavior test now passes for pointer move, button, and wheel;
  existing input/button/scroll behavior remains green, and native OLE drag
  reuses the same current-scale conversion while synthetic logical drag stays
  untouched.
- Phase F Step 554 makes Win32 pointer, button, wheel, and native drag coordinates DPI-aware through focused physical-to-logical conversion after live scale changes, closing the Win32 input band. Step 555 Wayland seat capability production behavior is next.

## 2026-07-11 Phase F Step 555 Wayland Seat Capability Audit

- Wayland production ownership lives under `src/platform/linux`, with the
  existing seat callback already extracted from the broad application source.
- Step 555 should audit capability removal and dependent input-state cleanup in
  addition to the already-present pointer/keyboard acquisition path; the
  public seat lifecycle should remain private to the Wayland backend.
- `handle_seat_capabilities` already acquires pointer/keyboard proxies and
  destroys them on removal, but keyboard removal nulls `keyboard_window_`
  without publishing the corresponding focus loss first. A window focused by
  keyboard enter can therefore retain stale lifecycle focus.
- Pointer removal already clears its target and pending scroll accumulator;
  Step 555 coverage should prove resource destruction, no delivery while the
  capability is absent, and clean proxy reacquisition when it returns.
- The test compositor currently sends pointer+keyboard capabilities only once
  during seat bind. It needs a focused runtime capability-change request and
  observable pointer/keyboard resource state for production behavior coverage.
- The focused `wayland_application_seat.cpp` owner and the core's single
  pointer/keyboard proxy fields are already the right module boundary; no
  public API or broad application-file change is needed.
- Server-side resource destruction callbacks already null the compositor's
  pointer/keyboard resource handles, so a test-facing wait predicate can
  observe client proxy teardown without adding production instrumentation.
- Step 555 behavior coverage should drive the live sequence
  pointer+keyboard -> none -> pointer+keyboard, requiring initial focus/input,
  removal-time blur and proxy release, then proxy reacquisition with renewed
  focus/pointer/key delivery.
- Test-side resource availability must be atomic because the compositor server
  thread owns Wayland resources while the test thread waits; server release
  handlers should destroy the resource so the bound state reflects protocol
  teardown rather than client disconnect only.
- The failed keyboard-only transition exposed a production leak: generated
  `wl_pointer_destroy` and `wl_keyboard_destroy` only destroy local proxies;
  they do not send protocol `release`. Capability removal must call the
  version-gated `wl_pointer_release`/`wl_keyboard_release`, falling back to
  local destroy only for protocol versions predating release requests.
- The final behavior path cleanly handles keyboard-only, no-input,
  pointer-only, and combined capability sets. Keyboard loss publishes blur
  before clearing the target, pointer loss clears target/position/serial/scroll
  state, and both proxies reacquire without stale server resources.
- Data-device binding remains available for any non-empty seat capability set,
  while text-input binding now follows keyboard availability so IME state is
  reset when keyboard capability disappears.
- Phase F Step 555 adds live Wayland seat capability transitions with version-aware pointer and keyboard release, removal-time focus loss, stale input-state cleanup, and proxy reacquisition. Step 556 Wayland keyboard layout and modifier production behavior is next.

## 2026-07-11 Phase F Step 556 Wayland Keyboard Layout/Modifier Audit

- Production already compiles compositor-provided XKB keymaps, updates state
  with depressed/latched/locked masks plus layout group, exposes effective
  Shift/Control/Alt/Super modifiers, and derives key text through XKB.
- Existing real Wayland coverage only drives depressed Shift in layout Group 0.
  It does not prove latched or locked masks, layout-group switching, combined
  modifier snapshots, or group-aware UTF-8 text generation.
- Step 556 can stay within the focused keyboard/test-compositor boundary by
  adding a second keymap group and a raw mask/group request alongside the
  existing boolean convenience request; no public application API is needed.
- Successful runtime keymap replacement currently creates a fresh `xkb_state`
  and loses the active depressed/latched/locked masks plus layout group until a
  later modifiers event arrives. The private keyboard state should retain
  those four values and replay them immediately onto a newly compiled keymap.
- A real compositor test can expose this by selecting Group 1 with combined
  depressed/latched/locked modifiers, verifying text/modifier snapshots,
  re-sending the keymap, then pressing the same key without another modifiers
  event. The post-reload result must remain identical.
- Effective Control intentionally transforms `Q` into control byte `0x11` in
  XKB UTF-8 output. The final test therefore proves readable `Q` with Shift and
  Group 1 first, then compares the combined-mask text byte-for-byte before and
  after reload while checking modifier snapshots independently.
- The private keyboard state now stores all four compositor values even when no
  XKB state exists, replays them immediately after successful keymap compile,
  preserves the prior live keymap on compile failure, and clears them only on
  full keyboard reset/capability loss.
- Phase F Step 556 preserves depressed, latched, and locked Wayland modifiers plus the active layout group across XKB keymap reloads, with real multi-layout text and modifier coverage. Step 557 Wayland pointer enter, leave, and motion production behavior is next.

## 2026-07-11 Phase F Step 557 Wayland Pointer Enter/Leave/Motion Audit

- The public pointer surface currently has move/button/scroll/capture only.
  Wayland enter records target/position/cursor without publishing the initial
  coordinates, and leave silently clears target/scroll without informing the
  runtime, so hover and cursor state can remain stale indefinitely.
- The narrow adaptation is to publish the enter coordinates as the existing
  `PointerMoved` event and add one explicit `PointerExited` event carrying the
  last position. A separate `PointerEntered` type would duplicate the initial
  move and add broader public/runtime complexity without new information.
- Wayland leave should clear target, enter serial, and pending scroll before
  publishing exit to the captured window pointer, preserving state-before-event
  semantics and preventing cursor updates from using a stale enter serial.
- Runtime input handling must recognize exit position for routing/records but
  clear the hovered element and restore the default cursor instead of hit
  testing it as another move.
- The resumed production path passes the real WSL compositor test: pointer
  enter publishes the initial coordinates immediately, motion publishes the
  updated coordinates, and leave publishes `PointerExited` with the last
  position.
- Existing source guards already own the relevant focused boundaries. Step 557
  should extend them in place for `pointer_exited`, target/serial/scroll
  cleanup, and runtime hover/default-cursor clearing, while a dedicated
  structure guard locks both focused behavior tests and authority handoff.
- Current touched-file caps remain within the established limits:
  `wayland_application_pointer.cpp` 77/80, `runtime_event_input.cpp` 153/160,
  `ui_event_pointer_internal.hpp` 84/90, and
  `ui_event_kind_internal.hpp` exactly 120/120 after compaction.
- The first synchronized Windows gate exposed one additional historical cap:
  `wayland_window_internal.hpp` reached 121/120 solely from the adjacent exit
  declaration. Removing an unnecessary separator before the registered-window
  include retains the 120-line cap and keeps both declarations readable in the
  same focused private window boundary.
- Phase F Step 557 publishes Wayland pointer enter coordinates immediately, delivers explicit leave with the last position, and clears runtime hover and cursor state. Step 558 Wayland pointer axis and frame production behavior is next.

## 2026-07-11 Phase F Step 558 Wayland Pointer Axis/Frame Audit

- Production currently accumulates vertical/horizontal `wl_pointer.axis`
  values and publishes once at `wl_pointer.frame`, with an immediate fallback
  for pointer protocol versions before frame support.
- `axis_source`, `axis_stop`, `axis_discrete`, `axis_value120`, and relative
  direction callbacks are currently no-ops. Consequently Wayland scroll events
  never expose the existing public `PointerScrolled::precise` distinction and
  high-resolution wheel metadata cannot affect frame output.
- The existing compositor test sends only paired axis values plus frame and
  verifies raw delta aggregation. Step 558 coverage must retain that compatible
  path while adding source/value120/stop and multi-axis frame behavior.
- Ownership should stay in the focused private
  `wayland_application_pointer_scroll.cpp` module and compact application input
  state; no broad Wayland application or runtime source needs to own protocol
  scroll bookkeeping.
- Wayland protocol defines `axis` as the scroll distance and `axis_value120` as
  high-resolution wheel metadata sent in the same frame; value120 must not
  replace or double-add the axis distance. One detent is 120, matching Win32's
  public normalization to one logical step.
- Precision mapping can therefore stay allocation-free and compatible:
  finger/continuous sources are precise, wheel/wheel-tilt are precise only when
  a same-frame value120 is not an integer multiple of 120, and legacy source-
  absent axis-only frames retain the current non-precise behavior.
- `axis_stop` is frame lifecycle metadata rather than a zero-delta scroll. It
  should clear per-axis pending metadata at frame completion without publishing
  a synthetic `PointerScrolled` event when no axis distance exists.
- The first real frame test failed on the fractional wheel case because both
  production registry binding and the test compositor capped `wl_seat` at v5.
  That made v8 `axis_value120` unreachable despite the listener callback being
  present. Step 558 must negotiate up to the generated protocol's v9 while
  retaining `min(server_version, 9)` compatibility.
- The v9 seat rerun still failed because the compositor's `seat_get_pointer`
  independently capped the child `wl_pointer` resource at v5. Source events
  (v5) arrived, but value120 (v8) did not. Raising that child-resource cap to
  v9 completes the test protocol path; keyboard remains independently capped.
- Phase F Step 558 aggregates Wayland axis frames with source-aware precision, high-resolution value120 metadata, stop-only cleanup, and v9 pointer negotiation. Step 559 Wayland fractional scale production behavior is next.

## 2026-07-11 Phase F Step 559 Wayland Fractional Scale Audit

- Current production binds only `wl_output` v2 integer scale, chooses the
  maximum entered-output integer factor, sets that as surface buffer scale,
  and multiplies logical size directly into framebuffer size.
- No fractional-scale or viewporter protocol interface, registry binding,
  application ownership, window object, or compositor coverage exists.
- Production fractional scaling requires both protocols: preferred scale is
  expressed in 120ths by `wp_fractional_scale_v1`, while `wp_viewporter` keeps
  the surface destination at logical size when the integer buffer scale is
  `ceil(preferred_scale / 120)`.
- `WindowState::scale` should expose the preferred float scale and framebuffer
  sizing should follow that scale; the native surface buffer scale remains the
  required integer ceiling. Integer output scale remains the compatibility
  fallback when either fractional protocol is unavailable.
- Ownership boundaries should be separate manual protocol interface/request
  modules matching the existing xdg/text-input pattern, application registry
  manager fields, and a focused per-window fractional-scale lifecycle module.
- The first combined behavior run proved fractional scale but regressed the
  historical integer-output test because the test compositor sent a default
  preferred-scale 120 immediately. Keeping the protocol advertised while
  withholding preferred scale until explicitly requested correctly exercises
  the production integer fallback and the dynamic fractional transition.
- Phase F Step 559 adds production Wayland fractional scaling with preferred 120-based scale, integer-ceiling buffer scale, viewporter logical destinations, and integer output fallback. Step 560 Wayland configure lifecycle production behavior is next.

## 2026-07-11 Phase F Step 560 Wayland Configure Lifecycle Audit

- The roadmap names configure lifecycle as the next production slice without
  prescribing a specific API expansion. Existing ownership is already focused
  in `wayland_window_configure.cpp`, its private configure state, and the real
  test compositor.
- Production already records pending toplevel size/state, acknowledges the
  following xdg-surface serial, and gates resize publication until the surface
  configure arrives. Step 560 should identify ordering, stale-state, or
  repeated-configure behavior gaps through real compositor sequences rather
  than moving logic into the broad window/application files.
- The concrete lifecycle violation is that `record_toplevel_configure_state`
  immediately mutates `logical_size_` and public framebuffer state before the
  matching `xdg_surface.configure` arrives. XDG toplevel size/state is pending
  configure data and must become current only when the surface configure is
  acknowledged.
- A split real-compositor sequence should prove that toplevel-only delivery
  changes neither `WindowState` nor resize events, then surface configure ack
  atomically applies the latest pending size. It should also replace an older
  pending non-zero size with a newer zero-size configure so stale dimensions
  cannot leak through the eventual surface configure.
- The shared test compositor is already 3682 lines, so Step 560 should not add
  another configure-state cluster directly to it. A focused test configure
  request/state module can own current request fields, split-phase flags,
  serial/ack observations, and snapshots; the broad compositor should retain
  only Wayland resource dispatch.
- Production can remove the separate resize-pending boolean: surface configure
  applies the latest pending non-zero size, compares the resulting framebuffer
  size with the prior public state, clears the consumed size marker, and emits
  `WindowResized` only when the committed size actually changed.
- The production change validates on the real compositor: pending toplevel-only
  sizes remain invisible, a newer zero-size configure cancels an older pending
  size, the later surface configure preserves the client-selected size, a
  committed non-zero size applies at ack, and a duplicate size does not emit a
  redundant resize event.
- Phase F Step 560 commits Wayland toplevel size and state only at surface configure acknowledgement, discards superseded pending sizes, and suppresses duplicate resize events. Step 561 Wayland cursor theme loading production behavior is next.

## 2026-07-11 Phase F Step 561 Wayland Cursor Theme Loading Audit

- Production currently maps public cursor shapes to Wayland cursor names and
  records deterministic unavailable diagnostics, but every application still
  submits a null cursor surface through `wl_pointer_set_cursor`.
- No `wl_cursor_theme`, cursor image/buffer ownership, cursor surface, hotspot,
  animation frame, environment theme/size, or scale-aware reload path exists.
  Step 561 must add real resource ownership in focused cursor modules and keep
  the broad Wayland application source orchestration-only.
- WSL exposes `wayland-cursor` 1.25.0 through pkg-config. The production path
  can use `wl_cursor_theme_load`, `wl_cursor_theme_get_cursor`,
  `wl_cursor_image_get_buffer`, a dedicated cursor surface, integer-ceiling
  window scale, and surface buffer scale instead of adding a custom protocol.
- The real test compositor can advertise standard shared-memory support with
  `wl_display_init_shm`. Its pointer set-cursor and surface attach/commit hooks
  should observe a non-null cursor surface, buffer attachment, hotspot, and
  shape reapplication without embedding cursor-image behavior in broad test
  callbacks.
- `wayland_window_state()` already exposes the active fractional/integer scale,
  but the application is not notified when that scale changes. A narrow
  window-scale callback can reapply only the currently hovered window cursor;
  the cursor resource module can cache/reload the theme at `ceil(scale)`.
- Keep libwayland-cursor types out of the broad application header through an
  opaque resource pointer. The focused implementation should own theme and
  cursor-surface destruction, environment theme/size parsing, image fallback,
  buffer scale/attach/damage/commit, and hotspot conversion.
- The first production run proved the complete native path through a temporary
  theme: three cursor applications used a non-null surface, attached and
  committed a real shm buffer, and preserved hotspot (1,2) at scale 1. The only
  failure was test bookkeeping: a second PointerMoved overwrote stage 2 back to
  1; compare-exchange now lets only the first move own the transition.
- Cursor regression coverage exposed a latent Step 560 ordering issue rather
  than a cursor defect: configure ack was flushed before pending size applied
  its viewport destination. Moving the flush after acknowledgement sends the
  ack and matching viewport state together before resize callbacks observe it.
- Phase F Step 561 loads real Wayland cursor themes through wl_shm and libwayland-cursor, applies scaled cursor surfaces with image buffers and hotspots, and reloads on window scale changes. Step 562 Wayland event-loop wakeup production behavior is next.

## 2026-07-11 Phase F Step 562 Wayland Event-Loop Wakeup Audit

- Production already creates a nonblocking close-on-exec pipe, polls it beside
  the Wayland display fd, drains readable bytes, publishes
  `WindowWakeupRequested` to registered windows, and uses the same path to wake
  `quit()`.
- No real Wayland test currently proves cross-thread wakeup delivery, burst
  coalescing/drain behavior, idle-loop quit latency, or correctness of the
  Wayland prepare-read/cancel-read sequence under wakeup-only polls.
- `running_` is a plain bool read by the event-loop thread and written by
  cross-thread `quit()`, which is a data race. The current wakeup-only poll also
  dispatches `WindowWakeupRequested` after `quit()` has already cleared the
  running flag, exposing an extra user event for an internal termination wake.
- The loop uses `dispatch_pending` followed by raw poll and `wl_display_dispatch`
  rather than the standard `prepare_read` / `read_events` / `cancel_read`
  sequence. Step 562 should make the display-read reservation explicit while
  preserving wakeup-pipe responsiveness and EAGAIN flush handling.
- The real behavior sequence now passes: 32 pre-run writes drain into one
  window wakeup, a later cross-thread request produces exactly one more, and
  idle `quit()` exits without publishing a third event. Existing compositor
  close, window lifecycle, and cursor theme paths remain green under the
  prepare-read loop.
- Phase F Step 562 makes Wayland wakeups thread-safe with atomic run state, prepare-read polling, EINTR-safe pipe draining, burst coalescing, and quit-only wake suppression. Step 563 Win32 Unicode clipboard production behavior is next.

## 2026-07-11 Phase F Step 563 Win32 Unicode Clipboard Audit

- The repository already has focused Win32 clipboard text/read/write modules,
  `CF_UNICODETEXT` system integration, and Windows tests that read and write
  non-ASCII UTF-8 through the real clipboard.
- Step 563 should deepen production semantics and failure coverage rather than
  adding a duplicate clipboard API. Audit targets are UTF-8/UTF-16 conversion,
  embedded NUL handling, CRLF conventions, clipboard-open contention,
  GlobalAlloc ownership transfer, and cleanup on every Win32 failure path.
- The concrete Unicode defect is permissive conversion: both Win32 conversion
  calls use flags 0, so invalid UTF-8/UTF-16 can be replacement-decoded and
  reported as success. Embedded NUL in the public `string_view` is also accepted
  even though `CF_UNICODETEXT` readers terminate there, causing silent data
  truncation.
- A focused Windows-only system test should preserve the original clipboard,
  prove emoji/CRLF/empty round trips, reject invalid UTF-8 and embedded NUL, and
  verify rejected writes leave the prior clipboard content untouched.
- Strict optional conversion now passes the real system test: emoji, CRLF, and
  empty text round-trip through `CF_UNICODETEXT`; malformed UTF-8 and embedded
  NUL are rejected before `OpenClipboard`, leaving the baseline clipboard
  unchanged. Existing cross-platform clipboard and structure coverage remains
  green.
- Phase F Step 563 makes Win32 Unicode clipboard conversion strict, preserves emoji, CRLF, and empty text, rejects invalid UTF-8 and embedded NUL, and leaves existing system content intact on rejection. Step 564 Win32 file clipboard production behavior is next.

## 2026-07-11 Phase F Step 564 Win32 File Clipboard Audit

- The public `Clipboard` leaf currently exposes text only, and the focused
  Win32 clipboard modules contain no `CF_HDROP` or `DROPFILES` behavior.
- Win32 OLE drag/drop already decodes `CF_HDROP`, but that module is owned by
  window drag/drop. Clipboard file transfer should use the same UTF-8 public
  path semantics in a separate `clipboard_win32_files.cpp` owner rather than
  coupling clipboard production to OLE window code.
- The compatibility-preserving public shape is default `read_files()` and
  `write_files(std::span<const std::string>)` virtual behavior on `Clipboard`;
  unsupported backends return no files/false while Win32 overrides both.
- A focused real-system test should prove multiple Unicode paths round-trip
  through a wide `DROPFILES` payload, empty and malformed path lists are
  rejected before opening the clipboard, and rejected writes preserve existing
  text content.
- The production path now passes those real-system cases while unsupported
  backends retain explicit default no-file behavior.
- Default-parallel xmake execution can overlap the three Windows system
  clipboard tests. A focused named-mutex test support header serializes only
  those processes, preserving normal parallelism for the rest of the suite.
- Phase F Step 564 adds Win32 CF_HDROP file clipboard read and write with strict UTF-8 paths, wide DROPFILES payloads, multi-file ordering, and rejection-safe system content preservation. Step 565 Wayland selection ownership and write production behavior is next.

## 2026-07-11 Phase F Step 565 Wayland Selection Ownership Audit

- The existing Wayland clipboard already creates a real `wl_data_source`,
  offers UTF-8/plain text MIME types, sets selection, dispatches source events,
  and writes the owned payload to compositor-provided file descriptors.
- The ownership replacement is not transactional: `write_text` destroys the
  prior source and overwrites `owned_text_` before the new selection flush has
  succeeded. A failed replacement therefore discards the previous valid
  ownership and payload.
- Existing real compositor coverage proves only one write/send cycle. Step 565
  should add replacement and cancellation controls, prove the newest source
  serves the newest payload, prove stale-source cancellation cannot clear the
  replacement, and keep source lifecycle logic in focused clipboard modules.
- The concrete production failure was display-lock starvation on the second
  write. Pausing dispatch around replacement both removes that starvation and
  creates a transactional point where the new selection is flushed before the
  old source is destroyed.
- Phase F Step 565 makes Wayland selection writes transactional, prevents display-lock starvation during replacement, preserves continuous ownership, and serves the newest payload across UTF-8 and plain-text MIME requests. Step 566 Wayland selection read production behavior is next.

## 2026-07-11 Phase F Step 566 Wayland Selection Read Audit

- Existing reads correctly roundtrip selection events, prefer UTF-8 text,
  request the chosen offer through a pipe, and return empty payloads as valid
  text, but existing tests use a fresh clipboard with no dispatch thread.
- When the same clipboard already owns a selection, its source dispatch thread
  continuously polls under `display_mutex_`. `read_text()` competes for that
  mutex without pausing dispatch and can suffer the same indefinite starvation
  fixed for replacement writes in Step 565.
- A focused real test should own text first, accept a compositor-provided
  replacement selection, and read its payload through the same connection.
- Phase F Step 566 makes Wayland selection reads dispatch-safe, processes replacement offers and payload transfer in one display transaction, and resumes source dispatch only when ownership remains. Step 567 Wayland clipboard MIME negotiation production behavior is next.

## 2026-07-11 Phase F Step 567 Wayland Clipboard MIME Audit

- MIME preference currently uses exact, case-sensitive string equality for
  `text/plain;charset=utf-8` and `text/plain`.
- MIME type/subtype and charset tokens are case-insensitive, and optional
  whitespace around parameters is valid. A legal mixed-case UTF-8 offer is
  therefore ignored today and can lose to a lower-quality plain offer.
- MIME normalization/ranking belongs in a focused private source; read
  orchestration should only select the highest-ranked original offer string.
- The focused ranker now accepts mixed-case UTF-8 charset parameters, preserves
  the compositor's original MIME spelling for `receive`, and rejects a
  non-UTF-8 charset so case-folded plain text can win as fallback.
- Phase F Step 567 adds case-insensitive Wayland text MIME parsing and ranking, accepts normalized UTF-8 charset parameters, preserves original offer strings for receive, and rejects unsupported charsets. Step 568 Wayland clipboard incremental transfer production behavior is next.

## 2026-07-11 Phase F Step 568 Wayland Clipboard Incremental Transfer Audit

- `read_offer_payload(...)` appends 4 KiB reads, but its fixed three-second
  total deadline expires even when a producer continues making regular
  progress. A valid slow chunked transfer can therefore be truncated solely
  because its total duration exceeds the deadline.
- The read-side deadline should represent idle time and reset after every
  successful chunk. Read interruption should also retry instead of converting
  an `EINTR` into transfer failure.
- `write_payload_to_fd(...)` already loops until the owned payload is written;
  the Step 568 behavior test should preserve this path with a payload larger
  than a pipe buffer while adding a delayed multi-chunk external read.
- Test-only chunk scheduling belongs in a focused clipboard transfer helper so
  the 3652-line compositor remains below its 3700-line structure cap.
- The first Step 568 search repeated a PowerShell wildcard path form that `rg`
  cannot expand; subsequent searches use directory roots plus `--glob`.
- Two assumed split filenames, `clipboard_wayland_write.cpp` and
  `wayland_test_compositor_clipboard.cpp`, do not exist; `rg --files` located
  the actual owners in `clipboard_wayland_source_io.cpp` and the main test
  compositor plus its focused clipboard source-state header.
- Phase F Step 568 keeps active Wayland clipboard reads alive across incremental chunks, retries nonblocking owned-selection writes after EAGAIN, preserves large payloads, and bounds stalled transfers with idle deadlines. Step 569 Wayland clipboard failure handling production behavior is next.

## 2026-07-11 Phase F Step 569 Wayland Clipboard Failure Handling Audit

- The incremental transfer test ignores `SIGPIPE` so it can inspect deliberate
  early-close behavior, but production itself does not suppress the signal.
  A clipboard receiver that closes its pipe before an owned selection writes
  can therefore terminate the entire host process instead of returning `EPIPE`.
- Failure handling should be scoped to the write syscall's thread rather than
  installing a process-global signal disposition. A focused Linux helper can
  temporarily block `SIGPIPE`, consume only a newly generated pending signal,
  restore the caller's mask, and return the original write result.
- A real compositor failure test should abandon one request, prove the process
  survives, then request the same owned selection normally to prove ownership
  and dispatch remain usable after the peer failure.
- Phase F Step 569 prevents abandoned Wayland clipboard receivers from terminating the host with SIGPIPE, scopes signal masking to the writer thread, preserves caller signal state, and keeps selection ownership usable after EPIPE. Step 570 Wayland clipboard diagnostics production behavior is next.

## 2026-07-11 Phase F Step 570 Wayland Clipboard Diagnostics Audit

- Existing clipboard APIs expose only coarse support and return values; async
  owned-selection send failures are invisible even though they occur on the
  dispatch thread after `write_text()` succeeds.
- Diagnostics should remain allocation-free and string-free: operation kind,
  failure reason, transferred bytes, and revision are sufficient for callers
  to distinguish installation, receiver-close, timeout, and recovery.
- The connection needs a focused atomic snapshot state because diagnostics are
  written by both the caller thread and Wayland source dispatch thread.
- A real test can reuse the abandoned receiver boundary from Step 569, observe
  the async `receiver_closed` snapshot, then prove a normal request advances to
  a successful send snapshot without losing ownership.
- Phase F Step 570 adds allocation-free Wayland clipboard diagnostic snapshots for write and async send operations, reports receiver-close and timeout failures with byte counts and revisions, and records recovery after failure. Step 571 Win32 OLE drop target production behavior is next.

## 2026-07-11 Phase F Step 571 Win32 OLE Drop Target Audit

- The repository already owns a focused `IDropTarget`, window registration and
  revocation lifecycle, OLE payload extraction, and window event forwarding.
- The remaining concrete target defect is COM input validation:
  `DragEnter`, `DragOver`, and `Drop` accept null required pointers, return
  `S_OK`, and can forward invalid calls into the owner.
- A focused target test should lock `QueryInterface`, reference count behavior,
  invalid-argument rejection with `DROPEFFECT_NONE`, and valid callback
  forwarding without requiring a real drag source.
- Phase F Step 571 validates Win32 OLE drop target COM inputs, returns E_INVALIDARG for missing data/effect pointers, clears rejected effects, and preserves QueryInterface, reference counting, and valid owner forwarding. Step 572 Win32 OLE drop source production behavior is next.

## 2026-07-11 Phase F Step 572 Win32 OLE Drop Source Audit

- No `IDropSource` or `DoDragDrop` execution boundary exists today.
- Source control and payload `IDataObject` formats are separate ownership
  concerns. Step 572 should own COM source lifecycle, cancel/drop decisions,
  feedback, allowed effects, and final result; later drag payload steps can
  build text/files/URI data objects over this runner.
- A function-pointer runner seam allows production to call `DoDragDrop` while
  tests exercise the real source object without opening a modal desktop drag.
- Phase F Step 572 adds a Win32 OLE IDropSource and injectable DoDragDrop runner, supports escape cancellation, button-release drop, default cursor feedback, allowed-effect propagation, and final effect reporting. Step 573 Wayland data-device accept and action negotiation production behavior is next.

## 2026-07-11 Phase F Step 573 Wayland Data-Device Negotiation

- Step 200 already issues real `wl_data_offer.accept`, `set_actions`, and
  `finish` requests, so Step 573 must deepen protocol behavior rather than add
  another negotiation facade.
- Wayland defines `set_actions.dnd_actions` as destination-supported actions;
  the current implementation incorrectly intersects that mask with source
  actions. This can advertise zero actions when `source_actions` arrives after
  `wl_data_device.enter`, even though the destination supports copy and move.
- `wl_data_offer.set_actions` may be called repeatedly during a drag. The offer
  listener currently stores late `source_actions` but cannot reach its owning
  `WaylandDataDevice`, so it never corrects the initial negotiation.
- `wl_data_offer.finish` is valid only after a non-null MIME acceptance and a
  selected action. The production state needs explicit acceptance tracking so
  rejected or actionless offers are destroyed without an invalid finish.
- Phase F Step 573 makes Wayland drag negotiation order-safe, advertises destination copy/move capabilities independently of source actions, renegotiates late offer events, preserves pending enter actions, and rejects invalid finish requests. Step 574 Wayland data-device finish negotiation production behavior is next.

## 2026-07-11 Phase F Step 574 Wayland Finish Negotiation

- A successful version-3 drop currently calls `wl_data_offer_finish` but keeps
  the offer proxy alive until a later `leave`. After finish, Wayland permits no
  request other than destroy, and the destination should release the offer
  immediately after the finish request.
- Step 574 should make successful finish plus destroy exactly-once, while
  rejected MIME, missing action, cancellation, and pre-version-3 offers stay on
  destroy-without-finish paths.
- The shared test compositor already observes server-side offer destruction,
  but its broad source is exactly 3700 lines. New lifecycle records must move
  into a focused drag-offer test-state module before adding assertions.
- Phase F Step 574 completes Wayland drop offers with exactly-once finish followed by immediate destroy, destroys rejected and pre-v3 offers without finish, and observes lifecycle completion before leave in focused test state. Step 575 Win32 OLE text drag payload production behavior is next.

## 2026-07-12 Phase F Step 575 Win32 OLE Text Payload

- The Win32 drop target already reads `CF_UNICODETEXT`, and Step 572 owns the
  `IDropSource` plus `DoDragDrop` runner, but there is no source-side
  `IDataObject` implementation that can provide text to a real drop target.
- The text data object belongs in a focused private Win32 leaf, not the shared
  drag/drop internal header or window source. It should expose only
  `CF_UNICODETEXT` through `TYMED_HGLOBAL` and compose with the existing runner.
- Source text conversion must match the strict clipboard boundary: reject
  malformed UTF-8 and embedded NUL, preserve emoji/CRLF, and encode empty text
  as a valid one-wide-NUL global-memory payload.
- Phase F Step 575 adds a strict Win32 OLE text IDataObject, enumerates CF_UNICODETEXT through owned HGLOBAL storage, preserves emoji, CRLF, and empty text, rejects malformed UTF-8 and embedded NUL, and composes with the drag runner. Step 576 Win32 OLE file drag payload production behavior is next.

## 2026-07-12 Phase F Step 576 Win32 OLE File Payload

- The existing Win32 clipboard writer demonstrates the required `DROPFILES`
  layout: a wide-character list with one NUL after each path and a final second
  NUL, `pFiles = sizeof(DROPFILES)`, and `fWide = TRUE`.
- The OLE file data object should own the same strict conversion and overflow
  checks in a focused drag leaf without opening or mutating the system
  clipboard. It should expose only `CF_HDROP` through `TYMED_HGLOBAL`.
- Production validation must reject an empty list, empty paths, malformed UTF-8,
  embedded NUL, and size overflow before the drag runner is invoked, while
  preserving multi-file order and Unicode paths.
- Phase F Step 576 adds a strict Win32 OLE file IDataObject, enumerates CF_HDROP through an owned wide DROPFILES HGLOBAL, preserves Unicode multi-file ordering, rejects invalid or empty paths before drag, and composes with the drag runner. Step 577 Wayland non-local URI-list drag policy production behavior is next.

## 2026-07-12 Phase F Step 577 Wayland URI-List Policy

- The current parser already ignores non-file schemes and non-local authorities,
  but URI parsing is buried in broad `wayland_helpers.cpp` and has no focused
  behavior coverage.
- Existing percent decoding preserves malformed escapes and permits `%00` to
  inject an embedded NUL into a public file path. Scheme and `localhost`
  matching are also incorrectly case-sensitive.
- The production policy should accept only absolute local file URIs with empty
  authority or case-insensitive `localhost`, keep valid local entries from a
  mixed list, and reject remote authorities, other schemes, relative paths,
  malformed percent escapes, and decoded NUL.
- Phase F Step 577 moves Wayland URI-list parsing into a focused policy leaf, accepts only absolute local file URIs with empty or localhost authority, preserves valid local entries, and rejects remote schemes/authorities, malformed escapes, queries/fragments, relative paths, and decoded NUL. Step 578 drag/drop cancellation and band closeout is next.

## 2026-07-12 Phase F Step 578 Drag/Drop Cancellation Closeout

- Win32 `Win32OleDropSource::QueryContinueDrag(...)` already returns
  `DRAGDROP_S_CANCEL` for Escape, but the drag runner result has no focused
  cancellation classification or closeout coverage across the Step 571-577
  source, target, and payload band.
- Wayland `handle_leave(...)` already emits `DragExited` and destroys the
  current offer without calling `finish`; the missing production-grade
  evidence is cancellation before drop, exactly-once exit delivery, and
  immunity to a later drop callback after the offer was cleared.
- Step 578 should keep ownership in the existing focused Win32 drop-source and
  Wayland data-device drag-event modules, add behavior coverage before any
  production change, and close the Steps 571-578 band with a dedicated
  structure/audit guard before handing off to Step 579 native menu trees.
- The focused Win32 RED test confirmed that `run_win32_ole_drag(...)` trusted
  an effect written by a runner that returned `DRAGDROP_S_CANCEL`; only a
  successful `DRAGDROP_S_DROP` may expose a final effect, while cancellation
  and failure must normalize it to `DROPEFFECT_NONE`.
- The real Wayland compositor test confirms that leave cancellation destroys
  the active offer without finish, emits one `DragExited`, and suppresses both
  a duplicate leave exit and a late drop callback after state was cleared.
- Phase F Step 578 normalizes Win32 OLE cancellation to a none effect, proves Wayland leave destroys offers without finish, suppresses duplicate exit and late drop callbacks, and closes the Steps 571-578 drag/drop band. Step 579 native menu tree production behavior is next.

## 2026-07-12 Phase F Step 579 Win32 Native Menu Tree

- The public `NativeMenuModel` and runtime installation chain already exist,
  but both Win32 and Wayland platform states currently retain only the model
  and report `supported = false`; no native menu tree is materialized.
- Step 579 ownership belongs in a focused Win32 menu-tree leaf that recursively
  creates and owns `HMENU` resources. `Win32NativeMenuState` should remain the
  installation/model boundary, while `Win32Application` applies the installed
  root menu to existing and subsequently created Win32 windows.
- Wayland has no equivalent in-process native global-menu protocol in the
  current target boundary, so its existing explicit unsupported result remains
  correct for this Win32 production slice.
- The roadmap summary checkbox for the completed Steps 571-578 drag/drop band
  remained unchecked after Step 578 and must be corrected during the next
  five-document authority synchronization.
- The public integration test reached RED at exit 3 because Win32 still
  reported the retained menu model as unsupported. The focused tree leaf now
  recursively creates `CreateMenu`/`CreatePopupMenu` resources, appends command
  and separator entries, and owns the full tree through one root `DestroyMenu`.
- `Win32NativeMenuState` commits a newly built tree only on success, and
  `Win32Application` detaches old handles before replacement then applies the
  current root to existing and subsequently created windows.
- The initial WSL shared gate exposed a historical test-runner path assumption:
  `win32_window_source_test` returned exit 2 because its four-level fallback
  could not escape the deeper WSL build root. Its xmake target now runs from
  `os.projectdir()`, matching the repository's other source structure guards.
- Phase F Step 579 builds and owns recursive Win32 HMENU trees in a focused leaf, attaches installed menus to existing and future windows, preserves nested Unicode titles and separators, and keeps Wayland explicitly unsupported. Step 580 native menu check/radio/enabled state production behavior is next.

## 2026-07-12 Phase F Step 580 Native Menu Item State

- Existing `NativeMenuItem` initialization sites use C++ designated fields, so
  adding a defaulted `radio` flag immediately before `children` preserves
  source compatibility while exposing the missing radio-check visual intent.
- The focused behavior target should cover disabled command state, ordinary
  checked state, and Win32 `MFT_RADIOCHECK` plus `MFS_CHECKED` without moving
  state logic back into `win32_native.cpp` or the application orchestrator.
- The expanded real-window test reached RED at exit 6 after menu hierarchy and
  counts passed, isolating the missing production behavior to Win32 item flags.
  The focused leaf now maps disabled, checked, and radio state for both commands
  and submenus through one flag helper.
- The first implementation attempt failed to compile because radio-check is
  `MFT_RADIOCHECK`, not an `AppendMenuW` `MF_*` flag. The leaf now uses
  `MENUITEMINFOW` and `InsertMenuItemW`, which cleanly separates type and state.
- Phase F Step 580 maps enabled, checked, and radio menu state through structured Win32 MENUITEMINFO records, adds a defaulted public radio flag, and verifies disabled, checkmark, and radio-check rendering on a real HMENU. Step 581 native menu dynamic update production behavior is next.

## 2026-07-12 Phase F Step 581 Native Menu Dynamic Update

- The existing Win32 application already detaches old menu handles before a
  transactional tree rebuild and applies the committed root to all live
  windows, so Step 581 should freeze multi-window replacement rather than add a
  second update mechanism.
- The concrete remaining gap is empty-model semantics: an empty installation
  currently creates and attaches an empty menu bar. Production clearing should
  commit a successful null-root tree so existing and future windows have no
  attached menu.
- The first test executable name contained `update` and Windows refused to run
  it with error 740 through installer/UAC filename detection. The registered
  target uses `replacement` instead; no product manifest change is warranted.
- The renamed behavior test reached RED at exit 6 after both live windows
  observed a successful transactional replacement, isolating the gap to empty
  model clearing. An empty model now builds a successful null-root tree so the
  application leaves current and future windows without a menu.
- Phase F Step 581 makes Win32 native menu installation dynamically replace all live window menus, preserves transactional tree ownership, treats an empty model as a successful clear, and keeps future windows aligned with the current menu state. Step 582 native menu accelerator display production behavior is next.

## 2026-07-12 Phase F Step 582 Native Menu Accelerator Display

- No existing platform key-label formatter covers native menu text. Ownership
  should be a focused Win32 accelerator-display leaf, leaving recursive HMENU
  construction and application orchestration unchanged.
- The display convention for this slice is a tab separator plus ordered
  `Ctrl`, `Alt`, `Shift`, and `Win` modifier names followed by a stable key
  label. Coverage should include letters, digits, function keys, and named
  navigation/edit keys before later steps own accelerator registration/dispatch.
- The real Win32 display test reached RED at exit 4 after installation and
  accelerator counting passed. A focused formatter leaf now owns modifier/key
  labels and the menu-tree leaf only consumes the resulting display title.
- The first formatter run still failed because Win32 function-key codes overlap
  lowercase ASCII values (`VK_F4 == 's'`). Native accelerator key codes follow
  virtual-key semantics and existing public examples use uppercase letters, so
  the ambiguous lowercase branch was removed.
- Phase F Step 582 formats Win32 native menu accelerator labels in a focused leaf, appends tab-aligned Ctrl/Alt/Shift/Win key text for letters, digits, function keys, and named keys, and preserves accelerator counts without claiming dispatch. Step 583 native menu accelerator registration production behavior is next.

## 2026-07-12 Phase F Step 583 Native Menu Accelerator Registration

- Win32 currently assigns recursive HMENU command ids from `0x1000`, but owns
  no `HACCEL` table and the application loop always routes key messages through
  `TranslateMessage` and `DispatchMessageW`.
- Accelerator-table ownership belongs in a focused RAII leaf next to, but not
  inside, display formatting or recursive HMENU construction. The native menu
  state should commit the menu tree and accelerator table together.
- Win32 `ACCEL` records support virtual-key key-down plus Ctrl/Alt/Shift flags;
  they cannot represent the public `super` modifier or key-up action. This step
  registers the representable subset while preserving the public descriptor
  count; later platform diagnostics can expose skipped descriptors.
- The real Win32 loop test reached RED at exit 5 after installation succeeded,
  then passed once the focused table was owned by native menu state and checked
  before ordinary key dispatch. Display, tree, and replacement regressions pass
  with it.
- Phase F Step 583 builds and transactionally owns a focused Win32 HACCEL table, shares recursive command ids with HMENU construction, translates supported Ctrl/Alt/Shift key-down accelerators before ordinary key dispatch, and preserves unsupported Win-key/key-up descriptors for diagnostics. Step 584 native menu command dispatch production behavior is next.

## 2026-07-12 Phase F Step 584 Native Menu Command Dispatch

- `WM_COMMAND` currently reaches the Win32 window procedure after menu
  selection or `TranslateAcceleratorW`, but no command-id/action mapping or
  platform event exists.
- A dedicated `NativeMenuCommand` public event should carry command id,
  action name, and menu-vs-accelerator source. The platform mapping can be an
  immutable shared object so window callbacks never borrow application state.
- Runtime ownership belongs in a focused event leaf that routes the command to
  the existing focused/view/window/app action dispatcher instead of teaching
  platform code about UI action registrations.
- The two focused behaviors compile-RED on the missing public command event,
  then pass after immutable Win32 mapping, `WM_COMMAND` event emission, and the
  runtime action bridge are present. Registration and menu-tree regressions
  remain green.
- Phase F Step 584 maps recursive Win32 menu command ids to immutable action names, emits menu/accelerator command events from WM_COMMAND, and routes them through existing scoped runtime action dispatch for root and additional windows. Step 585 native menu dispatch diagnostics production behavior is next.

## 2026-07-12 Phase F Step 585 Native Menu Dispatch Diagnostics

- `Win32NativeMenuAcceleratorTable` already tracks its registered entry count,
  while `PlatformMenuInstallationResult` only exposes declared accelerator
  count. Public registered/skipped counts can report unsupported Win-key and
  key-up descriptors without strings or hidden allocations.
- Runtime already owns a bounded `PlatformDiagnosticEvent` stream with menu
  kind, event kind, operation, success, and value count fields. Command source,
  handler success, and command id fit this existing record without widening it.
- The Win32 registration test reaches compile RED on the missing public counts,
  then passes with three declared accelerators, one registered accelerator, and
  two skipped Win-key/key-up descriptors. Runtime diagnostics record handled
  accelerator and unhandled menu-command dispatches separately.
- Phase F Step 585 reports declared, registered, and skipped native-menu accelerators across platform results, and records handled/unhandled menu command diagnostics with source, event kind, and command id. Step 586 native menu and accelerator closeout audit is next.

## 2026-07-12 Phase F Step 586 Native Menu And Accelerator Closeout

- Steps 579-585 already have focused structure guards for Win32 tree ownership,
  item state, replacement/clear, accelerator display and registration, command
  dispatch, and bounded runtime diagnostics.
- The closeout should remain audit-only: aggregate those guards, preserve the
  explicit Wayland unsupported/skipped result, and hand off to dialogs without
  widening any production module.
- Phase F Step 586 audits and closes the Steps 579-585 native-menu and accelerator band, freezing Win32 menu tree/state/replacement, accelerator display/registration, command dispatch, diagnostics, and explicit Wayland unsupported behavior. Step 587 open-file dialog production behavior is next.

## 2026-07-12 Phase F Step 587 Win32 Open-File Dialog

- The public file-dialog request/result and runtime diagnostic/result plumbing
  already exist, but Win32 always reports unsupported from `win32_native.cpp`.
- Native ownership belongs in a focused COM leaf. A pure plan maps single vs.
  multi-select flags and extension filters without showing UI; the execution
  path owns COM initialization, `IFileOpenDialog`, cancellation, and paths.
- Phase F Step 587 implements Win32 open-file and multi-file dialogs in a focused COM leaf, maps filters and selection flags through a pure plan, handles cancellation without acceptance, and preserves explicit unsupported behavior elsewhere. Step 588 Win32 save-file dialog production behavior is next.

## 2026-07-12 Phase F Step 588 Win32 Save-File Dialog

- Save behavior can reuse the Step 587 plan/filter/path helpers while keeping
  `IFileSaveDialog` execution in its own source leaf.
- The save plan must use overwrite prompting without file-must-exist, preserve
  the suggested name, and return exactly one filesystem path on acceptance.
- Phase F Step 588 implements Win32 save-file dialogs in a focused IFileSaveDialog leaf, preserves suggested names and filters, requests overwrite confirmation without file-must-exist, and returns one filesystem path on acceptance. Step 589 native directory-picker production behavior is next.

## 2026-07-12 Phase F Step 589 Native Directory Picker

- `NativeFileDialogKind` has no directory variant yet, while the existing
  `IFileOpenDialog` execution can return a filesystem folder with the same
  single-result extraction path.
- Directory planning should add `FOS_PICKFOLDERS | FOS_PATHMUSTEXIST`, omit
  file-must-exist/multi-select, and ignore file-extension filters.
- Phase F Step 589 adds an explicit native directory-picker request kind, maps Win32 folder selection to FOS_PICKFOLDERS and existing-path requirements, ignores file filters, and preserves the shared single-path result contract. Step 590 native message-dialog production behavior is next.

## 2026-07-12 Phase F Step 590 Native Message Dialog

- No message-dialog model exists. The platform boundary needs explicit kind,
  button, response, supported/accepted, backend, and error fields before UI
  result adapters can be added later.
- Win32 presentation belongs in a focused `MessageBoxW` leaf with pure flag
  and response mapping; Wayland/default remain explicit unsupported backends.
- Phase F Step 590 adds a public native message-dialog platform service, maps Win32 kinds/buttons/responses through a focused MessageBoxW leaf, and reports explicit unsupported results on Wayland and default backends. Step 591 native open-URL production behavior is next.

## 2026-07-12 Phase F Step 591 Native Open URL

- No open-URL result surface exists. A small platform result should distinguish
  backend support from successful launch and preserve an error message.
- Win32 ownership belongs in a focused `ShellExecuteW` leaf with pure URL and
  return-code helpers; Wayland/default remain explicitly unsupported here.
- Phase F Step 591 adds a result-bearing native open-URL platform service, launches valid Win32 URLs through focused ShellExecuteW handling, rejects empty/NUL URLs, classifies native return codes, and preserves explicit unsupported Wayland/default results. Step 592 quit/reopen lifecycle production behavior is next.

## 2026-07-12 Phase F Step 592 Quit And Reopen Lifecycle

- Quit already has platform-specific event-loop behavior. Reopen lacks a
  callback/result contract on Windows and Wayland.
- A base-owned optional callback avoids duplicate storage; protected dispatch
  lets each platform report its backend while preserving no-handler evidence.
- Phase F Step 592 adds explicit reopen callback/result lifecycle behavior shared by Win32 and Wayland backends, reports missing callbacks without dispatch, supports callback replacement/clearing, and preserves existing platform quit paths. Step 593 platform-service result and unsupported-policy production behavior is next.

## 2026-07-12 Phase F Step 593 Platform Service Results And Unsupported Policy

- Phase B Step 302 already established the intended boundary in
  `src/ui/runtime_platform_service_results.cpp`: runtime `try_*` adapters
  convert platform service support/failure into `Result<T>` and unsupported
  attempts do not overwrite the last successful service state.
- Step 593 should extend that focused result-adapter leaf plus
  `src/ui/app_context_services.cpp` for message-dialog, open-URL, and reopen
  access. Broad runtime/application files should remain declaration or
  forwarding owners only.
- The existing bounded `PlatformDiagnosticEvent` stream is the compatible
  diagnostic surface to reuse for service success, unsupported, and failure
  attempts instead of adding an unrelated logging channel.
- `PlatformDiagnosticKind` currently distinguishes menu and file-dialog
  services but has no message-dialog, open-URL, or reopen values. Step 593
  needs explicit kinds so the shared bounded stream can identify each new
  adapter without encoding the service only in free-form operation text.
- Existing service Result policy treats any supported backend response as a
  value, including a cancelled file dialog. The new adapters should therefore
  return `unsupported_platform` only for `supported == false`; supported
  message cancellation, URL failure, or missing reopen callback remain typed
  values with `accepted`, `opened`, or `requested` left false.
- To preserve the modular boundary and existing 110-line cap on
  `runtime_platform_service_results.cpp`, the three new adapters should live
  in a separate focused UI source with state declarations isolated in a small
  private include. `app_context_services.cpp` remains forwarding-only.
- `window_runtime.hpp` and `window_runtime_internal.hpp` are exactly at their
  240/260-line structure caps before Step 593. Move all five platform-service
  result state members into a focused private state include, and keep the new
  public runtime surface declaration-only with a narrowly raised cap backed by
  the Step 593 structure guard.
- Phase F Step 593 adds Runtime and AppContext Result adapters for native message dialogs, URL opening, and reopen requests, records typed bounded diagnostics, preserves supported incomplete results, and keeps the last supported service state across unsupported attempts. Step 594 dialogs and platform-services closeout audit is next.
