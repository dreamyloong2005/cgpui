#include "dx12_command_reuse_surface_epoch_internal.hpp"
#include <cmath>
#include <limits>

namespace cgpui {
std::uint64_t Dx12CommandReuseSurfaceEpoch::value() const noexcept { return epoch_; }
bool Dx12CommandReuseSurfaceEpoch::enabled() const noexcept { return epoch_ != 0 && !exhausted_; }
const Dx12CommandReuseSurfaceKey& Dx12CommandReuseSurfaceEpoch::key() const noexcept { return key_; }
Dx12CommandReuseSurfaceChange Dx12CommandReuseSurfaceEpoch::accept(
    const Dx12CommandReuseSurfaceKey& key) noexcept {
  if (!key.width || !key.height || !std::isfinite(key.scale) || key.scale <= 0.0F)
    return Dx12CommandReuseSurfaceChange::invalid;
  if (exhausted_) return Dx12CommandReuseSurfaceChange::exhausted;
  if (epoch_ && key.width == key_.width && key.height == key_.height &&
      key.scale == key_.scale && key.composition == key_.composition &&
      key.transparent == key_.transparent) return Dx12CommandReuseSurfaceChange::unchanged;
  key_ = key;
  if (epoch_ == std::numeric_limits<std::uint64_t>::max()) {
    exhausted_ = true;
    return Dx12CommandReuseSurfaceChange::exhausted;
  }
  ++epoch_;
  return Dx12CommandReuseSurfaceChange::changed;
}
} // namespace cgpui
