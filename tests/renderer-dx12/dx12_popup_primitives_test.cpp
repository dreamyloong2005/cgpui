#include "dx12_popup_test_support.hpp"
#include "cgpui/renderer/text/text_shape.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
// Exact Gaussian integral, independent of GPU planning and shader quadrature.
double interval_mass(double sample, double low, double high) {
  return 0.5 * (std::erf((high - sample) / std::sqrt(2.0)) -
                std::erf((low - sample) / std::sqrt(2.0)));
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !dx12_popup_desktop_ready())
    return dx12_fail(1, "popup primitives desktop/DPI/watchdog unavailable");
  CompositionTestWindows windows(48, 48);
  if (!windows.valid() || !IsWindowVisible(windows.popup()) || IsIconic(windows.popup()))
    return dx12_fail(2, "visible popup primitives fixture unavailable");
  auto renderer = create_dx12_renderer(dx12_popup_descriptor(windows.popup()));
  if (!renderer) return dx12_fail(3, renderer.error().message.c_str());
  const ImageAsset image{.id = {181}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {0, 0, 255, 128}}};
  const FontDescriptor font{.family = "Segoe UI"};
  const auto shaped = shape_text("M", font, 24, {1}, {.preferred_backend = TextShapingBackend::native});
  const TextDraw text{.bounds = {{4, 32}, {24, 28}}, .color = {1, 1, 1, 1},
      .font = font, .content = "M", .byte_length = 1, .font_size = 24,
      .scale = {1}, .device_font_size = 24,
      .glyphs = text_glyph_paint_metadata(shaped, {4, 32}),
      .clip_rect = Rect{{4, 32}, {24, 28}}};
  if (text.glyphs.empty() || text.glyphs.front().key.shaping_backend != TextShapingBackend::directwrite)
    return dx12_fail(4, "independently shaped native glyph unavailable");
  auto frame = (*renderer)->begin_frame();
  if (!frame) return dx12_fail(5, frame.error().message.c_str());
  (*frame)->clear({0, 0, 0, 0});
  (*frame)->upload_image(image);
  (*frame)->draw_image({.bounds = {{4, 4}, {16, 16}},
      .asset = describe_image_asset(image), .sampling = ImageSamplingMode::nearest});
  (*frame)->draw_text(text);
  // Translated shadow is [46,58)x[36,48), sigma=1. Its original box is
  // clipped out; all sampled shadow pixels lie outside that original box.
  (*frame)->draw_box_shadow({.rect = {{32, 36}, {12, 12}},
      .shadow = {.color = {0, 0, 0, 0.5F}, .offset = {14, 0}, .blur_radius = 2}});
  (*frame)->draw_rounded_rect({.rect = {{36, 4}, {20, 20}},
      .color = {1, 0, 0, 0.5F}, .radius = BorderRadii::all(6)});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(6, "mixed popup capture refused");
  if (const auto presented = (*frame)->present(); !presented)
    return dx12_fail(7, presented.error().message.c_str());
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!pixels || !pixels->valid() || pixels->width != 64 || pixels->height != 64)
    return dx12_fail(8, "mixed popup capture unavailable");
  for (unsigned y = 0; y < 64; ++y) {
    for (unsigned x = 0; x < 64; ++x) {
      const auto rgba = *pixels->pixel_rgba8(x, y);
      if (rgba[0] > rgba[3] + 1 || rgba[1] > rgba[3] + 1 || rgba[2] > rgba[3] + 1)
        return dx12_fail(9, "mixed popup has RGB exceeding premultiplied alpha");
    }
  }
  const auto edge_alpha = static_cast<unsigned char>(std::lround(
      127.5 * interval_mass(45.5, 46, 58) * interval_mass(42.5, 36, 48)));
  if (!dx12_pixel(pixels, 12, 12, {0, 0, 128, 128}) ||
      !dx12_pixel(pixels, 46, 14, {128, 0, 0, 128}) ||
      !dx12_pixel(pixels, 36, 4, {0, 0, 0, 0}) ||
      !dx12_pixel(pixels, 52, 42, {0, 0, 0, 128}, 2) ||
      !dx12_pixel(pixels, 45, 42, {0, 0, 0, edge_alpha}, 3) ||
      !dx12_pixel(pixels, 38, 42, {0, 0, 0, 0}) ||
      !dx12_pixel(pixels, 2, 2, {0, 0, 0, 0}) ||
      !dx12_pixel(pixels, 30, 30, {0, 0, 0, 0}))
    return dx12_fail(10, "image/rounded/shadow alpha or untouched transparent region differs");
  // Font outlines may vary. Check white premultiplied ink in its isolated
  // region, then choose an almost-solid glyph pixel for a literal white screen check.
  unsigned ink = 0, strongest = 0;
  POINT glyph{};
  for (unsigned y = 32; y < 60; ++y) {
    for (unsigned x = 4; x < 28; ++x) {
      const auto rgba = *pixels->pixel_rgba8(x, y);
      for (unsigned channel = 0; channel < 3; ++channel) {
        if (std::abs(int(rgba[channel]) - int(rgba[3])) > 1)
          return dx12_fail(11, "white glyph did not store premultiplied coverage");
      }
      ink += rgba[3] != 0;
      if (rgba[3] > strongest) { strongest = rgba[3]; glyph = {LONG(x), LONG(y)}; }
    }
  }
  if (ink < 10 || strongest < 248) return dx12_fail(12, "native text has insufficient visible coverage");
  if (!windows.expect({0, 255, 0}, windows.backdrop_point()) ||
      !windows.expect({0, 127, 128}, windows.popup_point(12, 12)) ||
      !windows.expect({128, 127, 0}, windows.popup_point(46, 14)) ||
      !windows.expect({0, 127, 0}, windows.popup_point(52, 42)) ||
      !windows.expect({0, 255 - edge_alpha, 0}, windows.popup_point(45, 42)) ||
      !windows.expect({255, 255, 255}, windows.popup_point(glyph.x, glyph.y)) ||
      !windows.expect({0, 255, 0}, windows.popup_point(36, 4)) ||
      !windows.expect({0, 255, 0}, windows.popup_point(30, 30)))
    return dx12_fail(13, "mixed popup primitives do not match the actual composed desktop");
  return 0;
}
