#include "dx12_image_request_validation_internal.hpp"
#include "../renderer_image_validation_internal.hpp"
#include <unordered_map>

namespace cgpui {
Result<void> dx12_validate_image_request_identities(std::span<const Dx12DrawCommand> draws) {
  std::unordered_map<std::uint64_t, ImageAssetDescriptor> identities;
  for (const auto& command : draws) {
    const auto* pending = std::get_if<Dx12PendingImageDraw>(&command.payload);
    if (!pending) continue;
    const auto& asset = pending->image.asset;
    const auto [entry, inserted] = identities.emplace(asset.id.value, asset);
    if (!inserted && !renderer_same_image_allocation(entry->second, asset))
      return std::unexpected(Error{.code = ErrorCode::invalid_argument,
          .message = "DX12 image draw requests conflict for one asset id"});
  }
  return {};
}
} // namespace cgpui
