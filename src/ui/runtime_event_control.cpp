#include "ui_internal.hpp"

namespace cgpui {

bool WindowRuntime::handle_window_control_event(const PlatformEvent& event) {
  if (std::holds_alternative<WindowCloseRequested>(event)) {
    const bool close_policy_pending =
        state_->windows.window_ != nullptr && state_->windows.window_->close_request_state().pending;
    if (state_->windows.window_ != nullptr && state_->windows.renderer_ != nullptr) {
      record_lifecycle_event(event);
    }
    if (state_->core.close_requested_callback_ && state_->windows.window_ != nullptr &&
        state_->windows.renderer_ != nullptr) {
      state_->core.close_requested_callback_(context());
    }
    if (close_policy_pending && state_->windows.window_ != nullptr) {
      if (state_->windows.window_->close_request_state().pending) {
        (void)state_->windows.window_->resolve_close_request(
            PlatformWindowCloseResolution::accept);
      }
      if (!state_->windows.window_->close_request_state().accepted) {
        return true;
      }
    }
    state_->core.should_quit_ = true;
    state_->core.application_.quit();
    return true;
  }

  if (std::holds_alternative<WindowActivated>(event) ||
      std::holds_alternative<WindowMinimized>(event) ||
      std::holds_alternative<WindowRestored>(event)) {
    if (state_->windows.window_ != nullptr && state_->windows.renderer_ != nullptr) {
      if (const auto* activated = std::get_if<WindowActivated>(&event);
          activated != nullptr) {
        state_->input.input_.focused = activated->active;
      } else if (const auto* minimized = std::get_if<WindowMinimized>(&event);
                 minimized != nullptr) {
        state_->input.input_.focused = !minimized->minimized;
      } else {
        state_->input.input_.focused = false;
      }
      record_lifecycle_event(event);
    }
    return true;
  }

  if (std::holds_alternative<WindowMoved>(event)) {
    if (state_->windows.window_ != nullptr && state_->windows.renderer_ != nullptr) {
      record_lifecycle_event(event);
    }
    return true;
  }

  if (const auto* resized = std::get_if<WindowResized>(&event);
      resized != nullptr) {
    handle_resize(*resized);
    return true;
  }

  if (std::holds_alternative<WindowRedrawRequested>(event)) {
    handle_redraw();
    return true;
  }

  if (std::holds_alternative<WindowWakeupRequested>(event)) {
    handle_wakeup();
    return true;
  }

  return false;
}

} // namespace cgpui
