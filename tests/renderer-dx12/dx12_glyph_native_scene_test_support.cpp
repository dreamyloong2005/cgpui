#include "dx12_glyph_native_test_support.hpp"
#include <stdexcept>

namespace cgpui_test {
void glyph_add_record(cgpui::Dx12State& state, unsigned id, unsigned width, unsigned height, unsigned stride) {
  cgpui::RasterizedGlyph glyph{.key = {.font_family = "native-glyph-fixture", .glyph_id = id},
      .bitmap = {.width = width, .height = height, .stride = stride,
                 .alpha = std::vector<std::uint8_t>(std::size_t(stride) * (height - 1) + width, 0xf1)}};
  for (unsigned y = 0; y < height; ++y) for (unsigned x = 0; x < width; ++x)
    glyph.bitmap.alpha[std::size_t(y) * stride + x] = ((id + x + y) % 3) ? 255 : 0;
  (void)state.text_cache->glyphs.allocate(glyph);
}
cgpui::Dx12TextureHandle glyph_publish(cgpui::Dx12State& state) {
  auto updated = cgpui::dx12_update_glyph_pages(state, *state.text_cache);
  if (!updated || state.text_cache->pages.size() != 1) return {};
  return state.text_cache->pages[0].texture;
}
cgpui::Dx12DrawCommand glyph_draw(cgpui::Dx12TextureHandle texture, float left, std::size_t command_index) {
  const float u = 16.0F / texture->width, v = 16.0F / texture->height;
  cgpui::Dx12TexturedDraw draw{.texture = std::move(texture), .vertices = {
      {{left, 0}, {0, 0}, {1, 1, 1, 1}}, {{left + 16, 0}, {u, 0}, {1, 1, 1, 1}},
      {{left, 16}, {0, v}, {1, 1, 1, 1}}, {{left, 16}, {0, v}, {1, 1, 1, 1}},
      {{left + 16, 0}, {u, 0}, {1, 1, 1, 1}}, {{left + 16, 16}, {u, v}, {1, 1, 1, 1}}},
      .sampling = cgpui::ImageSamplingMode::nearest, .glyph = true};
  return {.payload = std::move(draw), .kind = cgpui::RendererPrimitiveKind::text,
          .command_index = command_index};
}
bool glyph_submit(cgpui::Dx12State& state, std::span<const cgpui::Dx12DrawCommand> draws) {
  const auto result = cgpui::dx12_submit_frame(state, {0, 0, 0, 1}, draws, true, draws.size());
  return result && state.pixels && state.diagnostics &&
      diagnostic_work(&*state.diagnostics, draws.size(), draws.size(), draws.size()) &&
      state.diagnostics->submitted_draws.text_count == draws.size();
}
bool glyph_pixels(const cgpui::Dx12State& state, const cgpui::Dx12Texture& texture, unsigned left) {
  unsigned ink = 0;
  for (unsigned y = 0; y < 16; ++y) for (unsigned x = 0; x < 16; ++x) {
    const auto alpha = texture.pixels[std::size_t(y) * texture.stride + x];
    ink += alpha != 0;
    if (!dx12_pixel(&*state.pixels, left + x, y, {alpha, alpha, alpha, 255}, 0)) return false;
  }
  return ink != 0;
}
std::size_t glyph_copy_bytes(const cgpui::Dx12TextureCopyObserver& observer, cgpui::Dx12TextureCopyKind kind) {
  std::size_t result = 0;
  for (const auto& event : observer.events()) if (event.kind == kind) result += event.payload_bytes;
  return result;
}
std::size_t glyph_copy_count(const cgpui::Dx12TextureCopyObserver& observer, cgpui::Dx12TextureCopyKind kind) {
  std::size_t result = 0;
  for (const auto& event : observer.events()) result += event.kind == kind;
  return result;
}
} // namespace cgpui_test
