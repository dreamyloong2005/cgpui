# Box shadow and backdrop blur (C5, C6)

Both effects live in the shared Vulkan renderer (Win32 and Wayland). Metal
does not implement them yet; see the macOS plan.

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

- `vulkan_to_device_pixels(BoxShadowDraw)` scales rect, radii, offset,
  blur and spread by the surface DPI scale (like `530a6e96`).
- `vulkan_build_box_shadow_geometry` emits one quad per shadow covering the
  shape plus `3 sigma + 1` px, with composed opacity applied to the color
  and composed transforms applied to the corners. Each vertex also carries
  the untransformed local point, the shadow and box rects/radii and sigma.
- `shaders/box_shadow.frag.glsl` evaluates the closed-form Gaussian-blurred
  rounded rect from Evan Wallace's "Fast rounded rectangle shadows" (the
  approach GPUI/Zed uses): an erf approximation integrates along x exactly,
  4 samples integrate along y. Sigma < 0.05 uses a sharp signed-distance
  edge with a one-device-pixel ramp, so zero blur equals a solid rect.
- The pipeline shares the rounded rect layout, fixed state and blending;
  draws go through the existing draw-order cursor and pipeline-switch
  batching as `VulkanFrameDrawResourceKind::box_shadow`, so they keep paint
  order and clip scissors. Shadow buffers and draws are part of the
  command-reuse signature.
- SPIR-V is embedded in `vulkan_box_shadow_shader_binaries.cpp`, generated
  with `glslc -O --target-env=vulkan1.0` and checked with `spirv-val`
  (SDK 1.4.341.1), like the other embedded shaders. GLSL compilation is not
  a build step.

### Tests

- `vulkan_box_shadow_pixel_test` (Windows, real swapchain): falloff left of
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
