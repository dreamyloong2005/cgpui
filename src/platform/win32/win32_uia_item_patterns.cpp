#include "win32_internal.hpp"
#include "win32_uia_item_patterns_internal.hpp"

namespace cgpui {

HRESULT Win32UiaProvider::request_action(AccessibilityActionKind kind) const {
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  return request_win32_uia_action(tree_, AccessibilityActionRequested{
      .kind = kind,
      .element_id = state->node.element_id,
  });
}

bool get_win32_uia_item_pattern(
    Win32UiaProvider* owner,
    const Win32UiaProviderNode& node,
    PATTERNID pattern,
    IUnknown** provider) {
  if (pattern == UIA_ExpandCollapsePatternId) {
    if (node.patterns.expanded.has_value()) {
      *provider = static_cast<IExpandCollapseProvider*>(
          new Win32UiaItemPatterns(owner));
    }
    return true;
  }
  if (pattern == UIA_SelectionItemPatternId) {
    if (node.patterns.selected.has_value()) {
      *provider = static_cast<ISelectionItemProvider*>(
          new Win32UiaItemPatterns(owner));
    }
    return true;
  }
  return false;
}

Win32UiaItemPatterns::Win32UiaItemPatterns(Win32UiaProvider* owner)
    : owner_(owner) {
  owner_->AddRef();
}

Win32UiaItemPatterns::~Win32UiaItemPatterns() {
  owner_->Release();
}

HRESULT STDMETHODCALLTYPE Win32UiaItemPatterns::QueryInterface(
    REFIID iid,
    void** object) {
  if (object == nullptr) return E_POINTER;
  *object = nullptr;
  if (IsEqualIID(iid, IID_IUnknown) ||
      IsEqualIID(iid, IID_IExpandCollapseProvider)) {
    *object = static_cast<IExpandCollapseProvider*>(this);
  } else if (IsEqualIID(iid, IID_ISelectionItemProvider)) {
    *object = static_cast<ISelectionItemProvider*>(this);
  } else {
    return E_NOINTERFACE;
  }
  AddRef();
  return S_OK;
}

ULONG STDMETHODCALLTYPE Win32UiaItemPatterns::AddRef() {
  return ++reference_count_;
}

ULONG STDMETHODCALLTYPE Win32UiaItemPatterns::Release() {
  const ULONG count = --reference_count_;
  if (count == 0) delete this;
  return count;
}

} // namespace cgpui
