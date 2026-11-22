#include "cgpui/platform/platform_application.hpp"
#include "cgpui/platform/platform_window.hpp"

namespace cgpui {

// Native context menus and popup windows exist on macOS only so far; the
// other backends report them unsupported and apps keep in-window overlays.
PlatformContextMenuResult PlatformWindow::show_context_menu(
    const NativeMenuModel& menu, Point position) {
  (void)menu;
  (void)position;
  return PlatformContextMenuResult{
      .backend = "unsupported",
      .reason = "native context menus are unsupported on this backend",
  };
}

PlatformPopupPlacementResult PlatformWindow::place_popup(
    WindowPopupPlacement placement) {
  (void)placement;
  return PlatformPopupPlacementResult{
      .reason = "popup windows are unsupported on this backend"};
}

bool PlatformApplication::popup_windows_supported() const { return false; }

} // namespace cgpui
