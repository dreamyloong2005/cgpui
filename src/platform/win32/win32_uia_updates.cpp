#include "win32_internal.hpp"

#include <utility>

namespace cgpui {

void Win32UiaAccessibilityAdapter::update(
    PlatformAccessibilityTreeUpdate update) {
  std::vector<Win32UiaProviderNode> previous_nodes =
      std::move(provider_nodes_);
  last_update_ = std::move(update);
  live_updates_ = last_update_.live_updates;
  root_element_id_ = last_update_.root_element_id;
  node_count_ = last_update_.node_count;
  focused_node_count_ = last_update_.focused_node_count;
  text_input_node_count_ = 0;
  provider_nodes_.clear();
  provider_nodes_.reserve(last_update_.nodes.size());
  for (const PlatformAccessibilityNodeUpdate& node : last_update_.nodes) {
    if (node.role == PlatformAccessibilityRole::text_input) {
      text_input_node_count_ += 1;
    }
    provider_nodes_.push_back(Win32UiaProviderNode{
        .element_id = node.element_id,
        .parent_element_id = node.parent_element_id,
        .role = node.role,
        .name = node.name,
        .text = node.text,
        .value = node.value,
        .patterns = node.patterns,
        .enabled = node.enabled,
        .focusable = node.focusable,
        .focused = node.focused,
        .bounds = node.bounds,
        .child_count = node.child_count,
    });
  }
  last_lifetime_diagnostics_ = reconcile_win32_uia_provider_lifetime(
      provider_tree_, hwnd_, root_element_id_, previous_nodes,
      provider_nodes_, action_callback_, providers_);
  last_event_publication_ = publish_win32_uia_live_updates(
      live_updates_, previous_nodes, provider_nodes_, providers_,
      event_operations_);
}

} // namespace cgpui
