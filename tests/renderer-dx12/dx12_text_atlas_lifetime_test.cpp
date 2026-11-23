#include "dx12_test_support.hpp"
#include "cgpui/renderer/planning/glyph_geometry.hpp"

#include <algorithm>
#include <limits>
#include <vector>

using namespace cgpui;
using namespace cgpui_test;

namespace {
TextDraw text(const std::string& content, float size, Point origin, Color color) {
  const FontDescriptor font{.family = "Segoe UI"};
  const auto shaped = shape_text(content, font, size, {1},
      {.preferred_backend = TextShapingBackend::deterministic_fallback});
  return {.bounds = {origin, {1000, size}}, .color = color, .font = font,
          .content = content, .byte_length = content.size(), .font_size = size,
          .scale = {1}, .device_font_size = size,
          .glyphs = text_glyph_paint_metadata(shaped, origin)};
}

struct Reference {
  unsigned width, height;
  GlyphCache cache;
  std::vector<std::array<unsigned char, 4>> pixels;
  Reference(unsigned w, unsigned h) : width(w), height(h), pixels(w * h, {0, 0, 0, 255}) {}
  bool paint(const TextDraw& draw) {
    const auto quads = build_textured_glyph_quads(draw, cache);
    if (cache.rasterization_error() || quads.size() != draw.glyphs.size()) return false;
    for (const auto& quad : quads) {
      const auto uploads = cache.upload_records();
      const auto found = std::ranges::find_if(uploads, [&](const auto& upload) {
        return upload.page_index == quad.page_index &&
            upload.atlas_bounds.origin.x == quad.atlas_bounds.origin.x &&
            upload.atlas_bounds.origin.y == quad.atlas_bounds.origin.y;
      });
      if (found == uploads.end()) return false;
      for (unsigned y = 0; y < found->height; ++y) {
        for (unsigned x = 0; x < found->width; ++x) {
          const int px = static_cast<int>(quad.device_bounds.origin.x) + x;
          const int py = static_cast<int>(quad.device_bounds.origin.y) + y;
          if (px < 0 || py < 0 || px >= static_cast<int>(width) || py >= static_cast<int>(height)) continue;
          auto& pixel = pixels[std::size_t(py) * width + px];
          const float alpha = found->alpha[std::size_t(y) * found->stride + x] / 255.0F * draw.color.a;
          const float channels[] = {draw.color.r, draw.color.g, draw.color.b};
          for (unsigned channel = 0; channel < 3; ++channel) {
            pixel[channel] = static_cast<unsigned char>(std::lround(
                channels[channel] * 255 * alpha + pixel[channel] * (1 - alpha)));
          }
        }
      }
    }
    return true;
  }
};

int finish(Renderer& renderer, RenderFrame& frame, const Reference& reference) {
  if (!frame.request_pixel_capture()) return dx12_fail(30, "capture refused");
  if (auto result = frame.present(); !result) return dx12_fail(31, result.error().message.c_str());
  const auto* pixels = renderer.last_frame_pixels();
  if (!pixels || pixels->width != reference.width || pixels->height != reference.height)
    return dx12_fail(32, "capture extent differs");
  for (unsigned y = 0; y < reference.height; ++y) {
    for (unsigned x = 0; x < reference.width; ++x) {
      if (!dx12_pixel(pixels, x, y, reference.pixels[std::size_t(y) * reference.width + x]))
        return dx12_fail(33, "atlas/order coverage differs");
    }
  }
  return 0;
}

int same_frame(Renderer& renderer) {
  Reference reference(64, 64);
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(40, "begin failed");
  (*frame)->clear({0, 0, 0, 1});
  const auto first = text("M", 24, {4, 4}, {1, 0, 0, 1});
  (*frame)->draw_text(first);
  if (!reference.paint(first)) return dx12_fail(41, "first reference failed");
  const ImageAsset image{.id = {5101}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 255, 0, 255}}};
  (*frame)->upload_image(image);
  (*frame)->draw_image({.bounds = {{10, 8}, {20, 15}},
      .asset = describe_image_asset(image), .sampling = ImageSamplingMode::nearest});
  for (unsigned y = 8; y < 23; ++y)
    for (unsigned x = 10; x < 30; ++x) reference.pixels[y * 64 + x] = {255, 255, 0, 255};
  const auto second = text("B", 24, {8, 4}, {0, 1, 0, 1});
  (*frame)->draw_text(second); // Adds a glyph to the already-referenced atlas page.
  if (!reference.paint(second)) return dx12_fail(42, "second reference failed");
  const auto third = text("M", 24, {4, 34}, {0, 0, 1, 1});
  (*frame)->draw_text(third);
  if (!reference.paint(third)) return dx12_fail(43, "third reference failed");
  return finish(renderer, **frame, reference);
}

int multiple_pages(Renderer& renderer) {
  if (!renderer.resize({840, 840}, {1})) return dx12_fail(50, "large resize failed");
  auto draw = text("MBWM", 400, {}, {1, 1, 1, 1});
  if (draw.glyphs.size() != 4) return dx12_fail(51, "large shape differs");
  for (std::size_t i = 0; i < draw.glyphs.size(); ++i) {
    draw.glyphs[i].device_origin = {4.0F + float(i % 2) * 420, 4.0F + float(i / 2) * 420};
  }
  Reference reference(840, 840);
  if (!reference.paint(draw) || reference.cache.atlas_pages().size() < 3)
    return dx12_fail(52, "reference did not exercise multiple atlas pages");
  // The fourth glyph returns to the first page; page binding must not be sticky.
  for (int iteration = 0; iteration != 2; ++iteration) {
    auto frame = renderer.begin_frame();
    if (!frame) return dx12_fail(53, "multipage begin failed");
    (*frame)->clear({0, 0, 0, 1});
    (*frame)->draw_text(draw);
    if (const auto result = finish(renderer, **frame, reference)) return result;
  }
  return 0;
}

int invalid_then_valid(Renderer& renderer) {
  if (!renderer.resize({64, 64}, {1})) return dx12_fail(60, "small resize failed");
  for (const bool invalid_position : {true, false}) {
    auto draw = text("AZ", 16, {4, 4}, {1, 1, 1, 1});
    if (draw.glyphs.size() != 2) return dx12_fail(61, "invalid fixture shape differs");
    if (invalid_position) draw.glyphs[0].device_origin.x = std::numeric_limits<float>::quiet_NaN();
    else draw.glyphs[1].key.device_font_size = -1; // A was cached before Z fails.
    auto frame = renderer.begin_frame();
    if (!frame) return dx12_fail(62, "invalid begin failed");
    (*frame)->draw_text(draw);
    const auto result = (*frame)->present();
    if (result || result.error().code != ErrorCode::invalid_argument)
      return dx12_fail(63, "invalid glyph accepted or wrong error");
  }
  const auto valid = text("A", 16, {4, 4}, {1, 1, 1, 1});
  Reference reference(64, 64);
  if (!reference.paint(valid)) return dx12_fail(64, "recovery reference failed");
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(65, "rejected glyph poisoned renderer");
  (*frame)->clear({0, 0, 0, 1});
  (*frame)->draw_text(valid);
  return finish(renderer, **frame, reference);
}
}

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  if (const auto result = same_frame(**renderer)) return result;
  if (const auto result = multiple_pages(**renderer)) return result;
  return invalid_then_valid(**renderer);
}
