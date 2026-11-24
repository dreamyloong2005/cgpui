#include "renderer_image_validation_internal.hpp"
#include "cgpui/renderer/planning/clip_scissor.hpp"
#include <algorithm>
#include <cmath>

namespace cgpui {
namespace {
bool finite(Rect rect) {
  return std::isfinite(rect.origin.x) && std::isfinite(rect.origin.y) &&
      std::isfinite(rect.size.width) && std::isfinite(rect.size.height) &&
      std::isfinite(rect.origin.x + rect.size.width) &&
      std::isfinite(rect.origin.y + rect.size.height);
}
bool finite(const std::optional<Rect>& rect) { return !rect || finite(*rect); }
bool finite(Color color) {
  return std::isfinite(color.r) && std::isfinite(color.g) &&
      std::isfinite(color.b) && std::isfinite(color.a);
}
bool finite(PaintMetadata metadata) {
  const auto& t = metadata.transform;
  return std::isfinite(metadata.opacity) && std::isfinite(t.scale_x) &&
      std::isfinite(t.skew_y) && std::isfinite(t.skew_x) && std::isfinite(t.scale_y) &&
      std::isfinite(t.translate_x) && std::isfinite(t.translate_y);
}
Error invalid(const char* message) { return {.code = ErrorCode::invalid_argument, .message = message}; }
}
bool renderer_same_image_allocation(const ImageAssetDescriptor& lhs,
                                    const ImageAssetDescriptor& rhs) {
  // Stride/byte_size describe uploads, not allocation identity or readiness.
  return lhs.id == rhs.id && lhs.pixel_width == rhs.pixel_width &&
      lhs.pixel_height == rhs.pixel_height && lhs.format == rhs.format;
}
Result<RendererValidatedImageDraw> renderer_validate_image_draw(const ImageDraw& draw) {
  if (!draw.asset.id.value || !draw.asset.pixel_width || !draw.asset.pixel_height ||
      draw.asset.format != ImageFormat::rgba8_unorm ||
      (draw.sampling != ImageSamplingMode::nearest && draw.sampling != ImageSamplingMode::linear))
    return std::unexpected(invalid("image draw requires a valid identity, extent, format and sampling mode"));
  // Validate raw inputs before composition fallback, opacity/UV clamping or clip culling.
  if (!finite(draw.bounds) || !finite(draw.source_rect) ||
      (draw.tint && !finite(*draw.tint)) || !finite(draw.clip_rect) ||
      !finite(draw.clip_stack.current_clip_rect) || !finite(draw.metadata) ||
      !finite(draw.composition_stack.current_metadata) ||
      !std::ranges::all_of(draw.clip_stack.clips, [](Rect r) { return finite(r); }) ||
      !std::ranges::all_of(draw.composition_stack.entries, [](PaintMetadata m) { return finite(m); }))
    return std::unexpected(invalid("image draw contains nonfinite geometry, color, clip or composition"));
  RendererValidatedImageDraw result{
      .vertices = build_image_vertices(std::span{&draw, 1}),
      .clip_rect = resolve_effective_clip_rect(draw.clip_rect, draw.clip_stack)};
  const auto all_finite = [](const auto& values) {
    return std::ranges::all_of(values, [](float value) { return std::isfinite(value); });
  };
  if (!finite(result.clip_rect) || !std::ranges::all_of(result.vertices, [&](const auto& vertex) {
        return all_finite(vertex.position) && all_finite(vertex.image_uv) && all_finite(vertex.color);
      })) return std::unexpected(invalid("image draw derived geometry must remain finite"));
  return result;
}
} // namespace cgpui
