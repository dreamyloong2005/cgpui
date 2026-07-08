#include "win32_internal.hpp"

#include <utility>
#include <vector>

namespace cgpui {

PlatformFontDiscoveryResult win32_discover_fonts() {
  std::vector<FontFaceDescriptor> records{
      FontFaceDescriptor{
          .font = FontDescriptor{.family = "Segoe UI"},
          .postscript_name = "SegoeUI",
          .source = FontSource::platform,
          .path = "win32://Segoe UI",
      },
  };
  return make_platform_font_discovery_result(
      PlatformFontDiscoveryBackend::direct_write,
      PlatformFontDiscoveryStatus::deterministic_fallback,
      std::move(records));
}

} // namespace cgpui
