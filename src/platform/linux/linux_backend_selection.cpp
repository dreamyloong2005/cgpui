#include "linux_backend_selection_internal.hpp"

#include <cstdlib>

namespace cgpui {
namespace {

std::string_view environment_value(const char* name) {
  const char* value = std::getenv(name);
  return value == nullptr ? std::string_view{} : std::string_view{value};
}

}  // namespace

Result<LinuxPlatformBackend> select_linux_platform_backend(
    LinuxPlatformBackendEnvironment environment) {
  if (environment.override_value == "wayland") {
    return LinuxPlatformBackend::wayland;
  }
  if (environment.override_value == "x11") {
    return LinuxPlatformBackend::x11;
  }
  if (!environment.override_value.empty() &&
      environment.override_value != "auto") {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "CGPUI_LINUX_BACKEND must be auto, wayland, or x11",
    });
  }
  if (!environment.wayland_display.empty()) {
    return LinuxPlatformBackend::wayland;
  }
  if (!environment.x11_display.empty()) {
    return LinuxPlatformBackend::x11;
  }
  return std::unexpected(Error{
      .code = ErrorCode::unsupported_platform,
      .message =
          "no Linux display backend is available; set WAYLAND_DISPLAY or DISPLAY",
  });
}

Result<LinuxPlatformBackend> select_linux_platform_backend_from_environment() {
  return select_linux_platform_backend(LinuxPlatformBackendEnvironment{
      .override_value = environment_value("CGPUI_LINUX_BACKEND"),
      .wayland_display = environment_value("WAYLAND_DISPLAY"),
      .x11_display = environment_value("DISPLAY"),
  });
}

}  // namespace cgpui
