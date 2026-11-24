#include "dx12_text_draws_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_text_cache_internal.hpp"
#include "cgpui/renderer/planning/clip_scissor.hpp"
#include "cgpui/renderer/planning/composition_opacity.hpp"
#include "cgpui/renderer/planning/glyph_geometry.hpp"
#include "cgpui/renderer/planning/text_geometry.hpp"

#include <cmath>

namespace cgpui {
namespace {
bool finite_glyph_positions(const TextDraw& text) {
  for (const auto& glyph : text.glyphs) {
    if (!std::isfinite(glyph.device_origin.x) || !std::isfinite(glyph.device_origin.y) ||
        !std::isfinite(glyph.device_baseline)) return false;
  }
  return true;
}
} // namespace

Result<std::vector<Dx12TexturedDraw>> dx12_build_text_draws(
    Dx12State& state, const TextDraw& device_text) {
  if (state.text_cache) state.text_cache->lineage.prune_expired();
  // Native glyph placement rounds positions before rasterization validates the
  // font metrics; reject nonfinite pens before that integer conversion.
  if (!finite_glyph_positions(device_text)) return std::unexpected(Error{
      .code = ErrorCode::invalid_argument, .message = "DX12 text glyph positions must be finite"});
  if (!state.text_cache) state.text_cache = std::make_unique<Dx12TextCache>();
  auto& cache = *state.text_cache;
  Dx12GlyphLineageSweep sweep(cache.lineage);
  cache.glyphs.clear_rasterization_error();
  auto quads = build_textured_glyph_quads(device_text, cache.glyphs);
  if (cache.glyphs.rasterization_error())
    return std::unexpected(*cache.glyphs.rasterization_error());
  if (auto updated = dx12_update_glyph_pages(state, cache); !updated)
    return std::unexpected(updated.error());
  const auto clip = resolve_effective_clip_rect(device_text.clip_rect, device_text.clip_stack);
  const auto color = apply_composed_opacity(
      device_text.color, device_text.metadata, device_text.composition_stack);
  std::vector<Dx12TexturedDraw> draws;
  for (auto& quad : quads) {
    if (quad.page_index >= cache.pages.size() || !cache.pages[quad.page_index].texture)
      return std::unexpected(Error{.code = ErrorCode::renderer_initialization_failed,
          .message = "DX12 text quad references an unavailable atlas page"});
    const auto& texture = cache.pages[quad.page_index].texture;
    if (draws.empty() || draws.back().texture != texture) {
      draws.push_back({.texture = texture, .clip_rect = clip,
                       .sampling = ImageSamplingMode::linear, .glyph = true});
    }
    quad.color = color; // The neutral vertex builder applies transforms, not opacity.
    const auto vertices = build_text_vertices(std::span{&quad, 1});
    for (const auto& vertex : vertices) {
      draws.back().vertices.push_back({.position = vertex.position,
          .uv = vertex.atlas_uv, .color = vertex.color});
    }
  }
  return draws;
}
} // namespace cgpui
