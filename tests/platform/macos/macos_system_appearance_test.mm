#include "macos_system_appearance_internal.hpp"

#include "cgpui/platform/platform.hpp"

#import <AppKit/AppKit.h>

#include <cstddef>
#include <vector>

namespace {

// Runs the main run loop until `count` changes arrived or 2 s passed.
bool wait_for(const std::vector<cgpui::SystemAppearanceChange>& changes,
              std::size_t count) {
  NSDate* deadline = [NSDate dateWithTimeIntervalSinceNow:2.0];
  while (changes.size() < count && [deadline timeIntervalSinceNow] > 0) {
    [[NSRunLoop currentRunLoop]
        runMode:NSDefaultRunLoopMode
     beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.02]];
  }
  return changes.size() >= count;
}

bool unit_component(float value) { return value >= 0.0F && value <= 1.0F; }

int test_accessibility_source() {
  using Source = cgpui::SystemAppearanceChangeSource;
  const cgpui::SystemAppearance before{.supported = true, .backend = "macos"};
  cgpui::SystemAppearance contrast = before;
  contrast.high_contrast = true;
  contrast.client_area_animation_enabled = false;
  cgpui::SystemAppearance motion = before;
  motion.client_area_animation_enabled = false;
  cgpui::SystemAppearance transparency = before;
  transparency.transparency_effects_enabled = true;
  return cgpui::macos_accessibility_change_source(before, contrast) ==
                     Source::high_contrast &&
                 cgpui::macos_accessibility_change_source(before, motion) ==
                     Source::client_area_animation &&
                 cgpui::macos_accessibility_change_source(before, transparency) ==
                     Source::color_set &&
                 cgpui::macos_accessibility_change_source(before, before) ==
                     Source::color_set
             ? 0
             : 1;
}

}  // namespace

int main() {
  @autoreleasepool {
    if (test_accessibility_source() != 0) return 1;
    auto application = cgpui::create_platform_application();
    if (!application) return 2;
    cgpui::PlatformApplication& app = **application;

    const cgpui::PlatformDpiAwarenessState dpi = app.dpi_awareness();
    if (dpi.awareness != cgpui::PlatformDpiAwareness::per_monitor_aware_v2 ||
        !dpi.declared_by_framework || dpi.already_set_by_process ||
        dpi.backend != "macos") {
      return 3;
    }

    const cgpui::SystemAppearance initial = app.system_appearance();
    if (!initial.supported || initial.backend != "macos" ||
        !initial.unsupported_reason.empty() || !initial.accent_color_available ||
        !unit_component(initial.accent_color.r) ||
        !unit_component(initial.accent_color.g) ||
        !unit_component(initial.accent_color.b) ||
        initial.accent_color.a != 1.0F) {
      return 4;
    }

    std::vector<cgpui::SystemAppearanceChange> changes;
    app.set_system_appearance_callback(
        [&](const cgpui::SystemAppearanceChange& change) {
          changes.push_back(change);
        });

    // Forcing the app appearance changes NSApp.effectiveAppearance (KVO).
    // Switch to the mode the system is not in, then to the other one.
    using Mode = cgpui::SystemAppearanceMode;
    const Mode first = initial.mode == Mode::dark ? Mode::light : Mode::dark;
    for (const Mode mode : {first, initial.mode}) {
      const std::size_t before = changes.size();
      [NSApp setAppearance:[NSAppearance
                               appearanceNamed:(mode == Mode::dark
                                                    ? NSAppearanceNameDarkAqua
                                                    : NSAppearanceNameAqua)]];
      if (!wait_for(changes, before + 1)) return 5;
      const cgpui::SystemAppearanceChange& change = changes.back();
      if (change.source != cgpui::SystemAppearanceChangeSource::color_set ||
          !change.changed || change.appearance.mode != mode ||
          app.system_appearance().mode != mode) {
        return 6;
      }
    }

    // Repeated broadcasts are delivered with changed = false.
    std::size_t before = changes.size();
    [[NSNotificationCenter defaultCenter]
        postNotificationName:NSSystemColorsDidChangeNotification
                      object:nil];
    if (changes.size() != before + 1 ||
        changes.back().source != cgpui::SystemAppearanceChangeSource::colorization ||
        changes.back().changed) {
      return 7;
    }
    // AppKit reacts to the accessibility notification too (it re-posts the
    // system colors and may re-evaluate the appearance), so more than one
    // callback can arrive; none of them reports a change here.
    before = changes.size();
    [[[NSWorkspace sharedWorkspace] notificationCenter]
        postNotificationName:NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification
                      object:[NSWorkspace sharedWorkspace]];
    bool accessibility_seen = false;
    for (std::size_t index = before; index < changes.size(); ++index) {
      if (changes[index].changed) return 8;
      accessibility_seen = accessibility_seen ||
          changes[index].source == cgpui::SystemAppearanceChangeSource::color_set;
    }
    if (!accessibility_seen) return 9;

    // An empty callback stops delivery.
    app.set_system_appearance_callback({});
    before = changes.size();
    [[NSNotificationCenter defaultCenter]
        postNotificationName:NSSystemColorsDidChangeNotification
                      object:nil];
    [NSApp setAppearance:nil];
    return changes.size() == before ? 0 : 10;
  }
}
