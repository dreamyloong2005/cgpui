# Native text

CGPUI shapes text with the platform text stack, measures it with real font
metrics, and rasterizes glyphs by glyph id into the Vulkan glyph atlas.
Paragraph layout, wrapping, caret positions, hit testing and selection all
consume the shaped advances. Applications keep calling `PaintList::fill_text`,
`measure_text` and `shape_text`; the public API is unchanged apart from
additive fields.

| Platform | Shaping | Fallback | Metrics | Rasterization |
| --- | --- | --- | --- | --- |
| Windows | DirectWrite `IDWriteTextAnalyzer` (`AnalyzeScript`, `AnalyzeBidi`, `GetGlyphs`, `GetGlyphPlacements`) | `IDWriteFontFallback::MapCharacters`; CJK prefers "Microsoft YaHei UI" unless the language is `ja`/`ko` | `IDWriteFontFace::GetMetrics` | `CreateGlyphRunAnalysis` on a one-glyph `DWRITE_GLYPH_RUN` |
| Linux | HarfBuzz `hb_shape` on file-backed faces | fontconfig `FcFontSort` + per-character `FcCharSetHasChar` | FreeType face, OS/2 table | FreeType `FT_Load_Glyph` + `FT_Render_Glyph` grayscale |
| macOS | deterministic fallback (no backend) | coverage-declared test chains only | none | placeholder |

## Backend selection

`TextShapingOptions::preferred_backend` defaults to `TextShapingBackend::native`,
which resolves to `directwrite` on Windows and `harfbuzz` on Linux.
`text_shaping_backend_capabilities()` reports `directwrite_available` and
`harfbuzz_available`. A platform without a compiled backend returns the
deterministic fallback with `fallback_reason = backend_unavailable`; a backend
that cannot open a face or shape returns `shaping_failed`. Explicit fallback
chains containing `FontSource::test` faces have no native font file and shape
deterministically with `unsupported_font_source`.

`set_default_text_shaping_backend(TextShapingBackend)` selects what `native`
requests use for the process. Tests that assert fixed half-em advances set
`deterministic_fallback` at the start of `main`; nothing else should need it.

## Fonts

`FontDescriptor` carries `family`, `weight` (CSS 1-999, default 400), `style`
(`normal` / `italic`) and `features` (OpenType tags, e.g.
`font_feature("liga", 0)`). An empty family or a generic name resolves through
`resolve_platform_font_family`:

| Request | Windows | Linux |
| --- | --- | --- |
| `""`, `sans-serif`, `system-ui` | Segoe UI | fontconfig `sans-serif` |
| `monospace`, `ui-monospace` | Cascadia Mono when installed, else Consolas | fontconfig `monospace` |
| `serif` | Times New Roman | fontconfig `serif` |

`default_ui_font_descriptor()` and `default_code_font_descriptor()` return
these defaults; the code descriptor disables `liga` and `calt`. Any monospaced
face (DirectWrite `IsMonospacedFont`, fontconfig `FC_SPACING >= FC_MONO`) also
disables `liga` and `calt` unless `features` sets them explicitly, so `->` and
`!=` stay two cells in code.

## Shaping output

`TextShapeRun` for native backends contains:

- `glyphs` in logical order: font-specific `glyph_id`, `advance` and y-down
  `offset` in logical pixels, the UTF-8 byte range of the glyph's cluster
  (all glyphs of one cluster share it), and `font_fallback_face_index`.
- `font_fallback_faces`: the resolved faces, primary first, each with family,
  weight, style, file `path`, collection `face_index`, `synthetic_bold`,
  `synthetic_oblique` and `monospaced`.
- `font_runs`: contiguous runs per face, each with its `TextFontMetrics`.
- `font_metrics` of the primary face: ascent, descent, line gap, cap height,
  x-height, underline position (below the baseline) and thickness.
- `script_runs`, `missing_glyphs` (glyph id 0 for a visible character) and
  `color_glyphs` (emoji code points).

