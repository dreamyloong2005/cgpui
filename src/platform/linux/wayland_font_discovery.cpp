#include "wayland_application_internal.hpp"

#include <utility>
#include <vector>

namespace cgpui {

PlatformFontDiscoveryResult WaylandApplication::discover_font_discovery()
    const {
  std::vector<FontFaceDescriptor> records{
      FontFaceDescriptor{
          .font = FontDescriptor{.family = "sans-serif"},
          .postscript_name = "fontconfig:sans-serif",
          .source = FontSource::platform,
          .path = "fontconfig://sans-serif",
      },
  };
  return make_platform_font_discovery_result(
      PlatformFontDiscoveryBackend::fontconfig,
      PlatformFontDiscoveryStatus::deterministic_fallback,
      std::move(records));
}

} // namespace cgpui
