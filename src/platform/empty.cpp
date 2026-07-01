#include "cgpui/platform/platform.hpp"

#include "cgpui/ui/text.hpp"

namespace cgpui {

FontDatabase PlatformApplication::discover_fonts() const {
  return {};
}

void PlatformApplication::request_wakeup() {}

void cgpui_platform_anchor() {}
} // namespace cgpui
