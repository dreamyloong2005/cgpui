#pragma once

#include "cgpui/renderer/renderer_frame.hpp"

#include <memory>
#include <thread>

namespace cgpui_test {

// Capture the test window's actual owner thread when constructing its fixture.
// Empty/default means yield-only; a different current thread never pumps messages.
struct TestFrameWaitPolicy {
  std::thread::id message_owner;
};

// Tests only. Retry begin-only frame_not_ready within one absolute three-second
// window. Never retries other errors or Present, and never wraps the renderer.
[[nodiscard]] cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> wait_for_test_frame(
    cgpui::Renderer& renderer);
[[nodiscard]] cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> wait_for_test_frame(
    cgpui::Renderer& renderer, TestFrameWaitPolicy policy);

} // namespace cgpui_test
