#include "frame_wait_test_support_internal.hpp"

#if defined(_WIN32)
#include "frame_wait_win32_test_support.hpp"
#endif

namespace cgpui_test {
namespace {
std::chrono::steady_clock::time_point frame_wait_now(void*) {
  return std::chrono::steady_clock::now();
}
void frame_wait_yield(void*, std::chrono::milliseconds delay) {
  std::this_thread::sleep_for(delay);
}
cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> frame_wait_timeout() {
  return std::unexpected(cgpui::Error{cgpui::ErrorCode::frame_acquisition_failed,
      "test frame acquisition exceeded its absolute three-second deadline"});
}
} // namespace

cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> wait_for_test_frame(
    cgpui::Renderer& renderer) {
  return wait_for_test_frame(renderer, {});
}

cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> wait_for_test_frame(
    cgpui::Renderer& renderer, TestFrameWaitPolicy policy) {
  detail::FrameWaitHooks hooks{.now = frame_wait_now, .yield = frame_wait_yield};
#if defined(_WIN32)
  hooks.pump = detail::pump_frame_wait_win32_messages;
#endif
  return detail::wait_for_test_frame_with_hooks(renderer, policy, hooks);
}

cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> detail::wait_for_test_frame_with_hooks(
    cgpui::Renderer& renderer, TestFrameWaitPolicy policy, const FrameWaitHooks& hooks) {
  if (!hooks.now || !hooks.yield) {
    return std::unexpected(cgpui::Error{cgpui::ErrorCode::invalid_argument,
        "test frame wait requires clock and nonzero-yield hooks"});
  }
  const auto deadline = hooks.now(hooks.context) + std::chrono::seconds(3);
  const bool may_pump = policy.message_owner != std::thread::id{} &&
      policy.message_owner == std::this_thread::get_id();
  for (;;) {
    auto frame = renderer.begin_frame();
    // Preserve success (including null) and every non-pending error verbatim.
    if (frame || frame.error().code != cgpui::ErrorCode::frame_not_ready) return frame;
    if (hooks.now(hooks.context) >= deadline) return frame_wait_timeout();
    if (may_pump && hooks.pump) hooks.pump(hooks.context, 32);
    if (hooks.now(hooks.context) >= deadline) return frame_wait_timeout();
    hooks.yield(hooks.context, std::chrono::milliseconds(1));
    if (hooks.now(hooks.context) >= deadline) return frame_wait_timeout();
  }
}

} // namespace cgpui_test
