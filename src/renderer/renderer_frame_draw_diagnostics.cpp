#include "cgpui/renderer/renderer_frame_diagnostics.hpp"

#include <limits>

namespace cgpui {
namespace {

std::size_t saturating_add(
    std::size_t value,
    std::size_t increment,
    bool& saturated) {
  if (increment > std::numeric_limits<std::size_t>::max() - value) {
    saturated = true;
    return std::numeric_limits<std::size_t>::max();
  }
  return value + increment;
}

std::size_t& primitive_draw_count(
    RendererDrawCounts& counts,
    RendererPrimitiveKind primitive_kind) {
  switch (primitive_kind) {
    case RendererPrimitiveKind::solid_rect:
      return counts.solid_rect_count;
    case RendererPrimitiveKind::rounded_rect:
      return counts.rounded_rect_count;
    case RendererPrimitiveKind::text:
      return counts.text_count;
    case RendererPrimitiveKind::text_selection:
      return counts.text_selection_count;
    case RendererPrimitiveKind::text_caret:
      return counts.text_caret_count;
    case RendererPrimitiveKind::image:
      return counts.image_count;
  }
  return counts.solid_rect_count;
}

} // namespace

void renderer_add_draw_count(
    RendererDrawCounts& counts,
    RendererPrimitiveKind primitive_kind,
    std::size_t draw_count) {
  std::size_t& primitive_count = primitive_draw_count(counts, primitive_kind);
  primitive_count =
      saturating_add(primitive_count, draw_count, counts.saturated);
  counts.total_count =
      saturating_add(counts.total_count, draw_count, counts.saturated);
}

} // namespace cgpui
