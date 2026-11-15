# Native text rasterization

CGPUI's Windows Vulkan renderer rasterizes deterministic-fallback text clusters
with DirectWrite, then uploads their alpha coverage through the standard glyph
atlas. Applications continue to submit `TextDraw` commands using
`create_renderer`; no application-specific renderer wrapper or image conversion
is required.

This capability supplies real glyph silhouettes. It does not replace the text
shaper, layout measurements, wrapping, caret movement, or selection geometry.

## Module ownership

| Module | Responsibility |
| --- | --- |
| `include/cgpui/ui/text_glyph_rasterizer.hpp` | Platform-neutral `Result<RasterizedGlyph> rasterize_text_glyph(const TextGlyphPaint&, std::string_view)` contract and coordinate conventions. |
| `src/ui/text_glyph_rasterizer.cpp` | Input bounds, empty-source handling, platform dispatch, and the Windows shaping-backend guard. |
| `src/ui/text_glyph_raster_win32.cpp` | UTF-8 conversion, DirectWrite text layout and font fallback, fitting coverage to the supplied cell, baseline metrics, and cropping to ink. |
| `src/ui/text_glyph_coverage_win32.cpp` | `IDWriteTextRenderer`, glyph-run analysis, and grayscale alpha extraction. |
| `src/ui/text_glyph_*_win32_internal.hpp` | Private layout/coverage interfaces; Windows declarations stay out of the public leaf. |
| `src/renderer/vulkan/vulkan_report_text_quads.cpp` | Canonical cache identity, rasterizer invocation, and positioning atlas quads with native bearings. |
| `src/renderer/glyph_rasterization_error.cpp` | First-error storage and reset on `GlyphCache`. |
| `src/renderer/vulkan/vulkan_glyph_atlas_frame.cpp` | Frame-level error propagation and the existing atlas upload and draw-resource preparation. |

The rasterization implementation is linked by `cgpui_renderer`; Windows also
links `dwrite`. The UI wildcard excludes these renderer-owned implementation
files. Aggregate headers remain thin.

## Rendering path

1. `PaintList::fill_text` produces text content, shaping metadata, glyph origins,
   and wrap lines. `render_view` forwards those into `RenderFrame::draw_text`.
2. Vulkan resolves the source UTF-8 cluster and its canonical atlas key. A cache
   miss calls `rasterize_text_glyph`; an existing allocation reuses its coverage
   and positioning metrics.
3. On Windows, DirectWrite lays out that source cluster with the requested font
   family. Its platform fallback can select another installed font for Chinese
   and other characters absent from the requested family.
4. `CreateGlyphRunAnalysis` generates coverage at the requested device scale.
   ClearType's three coverage channels are averaged into grayscale alpha, then
   overlapping glyph runs are combined with source-over alpha. The atlas stores
   coverage only; text color and opacity remain draw properties.
5. The tight coverage bitmap enters `GlyphCache`, `GlyphUploadRecord`, and the
   existing alpha atlas upload plan. Vulkan draws textured glyph quads through
   its text pipeline, preserving clipping and composition metadata.

The current deterministic shaper supplies a fixed advance. Native outlines are
fitted horizontally when they exceed that cell and vertically to its line cell,
so this rasterization change does not move the existing caret or wrapping
positions. Wide characters can therefore look compressed until native shaping
and matching interaction metrics are implemented.

## Coordinates, whitespace, and cache identity

Bitmap dimensions, advance, bearings, and baseline are device pixels.
`left_bearing` and `top_bearing` locate the cropped bitmap's top-left relative to
the glyph cell's top-left. Vulkan adds those bearings to `glyph.device_origin`;
it does not add the baseline again. The baseline is measured downward from the
same cell origin and is retained separately in the atlas entry.

DPI changes cause coverage to be rasterized at the corresponding device
resolution. Both scale and device font size participate in the cache key. The
key also includes source UTF-8, font family, logical font size, shaping backend,
and fitted device advance. Source text prevents different characters with the
fallback shaper's positional glyph ID from sharing coverage. Canonicalization
removes positional offsets and indices for real source text, and clears that
positional glyph ID only for the deterministic backend, so repeated characters
can reuse allocations across draw positions.

Windows whitespace and zero-advance cells produce a transparent 1x1 bitmap with
no ink. A zero-advance cell retains its DirectWrite baseline without requesting
a degenerate glyph-analysis transform. An empty source is a metadata-only
geometry case: it preserves transparent cell dimensions rather than inventing
a visible placeholder glyph.

## Errors and support limits

Invalid or excessive dimensions are rejected before allocation. Invalid UTF-8,
DirectWrite layout/coverage failures, and unsupported Windows shaping input
return an error. Vulkan records the first rasterization error in `GlyphCache`
and returns it from glyph-atlas frame preparation; Windows does not silently
replace failed native coverage with filled rectangles. The error state is reset
when preparing the next frame.

The Windows implementation accepts source clusters produced by
`TextShapingBackend::deterministic_fallback`. Nonempty HarfBuzz input explicitly
returns `unsupported_platform`: font-face identity, native glyph indices,
contextual shaping, and multiple glyphs belonging to one cluster need a separate
integration. This implementation must not be described as complete native
shaping, general complex-script rendering, or color-emoji rendering.

macOS and Linux do not yet have native rasterization in this module. They retain
the deterministic placeholder backend, with ASCII whitespace made transparent.
Windows behavior is not evidence of native text support on those platforms.

## Validation boundaries

`native_text_rasterization_test` exercises the production atlas path without a
window: distinct Latin and Chinese masks, transparent whitespace, DPI, cache
identity, bearings, error behavior, and atlas uploads. The structure and header
cleanliness tests guard module ownership and the public include boundary.

Those checks do not replace a real Vulkan framebuffer capture or visual
verification in an application. `vulkan_text_pixel_test` provides a small GPU
text-capture entry point. Validation results should be recorded only after the
corresponding tests and application checks have actually run.
