# Box shadow and backdrop blur (C5, C6)

Both effects live in the shared Vulkan renderer (Win32 and Wayland) and in
the Metal renderer (macOS); see [Metal](#metal) for the differences.
The opt-in Windows [DX12 backend](dx12-backend.md) also implements these effects.
The Vulkan pipeline details below are not a description of DX12's native
resource topology; the shared Windows pixel scenes select either backend.

## C5 box shadow

### API

- `Style::box_shadow` / `ElementBuilder::shadow(BoxShadow)` (unchanged)
  paint through `PaintList::draw_box_shadow(Rect bounds, BoxShadow shadow,
  BorderRadii radius = {})`, emitted before the box's own fill.
- `BoxShadow { Color color; Point offset; float blur_radius; float
  spread_radius; }` (unchanged, logical pixels).
- New renderer command `BoxShadowDraw` in
  `include/cgpui/renderer/renderer_effect_draws.hpp` and
  `RenderFrame::draw_box_shadow(const BoxShadowDraw&)` (default: ignored,
  so other backends keep compiling). `RendererPrimitiveKind::box_shadow`,
  `RendererDrawCounts::box_shadow_count` and
  `FrameStatistics::box_shadow_command_count` count it.

### Conventions

- CSS box-shadow semantics: shadow shape = box moved by `offset` and grown
  by `spread_radius` on every side; the corner radii grow with the spread
  using the css-backgrounds-3 rule (`r + spread` when `r >= spread`, else
  `r + spread * (1 + (r / spread - 1)^3)`; a 0 radius stays sharp).
- Blur: `blur_radius` is the CSS blur radius, so the Gaussian standard
  deviation is `sigma = blur_radius / 2` (what browsers do). A shadow
  extends about `3 sigma = 1.5 * blur_radius` beyond its shape.
- Outer shadows are clipped out inside the box (the shadow does not show
  through a translucent box), like CSS.
- Inset shadows: `BoxShadow` has no inset flag, so inset shadows are not
  expressible. They are not supported; adding them means a new field and a
  second shader branch.
- Colors are sRGB-encoded and blended in sRGB space like every other draw
  (UNORM swapchain, `4931b6ed`).

### Implementation

- `renderer_to_device_pixels(BoxShadowDraw)` scales rect, radii, offset,
  blur and spread by the surface DPI scale (like `530a6e96`).
- `vulkan_build_box_shadow_geometry` emits one quad per shadow covering the
  shape plus `3 sigma + 1` px, with composed opacity applied to the color
  and composed transforms applied to the corners. Each vertex also carries
  the untransformed local point, the shadow and box rects/radii and sigma.
- `shaders/box_shadow.slang` evaluates the closed-form Gaussian-blurred
  rounded rect from Evan Wallace's "Fast rounded rectangle shadows" (the
  approach GPUI/Zed uses): an erf approximation integrates along x exactly,
  4 samples integrate along y. Sigma < 0.05 uses a sharp signed-distance
  edge with a one-device-pixel ramp, so zero blur equals a solid rect.
- The pipeline shares the rounded rect layout, fixed state and blending;
  draws go through the existing draw-order cursor and pipeline-switch
  batching as `VulkanFrameDrawResourceKind::box_shadow`, so they keep paint
  order and clip scissors. Shadow buffers and draws are part of the
  command-reuse signature.
- SPIR-V is embedded in `vulkan_box_shadow_shader_binaries.cpp` from
  `generated/box_shadow_spirv.inc`, which `scripts/dev/compile_shaders.py`
  writes with slangc (SPIR-V 1.3) and checks with `spirv-val`, like the
  other embedded shaders ([Getting Started](shaders.md)).
  Shader compilation is not a build step.

### Tests

- `renderer_box_shadow_pixel_test` (Windows, selected Vulkan/DX12 real swapchain): falloff left of
  and above a 24 px box with blur 8 at scale 1 and 1.5 against the exact
  `erf` blurred rect (tolerance 3/255); rounded corner falloff at scale 1.5
  against a C++ port of the shader formula; clip rect cuts the shadow; zero
  blur with offset, spread, opacity 0.5 and a translation equals a solid
  rect pixel for pixel (tolerance 1/255).
- `vulkan_box_shadow_geometry_test` (Windows and Linux, headless): sigma
  and spread conventions, quad extent, opacity, device-pixel conversion,
  draw order and pipeline switch.
- `box_shadow_submission_test`: styled paint emits shadow before fill and
  the shadow reaches `RenderFrame` with its clip.
- `box_shadow_structure_test`, `renderer_effect_draws_header_cleanliness`.

### Limits

- The y integral uses 4 samples; with large radii and small sigma the
  corner can differ from an exact convolution by a few 1/255 steps.
- Rotated or skewed transforms are evaluated in the box's local space; the
  sharp-edge ramp uses `fwidth` of the local point.

## C6 backdrop blur

### API

- `PaintList::draw_backdrop_blur(Rect bounds, BorderRadii radius, float
  blur_radius, std::optional<Color> tint = std::nullopt)` blurs what was
  painted before it inside the rounded rect. Paint the overlay's own fill
  after it.
