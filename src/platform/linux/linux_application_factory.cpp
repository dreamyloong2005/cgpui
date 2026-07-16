#include "linux_backend_selection_internal.hpp"
#include "wayland_application_internal.hpp"
#include "x11/x11_application_internal.hpp"

namespace cgpui {

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  auto backend = select_linux_platform_backend_from_environment();
  if (!backend) {
    return std::unexpected(backend.error());
  }
  switch (*backend) {
    case LinuxPlatformBackend::wayland:
      return create_wayland_application();
    case LinuxPlatformBackend::x11:
      return create_x11_application();
  }
  return std::unexpected(Error{
      .code = ErrorCode::unsupported_platform,
      .message = "selected Linux platform backend is not implemented",
  });
}

}  // namespace cgpui
