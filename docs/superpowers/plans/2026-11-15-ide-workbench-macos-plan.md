# IDE Workbench Capabilities: macOS Plan

Date: 2026-11-15
Status: in progress on a Mac since 2026-11-21; see [Status](#status).
Companion to `2026-11-15-ide-workbench-capability-plan.md` (Windows and
Linux implementation).

## Starting point

Phase H delivered a Cocoa + Metal backend (`src/platform/macos/*.mm`,
`src/renderer/metal/*.mm`) that passed 380/380 on macOS 26.5.2 with Xcode 26.6.
The capabilities below add macOS implementations behind the same public APIs
the Windows and Linux work introduces. Public headers must stay free of
AppKit, CoreText, QuartzCore, and Metal types, as today.

Requirement for every step: a real Mac host with the toolchain recorded in
`docs/platform-mac-readiness.md`. Nothing here can be verified from Windows
or WSL. Until it runs on a Mac, each row stays "planned".

## Per-capability plan

| ID | macOS mechanism | Module(s) | Verification |
|---|---|---|---|
| C1 DPI | `NSWindow.backingScaleFactor`, `NSWindowDidChangeBackingPropertiesNotification`; macOS is always per-monitor aware. Report `PlatformDpiAwareness::per_monitor_aware_v2` equivalent with backend `"cocoa"` | `macos_window_lifecycle.mm` (existing scale path), new `macos_dpi_awareness.mm` | Move a window between a Retina and a non-Retina display; scale and atlas re-rasterization |
| C2 Appearance | `NSApp.effectiveAppearance` (`bestMatchFromAppearancesWithNames:` Aqua / DarkAqua), KVO on `effectiveAppearance`; `NSColor.controlAccentColor`; `NSWorkspace.accessibilityDisplayShouldReduceTransparency`, `...ShouldReduceMotion`, `...ShouldIncreaseContrast`; `NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification`; `NSSystemColorsDidChangeNotification` | new `macos_system_appearance.mm`, hook in `macos_application.mm` | Toggle System Settings > Appearance and Accessibility > Display; callback fires once per change |
| C3 Backdrop | `NSVisualEffectView` behind the Metal layer host (material `.sidebar`, `.windowBackground`, `.hudWindow`, `.popover`; blending `.behindWindow`), `CAMetalLayer.opaque = NO`, premultiplied clear; `reduceTransparency` falls back to opaque | `macos_window_chrome.mm`, `macos_content_view.mm`, `metal_surface.mm` | Pixel capture shows transparent clear; visual check of vibrancy in light/dark |
| C4 Title bar | `NSWindowStyleMaskFullSizeContentView`, `titlebarAppearsTransparent`, `titleVisibility = hidden`; keep native traffic lights and reposition via `standardWindowButton:` frames; caption-region dragging with `performWindowDragWithEvent:`; double-click honours `AppleActionOnDoubleClick` | `macos_window_chrome.mm`, `macos_pointer.mm` | Drag, double-click zoom/minimize, full-screen transition, traffic-light hit areas |
| C5 Shadow | Shared paint command; Metal pipeline for blurred rounded-rect shadow matching the Vulkan shader math (`box_shadow.frag.glsl`: sigma = blur / 2, erf x-integral + 4 y samples, CSS spread radii, clip-out inside the box, local point per vertex for transforms); implement `RenderFrame::draw_box_shadow` | `shaders.metal`, new `metal_shadow.mm` | Pixel capture compared with the Vulkan reference image within tolerance |
| C6 Backdrop blur | Override `RenderFrame::draw_backdrop_blur` and `Renderer::backdrop_blur_support` (the default draws the tint only). Match Vulkan: sigma = `blur_radius` in logical px (CSS `backdrop-filter`), region = rect bounds + 3 sigma, render the frame to an offscreen scene split at each backdrop, downsample + separable Gaussian (or `MPSImageGaussianBlur`), composite with rounded mask and `mix(blurred, tint, tint.a)`; each backdrop blurs everything before it; no offscreen work without a backdrop | new `metal_backdrop_blur.mm` | Pixel capture; frame-time budget on an integrated GPU |
| C7 Shaping | CoreText: `CTTypesetter` / `CTLineCreateWithAttributedString`, `CTRunGetGlyphs`, `CTRunGetAdvances`, `CTRunGetStringIndices`, font fallback via `CTFontCreateForString`; glyph rasterization with `CTFontDrawGlyphs` into the atlas (grayscale). Must feed the shared `NativeShapeBuilder` (see `docs/native-text.md`): per-run face identity (font file path + collection index), `TextFontMetrics`, space glyph and advance, glyphs in logical order with UTF-8 cluster ranges; and implement `rasterize_native_glyph_id` (bearings relative to the pen on the baseline, quarter-pixel subpixel shift) and `native_font_family_installed` | new `macos_text_shaping.mm`, `macos_text_glyph_raster.mm` | Same shaping fixtures as DirectWrite / HarfBuzz (Latin, CJK, combining marks, ligature off for code fonts); caret and hit-test parity tests |
| C8 IME | `NSTextInputClient`: `setMarkedText:selectedRange:replacementRange:` gives the composition caret; underline/clause styles from `NSUnderlineStyleAttributeName` and `NSMarkedClauseSegmentAttributeName`, filling the public `ImeComposition` fields from C8: `preedit_cursor_begin` / `preedit_cursor_end` (UTF-8 bytes) and one `ImePreeditStyleSpan` per clause with `ImeClauseStyle` (thick underline -> `selected_converted`, else `converted` / `input`; see `docs/ime-composition.md`) | `macos_text_input.mm`, `macos_ime_placement.mm` | Pinyin and Japanese IME: caret inside preedit, clause underline, candidate window position |
| C9 Widgets | Splitter / tabs / tree are cross-platform; tooltip and context menu need popups: child `NSPanel` (non-activating) for tooltip/popover, `NSMenu popUpMenuPositioningItem:` for native context menus | `macos_window_factory.mm` (popup kind), `macos_native_menu.mm` | Popup placement at screen edges, focus not stolen, dismissal on outside click |
| C10 SDK | `scripts/ci/macos-package.sh` mirroring the Windows/Linux packages: universal or arm64 static libs, headers, manifest with toolchain and commit | new script | Package consumed by a clean sample project on a second Mac |

## Status

Host: macOS 27.2.1 (26B131), Xcode 27.2 (27B52), Apple M4, one 1x display.

| ID | State |
|---|---|
| C1 DPI | done: `dpi_awareness()` reports `per_monitor_aware_v2` / `macos`; the existing backing-scale path sends `WindowResized` with the new scale. Moving a window between displays of different scales not observed (one display) |
| C2 Appearance | done: `macos_system_appearance.mm` (snapshot), `macos_appearance_monitor.mm` (KVO + notifications), see `docs/system-appearance.md`. System Settings toggles not tried; the test switches `NSApp.appearance` and posts the notifications |
| C3 Backdrop | done: `NSVisualEffectView` content view wrapping the Metal view, Metal transparent surface reports premultiplied alpha; see `docs/window-backdrop.md`. Material not observed through the workbench (its root background is opaque) |
| C4 Title bar | done: full-size content view with native traffic lights, caption drag and double-click, `caption_buttons_leading_reserved_width`; see `docs/client-title-bar.md`. Drag and double-click not driven by a test |
| C5 Shadow | done: Metal shadow pipeline with the Vulkan math (shared Slang module); pixel test against the Vulkan references at scale 1 and 2 |
| C6 Backdrop blur | done: offscreen scene, `MPSImageGaussianBlur` per backdrop, masked tint composite; pixel test; the workbench context menu observed blurred on screen |
| C7 Shaping | done: `core_text` backend and glyph-id rasterizer, Metal glyph placement shared with Vulkan; see `docs/native-text.md`. Text observed in `example_workbench` |
| C8 IME | done: clause spans from `NSMarkedClauseSegmentAttributeName` / `NSUnderlineStyleAttributeName`, see `docs/ime-composition.md`. Not checked with a real Pinyin or Japanese input method |

## Order on a Mac

1. C1 + C2 (small; validates the new public APIs on a third backend).
2. C3 + C4 (window shape).
3. C5 + C6 (Metal shaders; reuse Vulkan reference images).
4. C7 + C8 (CoreText shaping, then IME detail).
5. C9 popups, then C10.

## Risks

- `NSVisualEffectView` under a `CAMetalLayer` requires a layer-backed content
  view and a non-opaque layer; the existing content view setup must be checked
  before changing it.
- CoreText advances are in points; the atlas works in device pixels. Keep the
  conversion in one place, as on Windows.
- The Phase H evidence (380/380) is tied to a specific macOS and Xcode
  version. Record the version used for this work separately.

## Findings from the Windows rendering fixes (2026-11-17, code reading only)

- Device pixel scale: `macos_window.mm` reports the backing-store size
  (`convertRectToBacking`) as `framebuffer_size`, and the runtime paints the
  view at `framebuffer_size / backingScaleFactor`. `metal_geometry.mm` and
  `metal_image.mm` pass `state.framebuffer_size` as the viewport and use
  draw rects unchanged, and `metal_clip_transform.mm` clamps clips against
  the same size. On a Retina display Metal therefore has the bug fixed for
  Vulkan in `vulkan_device_pixels.cpp`: logical rects, clips and transform
  translations are drawn 1:1 in device pixels (half size at 2x), while
  text glyphs (already device pixels) are right. Fix by converting each
  recorded draw with `state.scale` in one place, matching
  `vulkan_to_device_pixels`. Fixed 2026-11-21: the conversion moved to
  `src/renderer/renderer_device_pixels.cpp` (`renderer_to_device_pixels`)
  and `MetalFrame` applies it to every recorded draw;
  `metal_device_pixel_scale_test` checks rects, radii, clips, transform
  translations and image bounds at scale 2 (it fails without the fix).
- Color encoding: Metal already uses `MTLPixelFormatBGRA8Unorm` for the layer
  and pipelines and `RGBA8Unorm` / `R8Unorm` textures, so it does not have
  the double sRGB encoding that the Vulkan `B8G8R8A8_SRGB` swapchain had.
