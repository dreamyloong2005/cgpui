#include "win32_uia_pattern_properties_internal.hpp"

#include <UIAutomationClient.h>

namespace cgpui {

bool set_win32_uia_pattern_availability_property(
    const Win32UiaProviderNode& node,
    PROPERTYID property,
    VARIANT* value) {
  bool available = false;
  if (property == UIA_IsInvokePatternAvailablePropertyId) {
    available = node.patterns.invokable;
  } else if (property == UIA_IsValuePatternAvailablePropertyId) {
    available = node.patterns.value_settable;
  } else if (property == UIA_IsTogglePatternAvailablePropertyId) {
    available = node.patterns.toggled.has_value();
  } else if (property == UIA_IsRangeValuePatternAvailablePropertyId) {
    available = node.patterns.range.has_value();
  } else if (property == UIA_IsExpandCollapsePatternAvailablePropertyId) {
    available = node.patterns.expanded.has_value();
  } else if (property == UIA_IsSelectionItemPatternAvailablePropertyId) {
    available = node.patterns.selected.has_value();
  } else if (property == UIA_SelectionItemIsSelectedPropertyId &&
             node.patterns.selected.has_value()) {
    available = *node.patterns.selected;
  } else if (property == UIA_ExpandCollapseExpandCollapseStatePropertyId &&
             node.patterns.expanded.has_value()) {
    value->vt = VT_I4;
    value->lVal = *node.patterns.expanded ? ExpandCollapseState_Expanded
                                          : ExpandCollapseState_Collapsed;
    return true;
  } else {
    return false;
  }
  value->vt = VT_BOOL;
  value->boolVal = available ? VARIANT_TRUE : VARIANT_FALSE;
  return true;
}

} // namespace cgpui