- `Style::backdrop_blur` (`BackdropBlur { float radius; std::optional<Color>
  tint; }`), `Style::with_backdrop_blur`, `ElementBuilder::backdrop_blur`.
  A styled box paints shadow, then backdrop blur, then its fill. Not yet in
  `StyleOverlay`, the cascade, or tweening.
- Renderer: `BackdropBlurDraw`, `RenderFrame::draw_backdrop_blur`,
  `Renderer::backdrop_blur_support()` returning `RendererEffectSupport {
  bool supported; std::string reason; }`. Diagnostics:
  `RendererFrameDiagnosticSnapshot::backdrop_blur` (`blurred_backdrop_count`,
  `offscreen_pass_count`, `offscreen_targets_allocated`),
  `RendererDrawCounts::backdrop_blur_count`,
  `FrameStatistics::backdrop_blur_command_count`.

### Conventions

- `blur_radius` is the Gaussian standard deviation in logical pixels, like
  CSS `backdrop-filter: blur(r)`; this differs from the box-shadow blur
  radius, which is 2 sigma. It scales to device pixels with the DPI.
- Tint: `mix(blurred, tint.rgb, tint.a)` inside the mask; no tint mixes
  nothing. Composed opacity scales the mask, which has a one-device-pixel
  antialiased edge.
- The blur reads content up to 3 sigma outside the rect, clamped to the
  frame edge, so the panel edge is not darkened.
- Without support the frame draws the tint as a plain rounded rect. Vulkan
  reports `supported = false` with a reason when the swapchain format lacks
  optimal-tiling `COLOR_ATTACHMENT`, `SAMPLED_IMAGE` or
  `SAMPLED_IMAGE_FILTER_LINEAR`; backends without an override
  use the same tint-only default.

### Implementation and scheduling

