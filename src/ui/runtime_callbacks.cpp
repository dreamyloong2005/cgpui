#include "ui_internal.hpp"

namespace cgpui {

void WindowRuntime::set_after_frame_callback(
    WindowRuntimeFrameCallback callback) {
  state_->core.after_frame_callback_ = std::move(callback);
}

void WindowRuntime::set_after_render_callback(
    WindowRuntimeRenderCallback callback) {
  state_->core.after_render_callback_ = std::move(callback);
}

void WindowRuntime::set_after_event_callback(
    WindowRuntimeEventCallback callback) {
  state_->core.after_event_callback_ = std::move(callback);
}

void WindowRuntime::set_close_requested_callback(
    WindowRuntimeFrameCallback callback) {
  state_->core.close_requested_callback_ = std::move(callback);
}

void WindowRuntime::set_error_callback(WindowRuntimeErrorCallback callback) {
  state_->core.error_callback_ = std::move(callback);
}

} // namespace cgpui
