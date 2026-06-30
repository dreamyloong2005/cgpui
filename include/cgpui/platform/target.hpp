#pragma once

#include <string_view>

namespace cgpui {

enum class DesktopPlatformTarget {
  windows,
  linux_wayland,
  macos_cocoa,
};

[[nodiscard]] constexpr std::string_view desktop_platform_target_name(
    DesktopPlatformTarget target) {
  switch (target) {
    case DesktopPlatformTarget::windows:
      return "Windows/Win32";
    case DesktopPlatformTarget::linux_wayland:
      return "Linux/Wayland";
    case DesktopPlatformTarget::macos_cocoa:
      return "macOS/Cocoa";
  }

  return "Unknown";
}

} // namespace cgpui
