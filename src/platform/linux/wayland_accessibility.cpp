#include "wayland_internal.hpp"

namespace cgpui {
namespace {

std::string atspi_object_path_for(std::uint64_t element_id) {
  return "/org/a11y/atspi/accessible/" + std::to_string(element_id);
}

struct WaylandAtspiObjectNode {
  std::uint64_t element_id = 0;
  std::optional<std::uint64_t> parent_element_id;
  std::string object_path;
  std::optional<std::string> parent_object_path;
  PlatformAccessibilityRole role = PlatformAccessibilityRole::generic;
  std::string name;
  std::string text;
  std::string value;
  bool enabled = true;
  bool focusable = false;
  bool focused = false;
  std::optional<Rect> bounds;
  std::size_t child_count = 0;
};

} // namespace

class WaylandAtspiAccessibilityAdapter {
 public:
  void update(PlatformAccessibilityTreeUpdate update) {
    last_update_ = std::move(update);
    live_updates_ = last_update_.live_updates;
    root_element_id_ = last_update_.root_element_id;
    node_count_ = last_update_.node_count;
    focused_node_count_ = last_update_.focused_node_count;
    text_input_node_count_ = 0;
    atspi_object_nodes_.clear();
    atspi_object_nodes_.reserve(last_update_.nodes.size());
    for (const PlatformAccessibilityNodeUpdate& node : last_update_.nodes) {
      if (node.role == PlatformAccessibilityRole::text_input) {
        text_input_node_count_ += 1;
      }
      std::optional<std::string> parent_object_path;
      if (node.parent_element_id.has_value()) {
        parent_object_path = atspi_object_path_for(*node.parent_element_id);
      }
      atspi_object_nodes_.push_back(WaylandAtspiObjectNode{
          .element_id = node.element_id,
          .parent_element_id = node.parent_element_id,
          .object_path = atspi_object_path_for(node.element_id),
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
  }

  [[nodiscard]] const std::vector<WaylandAtspiObjectNode>& atspi_object_nodes()
      const {
    return atspi_object_nodes_;
  }

  [[nodiscard]] const std::vector<PlatformAccessibilityLiveUpdate>&
  last_live_updates() const {
    return live_updates_;
  }

 private:
  PlatformAccessibilityTreeUpdate last_update_;
  std::vector<WaylandAtspiObjectNode> atspi_object_nodes_;
  std::vector<PlatformAccessibilityLiveUpdate> live_updates_;
  std::uint64_t root_element_id_ = 0;
  std::size_t node_count_ = 0;
  std::size_t focused_node_count_ = 0;
  std::size_t text_input_node_count_ = 0;
};

void WaylandAtspiAccessibilityAdapterDeleter::operator()(
    WaylandAtspiAccessibilityAdapter* adapter) const {
  delete adapter;
}

WaylandAtspiAccessibilityAdapterPtr
create_wayland_atspi_accessibility_adapter() {
  return WaylandAtspiAccessibilityAdapterPtr(
      new WaylandAtspiAccessibilityAdapter());
}

void wayland_atspi_update_accessibility_tree(
    WaylandAtspiAccessibilityAdapter& adapter,
    PlatformAccessibilityTreeUpdate update) {
  adapter.update(std::move(update));
}

} // namespace cgpui
