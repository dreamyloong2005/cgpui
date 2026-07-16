#pragma once

#include "cgpui/core/error.hpp"

#include <string_view>

namespace cgpui {

enum class LinuxPlatformBackend {
  wayland,
  x11,
};

struct LinuxPlatformBackendEnvironment {
  std::string_view override_value;
  std::string_view wayland_display;
  std::string_view x11_display;
};

[[nodiscard]] Result<LinuxPlatformBackend> select_linux_platform_backend(
    LinuxPlatformBackendEnvironment environment);

[[nodiscard]] Result<LinuxPlatformBackend>
select_linux_platform_backend_from_environment();

}  // namespace cgpui
