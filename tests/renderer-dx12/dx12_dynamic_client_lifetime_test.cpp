#include "dx12_dynamic_client_test_support.hpp"
#include "dx12_client_alpha_test_support.hpp"
#include "dx12_client_menu_refusal_support.hpp"

int main() {
  using namespace cgpui;
  using namespace cgpui_test;
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return 1;
  auto descriptor = client_alpha_descriptor(desktop);
  descriptor.chrome.transparent_background = false;
  auto window = (*app)->create_window(descriptor, [](const PlatformEvent&) {});
  if (!window || !desktop.place(**window)) return 2;
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  client_alpha_regions(**window);
  ClientAlphaTopology initial;
  auto renderer = ordinary_renderer(**window, false);
  if (!renderer || !initial.capture(**window) || !dynamic_client_paint(**renderer, **window, false, false)) return 3;
  const auto* pixels = (*renderer)->last_frame_pixels();
  const auto* diagnostics = (*renderer)->last_frame_diagnostic_snapshot();
  for (const bool alpha : {true, false, true, false})
    if (!(*renderer)->set_transparent_surface(alpha) || (*renderer)->last_frame_pixels() != pixels ||
        (*renderer)->last_frame_diagnostic_snapshot() != diagnostics) return 4;
  if (!dynamic_client_paint(**renderer, **window, true, false)) return 5;
  // Actual opacity is restored, but the promoted native alpha lease deliberately
  // remains active: no premature suspend after asynchronous DComp Commit.
  auto system = descriptor.chrome; system.title_bar = WindowTitleBarMode::system;
  const auto locked = (*window)->apply_window_chrome(system);
  if (locked.reason.empty() || locked.applied.title_bar != WindowTitleBarMode::client ||
      (*app)->install_native_menu(refusal_native_menu()).supported || !initial.same(**window)) return 6;
  if (!(*renderer)->set_transparent_surface(true)) return 7;
  auto orphan = wait_for_test_frame(**renderer);
  if (!orphan) return 8;
  (*orphan)->clear({1, 0, 0, .5F});
  renderer->reset();
  if (ordinary_renderer(**window, true) || !(*orphan)->request_pixel_capture() || !(*orphan)->present() ||
      (*orphan)->present() || !initial.same(**window) || !client_alpha_hits(**window) ||
      !expect_screen_color({128, 127, 0}, {initial.origin.x + initial.client.right / 2,
                                         initial.origin.y + initial.client.bottom / 2})) return 9;
  orphan->reset();
  if (!(*app)->install_native_menu(refusal_native_menu()).supported || !GetMenu(initial.hwnd) ||
      !(*app)->install_native_menu({}).supported || GetMenu(initial.hwnd)) return 10;
  auto replacement = ordinary_renderer(**window, false);
  if (!replacement || !dynamic_client_resize(**replacement, **window) ||
      !(*replacement)->set_transparent_surface(true) || !dynamic_client_paint(**replacement, **window, true, true)) return 11;
  return 0;
}
