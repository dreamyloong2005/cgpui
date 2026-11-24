#include "dx12_image_resolve_internal.hpp"
#include "dx12_image_cache_internal.hpp"
#include "dx12_state_internal.hpp"
#include "../renderer_image_validation_internal.hpp"

namespace cgpui {
Result<Dx12ResolvedImageDraw> dx12_resolve_image_draw(
    Dx12State& state, const Dx12PendingImageDraw& request) {
  const auto& device = request.image; // Already device-scaled when recorded.
  auto geometry = renderer_validate_image_draw(device);
  if (!geometry) return std::unexpected(geometry.error());
  if (device.asset.pixel_width > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
      device.asset.pixel_height > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION)
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 image draw extent exceeds the texture limit"});
  auto texture = state.image_cache->lookup(device.asset);
  if (!texture) return std::unexpected(texture.error());
  if (!*texture) return Dx12ResolvedImageDraw{Dx12MissingImageDraw{geometry->clip_rect}};
  Dx12TexturedDraw draw{.texture = std::move(**texture),
      .clip_rect = geometry->clip_rect, .sampling = device.sampling};
  for (const auto& vertex : geometry->vertices)
    draw.vertices.push_back({vertex.position, vertex.image_uv, vertex.color});
  if (!dx12_finite_textured_draw(draw)) return std::unexpected(Error{
      .code = ErrorCode::invalid_argument,
      .message = "DX12 image draw must have a valid texture and finite vertices"});
  return Dx12ResolvedImageDraw{std::move(draw)};
}
} // namespace cgpui
