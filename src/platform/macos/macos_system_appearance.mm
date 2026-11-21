#include "macos_system_appearance_internal.hpp"

#import <AppKit/AppKit.h>

namespace cgpui {

SystemAppearance macos_read_system_appearance() {
  NSAppearance* appearance = [NSApp effectiveAppearance];
  const NSAppearanceName match = [appearance
      bestMatchFromAppearancesWithNames:@[ NSAppearanceNameAqua,
                                           NSAppearanceNameDarkAqua ]];
  SystemAppearance result{.supported = true, .backend = "macos"};
  result.mode = [match isEqualToString:NSAppearanceNameDarkAqua]
      ? SystemAppearanceMode::dark
      : SystemAppearanceMode::light;
  // controlAccentColor is dynamic: resolve it under the app's appearance.
  __block NSColor* accent = nil;
  [appearance performAsCurrentDrawingAppearance:^{
    accent = [[NSColor controlAccentColor]
        colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
  }];
  if (accent != nil) {
    result.accent_color_available = true;
    result.accent_color = Color{
        .r = static_cast<float>([accent redComponent]),
        .g = static_cast<float>([accent greenComponent]),
        .b = static_cast<float>([accent blueComponent]),
        .a = 1.0F};
  }
  NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
  result.transparency_effects_enabled =
      ![workspace accessibilityDisplayShouldReduceTransparency];
  result.client_area_animation_enabled =
      ![workspace accessibilityDisplayShouldReduceMotion];
  result.high_contrast = [workspace accessibilityDisplayShouldIncreaseContrast];
  return result;
}

SystemAppearanceChangeSource macos_accessibility_change_source(
    const SystemAppearance& before,
    const SystemAppearance& after) {
  if (before.high_contrast != after.high_contrast) {
    return SystemAppearanceChangeSource::high_contrast;
  }
  if (before.client_area_animation_enabled !=
      after.client_area_animation_enabled) {
    return SystemAppearanceChangeSource::client_area_animation;
  }
  return SystemAppearanceChangeSource::color_set;
}

} // namespace cgpui
