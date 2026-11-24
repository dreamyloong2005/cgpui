#include "runtime_surface_resizes_internal.hpp"
#include <functional>
#include <utility>

namespace {
class ScriptedRenderer final : public cgpui::Renderer {
 public:
  cgpui::Result<void> resize(cgpui::Size size, cgpui::DpiScale scale) override {
    ++calls; last = size; dpi = scale;
    if (auto callback = std::exchange(on_resize, {})) callback();
    return outcome;
  }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::unexpected(cgpui::Error{cgpui::ErrorCode::invalid_argument, "not a frame test"});
  }
  unsigned calls = 0;
  cgpui::Size last;
  cgpui::DpiScale dpi;
  cgpui::Result<void> outcome;
  std::function<void()> on_resize;
};
}
int main() {
  using namespace cgpui;
  RuntimeSurfaceResizes pending;
  ScriptedRenderer renderer;
  constexpr WindowRuntimeId window{1};
  bool nested_deferred = false;
  renderer.on_resize = [&] {
    auto result = pending.request(window, renderer, {900, 600}, DpiScale{2});
    nested_deferred = !result && result.error().code == ErrorCode::renderer_recovery_pending;
  };
  auto first = pending.request(window, renderer, {300, 200}, DpiScale{1});
  if (first || first.error().code != ErrorCode::renderer_recovery_pending ||
      !nested_deferred || renderer.calls != 1) return 1;
  if (!pending.retry(window, renderer) || renderer.calls != 2 ||
      renderer.last.width != 900 || renderer.last.height != 600 || renderer.dpi.value != 2) return 2;
  if (!pending.retry(window, renderer) || renderer.calls != 2) return 3;
  renderer.on_resize = [&] { pending.clear(window); };
  renderer.outcome = std::unexpected(Error{ErrorCode::renderer_recovery_pending, "closed while resizing"});
  if (!pending.request(window, renderer, {100, 100}, DpiScale{1})) return 4;
  if (!pending.retry(window, renderer) || renderer.calls != 3) return 5;
  renderer.outcome = std::unexpected(Error{ErrorCode::frame_not_ready, "bad resize contract"});
  auto invalid = pending.request(window, renderer, {100, 100}, DpiScale{1});
  if (invalid || invalid.error().code != ErrorCode::frame_acquisition_failed) return 6;
  if (!pending.retry(window, renderer) || renderer.calls != 4) return 7;
  renderer.outcome = std::unexpected(Error{ErrorCode::frame_retry_required, "not a resize result"});
  auto retry = pending.request(window, renderer, {100, 100}, DpiScale{1});
  if (retry || retry.error().code != ErrorCode::frame_acquisition_failed || pending.has_pending(window)) return 8;
  ScriptedRenderer replacement;
  replacement.outcome = std::unexpected(Error{ErrorCode::renderer_recovery_pending, "replacement still recovering"});
  renderer.outcome = {};
  const auto old_calls = renderer.calls;
  bool replacement_deferred = false;
  renderer.on_resize = [&] {
    pending.clear(window);
    auto result = pending.request(window, replacement, {1200, 800}, DpiScale{2});
    replacement_deferred = !result && result.error().code == ErrorCode::renderer_recovery_pending;
  };
  // Completion of the cancelled old request must not erase a new owner's entry
  // even when the replacement window uses the same runtime ID.
  if (!pending.request(window, renderer, {300, 200}, DpiScale{1}) ||
      !replacement_deferred || !pending.has_pending(window) ||
      renderer.calls != old_calls + 1 || replacement.calls != 1) return 9;
  replacement.outcome = {};
  if (!pending.retry(window, replacement) || pending.has_pending(window) ||
      replacement.calls != 2 || replacement.last.width != 1200 || replacement.last.height != 800 ||
      replacement.dpi.value != 2 || renderer.calls != old_calls + 1) return 10;
  return 0;
}
