#include "dx12_dynamic_alpha_test_support.hpp"
#include "dx12_ordinary_pixels_support.hpp"

namespace cgpui_test {
int dynamic_alpha_lifetime_case() {
  using namespace cgpui;
  Dx12RecoveryDeadline deadline;
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return 1;
  auto window = (*app)->create_window(desktop.descriptor(false), [](const PlatformEvent&) {});
  if (!window || !ordinary_install_menu(**app, **window) || !desktop.place(**window)) return 2;
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  auto renderer = ordinary_renderer(**window, false);
  if (!renderer || !ordinary_draw_bands(**renderer, **window, false) ||
      !ordinary_desktop_bands(**window, false, false)) return 3;
  const auto* previous = (*renderer)->last_frame_pixels();
  const auto* diagnostics = (*renderer)->last_frame_diagnostic_snapshot();
  auto held = wait_for_test_frame(**renderer);
  if (!held) return 4;
  auto refused = (*renderer)->set_transparent_surface(true);
  if (refused || refused.error().code != ErrorCode::invalid_argument ||
      !ordinary_composition((*renderer)->surface_composition(), false) ||
      (*renderer)->last_frame_pixels() != previous || (*renderer)->last_frame_diagnostic_snapshot() != diagnostics) return 5;
  held->reset();
  if (!(*renderer)->set_transparent_surface(true) || (*renderer)->last_frame_pixels() != previous) return 6;
  // Repeated changes before any Present coalesce without publishing a frame or
  // retaining an unbounded chain history; only the final intent will Commit.
  for (const bool alpha : {false, true, false, true})
    if (!(*renderer)->set_transparent_surface(alpha) ||
        !ordinary_composition((*renderer)->surface_composition(), alpha) ||
        (*renderer)->last_frame_pixels() != previous ||
        (*renderer)->last_frame_diagnostic_snapshot() != diagnostics) return 7;
  auto orphan = wait_for_test_frame(**renderer);
  if (!orphan) return 8;
  (*orphan)->clear({0, 0, 1, .5F});
  renderer->reset();
  if (!(*orphan)->request_pixel_capture() || !(*orphan)->present() || (*orphan)->present()) return 9;
  orphan->reset(); // Retires both the successful replacement and its window lease.
  auto replacement = ordinary_renderer(**window, false);
  if (!replacement || !(*replacement)->set_transparent_surface(true) ||
      !ordinary_draw_bands(**replacement, **window, true) || !ordinary_desktop_bands(**window, true, true) ||
      !(*replacement)->set_transparent_surface(false) || !ordinary_draw_bands(**replacement, **window, false) ||
      !ordinary_desktop_bands(**window, false, false)) return 10;
  const auto info = (*replacement)->backend_info();
  if (!dx12_remove_native_device(info)) return 11;
  const auto retry = (*replacement)->begin_frame();
  if (retry || retry.error().code != ErrorCode::frame_retry_required ||
      (*replacement)->backend_info().adapter_luid != info.adapter_luid ||
      (*replacement)->backend_info().device_generation != info.device_generation + 1 ||
      !ordinary_composition((*replacement)->surface_composition(), false) ||
      !ordinary_draw_bands(**replacement, **window, true) || !ordinary_desktop_bands(**window, true, false) ||
      !(*replacement)->set_transparent_surface(true) || !ordinary_draw_bands(**replacement, **window, false) ||
      !ordinary_desktop_bands(**window, false, true)) return 12;
  return 0;
}
} // namespace cgpui_test
