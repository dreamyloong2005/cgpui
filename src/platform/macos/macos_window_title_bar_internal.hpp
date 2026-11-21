#pragma once

#include "cgpui/core/window_title_bar.hpp"

#import <Foundation/Foundation.h>

#include <optional>
#include <vector>

namespace cgpui {

// The region under `point` (logical pixels), matched last to first like the
// other backends; nullopt outside every region.
[[nodiscard]] std::optional<WindowControlArea> macos_window_control_area_at(
    const std::vector<WindowControlRegion>& regions,
    Point point);

enum class MacOSTitleBarDoubleClick { zoom, minimize, none };

// System Settings > Desktop & Dock "Double-click a window's title bar to":
// the global AppleActionOnDoubleClick default ("Maximize", "Minimize",
// "None"; unset means Maximize, as AppKit treats it).
[[nodiscard]] MacOSTitleBarDoubleClick macos_title_bar_double_click_action(
    NSString* preference);

} // namespace cgpui
