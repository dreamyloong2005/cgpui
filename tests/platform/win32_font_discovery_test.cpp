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
          cgpui::PlatformFontDiscoveryBackend::direct_write ||
      discovery.diagnostics.status !=
          cgpui::PlatformFontDiscoveryStatus::native_available ||
      !discovery.diagnostics.native_backend ||
      discovery.diagnostics.fallback_used) {
    return 2;
  }
  if (discovery.diagnostics.face_count != discovery.font_faces().size() ||
      discovery.font_faces().size() <= 1) {
    return 3;
  }

  for (const cgpui::FontFaceDescriptor& face : discovery.font_faces()) {
    if (face.source != cgpui::FontSource::platform ||
        face.font.family.empty() ||
        face.path.empty() ||
        !has_prefix(face.path, "directwrite://")) {
      return 4;
    }
  }

  const cgpui::FontDatabase database = (*app)->discover_fonts();
  const cgpui::FontFaceDescriptor* resolved =
      database.resolve(discovery.font_faces().front().font);
  if (database.face_count() <= 1 || resolved == nullptr ||
      resolved->font.family != discovery.font_faces().front().font.family) {
    return 5;
  }

  return 0;
}
