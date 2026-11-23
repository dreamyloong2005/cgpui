#include "dx12_test_support.hpp"
#include "cgpui/renderer/planning/glyph_geometry.hpp"

#include <cmath>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "mixed primitives window failed");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());

  const FontDescriptor font{.family = "Segoe UI"};
  const auto shaped = shape_text("M", font, 24, {1},
      {.preferred_backend = TextShapingBackend::deterministic_fallback});
  const TextDraw text{.bounds = {{8, 8}, {40, 32}}, .color = {1, 1, 1, 1},
      .font = font, .content = "M", .byte_length = 1, .font_size = 24,
      .scale = {1}, .device_font_size = 24,
      .glyphs = text_glyph_paint_metadata(shaped, {8, 8})};
  // Only font coverage comes from the public CPU rasterizer. Expected paint
  // order and colors below are literal and do not reuse GPU draw planning.
  GlyphCache reference;
  const auto quads = build_textured_glyph_quads(text, reference);
  if (quads.size() != 1 || reference.upload_records().size() != 1) {
    return dx12_fail(3, "mixed primitives reference glyph failed");
  }
  const auto& mask = reference.upload_records().front();
  const auto bounds = quads[0].device_bounds;
  const int left = static_cast<int>(std::lround(bounds.origin.x));
  const int top = static_cast<int>(std::lround(bounds.origin.y));
  if (bounds.origin.x != left || bounds.origin.y != top ||
      mask.width < 6 || mask.height < 4 || left < 4 || top < 4 ||
      left + static_cast<int>(mask.width) > 32 ||
      top + static_cast<int>(mask.height) > 36) {
    return dx12_fail(4, "reference mask does not fit independent test regions");
  }
  const unsigned middle = mask.width / 3;
  const unsigned last = mask.width * 2 / 3;
  const ImageAsset red{.id = {71}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4,
          .pixels = {255, 0, 0, 255}}};
  const ImageAsset yellow{.id = {72}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4,
          .pixels = {255, 255, 0, 255}}};
  auto frame = (*renderer)->begin_frame();
  if (!frame) return dx12_fail(5, frame.error().message.c_str());
  (*frame)->clear({0, 0, 0, 1});
  (*frame)->upload_image(red);
  (*frame)->upload_image(yellow);
  (*frame)->draw_rect({.rect = {{0, 0}, {64, 64}}, .color = {0, 0, 1, 1}});
  // Force image -> glyph atlas -> untextured rect -> different image bindings.
  (*frame)->draw_image({.bounds = {{4, 4}, {56, 56}},
      .asset = describe_image_asset(red), .sampling = ImageSamplingMode::nearest});
  (*frame)->draw_text(text);
  (*frame)->draw_rect({
      .rect = {{static_cast<float>(left + middle), static_cast<float>(top)},
               {static_cast<float>(last - middle), static_cast<float>(mask.height)}},
      .color = {0, 1, 0, 1}});
  (*frame)->draw_image({
      .bounds = {{static_cast<float>(left + last), static_cast<float>(top)},
                 {static_cast<float>(mask.width - last), static_cast<float>(mask.height)}},
      .asset = describe_image_asset(yellow), .sampling = ImageSamplingMode::nearest});
  // Exercise both textured -> rounded and rounded -> textured -> solid paths.
  (*frame)->draw_rounded_rect({.rect = {{36, 36}, {20, 20}},
      .color = {0, 1, 1, 1}, .radius = BorderRadii::all(4)});
  (*frame)->draw_image({.bounds = {{44, 42}, {4, 12}},
      .asset = describe_image_asset(red), .sampling = ImageSamplingMode::nearest});
  (*frame)->draw_rect({.rect = {{46, 46}, {4, 4}}, .color = {0, 0, 1, 1}});
  auto capture = (*frame)->request_pixel_capture();
  if (!capture) return dx12_fail(6, capture.error().message.c_str());
  auto presented = (*frame)->present();
  if (!presented) return dx12_fail(7, presented.error().message.c_str());
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!pixels || !pixels->valid() || pixels->encoding != RendererFramePixelEncoding::linear) {
    return dx12_fail(8, "mixed primitives needs valid linear readback");
  }

  unsigned visible_ink = 0, rect_covered_ink = 0, image_covered_ink = 0;
  for (unsigned y = 0; y != mask.height; ++y) {
    for (unsigned x = 0; x != mask.width; ++x) {
      const auto alpha = mask.alpha[std::size_t(y) * mask.stride + x];
      // White text over opaque red yields (255, coverage, coverage).
      std::array<unsigned char, 4> expected{255, alpha, alpha, 255};
      if (x < middle) visible_ink += alpha != 0;
      else if (x < last) {
        expected = {0, 255, 0, 255};
        rect_covered_ink += alpha != 0;
      } else {
        expected = {255, 255, 0, 255};
        image_covered_ink += alpha != 0;
      }
      if (!dx12_pixel(pixels, left + x, top + y, expected)) {
        return dx12_fail(9, "image/text/rect/image ordering or rebinding differs");
      }
    }
  }
  if (visible_ink < 3 || rect_covered_ink < 3 || image_covered_ink < 3) {
    return dx12_fail(10, "reference did not exercise text in all three order regions");
  }
  if (!dx12_pixel(pixels, 2, 2, {0, 0, 255, 255}) ||
      !dx12_pixel(pixels, 5, 5, {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, 59, 59, {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, 38, 44, {0, 255, 255, 255}) ||
      !dx12_pixel(pixels, 54, 45, {0, 255, 255, 255}) ||
      !dx12_pixel(pixels, 45, 43, {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, 45, 47, {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, 47, 47, {0, 0, 255, 255}) ||
      !dx12_pixel(pixels, 49, 49, {0, 0, 255, 255})) {
    return dx12_fail(11, "rectangle/rounded/image order or pipeline restoration differs");
  }
  return 0;
}
