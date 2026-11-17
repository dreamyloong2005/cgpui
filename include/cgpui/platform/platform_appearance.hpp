#pragma once

#include "cgpui/core/geometry.hpp"

#include <functional>
#include <string>

namespace cgpui {

enum class SystemAppearanceMode {
  light,
  dark,
};

struct SystemAppearance {
  bool supported = false;
  std::string backend;
  SystemAppearanceMode mode = SystemAppearanceMode::light;
  bool accent_color_available = false;
  Color accent_color{};
  bool transparency_effects_enabled = false;
  bool client_area_animation_enabled = true;
  bool high_contrast = false;
  // Why the snapshot is unsupported; empty when supported is true.
  std::string unsupported_reason;
};

[[nodiscard]] bool operator==(
    const SystemAppearance& lhs,
    const SystemAppearance& rhs);

enum class SystemAppearanceChangeSource {
  color_set,
  colorization,
  client_area_animation,
  high_contrast,
};

struct SystemAppearanceChange {
  SystemAppearanceChangeSource source = SystemAppearanceChangeSource::color_set;
  SystemAppearance appearance;
  bool changed = false;
};

using SystemAppearanceCallback =
    std::function<void(const SystemAppearanceChange&)>;

} // namespace cgpui
