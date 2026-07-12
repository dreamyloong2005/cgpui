#include "win32_uia_navigation_internal.hpp"

#include <UIAutomationCoreApi.h>

#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace cgpui {

class Win32UiaProviderTree {
 public:
  Win32UiaProviderTree(
      HWND hwnd,
      std::uint64_t root_element_id,
      std::vector<Win32UiaProviderNode> nodes,
      std::function<void(AccessibilityActionRequested)> action_callback)
      : hwnd(hwnd),
        root_element_id(root_element_id),
        nodes(std::move(nodes)),
        action_callback(std::move(action_callback)) {
    for (std::size_t index = 0; index < this->nodes.size(); ++index) {
      const auto& node = this->nodes[index];
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

namespace {

HRESULT return_provider(
    Win32UiaProviderTree& tree,
    std::uint64_t element_id,
    IRawElementProviderFragment** result) {
  const auto provider = tree.providers.find(element_id);
  if (provider == tree.providers.end() || provider->second == nullptr) return S_OK;
  provider->second->AddRef();
  *result = provider->second;
  return S_OK;
}

} // namespace

Win32UiaProviderTreeHandle create_win32_uia_provider_tree(
    HWND hwnd,
    std::uint64_t root_element_id,
    std::vector<Win32UiaProviderNode> nodes,
    std::function<void(AccessibilityActionRequested)> action_callback) {
  return std::make_shared<Win32UiaProviderTree>(
      hwnd, root_element_id, std::move(nodes), std::move(action_callback));
}

void set_win32_uia_provider_tree_hwnd(
    const Win32UiaProviderTreeHandle& tree,
    HWND hwnd) {
  if (!tree) return;
  const std::lock_guard lock(tree->mutex);
  tree->hwnd = hwnd;
}

HWND win32_uia_provider_tree_hwnd(const Win32UiaProviderTreeHandle& tree) {
  if (!tree) return nullptr;
  const std::lock_guard lock(tree->mutex);
  return tree->hwnd;
}

void register_win32_uia_fragment(
    const Win32UiaProviderTreeHandle& tree,
    std::uint64_t element_id,
    IRawElementProviderFragment* provider) {
  if (!tree) return;
  const std::lock_guard lock(tree->mutex);
  tree->providers[element_id] = provider;
}

void unregister_win32_uia_fragment(
    const Win32UiaProviderTreeHandle& tree,
    std::uint64_t element_id,
    IRawElementProviderFragment* provider) {
  if (!tree) return;
  const std::lock_guard lock(tree->mutex);
  const auto current = tree->providers.find(element_id);
  if (current != tree->providers.end() && current->second == provider) {
    tree->providers.erase(current);
  }
}

HRESULT navigate_win32_uia_fragment(
    const Win32UiaProviderTreeHandle& tree,
    std::uint64_t element_id,
    NavigateDirection direction,
    IRawElementProviderFragment** result) {
  if (result == nullptr) return E_POINTER;
  *result = nullptr;
  if (!tree) return S_OK;
  const std::lock_guard lock(tree->mutex);
  const auto index = tree->node_indices.find(element_id);
  if (index == tree->node_indices.end()) return S_OK;
  const auto& node = tree->nodes[index->second];
  std::optional<std::uint64_t> target;
  if (direction == NavigateDirection_Parent) {
    target = node.parent_element_id;
  } else if (direction == NavigateDirection_FirstChild ||
             direction == NavigateDirection_LastChild) {
    const auto children = tree->children.find(element_id);
    if (children != tree->children.end() && !children->second.empty()) {
      target = direction == NavigateDirection_FirstChild
          ? children->second.front()
          : children->second.back();
    }
  } else if (direction == NavigateDirection_NextSibling ||
             direction == NavigateDirection_PreviousSibling) {
    if (node.parent_element_id.has_value()) {
      const auto siblings = tree->children.find(*node.parent_element_id);
      if (siblings != tree->children.end()) {
        const auto current = std::find(
            siblings->second.begin(), siblings->second.end(), element_id);
        if (direction == NavigateDirection_NextSibling &&
            current != siblings->second.end() &&
            std::next(current) != siblings->second.end()) {
          target = *std::next(current);
        } else if (direction == NavigateDirection_PreviousSibling &&
                   current != siblings->second.begin() &&
                   current != siblings->second.end()) {
          target = *std::prev(current);
        }
      }
    }
  } else {
    return E_INVALIDARG;
  }
  return target.has_value() ? return_provider(*tree, *target, result) : S_OK;
}

HRESULT get_win32_uia_fragment_root(
    const Win32UiaProviderTreeHandle& tree,
    IRawElementProviderFragmentRoot** result) {
  if (result == nullptr) return E_POINTER;
  *result = nullptr;
  if (!tree) return S_OK;
  const std::lock_guard lock(tree->mutex);
  const auto provider = tree->providers.find(tree->root_element_id);
  return provider == tree->providers.end() || provider->second == nullptr
      ? S_OK
      : provider->second->QueryInterface(
            IID_IRawElementProviderFragmentRoot,
            reinterpret_cast<void**>(result));
}

HRESULT get_win32_uia_focus(
    const Win32UiaProviderTreeHandle& tree,
    IRawElementProviderFragment** result) {
  if (result == nullptr) return E_POINTER;
  *result = nullptr;
  if (!tree) return S_OK;
  const std::lock_guard lock(tree->mutex);
  const auto focused = std::find_if(
      tree->nodes.begin(), tree->nodes.end(),
      [](const Win32UiaProviderNode& node) { return node.focused; });
  return focused == tree->nodes.end()
      ? S_OK
      : return_provider(*tree, focused->element_id, result);
}

HRESULT get_win32_uia_provider_from_point(
    const Win32UiaProviderTreeHandle& tree,
    double x,
    double y,
    IRawElementProviderFragment** result) {
  if (result == nullptr) return E_POINTER;
  *result = nullptr;
  if (!tree) return S_OK;
  const std::lock_guard lock(tree->mutex);
  POINT point{static_cast<LONG>(x), static_cast<LONG>(y)};
  if (tree->hwnd != nullptr) ScreenToClient(tree->hwnd, &point);
  const auto hit = std::find_if(
      tree->nodes.rbegin(), tree->nodes.rend(),
      [&](const Win32UiaProviderNode& node) {
        if (!node.bounds.has_value()) return false;
        const Rect& bounds = *node.bounds;
        return point.x >= bounds.origin.x && point.y >= bounds.origin.y &&
            point.x < bounds.origin.x + bounds.size.width &&
            point.y < bounds.origin.y + bounds.size.height;
      });
  return hit == tree->nodes.rend()
      ? S_OK
      : return_provider(*tree, hit->element_id, result);
}

HRESULT request_win32_uia_action(
    const Win32UiaProviderTreeHandle& tree,
    AccessibilityActionRequested action) {
  if (!tree) return UIA_E_ELEMENTNOTAVAILABLE;
  std::function<void(AccessibilityActionRequested)> callback;
  {
    const std::lock_guard lock(tree->mutex);
    const auto node = tree->node_indices.find(action.element_id);
    if (node == tree->node_indices.end()) return UIA_E_ELEMENTNOTAVAILABLE;
    if (!tree->nodes[node->second].enabled) return UIA_E_ELEMENTNOTENABLED;
    callback = tree->action_callback;
  }
  if (!callback) return UIA_E_NOTSUPPORTED;
  callback(std::move(action));
  return S_OK;
}

} // namespace cgpui
