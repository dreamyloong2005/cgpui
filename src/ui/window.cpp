#include "cgpui/app/window.hpp"

#include "cgpui/platform/platform_window.hpp"
#include "cgpui/ui/layout.hpp"
#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

Window::Window(WindowRuntime& runtime, WindowRuntimeId runtime_id)
    : runtime_(&runtime), runtime_id_(runtime_id) {}

WindowRuntimeId Window::runtime_id() const {
  return runtime_id_;
}

WindowDescriptor Window::descriptor() const {
  const WindowRuntimeRecord* runtime_record = record();
  return runtime_record == nullptr ? WindowDescriptor{}
                                   : runtime_record->descriptor;
}

ViewId Window::root_view_id() const {
  const WindowRuntimeRecord* runtime_record = record();
  return runtime_record == nullptr ? ViewId{} : runtime_record->root_view_id;
}

bool Window::active() const {
  const WindowRuntimeRecord* runtime_record = record();
  return runtime_record != nullptr && runtime_record->active;
}

Size Window::viewport_size() const {
  const WindowRuntimeRecord* runtime_record = record();
  if (runtime_record != nullptr && runtime_record->window != nullptr) {
    const WindowState state = runtime_record->window->state();
    return to_logical_pixels(state.framebuffer_size, state.scale);
  }
  return descriptor().size;
}

DpiScale Window::scale() const {
  const WindowRuntimeRecord* runtime_record = record();
  if (runtime_record != nullptr && runtime_record->window != nullptr) {
    return runtime_record->window->state().scale;
  }
  return {};
}

ViewInputState Window::input_state() const {
  return runtime_->input_state();
}

bool Window::focused() const {
  return input_state().focused;
}

void Window::request_render() const {
  runtime_->request_render();
}

void Window::request_layout() const {
  runtime_->request_layout();
}

void Window::request_paint() const {
  runtime_->request_paint();
}

const WindowRuntimeRecord* Window::record() const {
  return runtime_->window_runtime_record(runtime_id_);
}

} // namespace cgpui
