#pragma once

#include "cgpui/platform/platform_appearance.hpp"

#import <Foundation/Foundation.h>

@class CGPUIMacOSAppearanceObserver;

namespace cgpui {

// Snapshot from NSApp.effectiveAppearance (Aqua / DarkAqua),
// NSColor.controlAccentColor resolved under that appearance in sRGB, and
// the NSWorkspace accessibility display options (reduce transparency,
// reduce motion, increase contrast).
[[nodiscard]] SystemAppearance macos_read_system_appearance();

// NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification covers three
// settings. The source is the first one that differs: high_contrast, then
// client_area_animation; otherwise (transparency, or nothing) color_set,
// which is also where Windows reports a transparency toggle.
[[nodiscard]] SystemAppearanceChangeSource macos_accessibility_change_source(
    const SystemAppearance& before,
    const SystemAppearance& after);

// Observes effectiveAppearance (KVO on NSApp, color_set),
// NSSystemColorsDidChangeNotification (colorization) and the workspace
// accessibility notification from the first set_callback() on. Each one
// re-reads the snapshot and forwards a SystemAppearanceChange on the main
// thread, with `changed` against the previous snapshot.
class MacOSAppearanceMonitor {
 public:
  MacOSAppearanceMonitor() = default;
  ~MacOSAppearanceMonitor();
  MacOSAppearanceMonitor(const MacOSAppearanceMonitor&) = delete;
  MacOSAppearanceMonitor& operator=(const MacOSAppearanceMonitor&) = delete;

  void set_callback(SystemAppearanceCallback callback);
  void handle(SystemAppearanceChangeSource source);
  void handle_accessibility_change();

 private:
  __strong CGPUIMacOSAppearanceObserver* observer_ = nil;
  SystemAppearanceCallback callback_;
  SystemAppearance last_appearance_;
};

} // namespace cgpui
