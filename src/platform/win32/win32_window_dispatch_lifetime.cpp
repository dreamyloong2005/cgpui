#include "win32_window_dispatch_lifetime_internal.hpp"

#include <utility>

namespace cgpui {

Win32WindowDispatchLifetime::Win32WindowDispatchLifetime(
    PlatformEventCallback callback) : callback_(std::move(callback)) {}

void Win32WindowDispatchLifetime::invalidate_window_registration() {
  if (registration_) registration_->window = nullptr;
}

} // namespace cgpui
