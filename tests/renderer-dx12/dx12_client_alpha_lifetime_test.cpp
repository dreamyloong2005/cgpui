#include "../support/frame_wait_test_support.hpp"
#include "dx12_client_alpha_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return dx12_fail(1, "client lifetime fixture unavailable");
  const auto descriptor = client_alpha_descriptor(desktop);
  auto window = (*app)->create_window(descriptor, [](const PlatformEvent&) {});
  if (!window || !desktop.place(**window)) return dx12_fail(2, "client lifetime window unavailable");
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  client_alpha_regions(**window);
  ClientAlphaTopology initial;
  auto renderer = ordinary_renderer(**window, true);
  if (!renderer || !initial.capture(**window) || !client_alpha_hits(**window) ||
      !client_alpha_paint(**renderer, **window, false)) return dx12_fail(3, "client alpha warmup failed");
  const auto generation = (*renderer)->backend_info().device_generation;
  const auto saved = (*renderer)->last_frame_pixels()->rgba8;
  const auto unchanged = [&] {
    const auto* pixels = (*renderer)->last_frame_pixels();
    return initial.same(**window) && client_alpha_hits(**window) && pixels && pixels->rgba8 == saved &&
        ordinary_composition((*renderer)->surface_composition(), true) &&
        (*renderer)->backend_info().device_generation == generation;
  };
  for (unsigned change = 0; change != 3; ++change) {
    auto options = descriptor.chrome;
    if (change == 0) options.title_bar = WindowTitleBarMode::system;
    if (change == 1) options.titlebar_visible = false;
    if (change == 2) options.decorations = false;
    const auto refused = (*window)->apply_window_chrome(options);
    if (refused.reason.empty() || refused.applied.title_bar != WindowTitleBarMode::client ||
        !refused.applied.titlebar_visible || !refused.applied.decorations || !unchanged() ||
        !ordinary_desktop_bands(**window, false, true))
      return dx12_fail(4, "rejected client chrome transition changed geometry/style/capture");
  }
  if ((*window)->request_display_state(PlatformWindowDisplayState::fullscreen) || !unchanged())
    return dx12_fail(5, "fullscreen request changed active client alpha topology");
  if (ordinary_renderer(**window, true) || !unchanged())
    return dx12_fail(6, "duplicate client factory accepted or withdrew the live lease");
  for (const bool smaller : {true, false}) {
    if (!SetWindowPos(initial.hwnd, nullptr, 0, 0, smaller ? 376 : 400, smaller ? 248 : 280,
                      SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE)) return dx12_fail(7, "native resize failed");
    pump_messages_for(80);
    // Explicit renderer scale, not a physical-monitor DPI claim.
    if (!client_alpha_scale_pixels(**renderer, **window, true) ||
        !client_alpha_scale_pixels(**renderer, **window, false)) return dx12_fail(8, "scaled client pixels differ");
    const auto state = (*window)->state();
    if (!(*renderer)->resize(state.framebuffer_size, state.scale)) return dx12_fail(9, "client resize failed");
    client_alpha_regions(**window);
    if (!client_alpha_paint(**renderer, **window, smaller) || !client_alpha_hits(**window) ||
        ordinary_hwnd(**window) != initial.hwnd || GetWindowLongPtrW(initial.hwnd, GWL_STYLE) != initial.style ||
        GetWindowLongPtrW(initial.hwnd, GWL_EXSTYLE) != initial.extended)
      return dx12_fail(10, "resize changed client alpha/geometry/hit behavior");
  }
  const auto mica = (*window)->apply_window_backdrop(WindowBackdrop::mica, true);
  std::cout << "CLIENT_MICA applied=" << (mica.applied == WindowBackdrop::mica) << " reason=" << mica.reason << std::endl;
  if (mica.requested != WindowBackdrop::mica || !mica.transparent_surface ||
      (mica.applied != WindowBackdrop::mica && mica.applied != WindowBackdrop::none) ||
      (mica.applied == WindowBackdrop::none && mica.reason.empty()) ||
      !client_alpha_paint(**renderer, **window, false, false) || !client_alpha_hits(**window))
    return dx12_fail(11, "Mica changed client GPU pixels or hit regions");
  const auto none = (*window)->apply_window_backdrop(WindowBackdrop::none, true);
  if (none.requested != WindowBackdrop::none || none.applied != WindowBackdrop::none || !none.transparent_surface ||
      !client_alpha_paint(**renderer, **window, true) || !client_alpha_hits(**window) ||
      (*renderer)->backend_info().device_generation != generation)
    return dx12_fail(12, "Mica removal did not restore client desktop alpha");
  auto finished = cgpui_test::wait_for_test_frame(**renderer);
  if (!finished) return dx12_fail(13, "retained finished frame unavailable");
  (*finished)->clear({0, 0, 1, 0.5F});
  if (!(*finished)->present()) return dx12_fail(14, "retained finished frame failed");
  auto orphan = cgpui_test::wait_for_test_frame(**renderer);
  if (!orphan) return dx12_fail(15, "retained active frame unavailable");
  (*orphan)->clear({1, 0, 0, 0.5F});
  ClientAlphaTopology retained;
  if (!retained.capture(**window)) return dx12_fail(20, "retained client alpha topology unavailable");
  renderer->reset();
  const auto capture = (*orphan)->request_pixel_capture();
  const auto presented = (*orphan)->present();
  if (!capture || !presented || (*orphan)->present() || (*finished)->present() ||
      ordinary_renderer(**window, true) || !retained.same(**window) || !client_alpha_hits(**window) ||
      !expect_screen_color({128, 127, 0}, {retained.origin.x + retained.client.right / 2,
                                          retained.origin.y + retained.client.bottom / 2}))
    return dx12_fail(16, "healthy orphan alpha pixels, one-shot lifetime, occupied lease or topology differ");
  orphan->reset(); finished->reset();
  auto replacement = ordinary_renderer(**window, true);
  if (!replacement || !client_alpha_paint(**replacement, **window, false) || !client_alpha_hits(**window))
    return dx12_fail(17, "last retained frame did not release client HWND lease");
  auto pending = cgpui_test::wait_for_test_frame(**replacement);
  if (!pending) return dx12_fail(18, "pre-window-destruction frame unavailable");
  window->reset();
  const auto dead_frame = (*pending)->present();
  const auto dead_begin = (*replacement)->begin_frame();
  if (IsWindow(initial.hwnd) || dead_frame || dead_frame.error().code != ErrorCode::invalid_argument ||
      dead_begin || dead_begin.error().code != ErrorCode::invalid_argument)
    return dx12_fail(19, "renderer presented or rebuilt a destroyed client HWND");
  replacement->reset(); pending->reset();
  return 0;
}
