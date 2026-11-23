#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"
#include "cgpui/renderer/planning/glyph_geometry.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "mixed shadow window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  const FontDescriptor font{.family = "Segoe UI"};
  const auto run = shape_text("M", font, 24, {1},
      {.preferred_backend = TextShapingBackend::deterministic_fallback});
  const TextDraw text{.bounds = {{12, 12}, {40, 32}}, .color = {1, 1, 1, 1},
      .font = font, .content = "M", .byte_length = 1, .font_size = 24,
      .scale = {1}, .device_font_size = 24,
      .glyphs = text_glyph_paint_metadata(run, {12, 12})};
  // Only font coverage uses the native CPU rasterizer, never shadow geometry.
  GlyphCache reference;
  const auto quads = build_textured_glyph_quads(text, reference);
  if (quads.size() != 1 || reference.upload_records().size() != 1)
    return dx12_fail(3, "mixed shadow reference glyph failed");
  const auto& mask = reference.upload_records()[0];
  const int left = static_cast<int>(quads[0].device_bounds.origin.x);
  const int top = static_cast<int>(quads[0].device_bounds.origin.y);
  if (left < 10 || top < 10 || left + mask.width > 38 || top + mask.height > 38 ||
      mask.width < 6 || mask.height < 6)
    return dx12_fail(4, "mixed shadow glyph does not fit oracle regions");
  const auto middle = mask.width / 3, last = mask.width * 2 / 3;
  const ImageAsset red{.id = {8101}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 255}}};
  const ImageAsset yellow{.id = {8102}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 255, 0, 255}}};
  auto frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return dx12_fail(5, "mixed shadow begin failed");
  (*frame)->clear({1, 1, 1, 1});
  (*frame)->upload_image(red);
  (*frame)->upload_image(yellow);
  (*frame)->draw_image({.bounds = {{0, 0}, {64, 64}}, .asset = describe_image_asset(red)});
  (*frame)->draw_box_shadow({.rect = {{40, 40}, {32, 32}},
      .shadow = {.color = {0, 0, 0, 1}, .offset = {-32, -32}}});
  (*frame)->draw_text(text);
  const float blue_left = static_cast<float>(left + last);
  const float blue_width = static_cast<float>(mask.width - last);
  (*frame)->draw_box_shadow({.rect = {{46, 42}, {blue_width, static_cast<float>(mask.height)}},
      .shadow = {.color = {0, 0, 1, 1}, .offset = {blue_left - 46, float(top) - 42}}});
  (*frame)->draw_rect({.rect = {{float(left + middle), float(top)},
      {float(last - middle), float(mask.height)}}, .color = {0, 1, 0, 1}});
  (*frame)->draw_image({.bounds = {{blue_left, float(top + mask.height / 2)},
      {blue_width, float(mask.height - mask.height / 2)}}, .asset = describe_image_asset(yellow)});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(6, "mixed shadow capture failed");
  if (auto result = (*frame)->present(); !result) return dx12_fail(7, result.error().message.c_str());
  const auto* pixels = (*renderer)->last_frame_pixels();
  unsigned ink[3]{};
  for (unsigned y = 0; y < mask.height; ++y) {
    for (unsigned x = 0; x < mask.width; ++x) {
      const auto alpha = mask.alpha[std::size_t(y) * mask.stride + x];
      std::array<unsigned char, 4> expected{alpha, alpha, alpha, 255};
      if (x < middle) ink[0] += alpha != 0;
      else if (x < last) { ink[1] += alpha != 0; expected = {0, 255, 0, 255}; }
      else {
        ink[2] += alpha != 0;
        expected = y < mask.height / 2 ? std::array<unsigned char, 4>{0, 0, 255, 255}
                                      : std::array<unsigned char, 4>{255, 255, 0, 255};
      }
      if (!dx12_pixel(pixels, left + x, top + y, expected))
        return dx12_fail(8, "image/shadow/text/shadow/rect/image order differs");
    }
  }
  return ink[0] > 2 && ink[1] > 2 && ink[2] > 2 &&
      dx12_pixel(pixels, 2, 2, {255, 0, 0, 255}) &&
      dx12_pixel(pixels, 9, 9, {0, 0, 0, 255})
      ? 0 : dx12_fail(9, "mixed shadow coverage did not exercise all bindings");
}
