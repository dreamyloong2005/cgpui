#include "dx12_ordinary_pixels_support.hpp"
#include "dx12_ordinary_menu_loop_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool menu_matches(const MenuLoopObservation& reference, const MenuLoopObservation& actual) {
  const auto before = reference.after.different(actual.before);
  const auto opened = reference.opened.different(actual.opened);
  const auto after = reference.after.different(actual.after);
  const bool focus = reference.after_focus == actual.before_focus &&
      reference.opened_focus == actual.opened_focus && reference.after_focus == actual.after_focus;
  std::cout << "BACKDROP_NATIVE_MENU before=" << before << " opened=" << opened
            << " after=" << after << " focus_equal=" << focus << std::endl;
  return actual.ok && !before && !opened && !after && focus;
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  if (!deadline.done || !desktop.valid()) return dx12_fail(1, "ordinary backdrop desktop unavailable");
  auto app = create_platform_application();
  if (!app) return dx12_fail(2, app.error().message.c_str());
  MenuCommands commands;
  auto window = (*app)->create_window(desktop.descriptor(true),
      [&](const PlatformEvent& event) { commands.observe(event); });
  if (!window || !ordinary_install_menu(**app, **window) || !desktop.place(**window))
    return dx12_fail(3, "ordinary backdrop native-menu fixture differs");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  const HWND hwnd = ordinary_hwnd(**window);
  const HMENU menu = GetMenu(hwnd);
  const auto reference = native_menu_loop(hwnd, commands, "opaque-no-backdrop");
  if (!reference.ok) return dx12_fail(4, "opaque backdrop menu reference failed");
  auto renderer = ordinary_renderer(**window, true);
  if (!renderer || !ordinary_draw_bands(**renderer, **window, false) ||
      !ordinary_desktop_bands(**window, false, true))
    return dx12_fail(5, "initial ordinary alpha control failed");
  const auto generation = (*renderer)->backend_info().device_generation;
  const auto material = (*window)->apply_window_backdrop(WindowBackdrop::mica, true);
  std::cout << "MICA supported=" << material.supported << " applied="
            << (material.applied == WindowBackdrop::mica) << " reason=" << material.reason << std::endl;
  if (material.requested != WindowBackdrop::mica || !material.transparent_surface ||
      (material.applied != WindowBackdrop::mica && material.applied != WindowBackdrop::none) ||
      (material.applied == WindowBackdrop::none && material.reason.empty()))
    return dx12_fail(6, "ordinary Mica application returned an inconsistent material report");
  if (material.applied != WindowBackdrop::mica)
    std::cout << "MICA_MATERIAL_NOT_EXERCISED OS/policy did not apply the requested material" << std::endl;
  // A real Mica material need not be green. GPU premultiplied pixels and the
  // exact native-menu reference remain mandatory, with no background threshold.
  if (!ordinary_draw_bands(**renderer, **window, false) ||
      !ordinary_composition((*renderer)->surface_composition(), true) ||
      !menu_matches(reference, native_menu_loop(hwnd, commands, "mica-native-menu")))
    return dx12_fail(7, "Mica damaged GPU alpha or native menu RGB/focus/command");
  const auto restored = (*window)->apply_window_backdrop(WindowBackdrop::none, true);
  if (restored.requested != WindowBackdrop::none || restored.applied != WindowBackdrop::none ||
      !restored.transparent_surface || !ordinary_draw_bands(**renderer, **window, true) ||
      !ordinary_desktop_bands(**window, true, true))
    return dx12_fail(8, "removing Mica did not restore real desktop alpha and the new blue frame");
  if (!menu_matches(reference, native_menu_loop(hwnd, commands, "restored-none-native-menu")) ||
      ordinary_hwnd(**window) != hwnd || GetMenu(hwnd) != menu ||
      (*renderer)->backend_info().device_generation != generation || commands.commands != 3)
    return dx12_fail(9, "removing Mica changed native menu, HWND or device generation");
  return 0;
}
