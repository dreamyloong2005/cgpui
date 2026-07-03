#include "ui_internal.hpp"

namespace cgpui {

bool WindowRuntime::handle_window_control_event(const PlatformEvent& event) {
  if (std::holds_alternative<WindowCloseRequested>(event)) {
    should_quit_ = true;
    if (window_ != nullptr && renderer_ != nullptr) {
      record_lifecycle_event(event);
    }
    if (close_requested_callback_ && window_ != nullptr &&
        renderer_ != nullptr) {
      close_requested_callback_(context());
    }
    application_.quit();
    return true;
  }

  if (std::holds_alternative<WindowActivated>(event) ||
      std::holds_alternative<WindowMinimized>(event) ||
      std::holds_alternative<WindowRestored>(event)) {
    if (window_ != nullptr && renderer_ != nullptr) {
      if (const auto* activated = std::get_if<WindowActivated>(&event);
          activated != nullptr) {
        input_.focused = activated->active;
      } else if (const auto* minimized = std::get_if<WindowMinimized>(&event);
                 minimized != nullptr) {
        input_.focused = !minimized->minimized;
      } else {
        input_.focused = false;
      }
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