The shared `NativeShapeBuilder` (`src/ui/text_shaping_native.cpp`) turns the
backend's per-face glyphs into this form for both platforms. Tabs advance to
the next stop of `TextShapingOptions::tab_width_in_spaces` (default 4) space
advances of the primary face, measured from the start of the run, and draw
the space glyph. Other control characters get zero advance.

Itemization: DirectWrite splits by script and resolved bidi level, then by
`MapCharacters` font runs. Linux splits by fontconfig face (marks, ZWJ and
variation selectors stay with their base) and by Unicode script. Right-to-left
runs are shaped right-to-left and stored in logical order.

## Layout, caret, hit testing and selection

- `text_line_metrics_for_shape_run` builds the line box from the tallest face
  on the run: ascent and descent plus half of each face's line gap (CSS
  half-leading). `line_height` and `baseline` follow; device values are the
  logical values times the DPI scale. The deterministic fallback keeps
  `line_height = font_size` and the 0.8 baseline.
- `build_text_grapheme_columns` segments graphemes from the text (CRLF,
  combining marks, variation selectors, ZWJ sequences, flag pairs) and assigns
  each shaped cluster's advance to the graphemes it covers. A ligature covering
  several graphemes divides its advance evenly between them; the first owns the
  glyphs.
- Wrapping keeps the existing grapheme-level greedy breaks and hard breaks;
  widths are the column advances, so a wrapped line's width is the sum of its
  shaped advances.
- `hit_test_text_position` walks grapheme columns with the midpoint rule, so a
  hit lands on a grapheme boundary, including inside a ligature.
- Caret x (`text_caret_rect`) and selection rectangles
  (`paint_text_selection_ranges`) sum the same column advances, across fallback
  font runs.

## Caching

- Shape cache (`include/cgpui/ui/text_shape_cache.hpp`,
  `src/ui/text_shape_cache.cpp`): natively shaped runs are cached per thread,
  keyed by text, full `FontDescriptor` (family, weight, style, features), size,
  DPI scale, backend, direction, script, language and tab width. Lookups use a
  borrowed key view, so a hit does not allocate before the result copy.
  Least-recently-used eviction at 2048 entries by default
  (`set_text_shape_cache_capacity`). `text_shape_cache_statistics()` reports
  entries, lookups, hits, misses and evictions. Editing one line of an editor
  reshapes only that line's string. The deterministic fallback is not cached.
- Font resolution: DirectWrite faces reopened for rasterization are cached by
  file, index and simulations; fontconfig fallback sets are cached per family,
  weight and style; FreeType and HarfBuzz faces are cached per file and index.
- `TextMeasurementCache` and `TextParagraphLayoutCache` keep their existing
  keys; `FontDescriptor` now includes weight, style and features, so those
  caches separate them automatically.

## Rasterization by glyph id

`text_glyph_paint_metadata` gives native glyphs their face identity
(`GlyphAtlasKey::font_face_path`, `font_face_index`, `synthetic_bold`,
`synthetic_oblique`) and `TextGlyphPaint::device_baseline`, the line baseline
below the glyph's device origin. The Vulkan quad builder
(`src/renderer/vulkan/vulkan_text_glyph_placement.cpp`) then:

1. clears the cluster text, glyph index and byte range from the atlas key, so
   the key is glyph id + face + font size + DPI scale + subpixel bucket;
2. snaps the pen to a whole device pixel and stores the remainder as one of
   four quarter-pixel buckets (`GlyphAtlasKey::subpixel_x`);
3. snaps the baseline to a whole device row.

`rasterize_text_glyph` dispatches `directwrite` glyphs to
`src/ui/text_glyph_id_raster_win32.cpp` and `harfbuzz` glyphs on Linux to
`src/ui/text_glyph_id_raster_freetype.cpp`. Both return a tight grayscale
bitmap whose bearings are relative to the pen on the baseline. DirectWrite
ClearType coverage is averaged to one channel; the atlas stores coverage only.
Deterministic-fallback glyphs keep the earlier cell-fitted path
(`text_glyph_raster_win32.cpp` lays out the source cluster with DirectWrite on
Windows; other platforms draw placeholders). Windows rejects HarfBuzz glyph ids
with `unsupported_platform`.

