#include "linux_backend_selection_internal.hpp"

#include <string>

int main() {
  using cgpui::LinuxPlatformBackend;
  using cgpui::LinuxPlatformBackendEnvironment;

  auto wayland = cgpui::select_linux_platform_backend(
      {.wayland_display = "wayland-0", .x11_display = ":0"});
  if (!wayland || *wayland != LinuxPlatformBackend::wayland) return 1;

  auto x11 = cgpui::select_linux_platform_backend({.x11_display = ":0"});
  if (!x11 || *x11 != LinuxPlatformBackend::x11) return 2;

  auto forced_x11 = cgpui::select_linux_platform_backend({
      .override_value = "x11",
      .wayland_display = "wayland-0",
  });
  if (!forced_x11 || *forced_x11 != LinuxPlatformBackend::x11) return 3;

  auto invalid = cgpui::select_linux_platform_backend(
      LinuxPlatformBackendEnvironment{.override_value = "invalid"});
  if (invalid || invalid.error().code != cgpui::ErrorCode::invalid_argument ||
      invalid.error().message.find("auto, wayland, or x11") ==
          std::string::npos) {
    return 4;
  }

  auto unavailable = cgpui::select_linux_platform_backend({});
  if (unavailable ||
      unavailable.error().code != cgpui::ErrorCode::unsupported_platform ||
      unavailable.error().message.find("WAYLAND_DISPLAY or DISPLAY") ==
          std::string::npos) {
    return 5;
  }
  return 0;
}
