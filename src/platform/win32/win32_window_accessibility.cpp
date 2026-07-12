#include "win32_window_internal.hpp"

#include <utility>

namespace cgpui {

void Win32Window::configure_accessibility_actions() {
  uia_accessibility_.set_action_callback(
      [this](AccessibilityActionRequested action) {
        callback_(PlatformEvent{std::move(action)});
      });
}

void Win32Window::update_accessibility_tree(
    PlatformAccessibilityTreeUpdate update) {
  uia_accessibility_.update(std::move(update));
}

bool Win32Window::accessibility_object(
    WPARAM wparam,
    LPARAM lparam,
    LRESULT& result) {
  return uia_accessibility_.handle_get_object(wparam, lparam, result);
}

} // namespace cgpui
