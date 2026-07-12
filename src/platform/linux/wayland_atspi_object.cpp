#include "wayland_atspi_object_internal.hpp"

#include <utility>

namespace cgpui {

std::string wayland_atspi_object_path_for(std::uint64_t element_id) {
  return "/org/a11y/atspi/accessible/" + std::to_string(element_id);
}

std::vector<WaylandAtspiObjectNode> build_wayland_atspi_object_nodes(
    const PlatformAccessibilityTreeUpdate& update) {
  std::vector<WaylandAtspiObjectNode> objects;
  objects.reserve(update.nodes.size());
  for (const PlatformAccessibilityNodeUpdate& node : update.nodes) {
    std::optional<std::string> parent_object_path;
    if (node.parent_element_id.has_value()) {
      parent_object_path =
          wayland_atspi_object_path_for(*node.parent_element_id);
    }
    objects.push_back(WaylandAtspiObjectNode{
        .element_id = node.element_id,
        .parent_element_id = node.parent_element_id,
        .object_path = wayland_atspi_object_path_for(node.element_id),
        .parent_object_path = std::move(parent_object_path),
        .role = node.role,
        .name = node.name,
        .text = node.text,
        .value = node.value,
        .enabled = node.enabled,
        .focusable = node.focusable,
        .focused = node.focused,
        .bounds = node.bounds,
        .child_count = node.child_count,
    });
  }
  return objects;
}

} // namespace cgpui
