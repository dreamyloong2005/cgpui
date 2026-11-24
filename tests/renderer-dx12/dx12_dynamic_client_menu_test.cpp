#include "dx12_dynamic_client_test_support.hpp"
#include "dx12_client_alpha_test_support.hpp"
#include "dx12_client_menu_refusal_support.hpp"

int main() {
  using namespace cgpui;
  using namespace cgpui_test;
  Dx12RecoveryDeadline deadline;
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return 1;
  auto descriptor = client_alpha_descriptor(desktop);
  descriptor.chrome.transparent_background = false;
  auto window = (*app)->create_window(descriptor, [](const PlatformEvent&) {});
  if (!window || !desktop.place(**window)) return 2;
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  auto renderer = ordinary_renderer(**window, false);
  if (!renderer || !dynamic_client_resize(**renderer, **window) ||
      !dynamic_client_paint(**renderer, **window, false, false)) return 3;
  const auto identity = (*renderer)->backend_info();
  const auto pixels = (*renderer)->last_frame_pixels()->rgba8;
  // The opaque reservation is not a menu restriction; no assertion here claims
  // the legacy opaque client+HMENU layout is repaired by this renderer change.
  if (!(*app)->install_native_menu(refusal_native_menu()).supported) return 4;
  RefusalNativeSnapshot menu;
  if (!menu.capture(**window) || !menu.menu) return 5;
  auto refused = (*renderer)->set_transparent_surface(true);
  if (refused || refused.error().code != ErrorCode::unsupported_platform || !menu.same(**window) ||
      !ordinary_composition((*renderer)->surface_composition(), false) ||
      (*renderer)->last_frame_pixels()->rgba8 != pixels ||
      (*renderer)->backend_info().device_generation != identity.device_generation) return 6;
  auto system = descriptor.chrome; system.title_bar = WindowTitleBarMode::system;
  const auto changed = (*window)->apply_window_chrome(system);
  const auto restored = (*window)->apply_window_chrome(descriptor.chrome);
  if (changed.applied.title_bar != WindowTitleBarMode::system ||
      restored.applied.title_bar != WindowTitleBarMode::client || GetMenu(menu.hwnd) != menu.menu) return 7;
  if (!(*app)->install_native_menu({}).supported) return 8;
  auto undecorated = descriptor.chrome; undecorated.decorations = false;
  if ((*window)->apply_window_chrome(undecorated).applied.decorations) return 9;
  refused = (*renderer)->set_transparent_surface(true);
  if (refused || refused.error().code != ErrorCode::unsupported_platform) return 10;
  if (!(*window)->apply_window_chrome(descriptor.chrome).applied.decorations || !desktop.place(**window) ||
      !dynamic_client_resize(**renderer, **window) || !(*renderer)->set_transparent_surface(true) ||
      !dynamic_client_paint(**renderer, **window, true, true)) return 11;
  if ((*app)->install_native_menu(refusal_native_menu()).supported || GetMenu(menu.hwnd)) return 12;
  return 0;
}
