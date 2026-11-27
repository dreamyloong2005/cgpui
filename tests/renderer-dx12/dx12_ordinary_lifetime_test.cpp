#include "../support/frame_wait_test_support.hpp"
#include "dx12_ordinary_pixels_support.hpp"
#include "dx12_ordinary_menu_pixels_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  if (!deadline.done || !desktop.valid()) return dx12_fail(1, "ordinary lifetime desktop unavailable");
  auto app = create_platform_application();
  if (!app) return dx12_fail(2, app.error().message.c_str());
  auto window = (*app)->create_window(desktop.descriptor(true), [](const PlatformEvent&) {});
  if (!window || !ordinary_install_menu(**app, **window) || !desktop.place(**window))
    return dx12_fail(3, "ordinary lifetime native-menu fixture differs");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  const HWND hwnd = ordinary_hwnd(**window);
  const HMENU menu = GetMenu(hwnd);
  MenuPixels reference;
  MenuFocus reference_focus;
  if (!stable_menu(hwnd, reference, reference_focus)) return dx12_fail(4, "opaque menu reference unstable");
  Dx12Window peer_window;
  auto peer = create_dx12_renderer(peer_window.descriptor());
  if (!peer) return dx12_fail(5, peer.error().message.c_str());
  for (const bool destroy_window : {false, true}) {
    // Re-creation in the second pass proves release of this exact HWND's lease.
    auto renderer = ordinary_renderer(**window, true);
    if (!renderer || !ordinary_draw_bands(**renderer, **window, false) ||
        !ordinary_desktop_bands(**window, false, true))
      return dx12_fail(6, "ordinary lifetime renderer warmup/re-creation failed");
    const auto duplicate = ordinary_renderer(**window, true);
    if (duplicate) return dx12_fail(7, "duplicate ordinary alpha factory accepted occupied HWND");
    if (!ordinary_draw_bands(**renderer, **window, true) ||
        !ordinary_desktop_bands(**window, true, true) ||
        !ordinary_composition((*renderer)->surface_composition(), true))
      return dx12_fail(8, "failed duplicate ordinary factory withdrew the live alpha lease");
    MenuPixels observed;
    MenuFocus observed_focus;
    if (!stable_menu(hwnd, observed, observed_focus) || reference.different(observed) ||
        reference_focus != observed_focus || GetMenu(hwnd) != menu)
      return dx12_fail(9, "duplicate factory damaged native menu or focus");
    auto finished = cgpui_test::wait_for_test_frame(**renderer);
    if (!finished) return dx12_fail(10, "retained finished ordinary frame unavailable");
    (*finished)->clear({0, 0, 1, 0.5F});
    if (!(*finished)->request_pixel_capture() || !(*finished)->present() ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {0, 0, 128, 128}))
      return dx12_fail(11, "retained finished ordinary frame failed");
    auto orphan = cgpui_test::wait_for_test_frame(**renderer);
    if (!orphan) return dx12_fail(12, "retained active ordinary frame unavailable");
    (*orphan)->clear({1, 0, 0, 0.5F});
    const auto before = (*peer)->backend_info();
    if (before.adapter_luid != (*renderer)->backend_info().adapter_luid)
      return dx12_fail(13, "ordinary lifetime peer selected a different adapter");
    if (destroy_window) {
      window->reset(); // Public window destruction while renderer and both frames still exist.
      if (IsWindow(hwnd)) return dx12_fail(14, "public ordinary window destruction kept HWND alive");
    }
    renderer->reset();
    if (!dx12_remove_native_device(before)) return dx12_fail(15, "ordinary lifetime removal failed");
    const auto retry = (*peer)->begin_frame();
    if (retry || retry.error().code != ErrorCode::frame_retry_required ||
        (*peer)->backend_info().device_generation != before.device_generation + 1)
      return dx12_fail(16, "orphan ordinary alpha references blocked live opaque peer recovery");
    const auto rejected = (*orphan)->present();
    if (rejected || rejected.error().code != ErrorCode::invalid_argument || (*finished)->present())
      return dx12_fail(17, "ownerless or destroyed-HWND ordinary frame presented/rebuilt");
    auto active = cgpui_test::wait_for_test_frame(**peer);
    if (!active) return dx12_fail(18, "live peer frame unavailable after ordinary orphan purge");
    orphan->reset();
    finished->reset();
    const auto overlap = (*peer)->begin_frame();
    if (overlap || overlap.error().code != ErrorCode::invalid_argument)
      return dx12_fail(19, "ordinary orphan frame destruction unlocked opaque peer's frame");
    (*active)->clear({0, 1, 0, 1});
    if (!(*active)->request_pixel_capture() || !(*active)->present() ||
        !dx12_pixel((*peer)->last_frame_pixels(), 8, 8, {0, 255, 0, 255}) ||
        !dx12_recovered_native_device_clean((*peer)->backend_info()))
      return dx12_fail(20, "live opaque peer presentation/debug state failed after orphan cleanup");
  }
  return 0;
}
