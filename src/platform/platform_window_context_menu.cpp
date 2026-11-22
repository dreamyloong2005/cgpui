#include "cgpui/platform/platform_window.hpp"

namespace cgpui {

// Win32, Wayland, X11 and test windows have no native context menu yet;
// apps keep the in-window PopoverMenu there.
PlatformContextMenuResult PlatformWindow::show_context_menu(
    const NativeMenuModel& menu, Point position) {
  (void)menu;
  (void)position;
  return PlatformContextMenuResult{
      .backend = "unsupported",
      .reason = "native context menus are unsupported on this backend",
  };
}

} // namespace cgpui
