#include "win32_uia_provider_object_internal.hpp"

#include <UIAutomationCoreApi.h>

#include <utility>

namespace cgpui {

std::optional<Win32UiaProviderSnapshot> Win32UiaProvider::snapshot() const {
  const std::lock_guard lock(state_mutex_);
  if (!available_) return std::nullopt;
  return Win32UiaProviderSnapshot{node_, is_root_};
}

void Win32UiaProvider::replace_node(
    Win32UiaProviderNode node,
    bool is_root) {
  const std::lock_guard lock(state_mutex_);
  node_ = std::move(node);
  is_root_ = is_root;
}

void Win32UiaProvider::retire() {
  std::optional<std::uint64_t> element_id;
  {
    const std::lock_guard lock(state_mutex_);
    if (!available_) return;
    available_ = false;
    element_id = node_.element_id;
  }
  unregister_win32_uia_fragment(
      tree_, *element_id, static_cast<IRawElementProviderFragment*>(this));
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_HostRawElementProvider(
    IRawElementProviderSimple** provider) {
  if (provider == nullptr) return E_POINTER;
  *provider = nullptr;
  if (!snapshot().has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  const HWND hwnd = win32_uia_provider_tree_hwnd(tree_);
  return hwnd == nullptr ? S_OK : UiaHostProviderFromHwnd(hwnd, provider);
}

void replace_win32_uia_provider_node(
    IRawElementProviderSimple* provider,
    Win32UiaProviderNode node,
    bool is_root) {
  if (provider == nullptr) return;
  static_cast<Win32UiaProvider*>(provider)->replace_node(
      std::move(node), is_root);
}

void retire_win32_uia_provider(IRawElementProviderSimple* provider) {
  if (provider == nullptr) return;
  static_cast<Win32UiaProvider*>(provider)->retire();
}

} // namespace cgpui
