#include "win32_uia_control_type_internal.hpp"

namespace cgpui {

CONTROLTYPEID control_type_for(PlatformAccessibilityRole role) {
  switch (role) {
    case PlatformAccessibilityRole::label:
    case PlatformAccessibilityRole::text:
      return UIA_TextControlTypeId;
    case PlatformAccessibilityRole::button:
      return UIA_ButtonControlTypeId;
    case PlatformAccessibilityRole::text_input:
      return UIA_EditControlTypeId;
    case PlatformAccessibilityRole::image:
      return UIA_ImageControlTypeId;
    case PlatformAccessibilityRole::checkbox:
      return UIA_CheckBoxControlTypeId;
    case PlatformAccessibilityRole::radio:
      return UIA_RadioButtonControlTypeId;
    case PlatformAccessibilityRole::switch_control:
      return UIA_ButtonControlTypeId;
    case PlatformAccessibilityRole::slider:
      return UIA_SliderControlTypeId;
    case PlatformAccessibilityRole::list_item:
      return UIA_ListItemControlTypeId;
    case PlatformAccessibilityRole::menu_item:
      return UIA_MenuItemControlTypeId;
    case PlatformAccessibilityRole::tab_list:
      return UIA_TabControlTypeId;
    case PlatformAccessibilityRole::tab:
      return UIA_TabItemControlTypeId;
    case PlatformAccessibilityRole::tree:
      return UIA_TreeControlTypeId;
    case PlatformAccessibilityRole::tree_item:
      return UIA_TreeItemControlTypeId;
    case PlatformAccessibilityRole::menu:
      return UIA_MenuControlTypeId;
    case PlatformAccessibilityRole::separator:
      return UIA_SeparatorControlTypeId;
    case PlatformAccessibilityRole::tooltip:
      return UIA_ToolTipControlTypeId;
    case PlatformAccessibilityRole::generic:
      return UIA_PaneControlTypeId;
  }
  return UIA_PaneControlTypeId;
}

} // namespace cgpui
