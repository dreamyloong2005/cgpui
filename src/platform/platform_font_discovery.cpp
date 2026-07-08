#include "cgpui/platform/platform_application.hpp"

#include <utility>

namespace cgpui {

PlatformFontDiscoveryResult::PlatformFontDiscoveryResult(
    PlatformFontDiscoveryDiagnostics diagnostics,
    std::vector<FontFaceDescriptor> font_faces)
    : diagnostics(diagnostics), font_faces_(std::move(font_faces)) {}

bool PlatformFontDiscoveryResult::empty() const {
  return font_faces_.empty();
}

std::span<const FontFaceDescriptor> PlatformFontDiscoveryResult::font_faces()
    const {
  return font_faces_;
}

bool platform_font_backend_is_native(PlatformFontDiscoveryBackend backend) {
  return backend == PlatformFontDiscoveryBackend::direct_write ||
      backend == PlatformFontDiscoveryBackend::fontconfig ||
      backend == PlatformFontDiscoveryBackend::core_text;
}

PlatformFontDiscoveryResult make_platform_font_discovery_result(
    PlatformFontDiscoveryBackend backend,
    PlatformFontDiscoveryStatus status,
    std::vector<FontFaceDescriptor> font_faces) {
  PlatformFontDiscoveryDiagnostics diagnostics{
      .backend = backend,
      .status = status,
      .face_count = font_faces.size(),
      .native_backend =
          platform_font_backend_is_native(backend) &&
          status == PlatformFontDiscoveryStatus::native_available,
      .fallback_used =
          status == PlatformFontDiscoveryStatus::deterministic_fallback,
  };
  return PlatformFontDiscoveryResult{diagnostics, std::move(font_faces)};
}

PlatformFontDiscoveryResult PlatformApplication::discover_font_discovery()
    const {
  return make_platform_font_discovery_result(
      PlatformFontDiscoveryBackend::unsupported,
      PlatformFontDiscoveryStatus::unsupported,
      {});
}

std::vector<FontFaceDescriptor> PlatformApplication::discover_font_records()
    const {
  const PlatformFontDiscoveryResult discovery = discover_font_discovery();
  const std::span<const FontFaceDescriptor> font_faces =
      discovery.font_faces();
  return std::vector<FontFaceDescriptor>(font_faces.begin(), font_faces.end());
}

FontDatabase PlatformApplication::discover_fonts() const {
  const PlatformFontDiscoveryResult discovery = discover_font_discovery();
  return font_database_from_discovered_faces(discovery.font_faces());
}

} // namespace cgpui
