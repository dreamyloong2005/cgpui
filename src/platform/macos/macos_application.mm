#include "macos_application_internal.hpp"

#include <chrono>

namespace cgpui {

PlatformFontDiscoveryResult MacOSApplication::discover_font_discovery() const {
  return macos_discover_fonts();
}

std::uint64_t MacOSApplication::monotonic_time_ms() const {
  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  return static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return std::make_unique<MacOSApplication>();
}

}  // namespace cgpui
