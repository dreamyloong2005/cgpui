#include "wayland_atspi_object_internal.hpp"

#include <unordered_map>
#include <utility>

namespace cgpui {

std::string wayland_atspi_object_path_for(std::uint64_t element_id) {
  return "/org/a11y/atspi/accessible/" + std::to_string(element_id);
}

std::vector<WaylandAtspiObjectNode> build_wayland_atspi_object_nodes(
    const PlatformAccessibilityTreeUpdate& update) {
  std::vector<WaylandAtspiObjectNode> objects;
  objects.reserve(update.nodes.size());
  std::unordered_map<std::uint64_t, std::size_t> object_indices;
  object_indices.reserve(update.nodes.size());
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
    object_indices.insert_or_assign(node.element_id, objects.size() - 1);
  }
  for (WaylandAtspiObjectNode& object : objects) {
    if (!object.parent_element_id.has_value()) continue;
    const auto parent = object_indices.find(*object.parent_element_id);
    if (parent == object_indices.end()) continue;
    WaylandAtspiObjectNode& parent_object = objects[parent->second];
    object.index_in_parent =
        static_cast<std::int32_t>(parent_object.child_object_paths.size());
    parent_object.child_object_paths.push_back(object.object_path);
  }
  return objects;
}

} // namespace cgpui
