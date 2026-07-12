#include "win32_accessibility_internal.hpp"

#include <utility>

namespace cgpui {

void Win32UiaAccessibilityAdapter::set_action_callback(
    std::function<void(AccessibilityActionRequested)> callback) {
  action_callback_ = std::move(callback);
}

} // namespace cgpui
