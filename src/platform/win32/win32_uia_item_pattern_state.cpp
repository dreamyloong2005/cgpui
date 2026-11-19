#include "win32_internal.hpp"
#include "win32_uia_item_patterns_internal.hpp"

namespace cgpui {

HRESULT STDMETHODCALLTYPE Win32UiaItemPatterns::Expand() {
  const auto state = owner_->snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  if (!state->node.patterns.expanded.has_value()) return UIA_E_INVALIDOPERATION;
  return owner_->request_action(AccessibilityActionKind::expand);
}

HRESULT STDMETHODCALLTYPE Win32UiaItemPatterns::Collapse() {
  const auto state = owner_->snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  if (!state->node.patterns.expanded.has_value()) return UIA_E_INVALIDOPERATION;
  return owner_->request_action(AccessibilityActionKind::collapse);
}

HRESULT STDMETHODCALLTYPE Win32UiaItemPatterns::get_ExpandCollapseState(
    ExpandCollapseState* result) {
  if (result == nullptr) return E_POINTER;
  *result = ExpandCollapseState_LeafNode;
  const auto state = owner_->snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  if (state->node.patterns.expanded.has_value()) {
    *result = *state->node.patterns.expanded ? ExpandCollapseState_Expanded
                                             : ExpandCollapseState_Collapsed;
  }
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaItemPatterns::Select() {
  return owner_->request_action(AccessibilityActionKind::select);
}

HRESULT STDMETHODCALLTYPE Win32UiaItemPatterns::AddToSelection() {
  return owner_->request_action(AccessibilityActionKind::add_to_selection);
}

HRESULT STDMETHODCALLTYPE Win32UiaItemPatterns::RemoveFromSelection() {
  return owner_->request_action(AccessibilityActionKind::remove_from_selection);
}

HRESULT STDMETHODCALLTYPE Win32UiaItemPatterns::get_IsSelected(BOOL* result) {
  if (result == nullptr) return E_POINTER;
  *result = FALSE;
  const auto state = owner_->snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  *result = state->node.patterns.selected.value_or(false) ? TRUE : FALSE;
  return S_OK;
}

// The container is the parent node (the tree of a tree item, the tab list
// of a tab).
HRESULT STDMETHODCALLTYPE Win32UiaItemPatterns::get_SelectionContainer(
    IRawElementProviderSimple** result) {
  if (result == nullptr) return E_POINTER;
  *result = nullptr;
  IRawElementProviderFragment* parent = nullptr;
  const HRESULT status = owner_->Navigate(NavigateDirection_Parent, &parent);
  if (FAILED(status) || parent == nullptr) return status;
  const HRESULT query = parent->QueryInterface(
      IID_IRawElementProviderSimple, reinterpret_cast<void**>(result));
  parent->Release();
  return query;
}

} // namespace cgpui
