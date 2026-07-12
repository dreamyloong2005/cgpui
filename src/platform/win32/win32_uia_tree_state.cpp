#include "win32_uia_tree_state_internal.hpp"

#include <utility>

namespace cgpui {

void replace_win32_uia_provider_tree_state(
    const Win32UiaProviderTreeHandle& tree,
    HWND hwnd,
    std::uint64_t root_element_id,
    std::vector<Win32UiaProviderNode> nodes,
    std::function<void(AccessibilityActionRequested)> action_callback) {
  if (!tree) return;
  const std::lock_guard lock(tree->mutex);
  tree->hwnd = hwnd;
  tree->root_element_id = root_element_id;
  tree->nodes = std::move(nodes);
  tree->action_callback = std::move(action_callback);
  tree->node_indices.clear();
  tree->children.clear();
  for (std::size_t index = 0; index < tree->nodes.size(); ++index) {
    const auto& node = tree->nodes[index];
    tree->node_indices.emplace(node.element_id, index);
    if (node.parent_element_id.has_value()) {
      tree->children[*node.parent_element_id].push_back(node.element_id);
    }
  }
}

void invalidate_win32_uia_provider_tree(
    const Win32UiaProviderTreeHandle& tree) {
  if (!tree) return;
  const std::lock_guard lock(tree->mutex);
  tree->hwnd = nullptr;
  tree->root_element_id = 0;
  tree->nodes.clear();
  tree->node_indices.clear();
  tree->children.clear();
  tree->providers.clear();
  tree->action_callback = {};
}

} // namespace cgpui
