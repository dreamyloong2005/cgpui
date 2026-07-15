#include "macos_text_services_internal.hpp"

#include "cgpui/platform/platform.hpp"

#include <algorithm>
#include <string>

int main() {
  @autoreleasepool {
    const auto direct = cgpui::macos_discover_fonts();
    if (direct.empty() ||
        direct.diagnostics.backend !=
            cgpui::PlatformFontDiscoveryBackend::core_text ||
        direct.diagnostics.status !=
            cgpui::PlatformFontDiscoveryStatus::native_available ||
        !direct.diagnostics.native_backend || direct.diagnostics.fallback_used ||
        direct.diagnostics.face_count != direct.font_faces().size()) {
      return 1;
    }
    std::string previous;
    for (const auto& face : direct.font_faces()) {
      if (face.font.family.empty() || face.postscript_name.empty() ||
          face.source != cgpui::FontSource::platform || face.path.empty()) {
        return 2;
      }
      const std::string key = face.font.family + "\n" + face.postscript_name;
      if (!previous.empty() && key < previous) return 3;
      previous = key;
    }

    auto application = cgpui::create_platform_application();
    if (!application) return 4;
    const auto through_application = (*application)->discover_font_discovery();
    if (through_application.diagnostics.backend !=
            cgpui::PlatformFontDiscoveryBackend::core_text ||
        through_application.diagnostics.face_count !=
            direct.diagnostics.face_count) {
      return 5;
    }
  }
  return 0;
}
