#pragma once

#include "cgpui/core/error.hpp"
#include "cgpui/renderer/renderer_types.hpp"
#include <span>
#include <vector>

namespace cgpui {
struct Dx12State;
struct Dx12DrawCommand;
// Pending public requests, not installed immutable snapshots or native resources.
class Dx12ImageBatch {
 public:
  [[nodiscard]] bool has_mutations() const noexcept;
  void upload(const ImageAsset& image);
  void invalidate(ImageAssetId id);
  Result<void> prepare(Dx12State& state, std::span<Dx12DrawCommand> draws);

 private:
  std::vector<ImageAsset> uploads_;
  std::vector<ImageAssetId> invalidations_;
};
Result<void> dx12_require_resolved_images(std::span<const Dx12DrawCommand> draws);
} // namespace cgpui
