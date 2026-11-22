#pragma once

#include "win32_appearance_monitor_internal.hpp"

#include "cgpui/platform/platform_application.hpp"

namespace cgpui {

// Process-wide Win32 system state shared by the application: the DPI
// awareness declaration (made before any window exists) and the system
// appearance query and change monitor.
class Win32SystemApplication : public PlatformApplication {
 public:
  Win32SystemApplication();

  [[nodiscard]] SystemAppearance system_appearance() const override;
  void set_system_appearance_callback(
      SystemAppearanceCallback callback) override;
  [[nodiscard]] PlatformDpiAwarenessState dpi_awareness() const override;
  // Popups present through DirectComposition: true when its probe passes.
  [[nodiscard]] bool popup_windows_supported() const override;

 private:
  PlatformDpiAwarenessState dpi_awareness_;
  Win32AppearanceMonitor appearance_monitor_;
};

} // namespace cgpui
