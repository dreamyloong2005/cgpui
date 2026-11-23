#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"
#include "cgpui/renderer/planning/glyph_geometry.hpp"

#include <algorithm>
#include <vector>

using namespace cgpui;
using namespace cgpui_test;

namespace {
TextDraw text_draw(const std::string& content, DpiScale scale,
                   TextShapingBackend backend) {
  const FontDescriptor font{.family = "Segoe UI"};
  const auto shaped = shape_text(content, font, 16, scale,
      {.preferred_backend = backend});
  return {.bounds = {{4, 4}, {48, 24}}, .color = {1, 0, 0, 0.8F},
          .font = font, .content = content, .byte_length = content.size(),
          .font_size = 16, .scale = scale, .device_font_size = 16 * scale.value,
          .glyphs = text_glyph_paint_metadata(shaped, {4, 4})};
}

int deterministic_pixels(Renderer& renderer, DpiScale scale, const std::string& content) {
  auto draw = text_draw(content, scale, TextShapingBackend::deterministic_fallback);
  // The deterministic shaper still uses the production Windows alpha rasterizer.
  // A public CPU reference supplies coverage; all transport, sampling, clipping,
  // transformation, blending and order below are checked through GPU readback.
  GlyphCache reference;
  const auto quads = build_textured_glyph_quads(draw, reference);
  if (quads.size() != 1 || reference.upload_records().size() != 1)
    return dx12_fail(10, "reference glyph rasterization failed");
  const auto& upload = reference.upload_records().front();
  const int left = static_cast<int>(quads[0].device_bounds.origin.x + 2 * scale.value);
  const int top = static_cast<int>(quads[0].device_bounds.origin.y + 3 * scale.value);
  const int clip_left = left + static_cast<int>(upload.width / 3);
  const Rect clip{{clip_left / scale.value, 0}, {64 / scale.value, 64 / scale.value}};
  // Composition-stack entries store cumulative metadata, not independent factors.
  const PaintMetadata ancestry[] = {{.opacity = 0.5F},
      {.opacity = 0.25F, .transform = AffineTransform::translation(2, 3)}};
  draw.metadata = ancestry[1];
  draw.composition_stack = renderer_composition_stack_record(ancestry);
  draw.clip_rect = clip;
  draw.clip_stack = renderer_clip_stack_record(std::span{&clip, 1});
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return dx12_fail(11, frame.error().message.c_str());
  (*frame)->clear({0, 0, 1, 1});
  (*frame)->draw_text(draw);
  // This later primitive must not be moved behind an aggregated text pass.
  (*frame)->draw_rect({.rect = {{0, 0}, {64 / scale.value, 10 / scale.value}},
                       .color = {0, 1, 0, 1}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(12, "capture refused");
  const auto presented = (*frame)->present();
  if (!presented) return dx12_fail(13, presented.error().message.c_str());
  const auto* pixels = renderer.last_frame_pixels();
  if (!pixels || !pixels->valid()) return dx12_fail(14, "text produced no pixels");
  std::size_t ink = 0;
  for (unsigned y = 0; y < 64; ++y) {
    for (unsigned x = 0; x < 64; ++x) {
      const int gx = static_cast<int>(x) - left, gy = static_cast<int>(y) - top;
      unsigned alpha = 0;
      if (static_cast<int>(x) >= clip_left && gx >= 0 && gy >= 0 &&
          gx < static_cast<int>(upload.width) && gy < static_cast<int>(upload.height))
        alpha = upload.alpha[std::size_t(gy) * upload.stride + gx];
      const auto red = static_cast<unsigned char>(std::lround(alpha * 0.2));
      const std::array<unsigned char, 4> expected = y < 10
          ? std::array<unsigned char, 4>{0, 255, 0, 255}
          : std::array<unsigned char, 4>{red, 0, static_cast<unsigned char>(255 - red), 255};
      if (!dx12_pixel(pixels, x, y, expected)) return dx12_fail(15, "glyph pixels differ");
      if (alpha && y >= 10) ++ink;
    }
  }
  return ink > 5 ? 0 : dx12_fail(16, "reference did not exercise visible text");
}

int native_smoke(Renderer& renderer) {
  std::vector<std::vector<std::uint8_t>> captures;
  for (const auto* content : {"A", "B", " "}) {
    auto draw = text_draw(content, {1}, TextShapingBackend::native);
    draw.color = {1, 1, 1, 1};
    if (draw.glyphs.empty() || draw.glyphs[0].key.shaping_backend != TextShapingBackend::directwrite)
      return dx12_fail(20, "native DirectWrite shaping was not exercised");
    auto frame = cgpui_test::wait_for_test_frame(renderer);
    if (!frame) return dx12_fail(21, "native text begin failed");
    (*frame)->clear({0, 0, 0, 1});
    (*frame)->draw_text(draw);
    if (!(*frame)->request_pixel_capture()) return dx12_fail(22, "native capture refused");
    auto presented = (*frame)->present();
    if (!presented) return dx12_fail(23, presented.error().message.c_str());
    const auto* pixels = renderer.last_frame_pixels();
    if (!pixels || !pixels->valid()) return dx12_fail(24, "native capture missing");
    unsigned ink = 0;
    for (std::size_t i = 0; i < pixels->rgba8.size(); i += 4) ink += pixels->rgba8[i] > 0;
    if (std::string_view(content) == " " ? ink != 0 : ink < 10)
      return dx12_fail(25, "native ink/whitespace differs");
    captures.push_back(pixels->rgba8);
  }
  return captures[0] != captures[1] ? 0 : dx12_fail(26, "native A/B masks alias");
}
}

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  for (const float scale : {1.0F, 2.0F}) {
    if (!(*renderer)->resize({64, 64}, {scale})) return dx12_fail(3, "scale resize failed");
    // Returning to M after a different glyph catches stale/corrupted atlas snapshots.
    for (const auto* content : {"M", "B", "M"}) {
      if (const int result = deterministic_pixels(**renderer, {scale}, content)) return result;
    }
  }
  if (!(*renderer)->resize({64, 64}, {1})) return dx12_fail(4, "native resize failed");
  return native_smoke(**renderer);
}
