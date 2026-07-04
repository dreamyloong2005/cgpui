#include "cgpui/ui/test_context.hpp"

#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

void TestContextCapability::dispatch_window_activation(bool active) const {
  context_->runtime.handle_event(PlatformEvent{WindowActivated{
      .active = active,
  }});
}

void TestContextCapability::dispatch_window_focus(bool focused) const {
  context_->runtime.handle_event(PlatformEvent{WindowFocused{
      .focused = focused,
  }});
}

void TestContextCapability::focus(ElementId element_id) const {
  context_->runtime.request_keyboard_focus(element_id);
}

void TestContextCapability::release_focus(ElementId element_id) const {
  context_->runtime.release_keyboard_focus(element_id);
}

} // namespace cgpui
