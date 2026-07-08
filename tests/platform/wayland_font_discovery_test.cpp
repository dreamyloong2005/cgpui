#include "cgpui/platform/platform.hpp"

#include <string_view>

namespace {

bool has_prefix(std::string_view value, std::string_view prefix) {
  return value.substr(0, prefix.size()) == prefix;
}

} // namespace

int main() {
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }

  const cgpui::PlatformFontDiscoveryResult discovery =
      (*app)->discover_font_discovery();
  if (discovery.diagnostics.backend !=
      cgpui::PlatformFontDiscoveryBackend::fontconfig) {
    return 2;
  }
  if (discovery.diagnostics.status !=
          cgpui::PlatformFontDiscoveryStatus::native_available &&
      discovery.diagnostics.status !=
          cgpui::PlatformFontDiscoveryStatus::deterministic_fallback) {
    return 3;
  }

  const bool native_available =
      discovery.diagnostics.status ==
      cgpui::PlatformFontDiscoveryStatus::native_available;
  if (discovery.diagnostics.native_backend != native_available ||
      discovery.diagnostics.fallback_used == native_available) {
    return 4;
  }
  if (discovery.diagnostics.face_count != discovery.font_faces().size() ||
      discovery.font_faces().empty()) {
    return 5;
  }

  for (const cgpui::FontFaceDescriptor& face : discovery.font_faces()) {
    if (face.source != cgpui::FontSource::platform ||
        face.font.family.empty() || face.path.empty()) {
      return 6;
    }
  }

  if (!native_available) {
    const cgpui::FontFaceDescriptor& fallback = discovery.font_faces().front();
    if (fallback.font.family != "sans-serif" ||
        fallback.postscript_name != "fontconfig:sans-serif" ||
        !has_prefix(fallback.path, "fontconfig://")) {
      return 7;
    }
  }

  const cgpui::FontDatabase database = (*app)->discover_fonts();
  const cgpui::FontFaceDescriptor* resolved =
      database.resolve(discovery.font_faces().front().font);
  if (database.face_count() != discovery.font_faces().size() ||
      resolved == nullptr) {
    return 8;
  }

  return 0;
}
