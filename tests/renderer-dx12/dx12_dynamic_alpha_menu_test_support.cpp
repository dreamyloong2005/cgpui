#include "dx12_dynamic_alpha_test_support.hpp"
#include "dx12_ordinary_pixels_support.hpp"
#include "dx12_ordinary_menu_loop_support.hpp"

namespace cgpui_test {
namespace {
bool same_menu(const MenuLoopObservation& reference, const MenuLoopObservation& observed) {
  return observed.ok && reference.after.different(observed.before) == 0 &&
      reference.opened.different(observed.opened) == 0 && reference.after.different(observed.after) == 0 &&
      reference.after_focus == observed.before_focus && reference.opened_focus == observed.opened_focus &&
      reference.after_focus == observed.after_focus;
}
}
int dynamic_alpha_menu_case(bool cancel) {
  using namespace cgpui;
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return dx12_fail(1, "dynamic menu fixture unavailable");
  MenuCommands commands;
  auto window = (*app)->create_window(desktop.descriptor(false), [&](const PlatformEvent& e) { commands.observe(e); });
  if (!window || !ordinary_install_menu(**app, **window) || !desktop.place(**window))
    return dx12_fail(2, "dynamic ordinary window/menu fixture unavailable");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  const HWND hwnd = ordinary_hwnd(**window);
  const HMENU menu = GetMenu(hwnd);
  const auto style = GetWindowLongPtrW(hwnd, GWL_STYLE), extended = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  const auto reference = native_menu_loop(hwnd, commands, "before-any-renderer", cancel);
  if (!reference.ok) return dx12_fail(3, "unmeasured native menu reference");
  auto renderer = ordinary_renderer(**window, false);
  if (!renderer || !ordinary_composition((*renderer)->surface_composition(), false)) return dx12_fail(4, "opaque route failed");
  const auto info = (*renderer)->backend_info();
  unsigned index = 0;
  for (const bool alpha : {false, true, false, true}) {
    const bool blue = (++index % 2) == 0;
    auto changed = (*renderer)->set_transparent_surface(alpha);
    if (!changed || !ordinary_composition(*changed, alpha) ||
        !ordinary_draw_bands(**renderer, **window, blue) || !ordinary_desktop_bands(**window, blue, alpha))
      return dx12_fail(5, "same-host dynamic GPU/desktop alpha or opaque semantics differ");
    const auto observed = native_menu_loop(hwnd, commands, alpha ? "dynamic-alpha" : "dynamic-opaque", cancel);
    if (!same_menu(reference, observed)) return dx12_fail(6, "whole menu RGB/focus/native command provenance differs");
    if (ordinary_hwnd(**window) != hwnd || GetMenu(hwnd) != menu ||
        GetWindowLongPtrW(hwnd, GWL_STYLE) != style || GetWindowLongPtrW(hwnd, GWL_EXSTYLE) != extended ||
        (*renderer)->backend_info().device_generation != info.device_generation ||
        (*renderer)->backend_info().adapter_luid != info.adapter_luid ||
        !ordinary_desktop_bands(**window, blue, alpha)) return dx12_fail(7, "dynamic route changed native identity or stale frame");
  }
  return commands.commands == (cancel ? 0U : 5U) ? 0 : dx12_fail(8, "native command count differs");
}
} // namespace cgpui_test
