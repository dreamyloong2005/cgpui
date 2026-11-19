#pragma once

#include "win32_uia_provider_object_internal.hpp"

#include <UIAutomationClient.h>
#include <UIAutomationCore.h>

#include <atomic>

namespace cgpui {

// Tear-off for the item patterns of tree items, tabs and submenu items:
// ExpandCollapse (patterns.expanded) and SelectionItem (patterns.selected).
// State is read from the owner's current node; actions go through the
// owner's action callback (AccessibilityActionKind::expand / collapse /
// select / add_to_selection / remove_from_selection).
class Win32UiaItemPatterns final
    : public IExpandCollapseProvider,
      public ISelectionItemProvider {
 public:
  explicit Win32UiaItemPatterns(Win32UiaProvider* owner);
  ~Win32UiaItemPatterns();

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;

  HRESULT STDMETHODCALLTYPE Expand() override;
  HRESULT STDMETHODCALLTYPE Collapse() override;
  HRESULT STDMETHODCALLTYPE get_ExpandCollapseState(
      ExpandCollapseState* result) override;

  HRESULT STDMETHODCALLTYPE Select() override;
  HRESULT STDMETHODCALLTYPE AddToSelection() override;
  HRESULT STDMETHODCALLTYPE RemoveFromSelection() override;
  HRESULT STDMETHODCALLTYPE get_IsSelected(BOOL* result) override;
  HRESULT STDMETHODCALLTYPE get_SelectionContainer(
      IRawElementProviderSimple** result) override;

 private:
  std::atomic<ULONG> reference_count_{1};
  Win32UiaProvider* owner_ = nullptr;
};

// GetPatternProvider for UIA_ExpandCollapsePatternId and
// UIA_SelectionItemPatternId; S_OK with nullptr when the node has no such
// state. Returns false for other patterns.
bool get_win32_uia_item_pattern(
    Win32UiaProvider* owner,
    const Win32UiaProviderNode& node,
    PATTERNID pattern,
    IUnknown** provider);

} // namespace cgpui
