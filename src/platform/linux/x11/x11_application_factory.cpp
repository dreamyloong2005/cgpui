#include "x11_application_internal.hpp"

namespace cgpui {

Result<std::unique_ptr<PlatformApplication>> create_x11_application() {
  auto application = X11Application::create();
  if (!application) return std::unexpected(application.error());
  return std::unique_ptr<PlatformApplication>(std::move(*application));
}

}  // namespace cgpui
