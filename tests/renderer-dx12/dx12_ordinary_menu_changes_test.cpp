#include "dx12_ordinary_pixels_support.hpp"
#include "dx12_ordinary_menu_loop_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool install(PlatformApplication& app, PlatformWindow& window, int model) {
  auto menu = model == 0 ? NativeMenuModel{} : ordinary_native_menu();
  if (model == 2) menu.items[1].title = "Replacement native item";
  const auto result = app.install_native_menu(std::move(menu));
  pump_messages_for(80);
  HMENU native = GetMenu(ordinary_hwnd(window));
  if (!result.supported) return false;
  if (model == 0) return !native;
  if (!native || GetMenuItemCount(native) != 2 || !GetSubMenu(native, 0)) return false;
  wchar_t text[64]{};
  return GetMenuStringW(native, 1, text, 64, MF_BYPOSITION) > 0 &&
      lstrcmpW(text, model == 2 ? L"Replacement native item" : L"Native text item") == 0;
}

bool present_both(Renderer& renderer, const PlatformWindow& window) {
  const auto state = window.state();
  RECT client{};
  if (!GetClientRect(ordinary_hwnd(window), &client) ||
      state.framebuffer_size.width != float(client.right) ||
      state.framebuffer_size.height != float(client.bottom) ||
      !renderer.resize(state.framebuffer_size, state.scale) ||
      !ordinary_composition(renderer.surface_composition(), true)) return false;
  for (const bool blue : {false, true}) {
    if (!ordinary_draw_bands(renderer, window, blue) ||
        !ordinary_desktop_bands(window, blue, true)) return false;
  }
  return true;
}

bool menu_matches(const MenuLoopObservation& reference, const MenuLoopObservation& actual) {
  const auto before = reference.after.different(actual.before);
  const auto opened = reference.opened.different(actual.opened);
  const auto after = reference.after.different(actual.after);
  const bool focus = reference.after_focus == actual.before_focus &&
      reference.opened_focus == actual.opened_focus && reference.after_focus == actual.after_focus;
  std::cout << "EXACT_CHANGED_MENU before=" << before << " opened=" << opened
            << " after=" << after << " focus_equal=" << focus << std::endl;
  return reference.ok && actual.ok && before == 0 && opened == 0 && after == 0 && focus;
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  OrdinaryDesktop desktop;
  if (!deadline.done || !desktop.valid()) return dx12_fail(1, "menu changes desktop unavailable");
  auto app = create_platform_application();
  if (!app) return dx12_fail(2, app.error().message.c_str());
  MenuCommands commands;
  auto window = (*app)->create_window(desktop.descriptor(true),
      [&](const PlatformEvent& event) { commands.observe(event); });
  if (!window || !desktop.place(**window)) return dx12_fail(3, "menu changes window unavailable");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  const HWND hwnd = ordinary_hwnd(**window);
  const LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
  const LONG_PTR extended = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  // Both references are native, opaque and on this same HWND before any renderer.
  if (!install(**app, **window, 1)) return dx12_fail(4, "native model A reference unavailable");
  const auto reference_a = native_menu_loop(hwnd, commands, "opaque-model-a");
  if (!reference_a.ok || !install(**app, **window, 2))
    return dx12_fail(5, "native model A reference or model B installation failed");
  const auto reference_b = native_menu_loop(hwnd, commands, "opaque-model-b");
  if (!reference_b.ok || !install(**app, **window, 0))
    return dx12_fail(6, "native model B reference or initial menu clear failed");
  auto renderer = ordinary_renderer(**window, true);
  if (!renderer || !present_both(**renderer, **window))
    return dx12_fail(7, "ordinary initial alpha without a native menu failed");
  const auto generation = (*renderer)->backend_info().device_generation;
  for (const int model : {1, 2, 0, 1}) {
    std::cout << "MENU_CHANGE model=" << model << std::endl;
    if (!install(**app, **window, model) || !present_both(**renderer, **window))
      return dx12_fail(8, "late/replace/clear/reinstall menu changed GPU or desktop alpha");
    if (model != 0) {
      const auto observed = native_menu_loop(hwnd, commands, model == 2 ? "alpha-model-b" : "alpha-model-a");
      if (!menu_matches(model == 2 ? reference_b : reference_a, observed))
        return dx12_fail(9, "changed native menu RGB/focus or genuine command differs");
      if (!ordinary_desktop_bands(**window, true, true))
        return dx12_fail(10, "changed menu interaction damaged client alpha");
    }
    if (ordinary_hwnd(**window) != hwnd || GetWindowLongPtrW(hwnd, GWL_STYLE) != style ||
        GetWindowLongPtrW(hwnd, GWL_EXSTYLE) != extended ||
        (*renderer)->backend_info().device_generation != generation)
      return dx12_fail(11, "native menu changes replaced HWND/device or changed its style");
  }
  return commands.commands == 5 ? 0 : dx12_fail(12, "unexpected changed-menu command count");
}
