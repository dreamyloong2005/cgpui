#include "dx12_ordinary_pixels_support.hpp"
#include "dx12_ordinary_menu_loop_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  if (!deadline.done || !desktop.valid()) return dx12_fail(1, "ordinary menu-cancel desktop unavailable");
  auto app = create_platform_application();
  if (!app) return dx12_fail(2, app.error().message.c_str());
  MenuCommands commands;
  auto window = (*app)->create_window(desktop.descriptor(true),
      [&](const PlatformEvent& event) { commands.observe(event); });
  if (!window || !ordinary_install_menu(**app, **window) || !desktop.place(**window))
    return dx12_fail(3, "ordinary menu-cancel native fixture differs");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  const HWND hwnd = ordinary_hwnd(**window);
  const HMENU menu = GetMenu(hwnd);
  // Cancellation has its own opaque keyboard-navigation reference; it must
  // never borrow a successful-command reference with a different focus state.
  const auto reference = native_menu_loop(hwnd, commands, "opaque-cancel", true);
  if (!reference.ok || commands.commands) return dx12_fail(4, "opaque native Escape reference failed");
  auto renderer = ordinary_renderer(**window, true);
  if (!renderer) return dx12_fail(5, renderer.error().message.c_str());
  for (const bool blue : {false, true}) {
    if (!ordinary_draw_bands(**renderer, **window, blue) ||
        !ordinary_desktop_bands(**window, blue, true))
      return dx12_fail(6, "native menu-cancel GPU/desktop alpha differs");
    const auto observed = native_menu_loop(hwnd, commands, blue ? "blue-cancel" : "red-cancel", true);
    const auto before = reference.after.different(observed.before);
    const auto opened = reference.opened.different(observed.opened);
    const auto after = reference.after.different(observed.after);
    const bool focus = reference.after_focus == observed.before_focus &&
        reference.opened_focus == observed.opened_focus && reference.after_focus == observed.after_focus;
    std::cout << "EXACT_CANCELLED_MENU before=" << before << " opened=" << opened
              << " after=" << after << " focus_equal=" << focus << std::endl;
    if (!observed.ok || before || opened || after || !focus || commands.commands ||
        ordinary_hwnd(**window) != hwnd || GetMenu(hwnd) != menu)
      return dx12_fail(7, "native Escape cancellation changed menu RGB/focus or emitted a command");
    if (!ordinary_desktop_bands(**window, blue, true))
      return dx12_fail(8, "native Escape cancellation damaged client alpha");
  }
  return 0;
}
