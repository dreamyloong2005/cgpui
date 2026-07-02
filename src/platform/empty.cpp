#include "cgpui/platform/platform.hpp"

#include "cgpui/ui/text.hpp"

namespace cgpui {

std::vector<FontFaceDescriptor> PlatformApplication::discover_font_records()
    const {
  return {};
}

FontDatabase PlatformApplication::discover_fonts() const {
  const std::vector<FontFaceDescriptor> records = discover_font_records();
  return font_database_from_discovered_faces(
      std::span<const FontFaceDescriptor>(records.data(), records.size()));
}

void PlatformWindow::update_accessibility_tree(
    PlatformAccessibilityTreeUpdate update) {
  (void)update;
}

void PlatformApplication::request_wakeup() {}

void cgpui_platform_anchor() {}
} // namespace cgpui