- `vulkan_plan_backdrop_blur` (headless) computes, per visible backdrop, the
  region (device bounds of the transformed rect + 1 px, grown by
  `ceil(3 sigma)`, clamped), a downsample factor (1 up to sigma 4 device px,
  2 up to 16, 4 up to 48, else 8), the texel sigma
  `sqrt(sigma^2 - (f^2 - 1) / 12) / f` (removes the box filter's variance),
  a tap radius `ceil(3 sigma')` capped at 48, and a 6-vertex composite quad.
- A frame with at least one planned backdrop renders into an offscreen
  scene image (swapchain format) instead of the swapchain. The draw order
  is split at each backdrop: a scene pass (clear first, load later), then
  three small passes over the region (box downsample scene to ping,
  horizontal Gaussian ping to pong, vertical pong to ping); the next scene
  pass starts with the composite (ping sampled linearly, rounded mask,
  tint) and continues with the following draws. The swapchain pass copies
  the scene with `texelFetch`, so presentation and pixel capture are
  unchanged. Offscreen passes are render-pass compatible with the swapchain
  pass, so all existing pipelines draw into them; each ends with a barrier
  to `SHADER_READ_ONLY_OPTIMAL`.
- Multiple overlays: each backdrop blurs everything drawn before it,
  including earlier composites. No count limit; each adds four passes
  (`4 * backdrops + 1` offscreen passes per frame).
- Zero cost without a backdrop: the plan is cleared, recording keeps the
  single swapchain pass, and no image, pipeline or buffer exists until the
  first backdrop frame (asserted by the pixel test through
  `offscreen_pass_count == 0` and `offscreen_targets_allocated == false`).
  After that the three targets (about 14.6 MB at 1350 x 900), four
  pipelines and three render passes are kept and reused, and are recreated
  only when the swapchain format or extent changes.
- Backdrop frames are never replayed by command-buffer reuse.
- The six passes are entry points of `shaders/backdrop.slang`; their SPIR-V
  (`generated/backdrop_blur_spirv.inc`) is embedded in
  `vulkan_backdrop_blur_shader_binaries.cpp`.

### Tests

- `renderer_backdrop_blur_pixel_test` (Windows, selected Vulkan/DX12 real swapchain) over 2 px
  black/white stripes: every pixel of a row inside the panel is 128 +- 4
  (the stripe mean; sigma 4 removes the 4 px period); pixels outside and in
  the corner cut by the 12 px radius keep exact 0/255; a 50% red tint gives
  (192, 64, 64) +- 5; a rect drawn afterwards lands on top unblurred; two
  backdrops preserve their semantic counts and the second blurs a black square
  drawn after the first; scale 1.5 blurs inside and keeps outside exact; a
  frame without a backdrop reports 0 passes and no targets.
- Exact work topology is checked separately by
  `vulkan_backdrop_pass_diagnostics_test` (0/5/9 passes for zero/one/two effects)
  and `dx12_backdrop_pass_diagnostics_test` (0/3/6). They use explicit named
  factories; neither the counts nor historical Vulkan-only results are
  cross-backend performance/validation evidence. Earlier Vulkan measurements
  recorded `VUID-vkQueueSubmit-pSignalSemaphores-00067` with one `render_finished`
  semaphore for all swapchain images; that historical observation is not an
  allowance for validation errors in a new run.
- `vulkan_backdrop_blur_plan_test` (Windows and Linux, headless),
  `backdrop_blur_submission_test` (paint order, tint-only default),
  `backdrop_blur_structure_test`.

### Real window (Windows 11, 144 DPI)

`example_backdrop_blur` (`examples/backdrop_blur`, `examples/backdrop_blur/mcpp.toml`): 900 x 600 logical =
1350 x 900 physical client; 6 px colored stripes and 48 px text behind a
420 x 260 panel with radius 18, box shadow (offset 0,16, blur 48, black
45%), backdrop blur 20 with a 35% white tint, 25% white fill, 1 px border.
Captured 2026-11-17 with `D:\Dev\shots\shot.ps1` to
`D:\Dev\shots\c6_backdrop_blur.jpg` (window rect 1372 x 956):

- Inside the panel, row y = 550 (capture coordinates) from x = 420 to 940
  stays within 229..239 per channel, while the same stripes outside are full
  contrast (red 213,41,37 next to white 247..255). Text behind the panel is
  not legible; stripes read as soft pastel bands.
- Corners are rounded; stripes and text are unchanged up to the panel edge.
- Shadow below the panel at x = 1010: 218 at y = 690, then 223, 227, 232
  (y = 720), 241 (740), 246 (760), 249 (780), 250 (800, background).

### Measured frame time (debug build, 1350 x 900)

`CGPUI_EXAMPLE_BENCH_FRAMES=300`: 300 repainted frames after 10 warm-up
frames, mean renderer CPU stage timers (`RendererFrameTimings`), two runs:

| | total | pacing wait | resource prep | recording | offscreen passes |
|---|---|---|---|---|---|
| backdrop blur | 4.39 / 4.87 ms | 0.90 / 1.11 ms | 1.32 / 1.31 ms | 0.094 / 0.092 ms | 5 |
| no blur (`CGPUI_EXAMPLE_NO_BLUR=1`) | 4.63 / 4.58 ms | 0.83 / 0.79 ms | 1.31 / 1.32 ms | 0.023 / 0.024 ms | 0 |

Totals are within run-to-run noise; recording costs +0.07 ms. GPU time is
not measured directly (no timestamp queries); it shows only indirectly in
the next frame's pacing wait (+0.1 to 0.3 ms here).

### Not verified

- Wayland real window and Linux pixel output (only headless tests run in
  WSL); Metal: see below.
- The tint-only fallback on a device lacking the format features (none
  available; covered by the headless support test and the default
  `RenderFrame` test).
- GPU cost on an integrated GPU; large sigma (factor 8) visually; a
  release build.
- Box shadows in a real window beyond the example panel.

## Metal

Source: src/renderer/metal/metal_box_shadow.mm, metal_backdrop_blur.mm,
metal_frame_draws.mm; shaders in src/renderer/metal/shaders/cgpui_metal.slang
and the shared module src/renderer/shaders/cgpui_box_shadow.slang.

- Box shadow: the same CSS semantics and quad as Vulkan (sigma = blur / 2,
  spread radii, quad = shape + 3 sigma + 1 px, untransformed `local` point,
  composed opacity in the color). `cgpui_box_shadow_fragment` evaluates the
  same erf formula through `cgpui_box_shadow.slang`; the Vulkan shader keeps
  its own copy until its SPIR-V is regenerated and checked with spirv-val.
  Device pixels come from `renderer_to_device_pixels`, like every Metal draw.
- Backdrop blur: a frame with a backdrop draws into an offscreen scene
  texture (allocated once per drawable size). At each backdrop the encoder
  ends, `MPSImageGaussianBlur` (sigma = blur radius in device pixels, edge
  clamp) blurs the transformed rect's bounds plus 3 sigma into a second
  texture, and a new encoder on the scene composites the blurred texels
  under the rounded mask with `mix(blurred, tint, tint.a)` and the composed
  opacity. So each backdrop blurs what precedes it. At the end the scene is
  copied to the drawable without blending. Frames without a backdrop draw
  straight into the drawable and do no offscreen work.
  `backdrop_blur_support()` reports supported; the frame snapshot reports
  the backdrop count, one scene pass plus one per backdrop, and whether the
  targets exist.
- Tests (macOS 27.2.1, Apple M4, 2026-11-22): `metal_box_shadow_pixel_test`
  compares the falloff at scale 1 and 2, a rounded corner at scale 2 and a
  clip against the same references as `renderer_box_shadow_pixel_test`
  (tolerance 3 of 255). `metal_backdrop_blur_pixel_test` blurs 2 px stripes
  to mid gray (107..148) inside the backdrop, keeps them exact outside,
  mixes a 50 % red tint, keeps a rect drawn after the backdrop sharp, and
  checks the diagnostics of a frame with and without backdrops.
  On screen (2026-11-22, 1x display): `example_workbench` with
  `CGPUI_WORKBENCH_BLUR=1`, dark theme, right click on the tree. The menu
  shows its shadow and, through the translucent surface, the editor text
  behind it blurred beyond reading; outside the menu the text is sharp.
