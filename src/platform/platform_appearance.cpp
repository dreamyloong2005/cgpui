#include "cgpui/platform/platform_application.hpp"

namespace cgpui {

bool operator==(const SystemAppearance& lhs, const SystemAppearance& rhs) {
  return lhs.supported == rhs.supported && lhs.backend == rhs.backend &&
         lhs.mode == rhs.mode &&
         lhs.accent_color_available == rhs.accent_color_available &&
         lhs.accent_color.r == rhs.accent_color.r &&
         lhs.accent_color.g == rhs.accent_color.g &&
         lhs.accent_color.b == rhs.accent_color.b &&
         lhs.accent_color.a == rhs.accent_color.a &&
         lhs.transparency_effects_enabled == rhs.transparency_effects_enabled &&
         lhs.client_area_animation_enabled ==
             rhs.client_area_animation_enabled &&
         lhs.high_contrast == rhs.high_contrast;
}

// Platforms without a native appearance source report a typed unsupported
// snapshot with the neutral light defaults instead of fabricating a reading.
SystemAppearance PlatformApplication::system_appearance() const {
  return SystemAppearance{
      .supported = false,
      .backend = "unsupported",
      .mode = SystemAppearanceMode::light,
  };
}

void PlatformApplication::set_system_appearance_callback(
    SystemAppearanceCallback) {}

} // namespace cgpui
