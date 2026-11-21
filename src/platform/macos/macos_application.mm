#include "macos_application_internal.hpp"
#include "macos_dpi_awareness_internal.hpp"

#include <chrono>
#include <utility>

namespace cgpui {

PlatformFontDiscoveryResult MacOSApplication::discover_font_discovery() const {
  return macos_discover_fonts();
}

SystemAppearance MacOSApplication::system_appearance() const {
  return macos_read_system_appearance();
}

void MacOSApplication::set_system_appearance_callback(
    SystemAppearanceCallback callback) {
  appearance_monitor_.set_callback(std::move(callback));
}

PlatformDpiAwarenessState MacOSApplication::dpi_awareness() const {
  return macos_dpi_awareness_state();
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
