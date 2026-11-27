#include "dx12_ordinary_pixels_support.hpp"
#include <atomic>
#include <thread>

int main() {
  using namespace cgpui;
  using namespace cgpui_test;
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return 1;
  auto window = (*app)->create_window(desktop.descriptor(false), [](const PlatformEvent&) {});
  if (!window || !ordinary_install_menu(**app, **window) || !desktop.place(**window)) return 2;
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  auto created = ordinary_renderer(**window, false); // Managed owner-thread creation chooses IGNORE composition.
  if (!created) return 3;
  const auto geometry = (*window)->state();
  const HWND hwnd = ordinary_hwnd(**window); const HMENU menu = GetMenu(hwnd);
  std::atomic<bool> done{false};
  int result = 0;
  std::thread worker([renderer = std::move(*created), geometry, &done, &result]() mutable {
    auto frame = wait_for_test_frame(*renderer);
    if (!frame) result = 4;
    else {
      (*frame)->clear({1, 0, 0, .5F});
      if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
          !dx12_pixel(renderer->last_frame_pixels(), 8, 8, {255, 0, 0, 128})) result = 5;
      frame->reset();
      if (!renderer->resize(geometry.framebuffer_size, geometry.scale)) result = 6;
      const auto alpha = renderer->set_transparent_surface(true);
      if (alpha || alpha.error().code != ErrorCode::invalid_argument ||
          !ordinary_composition(renderer->surface_composition(), false)) result = 7;
    }
    renderer.reset(); // Bounded native retirement, then token-only posted reservation release.
    done.store(true, std::memory_order_release);
  });
  while (!done.load(std::memory_order_acquire)) pump_messages_for(10);
  worker.join();
  pump_messages_for(20);
  if (result) return result;
  if (ordinary_hwnd(**window) != hwnd || GetMenu(hwnd) != menu) return 8;
  auto alpha = ordinary_renderer(**window, true); // No HWND-flip history was fabricated by opaque worker use.
  if (!alpha || !ordinary_draw_bands(**alpha, **window, true) || !ordinary_desktop_bands(**window, true, true)) return 9;
  return 0;
}
