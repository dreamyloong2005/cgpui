#include "cgpui/ui/test_context.hpp"

#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

void TestContextCapability::dispatch_pointer_move(Point position) const {
  context_->runtime.handle_event(PlatformEvent{PointerMoved{
      .position = position,
  }});
}

void TestContextCapability::dispatch_pointer_button(
    MouseButton button,
    bool pressed,
    Point position) const {
  context_->runtime.handle_event(PlatformEvent{PointerButton{
      .button = button,
      .pressed = pressed,
      .position = position,
  }});
}

void TestContextCapability::dispatch_pointer_scroll(
    Point delta,
    Point position) const {
  context_->runtime.handle_event(PlatformEvent{PointerScrolled{
      .delta = delta,
      .position = position,
  }});
}

} // namespace cgpui
