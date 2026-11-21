#pragma once

#include "cgpui/core/window_backdrop.hpp"

#import <AppKit/AppKit.h>

#include <string>

namespace cgpui {

// What apply_window_backdrop does on a Cocoa window. The material is drawn
// by an NSVisualEffectView (blending behindWindow) that becomes the
// window's content view, with the CGPUI content view as its only subview.
struct MacOSBackdropPlan {
  WindowBackdrop applied = WindowBackdrop::none;
  NSVisualEffectMaterial material = NSVisualEffectMaterialWindowBackground;
  std::string mechanism;
  std::string reason;
};

// none -> no view; Reduce Transparency or an opaque renderer surface ->
// no view with a reason; otherwise mica -> windowBackground, mica_alt ->
// underWindowBackground, acrylic -> popover, blur -> sidebar.
[[nodiscard]] MacOSBackdropPlan macos_plan_window_backdrop(
    WindowBackdrop requested,
    bool transparent_surface,
    bool reduce_transparency);

} // namespace cgpui
