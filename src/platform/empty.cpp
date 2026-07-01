#include "cgpui/platform/platform.hpp"

#include "cgpui/ui/text.hpp"

namespace cgpui {

FontDatabase PlatformApplication::discover_fonts() const {
  return {};
}

void PlatformWindow::update_accessibility_tree(
    PlatformAccessibilityTreeUpdate update) {
  (void)update;
}

void PlatformApplication::request_wakeup() {}

void cgpui_platform_anchor() {}
} // namespace cgpui
