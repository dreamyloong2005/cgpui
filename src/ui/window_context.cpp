#include "cgpui/ui/window_context.hpp"

#include "cgpui/ui/runtime_context.hpp"

namespace cgpui {

WindowContextCapability::WindowContextCapability(
    const WindowRuntimeContext& context)
    : context_(&context) {}

Window WindowContextCapability::window() const {
  return context_->window();
}

Window WindowContextCapability::current_window() const {
  return context_->current_window();
}

WindowRuntimeId WindowContextCapability::runtime_id() const {
  return window().runtime_id();
}

WindowDescriptor WindowContextCapability::descriptor() const {
  return window().descriptor();
}

ViewId WindowContextCapability::root_view_id() const {
  return window().root_view_id();
}

bool WindowContextCapability::active() const {
  return window().active();
}

Size WindowContextCapability::viewport_size() const {
  return window().viewport_size();
}

DpiScale WindowContextCapability::scale() const {
  return window().scale();
}

ViewInputState WindowContextCapability::input_state() const {
  return window().input_state();
}

bool WindowContextCapability::focused() const {
  return window().focused();
}

void WindowContextCapability::request_render() const {
  window().request_render();
}

void WindowContextCapability::request_layout() const {
  window().request_layout();
}

void WindowContextCapability::request_paint() const {
  window().request_paint();
}

Window WindowRuntimeContext::window() const {
  return current_window();
}

Window WindowRuntimeContext::current_window() const {
  return Window(runtime, window_runtime_id);
}

WindowContextCapability WindowRuntimeContext::window_context() const {
  return WindowContextCapability(*this);
}

} // namespace cgpui
