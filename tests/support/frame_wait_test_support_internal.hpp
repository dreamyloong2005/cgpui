#pragma once

#include "frame_wait_test_support.hpp"

#include <chrono>

namespace cgpui_test::detail {

// Private deterministic seam for the helper's own tests; no production hooks,
// global/thread-local fault state, renderer wrappers, or callback ownership.
struct FrameWaitHooks {
  void* context = nullptr;
  std::chrono::steady_clock::time_point (*now)(void*) = nullptr;
  void (*yield)(void*, std::chrono::milliseconds) = nullptr;
  void (*pump)(void*, unsigned maximum_messages) = nullptr;
};

[[nodiscard]] cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> wait_for_test_frame_with_hooks(
    cgpui::Renderer& renderer, TestFrameWaitPolicy policy, const FrameWaitHooks& hooks);

} // namespace cgpui_test::detail
