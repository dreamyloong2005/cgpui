#include "cgpui/ui/test_context.hpp"

#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

void TestContextCapability::request_redraw() const {
  context_->runtime.schedule_redraw();
}

Result<void> TestContextCapability::try_draw_frame() const {
  return context_->runtime.try_draw_frame();
}

void TestContextCapability::draw_frame() const {
  context_->runtime.handle_event(WindowRedrawRequested{});
}

} // namespace cgpui
