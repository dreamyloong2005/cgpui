#include "text_glyph_id_raster_internal.hpp"

#include "text_coretext_macos_internal.hpp"

#include <CoreGraphics/CoreGraphics.h>

#include <cmath>
#include <vector>

namespace cgpui {
namespace {

Error raster_error(const char* message) {
  return Error{.code = ErrorCode::renderer_initialization_failed,
               .message = std::string("Core Text glyph rasterization: ") + message};
}

}  // namespace

// Draws the glyph in white on black into an 8-bit gray bitmap with
// antialiasing and no font smoothing, so the bytes are linear coverage.
// Core Graphics is y-up with the origin at the bottom left; the bitmap
// memory starts at the top row.
Result<RasterizedGlyph> rasterize_native_glyph_id(const TextGlyphPaint& glyph) {
  const GlyphAtlasKey& key = glyph.key;
  CTFontRef font = core_text_font_for_face(key.font_face_index, key.device_font_size);
  if (font == nullptr) return std::unexpected(raster_error("font face is unavailable"));
  const auto id = static_cast<CGGlyph>(key.glyph_id);
  const CGFloat shift = static_cast<CGFloat>(key.subpixel_x) * 0.25;
  CGRect ink{};
  CTFontGetBoundingRectsForGlyphs(font, kCTFontOrientationHorizontal, &id, &ink, 1);
  RasterizedGlyph output{
      .key = key,
      .bitmap = {.width = 1, .height = 1, .stride = 1, .alpha = {0}},
      .advance = glyph.device_advance,
      .device_font_size = key.device_font_size};
  if (CGRectIsEmpty(ink) || CGRectIsNull(ink)) {
    CFRelease(font);
    return output;
  }
  // One pixel of padding on each side for antialiased edges.
  const double left = std::floor(ink.origin.x + shift) - 1.0;
  const double right = std::ceil(ink.origin.x + ink.size.width + shift) + 1.0;
  const double bottom = std::floor(ink.origin.y) - 1.0;
  const double top = std::ceil(ink.origin.y + ink.size.height) + 1.0;
  const auto width = static_cast<std::size_t>(right - left);
  const auto height = static_cast<std::size_t>(top - bottom);
  if (width == 0 || height == 0 || width > 4096 || height > 4096) {
    CFRelease(font);
    return std::unexpected(raster_error("unexpected glyph bounds"));
  }
  std::vector<std::uint8_t> pixels(width * height);
  CGColorSpaceRef gray = CGColorSpaceCreateDeviceGray();
  CGContextRef context = CGBitmapContextCreate(
      pixels.data(), width, height, 8, width, gray, kCGImageAlphaNone);
  CGColorSpaceRelease(gray);
  if (context == nullptr) {
    CFRelease(font);
    return std::unexpected(raster_error("bitmap context creation failed"));
  }
  CGContextSetAllowsAntialiasing(context, true);
  CGContextSetShouldAntialias(context, true);
  CGContextSetAllowsFontSmoothing(context, false);
  CGContextSetShouldSmoothFonts(context, false);
  CGContextSetAllowsFontSubpixelPositioning(context, true);
  CGContextSetShouldSubpixelPositionFonts(context, true);
  CGContextSetAllowsFontSubpixelQuantization(context, false);
  CGContextSetShouldSubpixelQuantizeFonts(context, false);
  CGContextSetGrayFillColor(context, 1.0, 1.0);
  const CGPoint origin{shift - left, -bottom};
  CTFontDrawGlyphs(font, &id, &origin, 1, context);
  CGContextRelease(context);
  CFRelease(font);
  output.bitmap = GlyphBitmap{.width = static_cast<std::uint32_t>(width),
                              .height = static_cast<std::uint32_t>(height),
                              .stride = static_cast<std::uint32_t>(width),
                              .alpha = std::move(pixels)};
  output.left_bearing = static_cast<float>(left);
  output.top_bearing = -static_cast<float>(top);
  return output;
}

}  // namespace cgpui
