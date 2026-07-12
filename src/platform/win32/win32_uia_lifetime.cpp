#include "win32_uia_lifetime_internal.hpp"

#include <unordered_map>
#include <utility>

namespace cgpui {

Win32UiaProviderLifetimeDiagnostics reconcile_win32_uia_provider_lifetime(
    Win32UiaProviderTreeHandle& tree,
    HWND hwnd,
    std::uint64_t root_element_id,
    const std::vector<Win32UiaProviderNode>& previous_nodes,
    const std::vector<Win32UiaProviderNode>& current_nodes,
    std::function<void(AccessibilityActionRequested)> action_callback,
    std::vector<IRawElementProviderSimple*>& providers) {
  Win32UiaProviderLifetimeDiagnostics diagnostics;
  if (!tree) {
    tree = create_win32_uia_provider_tree(
        hwnd, root_element_id, current_nodes, action_callback);
  } else {
    replace_win32_uia_provider_tree_state(
        tree, hwnd, root_element_id, current_nodes, action_callback);
  }

  std::unordered_map<std::uint64_t, IRawElementProviderSimple*> retained;
  for (std::size_t index = 0;
       index < previous_nodes.size() && index < providers.size(); ++index) {
    retained.emplace(previous_nodes[index].element_id, providers[index]);
  }

  std::vector<IRawElementProviderSimple*> next;
  next.reserve(current_nodes.size());
  for (const Win32UiaProviderNode& node : current_nodes) {
    const auto existing = retained.find(node.element_id);
    if (existing == retained.end()) {
      IRawElementProviderSimple* provider = create_win32_uia_provider(
          tree, node, node.element_id == root_element_id);
      next.push_back(provider);
      diagnostics.created_count += provider != nullptr ? 1U : 0U;
      diagnostics.active_count += provider != nullptr ? 1U : 0U;
      continue;
    }
    replace_win32_uia_provider_node(
        existing->second, node, node.element_id == root_element_id);
    next.push_back(existing->second);
    diagnostics.reused_count += existing->second != nullptr ? 1U : 0U;
    diagnostics.active_count += existing->second != nullptr ? 1U : 0U;
    retained.erase(existing);
  }

  for (const auto& [element_id, provider] : retained) {
    (void)element_id;
    retire_win32_uia_provider(provider);
    if (provider != nullptr) provider->Release();
    diagnostics.retired_count += provider != nullptr ? 1U : 0U;
  }
  providers = std::move(next);
  return diagnostics;
}

void retire_win32_uia_provider_lifetime(
    Win32UiaProviderTreeHandle& tree,
    std::vector<IRawElementProviderSimple*>& providers) {
  for (IRawElementProviderSimple* provider : providers) {
    retire_win32_uia_provider(provider);
    if (provider != nullptr) provider->Release();
  }
  providers.clear();
  invalidate_win32_uia_provider_tree(tree);
  tree.reset();
}

} // namespace cgpui
