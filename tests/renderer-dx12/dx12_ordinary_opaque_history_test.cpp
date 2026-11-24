#include "../support/frame_wait_test_support.hpp"
#include "dx12_ordinary_test_support.hpp"
#include "dx12_ordinary_menu_pixels_support.hpp"
#include <atomic>
#include <thread>

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool establish_actual_flip_history(const PlatformWindow& window) {
  const auto geometry = window.state();
  const RenderSurfaceDescriptor descriptor{.native_surface = window.native_surface(),
      .framebuffer_size = geometry.framebuffer_size, .scale = geometry.scale};
  std::atomic<bool> done{false};
  bool passed = false;
  std::thread worker([&] {
    { // The foreign-thread factory deliberately retains the real HWND flip route.
      auto renderer = create_dx12_renderer(descriptor);
      if (renderer) {
        auto frame = wait_for_test_frame(**renderer);
        if (frame) { (*frame)->clear({0, 1, 1, 1}); passed = bool((*frame)->present()); }
      }
    }
    done.store(true, std::memory_order_release);
  });
  while (!done.load(std::memory_order_acquire)) pump_messages_for(10);
  worker.join();
  return passed;
}
bool opaque_frame(Renderer& renderer, const PlatformWindow& window) {
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return false;
  (*frame)->clear({0, 1, 1, 1});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
      !dx12_pixel(renderer.last_frame_pixels(), 8, 8, {0, 255, 255, 255})) return false;
  POINT point{8, 8};
  return ClientToScreen(ordinary_hwnd(window), &point) && expect_screen_color({0, 255, 255}, point);
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  OrdinaryDesktop desktop;
  if (!deadline.done || !desktop.valid()) return dx12_fail(1, "ordinary history desktop unavailable");
  auto app = create_platform_application();
  if (!app) return dx12_fail(2, app.error().message.c_str());
  auto window = (*app)->create_window(desktop.descriptor(false), [](const PlatformEvent&) {});
  if (!window) return dx12_fail(3, window.error().message.c_str());
  if (!ordinary_install_menu(**app, **window) || !desktop.place(**window))
    return dx12_fail(4, "ordinary opaque native-menu fixture differs");
  const HWND hwnd = ordinary_hwnd(**window);
  const HMENU menu = GetMenu(hwnd);
  const LONG_PTR extended = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  if (!establish_actual_flip_history(**window)) return dx12_fail(12, "actual foreign-thread HWND flip control failed");
  auto renderer = ordinary_renderer(**window, false);
  if (!renderer || !ordinary_composition((*renderer)->surface_composition(), false) ||
      !opaque_frame(**renderer, **window))
    return dx12_fail(5, "ordinary opaque renderer did not present its control frame");
  MenuPixels reference;
  MenuFocus reference_focus;
  if (!stable_menu(hwnd, reference, reference_focus)) return dx12_fail(7, "opaque menu reference unstable");
  const auto generation = (*renderer)->backend_info().device_generation;
  const auto rejected_change = (*renderer)->set_transparent_surface(true);
  if (rejected_change || rejected_change.error().code != ErrorCode::unsupported_platform ||
      !ordinary_composition((*renderer)->surface_composition(), false) ||
      (*renderer)->backend_info().device_generation != generation ||
      !opaque_frame(**renderer, **window))
    return dx12_fail(6, "opaque-to-alpha refusal changed the existing renderer contract");
  MenuPixels after_change;
  MenuFocus after_change_focus;
  if (!stable_menu(hwnd, after_change, after_change_focus) || reference.different(after_change) != 0 ||
      reference_focus != after_change_focus || GetMenu(hwnd) != menu ||
      GetWindowLongPtrW(hwnd, GWL_EXSTYLE) != extended)
    return dx12_fail(11, "opaque-to-alpha refusal changed native menu, style or focus");
  renderer->reset(); // A presented HWND flip-chain history survives renderer destruction.
  auto rejected_creation = ordinary_renderer(**window, true);
  if (rejected_creation || rejected_creation.error().code != ErrorCode::unsupported_platform)
    return dx12_fail(8, "same HWND with opaque flip history accepted unsafe alpha creation");
  MenuPixels observed;
  MenuFocus observed_focus;
  if (!stable_menu(hwnd, observed, observed_focus) || reference.different(observed) != 0 ||
      reference_focus != observed_focus || ordinary_hwnd(**window) != hwnd || GetMenu(hwnd) != menu ||
      GetWindowLongPtrW(hwnd, GWL_EXSTYLE) != extended)
    return dx12_fail(9, "history refusal mutated native menu, HWND, style or focus");
  auto replacement = ordinary_renderer(**window, false);
  if (!replacement || !ordinary_composition((*replacement)->surface_composition(), false) ||
      !opaque_frame(**replacement, **window))
    return dx12_fail(10, "history refusal poisoned subsequent ordinary opaque rendering");
  return 0;
}
