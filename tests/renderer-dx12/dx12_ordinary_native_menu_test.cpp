#include "dx12_ordinary_pixels_support.hpp"
#include "dx12_ordinary_menu_loop_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool same_native_menu(const MenuLoopObservation& reference, const MenuLoopObservation& actual) {
  // The first keyboard menu interaction can reveal accelerator cues. Closed
  // alpha snapshots therefore use the reference AFTER that same interaction.
  const auto before = reference.after.different(actual.before);
  const auto opened = reference.opened.different(actual.opened);
  const auto after = reference.after.different(actual.after);
  const bool focus = reference.after_focus == actual.before_focus &&
      reference.opened_focus == actual.opened_focus && reference.after_focus == actual.after_focus;
  std::cout << "EXACT_NATIVE_MENU before=" << before << " opened=" << opened
            << " after=" << after << " focus_equal=" << focus << std::endl;
  return actual.ok && before == 0 && opened == 0 && after == 0 && focus;
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  OrdinaryDesktop desktop;
  if (!deadline.done || !desktop.valid()) return dx12_fail(1, "ordinary menu desktop unavailable");
  auto app = create_platform_application();
  if (!app) return dx12_fail(2, app.error().message.c_str());
  MenuCommands commands;
  auto window = (*app)->create_window(desktop.descriptor(true),
      [&](const PlatformEvent& event) { commands.observe(event); });
  if (!window) return dx12_fail(3, window.error().message.c_str());
  if (!ordinary_install_menu(**app, **window) || !desktop.place(**window))
    return dx12_fail(4, "ordinary system-caption/native-menu fixture differs");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  const HWND hwnd = ordinary_hwnd(**window);
  const HMENU menu = GetMenu(hwnd);
  // No renderer or DXGI swapchain has existed on this HWND. Its OS-rendered
  // opaque menu is the reference, including the genuine highlighted menu loop.
  const auto reference = native_menu_loop(hwnd, commands, "opaque-no-renderer");
  if (!reference.ok) return dx12_fail(5, "opaque native menu reference loop failed");
  auto renderer = ordinary_renderer(**window, true);
  if (!renderer) return dx12_fail(6, renderer.error().message.c_str());
  if (!ordinary_composition((*renderer)->surface_composition(), true))
    return dx12_fail(7, "ordinary menu renderer did not expose actual alpha composition");
  for (const bool blue : {false, true}) {
    if (!ordinary_draw_bands(**renderer, **window, blue) ||
        !ordinary_desktop_bands(**window, blue, true))
      return dx12_fail(8, "native-menu ordinary GPU/desktop alpha differs");
    const auto observed = native_menu_loop(hwnd, commands, blue ? "alpha-blue" : "alpha-red");
    if (!same_native_menu(reference, observed))
      return dx12_fail(9, "whole native menu RGB or focus changed before/during/after its loop");
    if (!ordinary_desktop_bands(**window, blue, true) ||
        ordinary_hwnd(**window) != hwnd || GetMenu(hwnd) != menu)
      return dx12_fail(10, "native menu loop replaced the window/menu or damaged client alpha");
  }
  return commands.commands == 3 ? 0 : dx12_fail(11, "unexpected public native menu command count");
}
