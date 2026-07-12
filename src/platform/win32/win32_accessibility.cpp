#include "win32_internal.hpp"

#include <UIAutomationCoreApi.h>

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
  retire_win32_uia_provider_lifetime(provider_tree_, providers_);
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

void Win32UiaAccessibilityAdapter::set_event_operations(
    Win32UiaEventOperations operations) {
  event_operations_ = operations;
}

Win32UiaEventPublication
Win32UiaAccessibilityAdapter::last_event_publication() const {
  return last_event_publication_;
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