## Module ownership

All shaping, metrics, cache and rasterization sources are linked by
`cgpui_renderer` through `build/xmake/capabilities/native_text_shaping_targets.lua`
and removed from the `cgpui_ui` wildcard. Windows links `dwrite`; Linux uses the
system xmake packages `harfbuzz`, `freetype` and `fontconfig` and defines
`CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND` and `CGPUI_HAS_FREETYPE_GLYPH_RASTERIZER`.
Windows and HarfBuzz/FreeType/fontconfig types stay in `src/ui/*_internal.hpp`;
`native_text_shaping_structure_test` guards line limits, header cleanliness and
which module calls which native API.

## Limits

- Color emoji are not rendered in color. Emoji shape and fall back to a color
  font (Segoe UI Emoji, Noto Color Emoji when installed), and `color_glyphs`
  records them, but the atlas holds grayscale coverage only, so on Windows they
  are expected to draw as monochrome outlines (not visually verified). FreeType renders only outline glyphs
  (`FT_LOAD_NO_BITMAP`), so bitmap-only color fonts produce empty ink on Linux.
- Bidirectional text is shaped per bidi run with the correct direction, but
  lines are not visually reordered: right-to-left runs are painted in logical
  order, as before this change.
- Line breaks are still grapheme-level greedy breaks, not UAX #14 word breaks.
- Tab stops are measured from the start of the shaped string, not from the
  wrapped line start.
- Variable-font axes other than fontconfig named instances are not applied;
  DirectWrite uses the face's default instance.
- Rasterization takes the FreeType lock for every glyph on Linux; glyphs are
  cached in the atlas, so this is per new glyph, not per frame.
- macOS has no native backend yet; it reports `backend_unavailable` and keeps
  the deterministic fallback.

## Measured results (2026-11-18)

| Check | Windows (DirectWrite) | Linux WSL2 Arch (HarfBuzz) |
| --- | --- | --- |
| "Floating panel" at 28 px: shaped width vs platform layout width | 175.355 vs 175.355 px (Segoe UI, `IDWriteTextLayout`) | 199.078 vs 199.078 px (DejaVu Sans, direct `hb_shape`) |
| CJK fallback face for U+4E2D U+6587 | Microsoft YaHei UI | WenQuanYi Zen Hei |
| Code font cell at 16 px | Cascadia Mono, 9.375 px | fontconfig `monospace` (DejaVu Sans Mono), 9.641 px |
| Widest gap between letter ink inside a word, "Floating panel" 28 px, scale 1 / 1.5 | 3 / 5 px | 4 / 7 px |
| Same, deterministic fixed-advance fallback | 11 px | not measured |

`native_text_shaping_test` covers the width comparison (0.5 px tolerance),
font metrics, CJK fallback runs with full-em advances, combining marks as one
cluster, `liga` on/off with Calibri (Windows) / DejaVu Sans (Linux) and hit
testing inside the `ffi` ligature, monospace columns and tab stops, caret and
hit-test round trips, selection across fallback runs, wrapping widths, DPI 1.0 /
1.5 and the shape cache. `native_text_glyph_spacing_test` measures ink gaps
through the production Vulkan quad builder at scale 1 and 1.5 with a 0.2 em
limit. `vulkan_text_ink_extent_pixel_test` (Windows) renders "blur", "f",
"W", "j" and italics at 48 px, scale 1 and 1.5, and requires the rendered ink
extent to match the glyph bitmaps on both sides (no clip at the last pen
position or at a negative left bearing).

Real window: `example_backdrop_blur` captured on Windows at 1350 x 900 shows
"CGPUI backdrop blur" and "Floating panel" with natural proportional spacing
(no gaps after l, i, r) and the Chinese line in Microsoft YaHei UI at full
width. The final "r" of the title looks cut at 150 % because its arm ends at
logical x 484, over the black stripe at 480..486; drawing the title in green
shows the arm's ink continuing over the stripe (2026-11-18). The Linux Wayland window was not captured.
