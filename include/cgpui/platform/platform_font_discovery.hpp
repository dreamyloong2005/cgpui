#pragma once

#include "cgpui/ui/text_font.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace cgpui {

enum class PlatformFontDiscoveryBackend {
  unsupported,
  direct_write,
  fontconfig,
  core_text,
  deterministic,
};

enum class PlatformFontDiscoveryStatus {
  unsupported,
  native_available,
  native_unavailable,
  deterministic_fallback,
};

struct PlatformFontDiscoveryDiagnostics {
  PlatformFontDiscoveryBackend backend =
      PlatformFontDiscoveryBackend::unsupported;
  PlatformFontDiscoveryStatus status =
      PlatformFontDiscoveryStatus::unsupported;
  std::size_t face_count = 0;
  bool native_backend = false;
  bool fallback_used = false;
};

class PlatformFontDiscoveryResult {
 public:
  PlatformFontDiscoveryResult() = default;
  PlatformFontDiscoveryResult(
      PlatformFontDiscoveryDiagnostics diagnostics,
      std::vector<FontFaceDescriptor> font_faces);

  [[nodiscard]] bool empty() const;
  [[nodiscard]] std::span<const FontFaceDescriptor> font_faces() const;

  PlatformFontDiscoveryDiagnostics diagnostics;

 private:
  std::vector<FontFaceDescriptor> font_faces_;
};

[[nodiscard]] bool platform_font_backend_is_native(
    PlatformFontDiscoveryBackend backend);

[[nodiscard]] PlatformFontDiscoveryResult make_platform_font_discovery_result(
    PlatformFontDiscoveryBackend backend,
    PlatformFontDiscoveryStatus status,
    std::vector<FontFaceDescriptor> font_faces);

} // namespace cgpui
