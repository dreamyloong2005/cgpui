#pragma once

#include "cgpui/platform/platform_accessibility.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace cgpui {

struct WaylandAtspiObjectNode {
  std::uint64_t element_id = 0;
  std::optional<std::uint64_t> parent_element_id;
  std::string object_path;
  std::optional<std::string> parent_object_path;
  PlatformAccessibilityRole role = PlatformAccessibilityRole::generic;
  std::string name;
  std::string text;
  std::string value;
  PlatformAccessibilityPatternState patterns;
  bool enabled = true;
  bool focusable = false;
  bool focused = false;
  std::optional<Rect> bounds;
  std::size_t child_count = 0;
  std::vector<std::string> child_object_paths;
  std::int32_t index_in_parent = -1;
};

[[nodiscard]] std::string wayland_atspi_object_path_for(
    std::uint64_t element_id);
[[nodiscard]] std::vector<WaylandAtspiObjectNode>
build_wayland_atspi_object_nodes(
    const PlatformAccessibilityTreeUpdate& update);

} // namespace cgpui
