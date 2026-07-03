#include "win32_internal.hpp"

#include <utility>

namespace cgpui {

void Win32UiaAccessibilityAdapter::update(
    PlatformAccessibilityTreeUpdate update) {
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
        .enabled = node.enabled,
        .focusable = node.focusable,
        .focused = node.focused,
        .bounds = node.bounds,
        .child_count = node.child_count,
    });
  }
}

std::uint64_t Win32UiaAccessibilityAdapter::root_element_id() const {
  return root_element_id_;
}

std::size_t Win32UiaAccessibilityAdapter::node_count() const {
  return node_count_;
}

std::size_t Win32UiaAccessibilityAdapter::focused_node_count() const {
  return focused_node_count_;
}

std::size_t Win32UiaAccessibilityAdapter::text_input_node_count() const {
  return text_input_node_count_;
}

const std::vector<Win32UiaProviderNode>&
Win32UiaAccessibilityAdapter::uia_provider_nodes() const {
  return provider_nodes_;
}

const std::vector<PlatformAccessibilityLiveUpdate>&
Win32UiaAccessibilityAdapter::last_live_updates() const {
  return live_updates_;
}

} // namespace cgpui
