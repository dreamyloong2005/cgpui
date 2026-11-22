#include "cgpui/ui/popover_menu.hpp"

#include "cgpui/platform/platform_window.hpp"
#include "cgpui/ui/runtime_context.hpp"

namespace cgpui {
namespace {

std::vector<NativeMenuItem> native_items(const std::vector<MenuItem>& items) {
  std::vector<NativeMenuItem> result;
  result.reserve(items.size());
  for (const MenuItem& item : items) {
    switch (item.kind) {
      case MenuItemKind::separator:
        result.push_back({.kind = NativeMenuItemKind::separator});
        break;
      case MenuItemKind::submenu:
        result.push_back({.kind = NativeMenuItemKind::submenu,
                          .title = item.label,
                          .enabled = item.enabled,
                          .children = native_items(item.children)});
        break;
      case MenuItemKind::action:
        result.push_back({.title = item.label, .action_name = item.id,
                          .enabled = item.enabled});
        break;
    }
  }
  return result;
}

} // namespace

// The native menu runs modally and returns the chosen item, which goes to
// the activate handler like an in-window choice.
bool PopoverMenu::open_native(
    Point position,
    const std::vector<MenuItem>& items,
    const WindowRuntimeContext& context) {
  const PlatformContextMenuResult result = context.platform_window.show_context_menu(
      NativeMenuModel{.items = native_items(items)}, position);
  if (!result.supported || !result.shown) return false;
  if (result.action_name && activate_handler_) activate_handler_(*result.action_name);
  return true;
}

} // namespace cgpui
