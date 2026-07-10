#include "ui_internal.hpp"

namespace cgpui {

bool WindowRuntimeContext::accept_window_close() const {
  return platform_window.resolve_close_request(
      PlatformWindowCloseResolution::accept);
}

bool WindowRuntimeContext::cancel_window_close() const {
  return platform_window.resolve_close_request(
      PlatformWindowCloseResolution::cancel);
}

} // namespace cgpui
