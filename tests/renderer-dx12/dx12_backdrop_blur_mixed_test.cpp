#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"
#include "cgpui/renderer/planning/glyph_geometry.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "mixed backdrop window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  const FontDescriptor font{.family = "Segoe UI"};
  const auto run = shape_text("M", font, 20, {1},
      {.preferred_backend = TextShapingBackend::deterministic_fallback});
  TextDraw before{.bounds = {{8, 8}, {24, 32}}, .color = {1, 1, 1, 1},
      .font = font, .content = "M", .byte_length = 1, .font_size = 20,
      .scale = {1}, .device_font_size = 20,
      .glyphs = text_glyph_paint_metadata(run, {8, 8})};
  TextDraw after = before;
  after.bounds.origin.x = 36;
  after.glyphs = text_glyph_paint_metadata(run, {36, 8});
  // Native font coverage is the sole shared oracle. Colors, compositing order
  // and shadow locations below are independent literal/linear-blend examples.
  GlyphCache reference;
  const auto before_quads = build_textured_glyph_quads(before, reference);
  const auto after_quads = build_textured_glyph_quads(after, reference);
  if (before_quads.size() != 1 || after_quads.size() != 1 || reference.upload_records().size() != 1)
    return dx12_fail(3, "mixed backdrop reference glyph failed");
  const auto& mask = reference.upload_records()[0];
  const auto first = before_quads[0].device_bounds.origin;
  const auto last = after_quads[0].device_bounds.origin;
  if (first.x < 4 || first.y < 4 || first.x + mask.width >= 32 ||
      first.y + mask.height >= 40 || last.x < 32 || last.x + mask.width >= 64 ||
      last.y + mask.height >= 40 || mask.width < 4 || mask.height < 4)
    return dx12_fail(4, "mixed backdrop glyph does not fit oracle regions");
  const ImageAsset red{.id = {8201}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 255}}};
  const ImageAsset yellow{.id = {8202}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 255, 0, 255}}};
  auto frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return dx12_fail(5, "mixed backdrop begin failed");
  (*frame)->clear({1, 1, 1, 1});
  (*frame)->upload_image(red);
  (*frame)->upload_image(yellow);
  (*frame)->draw_image({.bounds = {{0, 0}, {64, 64}}, .asset = describe_image_asset(red)});
  (*frame)->draw_box_shadow({.rect = {{80, 80}, {28, 36}},
      .shadow = {.color = {0, 0, 0, 1}, .offset = {-76, -76}}});
  (*frame)->draw_text(before);
  (*frame)->draw_backdrop_blur({.rect = {{0, 0}, {64, 64}},
      .blur_radius = 0, .tint = Color{0, 0, 1, 0.5F}});
  (*frame)->draw_rect({.rect = {{32, 0}, {32, 64}}, .color = {0, 1, 0, 1}});
  (*frame)->draw_backdrop_blur({.rect = {{0, 0}, {64, 64}},
      .blur_radius = 0, .tint = Color{1, 1, 1, 0.5F}});
  (*frame)->draw_image({.bounds = {{4, 46}, {16, 12}}, .asset = describe_image_asset(yellow)});
  (*frame)->draw_text(after);
  (*frame)->draw_box_shadow({.rect = {{80, 80}, {16, 12}},
      .shadow = {.color = {0, 0, 1, 1}, .offset = {-40, -32}}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(6, "mixed backdrop capture refused");
  if (auto result = (*frame)->present(); !result)
    return dx12_fail(7, result.error().message.c_str());
  const auto* pixels = (*renderer)->last_frame_pixels();
  // Sigma zero isolates snapshot/command order, not blur-kernel accuracy
  // (covered by Gaussian effects tests). The second effect must see BOTH the
  // first effect's purple and the intervening green, not a frame-start copy.
  if (!dx12_pixel(pixels, 2, 44, {191, 128, 191, 255}, 2) ||
      !dx12_pixel(pixels, 60, 44, {128, 255, 128, 255}, 2) ||
      !dx12_pixel(pixels, 5, 38, {128, 128, 191, 255}, 2) ||
      !dx12_pixel(pixels, 10, 50, {255, 255, 0, 255}) ||
      !dx12_pixel(pixels, 48, 52, {0, 0, 255, 255}))
    return dx12_fail(8, "multiple backdrop, image or shadow ordering differs");
  unsigned ink = 0;
  for (unsigned y = 0; y < mask.height; ++y) {
    for (unsigned x = 0; x < mask.width; ++x) {
      const auto alpha = mask.alpha[std::size_t(y) * mask.stride + x];
      ink += alpha != 0;
      const auto quarter = static_cast<unsigned char>(std::lround(127.5 + alpha * 0.25));
      const auto blue = static_cast<unsigned char>(std::lround(191.25 + alpha * 0.25));
      const auto pale = static_cast<unsigned char>(std::lround(127.5 + alpha * 0.5));
      if (!dx12_pixel(pixels, unsigned(first.x) + x, unsigned(first.y) + y,
                      {quarter, quarter, blue, 255}, 2) ||
          !dx12_pixel(pixels, unsigned(last.x) + x, unsigned(last.y) + y,
                      {pale, 255, pale, 255}, 2))
        return dx12_fail(9, "text before/after backdrop lost coverage or restored bindings");
    }
  }
  return ink > 8 ? 0 : dx12_fail(10, "mixed backdrop text oracle had insufficient ink");
}
