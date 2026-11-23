#include "dx12_backdrop_blur_internal.hpp"
#include "cgpui/renderer/planning/backdrop_blur_parameters.hpp"
#include <cmath>

namespace cgpui {
namespace {
bool finite_rect(const Rect& rect) {
  return std::isfinite(rect.origin.x) && std::isfinite(rect.origin.y) &&
      std::isfinite(rect.size.width) && std::isfinite(rect.size.height) &&
      std::isfinite(rect.origin.x + rect.size.width) &&
      std::isfinite(rect.origin.y + rect.size.height);
}
template <std::size_t Size>
bool finite(const std::array<float, Size>& values) {
  for (float value : values) if (!std::isfinite(value)) return false;
  return true;
}
} // namespace

bool dx12_valid_backdrop_blur_input(const BackdropBlurDraw& draw) {
  if (!finite_rect(draw.rect)) return false;
  if (draw.tint && (!std::isfinite(draw.tint->r) || !std::isfinite(draw.tint->g) ||
      !std::isfinite(draw.tint->b) || !std::isfinite(draw.tint->a))) return false;
  if (draw.clip_rect && !finite_rect(*draw.clip_rect)) return false;
  if (draw.clip_stack.current_clip_rect && !finite_rect(*draw.clip_stack.current_clip_rect)) return false;
  for (const auto& clip : draw.clip_stack.clips) if (!finite_rect(clip)) return false;
  return true;
}

bool dx12_valid_backdrop_blur_plan(const BackdropBlurPlan& plan, PixelExtent extent) {
  if (plan.vertices.size() > UINT_MAX / sizeof(BackdropBlurVertex)) return false;
  for (const auto& vertex : plan.vertices) {
    if (!finite(vertex.position) || !finite(vertex.local) || !finite(vertex.rect) ||
        !finite(vertex.radii) || !finite(vertex.tint) || !finite(vertex.region)) return false;
  }
  for (const auto& pass : plan.passes) {
    const auto region = pass.region;
    if (region.x < 0 || region.y < 0 || !region.width || !region.height ||
        UINT(region.x) > extent.width || UINT(region.y) > extent.height ||
        region.width > extent.width - UINT(region.x) || region.height > extent.height - UINT(region.y) ||
        !pass.texels.width || !pass.texels.height ||
        pass.texels.width > extent.width || pass.texels.height > extent.height ||
        (pass.factor != 1 && pass.factor != 2 && pass.factor != 4 && pass.factor != 8) ||
        !std::isfinite(pass.sigma) || pass.sigma < 0 || pass.radius < 0 ||
        pass.radius > kBackdropBlurMaxRadius || pass.first_vertex > plan.vertices.size() ||
        kBackdropBlurVerticesPerPass > plan.vertices.size() - pass.first_vertex ||
        (pass.clip_rect && !finite_rect(*pass.clip_rect))) return false;
  }
  return true;
}
} // namespace cgpui
