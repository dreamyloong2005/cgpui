#include "win32_window_registry_internal.hpp"
#include "win32_window_internal.hpp"

#include <algorithm>

namespace cgpui {

void Win32WindowRegistry::prune() {
  std::erase_if(windows_, [](const auto& weak) {
    const auto registration = weak.lock();
    return !registration || registration->window == nullptr;
  });
}

void Win32WindowRegistry::add(Win32Window& window) {
  if (window.registration_) window.registration_->window = nullptr;
  prune();
  window.registration_ = std::make_shared<Win32WindowRegistration>(
      Win32WindowRegistration{.window = &window});
  windows_.push_back(window.registration_);
}

void Win32WindowRegistry::visit(const std::function<void(Win32Window*)>& callback) {
  prune();
  const auto snapshot = windows_;
  for (const auto& weak : snapshot) {
    const auto registration = weak.lock();
    if (registration && registration->window != nullptr) {
      callback(registration->window);
    }
  }
}

} // namespace cgpui
