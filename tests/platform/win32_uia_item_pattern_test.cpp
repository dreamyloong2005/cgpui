// UIA ExpandCollapse and SelectionItem patterns of tree items and tabs,
// queried through the Win32 provider.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ole2.h>
#include <UIAutomationCore.h>
#include <UIAutomationCoreApi.h>
#include <UIAutomationClient.h>
#include <OleAuto.h>

#include "win32_accessibility_internal.hpp"

#include <optional>
#include <vector>

namespace {

using cgpui::PlatformAccessibilityRole;

cgpui::PlatformAccessibilityTreeUpdate tree_update(bool expanded) {
  return cgpui::PlatformAccessibilityTreeUpdate{
      .root_element_id = 1,
      .node_count = 6,
      .nodes = {
          {.element_id = 1},
          {.element_id = 2, .parent_element_id = 1, .role = PlatformAccessibilityRole::tree},
          {.element_id = 3, .parent_element_id = 2, .role = PlatformAccessibilityRole::tree_item,
           .name = "src",
           .patterns = {.expanded = expanded, .selected = true, .level = 1}},
          {.element_id = 4, .parent_element_id = 2, .role = PlatformAccessibilityRole::tree_item,
           .name = "README.md", .patterns = {.selected = false, .level = 1}},
          {.element_id = 5, .parent_element_id = 1, .role = PlatformAccessibilityRole::tab_list},
          {.element_id = 6, .parent_element_id = 5, .role = PlatformAccessibilityRole::tab,
           .name = "main.cpp", .patterns = {.selected = true}},
      }};
}

template <typename T>
T* pattern(cgpui::Win32UiaAccessibilityAdapter& adapter, std::uint64_t id, PATTERNID which) {
  IUnknown* provider = reinterpret_cast<IUnknown*>(1);
  if (adapter.provider_for_element(id)->GetPatternProvider(which, &provider) != S_OK) {
    return nullptr;
  }
  return reinterpret_cast<T*>(provider);
}

std::optional<VARIANT> property(cgpui::Win32UiaAccessibilityAdapter& adapter,
                                std::uint64_t id, PROPERTYID which) {
  VARIANT value{};
  if (adapter.provider_for_element(id)->GetPropertyValue(which, &value) != S_OK) {
    return std::nullopt;
  }
  return value;
}

long control_type(IRawElementProviderSimple* provider) {
  VARIANT value{};
  if (provider == nullptr ||
      provider->GetPropertyValue(UIA_ControlTypePropertyId, &value) != S_OK ||
      value.vt != VT_I4) {
    return 0;
  }
  return value.lVal;
}

} // namespace

int main() {
  using Kind = cgpui::AccessibilityActionKind;
  std::vector<cgpui::AccessibilityActionRequested> actions;
  cgpui::Win32UiaAccessibilityAdapter adapter;
  adapter.set_action_callback([&](cgpui::AccessibilityActionRequested action) {
    actions.push_back(std::move(action));
  });
  adapter.update(tree_update(false));

  // ExpandCollapse: tree items with children only.
  auto* expand = pattern<IExpandCollapseProvider>(adapter, 3, UIA_ExpandCollapsePatternId);
  ExpandCollapseState state = ExpandCollapseState_LeafNode;
  if (expand == nullptr || expand->get_ExpandCollapseState(&state) != S_OK ||
      state != ExpandCollapseState_Collapsed) return 1;
  if (expand->Expand() != S_OK || actions.size() != 1 ||
      actions.back().kind != Kind::expand || actions.back().element_id != 3) return 2;
  if (expand->Collapse() != S_OK || actions.back().kind != Kind::collapse) return 3;
  adapter.update(tree_update(true));
  if (expand->get_ExpandCollapseState(&state) != S_OK ||
      state != ExpandCollapseState_Expanded) return 4;
  expand->Release();
  if (pattern<IUnknown>(adapter, 4, UIA_ExpandCollapsePatternId) != nullptr) return 5;

  // SelectionItem: tree items and tabs.
  auto* item = pattern<ISelectionItemProvider>(adapter, 3, UIA_SelectionItemPatternId);
  BOOL selected = FALSE;
  if (item == nullptr || item->get_IsSelected(&selected) != S_OK || selected != TRUE) return 10;
  IRawElementProviderSimple* container = nullptr;
  if (item->get_SelectionContainer(&container) != S_OK ||
      control_type(container) != UIA_TreeControlTypeId) return 11;
  container->Release();
  if (item->Select() != S_OK || actions.back().kind != Kind::select ||
      item->AddToSelection() != S_OK || actions.back().kind != Kind::add_to_selection ||
      item->RemoveFromSelection() != S_OK ||
      actions.back().kind != Kind::remove_from_selection ||
      actions.back().element_id != 3) return 12;
  item->Release();
  auto* leaf = pattern<ISelectionItemProvider>(adapter, 4, UIA_SelectionItemPatternId);
  if (leaf == nullptr || leaf->get_IsSelected(&selected) != S_OK || selected != FALSE) return 13;
  leaf->Release();
  auto* tab = pattern<ISelectionItemProvider>(adapter, 6, UIA_SelectionItemPatternId);
  if (tab == nullptr || tab->get_IsSelected(&selected) != S_OK || selected != TRUE ||
      tab->get_SelectionContainer(&container) != S_OK ||
      control_type(container) != UIA_TabControlTypeId) return 14;
  container->Release();
  tab->Release();
  if (pattern<IUnknown>(adapter, 2, UIA_SelectionItemPatternId) != nullptr) return 15;

  // Pattern availability and state properties.
  const auto available = [&](std::uint64_t id, PROPERTYID which) {
    const auto value = property(adapter, id, which);
    return value.has_value() && value->vt == VT_BOOL && value->boolVal == VARIANT_TRUE;
  };
  if (!available(3, UIA_IsExpandCollapsePatternAvailablePropertyId) ||
      available(4, UIA_IsExpandCollapsePatternAvailablePropertyId) ||
      !available(6, UIA_IsSelectionItemPatternAvailablePropertyId) ||
      !available(6, UIA_SelectionItemIsSelectedPropertyId) ||
      available(4, UIA_SelectionItemIsSelectedPropertyId)) return 20;
  const auto expanded = property(adapter, 3, UIA_ExpandCollapseExpandCollapseStatePropertyId);
  if (!expanded.has_value() || expanded->vt != VT_I4 ||
      expanded->lVal != ExpandCollapseState_Expanded) return 21;
  return 0;
}
