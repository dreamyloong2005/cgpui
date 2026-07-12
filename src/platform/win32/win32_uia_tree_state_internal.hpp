#pragma once

#include "win32_uia_navigation_internal.hpp"

#include <mutex>
#include <unordered_map>
#include <utility>

namespace cgpui {

class Win32UiaProviderTree {
 public:
  Win32UiaProviderTree(
      HWND initial_hwnd,
      std::uint64_t initial_root_element_id,
      std::vector<Win32UiaProviderNode> initial_nodes,
      std::function<void(AccessibilityActionRequested)> initial_callback)
      : hwnd(initial_hwnd),
        root_element_id(initial_root_element_id),
        nodes(std::move(initial_nodes)),
        action_callback(std::move(initial_callback)) {
    for (std::size_t index = 0; index < nodes.size(); ++index) {
      const auto& node = nodes[index];
      node_indices.emplace(node.element_id, index);
      if (node.parent_element_id.has_value()) {
        children[*node.parent_element_id].push_back(node.element_id);
      }
    }
  }

  std::mutex mutex;
  HWND hwnd = nullptr;
  std::uint64_t root_element_id = 0;
  std::vector<Win32UiaProviderNode> nodes;
  std::unordered_map<std::uint64_t, std::size_t> node_indices;
  std::unordered_map<std::uint64_t, std::vector<std::uint64_t>> children;
  std::unordered_map<std::uint64_t, IRawElementProviderFragment*> providers;
  std::function<void(AccessibilityActionRequested)> action_callback;
};

} // namespace cgpui
