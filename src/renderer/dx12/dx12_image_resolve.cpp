#include "dx12_image_resolve_internal.hpp"
#include "dx12_image_cache_internal.hpp"
#include "dx12_state_internal.hpp"
#include "cgpui/renderer/planning/image_geometry.hpp"
#include "cgpui/renderer/planning/clip_scissor.hpp"

namespace cgpui {
Result<Dx12TexturedDraw> dx12_resolve_image_draw(
    Dx12State& state, const Dx12PendingImageDraw& request) {
  const auto& device = request.image; // Already device-scaled when recorded.
  auto texture = state.image_cache->find(device.asset);
  if (!texture) return std::unexpected(texture.error());
  Dx12TexturedDraw draw{.texture = std::move(*texture),
      .clip_rect = resolve_effective_clip_rect(device.clip_rect, device.clip_stack),
      .sampling = device.sampling};
  for (const auto& vertex : build_image_vertices(std::span{&device, 1}))
    draw.vertices.push_back({vertex.position, vertex.image_uv, vertex.color});
  if (!dx12_finite_textured_draw(draw)) return std::unexpected(Error{
      .code = ErrorCode::invalid_argument,
      .message = "DX12 image draw must have a valid texture and finite vertices"});
  return draw;
}
} // namespace cgpui
