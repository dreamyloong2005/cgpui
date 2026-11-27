#include "dx12_ordinary_pixels_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  if (!deadline.done || !desktop.valid()) return dx12_fail(1, "ordinary desktop fixture unavailable");
  auto app = create_platform_application();
  if (!app) return dx12_fail(2, app.error().message.c_str());
  auto window = (*app)->create_window(desktop.descriptor(true), [](const PlatformEvent&) {});
  if (!window) return dx12_fail(3, window.error().message.c_str());
  if (!ordinary_install_menu(**app, **window) || !desktop.place(**window))
    return dx12_fail(4, "ordinary system-caption/native-menu fixture differs");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  const HWND hwnd = ordinary_hwnd(**window);
  const HMENU menu = GetMenu(hwnd);
  auto renderer = ordinary_renderer(**window, true);
  if (!renderer) return dx12_fail(5, renderer.error().message.c_str());
  if (!ordinary_composition((*renderer)->surface_composition(), true))
    return dx12_fail(6, "ordinary initial transparency is not premultiplied composition");
  for (const bool blue : {false, true}) {
    if (!ordinary_draw_bands(**renderer, **window, blue) ||
        !ordinary_desktop_bands(**window, blue, true))
      return dx12_fail(7, "ordinary initial transparency GPU/desktop pixels differ");
    if (ordinary_hwnd(**window) != hwnd || GetMenu(hwnd) != menu)
      return dx12_fail(8, "ordinary initial presentation replaced HWND or native menu");
  }
  return 0;
}
