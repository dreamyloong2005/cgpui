#include "dx12_image_batch_internal.hpp"
#include "dx12_image_resolve_internal.hpp"
#include "dx12_image_cache_internal.hpp"
#include "dx12_state_internal.hpp"
#include <utility>

namespace cgpui {
Result<void> Dx12ImageBatch::prepare(Dx12State& state, std::span<Dx12DrawCommand> draws) {
  // Local request ownership guarantees raw upload bytes are destroyed on every
  // return/exception, before native submission and retained-image publication.
  std::vector<ImageAsset> uploads;
  std::vector<ImageAssetId> invalidations;
  uploads.swap(uploads_);
  invalidations.swap(invalidations_);
  for (const auto id : invalidations) state.image_cache->invalidate(id);
  for (const auto& image : uploads) {
    if (auto uploaded = state.image_cache->upload(state, image); !uploaded) return uploaded;
  }
  for (auto& command : draws) {
    const auto* pending = std::get_if<Dx12PendingImageDraw>(&command.payload);
    if (!pending) continue;
    auto resolved = dx12_resolve_image_draw(state, *pending);
    if (!resolved) return std::unexpected(resolved.error());
    command.payload = std::move(*resolved); // Preserve kind/index/offset and draw order.
  }
  return dx12_require_resolved_images(draws);
}
Result<void> dx12_require_resolved_images(std::span<const Dx12DrawCommand> draws) {
  for (const auto& command : draws) {
    if (std::holds_alternative<Dx12PendingImageDraw>(command.payload))
      return std::unexpected(Error{.code = ErrorCode::invalid_argument,
          .message = "DX12 image request reached submission before resolution"});
  }
  return {};
}
} // namespace cgpui
