#include "win32_internal.hpp"

#include <UIAutomationCoreApi.h>

#include <utility>

namespace cgpui {

Win32UiaAccessibilityAdapter::~Win32UiaAccessibilityAdapter() {
  release_providers();
}

void Win32UiaAccessibilityAdapter::attach(HWND hwnd) {
  hwnd_ = hwnd;
  set_win32_uia_provider_tree_hwnd(provider_tree_, hwnd);
}

void Win32UiaAccessibilityAdapter::detach() {
  attach(nullptr);
}

void Win32UiaAccessibilityAdapter::release_providers() {
  for (IRawElementProviderSimple* provider : providers_) {
    if (provider != nullptr) provider->Release();
  }
  providers_.clear();
}

void Win32UiaAccessibilityAdapter::update(
    PlatformAccessibilityTreeUpdate update) {
  last_update_ = std::move(update);
  live_updates_ = last_update_.live_updates;
  root_element_id_ = last_update_.root_element_id;
  node_count_ = last_update_.node_count;
  focused_node_count_ = last_update_.focused_node_count;
  text_input_node_count_ = 0;
  release_providers();
  provider_tree_.reset();
  provider_nodes_.clear();
  provider_nodes_.reserve(last_update_.nodes.size());
  for (const PlatformAccessibilityNodeUpdate& node : last_update_.nodes) {
    if (node.role == PlatformAccessibilityRole::text_input) {
      text_input_node_count_ += 1;
    }
    Win32UiaProviderNode provider_node{
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
    };
    provider_nodes_.push_back(std::move(provider_node));
  }
  provider_tree_ = create_win32_uia_provider_tree(
      hwnd_, root_element_id_, provider_nodes_);
  providers_.reserve(provider_nodes_.size());
  for (const Win32UiaProviderNode& node : provider_nodes_) {
    providers_.push_back(create_win32_uia_provider(
        provider_tree_, node, node.element_id == root_element_id_));
  }
}

bool Win32UiaAccessibilityAdapter::handle_get_object(
    WPARAM wparam,
    LPARAM lparam,
    LRESULT& result) const {
  IRawElementProviderSimple* root = root_provider();
  if (hwnd_ == nullptr || lparam != UiaRootObjectId || root == nullptr) {
    return false;
  }
  result = UiaReturnRawElementProvider(hwnd_, wparam, lparam, root);
  return true;
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

IRawElementProviderSimple* Win32UiaAccessibilityAdapter::root_provider() const {
  return provider_for_element(root_element_id_);
}

IRawElementProviderSimple* Win32UiaAccessibilityAdapter::provider_for_element(
    std::uint64_t element_id) const {
  for (std::size_t index = 0; index < provider_nodes_.size(); ++index) {
    if (provider_nodes_[index].element_id == element_id) return providers_[index];
  }
  return nullptr;
}

} // namespace cgpui
