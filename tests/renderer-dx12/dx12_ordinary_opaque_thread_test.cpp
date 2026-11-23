#include "dx12_ordinary_test_support.hpp"

#include <atomic>
#include <thread>

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool opaque_frame(Renderer& renderer, DpiScale scale) {
  auto frame = renderer.begin_frame();
  if (!frame) return false;
  (*frame)->clear({0, 1, 1, 1});
  (*frame)->draw_rect({.rect = {{16 / scale.value, 16 / scale.value},
      {24 / scale.value, 24 / scale.value}}, .color = {1, 0, 0, 1}});
  return (*frame)->request_pixel_capture() && (*frame)->present() &&
      dx12_pixel(renderer.last_frame_pixels(), 8, 8, {0, 255, 255, 255}) &&
      dx12_pixel(renderer.last_frame_pixels(), 20, 20, {255, 0, 0, 255});
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return dx12_fail(1, "opaque worker fixture unavailable");
  auto window = (*app)->create_window(desktop.descriptor(false), [](const PlatformEvent&) {});
  if (!window || !ordinary_install_menu(**app, **window) || !desktop.place(**window))
    return dx12_fail(2, "opaque worker native window unavailable");
  // Only the UI thread accesses PlatformWindow. The worker receives a value
  // snapshot, retaining the original opaque HWND factory's threading contract.
  const auto state = (*window)->state();
  const RenderSurfaceDescriptor descriptor{.native_surface = (*window)->native_surface(),
      .framebuffer_size = state.framebuffer_size, .scale = state.scale, .transparent_background = false};
  const HWND hwnd = ordinary_hwnd(**window);
  const HMENU menu = GetMenu(hwnd);
  const auto style = GetWindowLongPtrW(hwnd, GWL_STYLE);
  const auto extended = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
  std::atomic<bool> finished{false};
  int worker_result = 0;
  std::thread worker([descriptor, &finished, &worker_result] {
    {
      auto renderer = create_dx12_renderer(descriptor);
      if (!renderer) {
        worker_result = dx12_fail(3, renderer.error().message.c_str());
      } else if (!ordinary_composition((*renderer)->surface_composition(), false) ||
                 !opaque_frame(**renderer, descriptor.scale)) {
        worker_result = dx12_fail(4, "foreign-thread opaque factory did not draw/capture its control frame");
      }
    } // Renderer and all native resources retire on the worker before done.
    finished.store(true, std::memory_order_release);
  });
  // DXGI and the token-only opaque-history handshake may synchronously message
  // the owner. Do not join until the UI has serviced those messages.
  while (!finished.load(std::memory_order_acquire)) pump_messages_for(10);
  worker.join();
  if (worker_result) return worker_result;
  auto transparent = descriptor;
  transparent.transparent_background = true;
  const auto rejected = create_dx12_renderer(transparent);
  if (rejected || rejected.error().code != ErrorCode::unsupported_platform)
    return dx12_fail(5, "worker HWND-flip history did not prevent unsafe UI alpha creation");
  if (ordinary_hwnd(**window) != hwnd || GetMenu(hwnd) != menu ||
      GetWindowLongPtrW(hwnd, GWL_STYLE) != style || GetWindowLongPtrW(hwnd, GWL_EXSTYLE) != extended)
    return dx12_fail(6, "worker/history refusal changed the native window or menu");
  auto replacement = create_dx12_renderer(descriptor);
  if (!replacement || !ordinary_composition((*replacement)->surface_composition(), false) ||
      !opaque_frame(**replacement, descriptor.scale))
    return dx12_fail(7, "worker retirement/history refusal poisoned UI opaque rendering");
  return 0;
}
