#include "runtime_surface_resizes_internal.hpp"
#include <limits>
#include <utility>

namespace cgpui {
struct RuntimeSurfaceResizes::Pending {
  Size size;
  DpiScale scale;
  std::uint64_t revision = 0;
  bool applying = false;
  bool cancelled = false;
};
namespace {
Result<void> deferred_resize() {
  return std::unexpected(Error{ErrorCode::renderer_recovery_pending,
      "latest renderer size/scale has not been applied"});
}
}
RuntimeSurfaceResizes::RuntimeSurfaceResizes() = default;
RuntimeSurfaceResizes::~RuntimeSurfaceResizes() = default;

Result<void> RuntimeSurfaceResizes::request(
    WindowRuntimeId window, Renderer& renderer, Size size, DpiScale scale) {
  auto& slot = pending_[window.value];
  if (!slot) slot = std::make_shared<Pending>();
  auto pending = slot;
  if (pending->revision == std::numeric_limits<std::uint64_t>::max())
    return std::unexpected(Error{ErrorCode::frame_acquisition_failed,
        "renderer resize revision exhausted"});
  pending->size = size;
  pending->scale = scale;
  ++pending->revision;
  if (pending->applying) return deferred_resize();
  return apply(window, renderer, pending);
}
Result<void> RuntimeSurfaceResizes::retry(WindowRuntimeId window, Renderer& renderer) {
  const auto found = pending_.find(window.value);
  if (found == pending_.end()) return {};
  const auto pending = found->second;
  if (pending->applying) return deferred_resize();
  return apply(window, renderer, pending);
}
bool RuntimeSurfaceResizes::has_pending(WindowRuntimeId window) const {
  return pending_.contains(window.value);
}
Result<void> RuntimeSurfaceResizes::apply(
    WindowRuntimeId window, Renderer& renderer, const std::shared_ptr<Pending>& pending) {
  const auto revision = pending->revision;
  const auto size = pending->size;
  const auto scale = pending->scale;
  pending->applying = true;
  auto result = renderer.resize(size, scale);
  pending->applying = false;
  // Platform callbacks may close this window or start a new runtime session.
  // A cancelled entry must not erase/retry another entry using the same ID.
  if (pending->cancelled) return {};
  if (!result && result.error().code == ErrorCode::renderer_recovery_pending)
    return result;
  if (!result && result.error().code == ErrorCode::frame_not_ready)
    result = std::unexpected(Error{ErrorCode::frame_acquisition_failed,
        "resize violated begin-only frame_not_ready contract: " + result.error().message});
  if (!result && result.error().code == ErrorCode::frame_retry_required)
    result = std::unexpected(Error{ErrorCode::frame_acquisition_failed,
        "resize cannot report a completed frame retry: " + result.error().message});
  if (result && revision != pending->revision) return deferred_resize();
  const auto found = pending_.find(window.value);
  if (found != pending_.end() && found->second == pending) pending_.erase(found);
  return result;
}
void RuntimeSurfaceResizes::clear(WindowRuntimeId window) {
  const auto found = pending_.find(window.value);
  if (found == pending_.end()) return;
  found->second->cancelled = true;
  pending_.erase(found);
}
void RuntimeSurfaceResizes::clear_all() {
  auto retired = std::exchange(pending_, {});
  for (auto& [id, pending] : retired) pending->cancelled = true;
}
} // namespace cgpui
