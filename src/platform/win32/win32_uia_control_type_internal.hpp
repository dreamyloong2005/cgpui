#pragma once

#include "cgpui/platform/platform_accessibility.hpp"

#include <UIAutomationClient.h>

namespace cgpui {

// UIA control type for a platform accessibility role (Pane for generic).
[[nodiscard]] CONTROLTYPEID control_type_for(PlatformAccessibilityRole role);

} // namespace cgpui
